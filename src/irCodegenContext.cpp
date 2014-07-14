
#include "ast.hpp"
#include "token.hpp"
#include "irCodegenContext.hpp"

#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include "message.hpp"

#include <fstream>
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace llvm;

IRTranslationUnit::IRTranslationUnit(IRCodegenContext *c, TranslationUnit *u) :
    context(c), unit(u) {
    module = new Module("", context->context);
    debug = new IRDebug(context, this);
    scope = new IRScope(u->getScope(), debug->getCompileUnit());
}

SourceLocation currentLoc;

void IRCodegenContext::dwarfStopPoint(int ln)
{
    llvm::DebugLoc loc = llvm::DebugLoc::get(ln, 1, diScope());
    assert_message(!loc.isUnknown(), msg::FAILURE, "unknown debug location");
    ir->SetCurrentDebugLocation(loc);
}

void IRCodegenContext::dwarfStopPoint(SourceLocation l)
{
    currentLoc = l;
    llvm::DebugLoc loc = llvm::DebugLoc::get(l.line, l.ch, diScope());
    assert_message(!loc.isUnknown(), msg::FAILURE, "unknown debug location");
    ir->SetCurrentDebugLocation(loc);
}

llvm::Type *IRCodegenContext::codegenUserType(ASTType *ty)
{
    ASTUserType *userty = ty->userType();
    UserTypeDeclaration *utdecl = userty->getDeclaration();
    if(!userty) {
        emit_message(msg::FAILURE, "invalid user type");
    }

    if(userty->identifier->isUndeclared()){ //NOTE superfluous test
        emit_message(msg::WARNING, "type should be resolved by now");
        userty->identifier = lookup(ty->getName());
        if(userty->identifier->isUndeclared()){
            emit_message(msg::ERROR, "undeclared struct: " + userty->getName());
            return NULL;
        }
    }

    if(StructType *sty = module->getTypeByName(ty->getName())) {
        return sty;
    }

    if(unit->types.count(ty->getName())){
        Type *llty = unit->types[ty->getName()];
        return llty;
    }

    // if interface create an interface struct
    // will still need to populate the interface members though
    // XXX seems a bit messy
    if(ty->isInterface())
        return codegenType(ast->getRuntimeUnit()->lookup("Interface")->getDeclaredType());


    StructType *sty = StructType::create(context);
    sty->setName(ty->getName());
    unit->types[ty->getName()] = IRType(ty, sty);


    std::vector<Type*> structVec;

    // add base type to member set
    if(ASTType *base = userty->getBaseType()){
        Type *basety = codegenType(base);
        if(base->isReference()) basety = basety->getPointerElementType();
        structVec.push_back(basety);
    }

    //
    //TODO: use iterator
    //
    for(int i = 0; i < userty->length(); i++)
    {
        if(VariableDeclaration *vd = dynamic_cast<VariableDeclaration*>(userty->getMember(i)))
        {
            structVec.push_back(codegenType(vd->type));
        } else {
            emit_message(msg::UNIMPLEMENTED, "this cant be declared in a struct (yet?)");
        }
    }

    if(!userty->isOpaque())
    {
        sty->setBody(structVec);
    }

    unit->debug->createUserType(ty);

    if(ty->isClass()) {
        userty->getDeclaration()->classDeclaration()->typeinfo = createTypeInfo(ty);
    }

    return sty;
}

llvm::Type *IRCodegenContext::codegenStructType(ASTType *ty)
{
    return codegenUserType(ty);
}

//TODO: fix union types. codegen is incorrect (calls 'codegen usertype')
llvm::Type *IRCodegenContext::codegenUnionType(ASTType *ty)
{
    //return codegenUserType(ty);

    // XXX useless below
    if(!ty->isUnion()) {
        emit_message(msg::FAILURE, "unknown union type");
    }

    ASTUserType *userty = ty->userType();

    unsigned align = 0;
    unsigned size = 0;
    ASTType *alignedType = 0;
    std::vector<Type*> unionVec;
    for(int i = 0; i < userty->length(); i++)
    {
        if(VariableDeclaration *vd = dynamic_cast<VariableDeclaration*>(userty->getMember(i)))
        {
            if(vd->getType()->getAlign() > align)
            {
                alignedType = vd->type;
                align = vd->getType()->getAlign();
            }

            if(vd->getType()->getSize() > size)
            {
                size = vd->getType()->getSize();
            }
            //unionVec.push_back(codegenType(vd->type));
        } else
            emit_message(msg::UNIMPLEMENTED, "this cant be declared in a union (yet?)");
    }

    if(!userty->isOpaque())
    {
        unionVec.push_back(codegenType(alignedType));
        if(size - alignedType->getSize())
            unionVec.push_back(ArrayType::get(Type::getInt8Ty(context),
                        size - alignedType->getSize()));
        StructType *sty = StructType::create(context, userty->identifier->getName());
        sty->setBody(unionVec);
        ty->cgType = sty;
    }
    else ty->cgType = Type::getInt8Ty(context); //allow fwd declared types, TODO: cleaner

    unit->debug->createUnionType(ty);
    return(llvm::Type*) ty->cgType;
}

llvm::Type *IRCodegenContext::codegenClassType(ASTType *ty) //TODO: actual codegen
{
    return codegenUserType(ty);
}

llvm::Type *IRCodegenContext::codegenTupleType(ASTType *ty)
{
    ASTTupleType *tupty = dynamic_cast<ASTTupleType*>(ty);
    if(ty->kind != TYPE_TUPLE || !tupty)
    {
        emit_message(msg::FAILURE, "invalid tuple type codegen");
        return NULL;
    }

    if(ty->cgType) return ty->cgType;

    if(!tupty->types.size())
    {
        emit_message(msg::ERROR, "invalid 0-tuple");
        return NULL;
    }

    std::vector<Type*> tupleVec;
    for(int i = 0; i < tupty->types.size(); i++)
    {
        tupleVec.push_back(codegenType(tupty->types[i]));
    }

    StructType *tty = StructType::create(context);
    tty->setBody(tupleVec);
    ty->cgType = tty;
    unit->debug->createType(ty); // TODO
    return tty;
}


/*
 * dynamic array is equivilent to:
 * struct Array {
 *  void *arr;
 *  long size;
 * }
 */
llvm::Type *IRCodegenContext::codegenArrayType(ASTType *ty)
{
    ASTArrayType *arrty = dynamic_cast<ASTArrayType*>(ty);
    if(!arrty) {
        emit_message(msg::FAILURE, "attempt to codegen invalid array type");
    }

    if(ty->cgType) return ty->cgType;

    if(arrty->isDynamic())
    {
        //Identifier *id = ast->getRuntimeUnit()->getScope()->lookup("DynamicArray");
        //ASTType *arrayty = id->getDeclaredType();
        //ty->cgType = codegenType(arrayty);

        vector<Type*> members;
        members.push_back(codegenType(arrty->arrayOf->getPointerTy()));
        members.push_back(codegenType(ASTType::getLongTy()));
        StructType *aty = StructType::create(context, ty->getName());
        aty->setBody(members);
        ty->cgType = aty;

        unit->debug->createDynamicArrayType(ty);
    } else
    {
        llvm::ArrayType *aty = ArrayType::get(codegenType(arrty->arrayOf), arrty->length());
        ty->cgType = aty;

        unit->debug->createArrayType(ty);
    }

    return (llvm::Type*) ty->cgType;
}

llvm::Type *IRCodegenContext::codegenFunctionType(ASTType *ty) {
    ASTFunctionType *astfty = ty->functionType();
    if(!astfty) {
        emit_message(msg::FAILURE, "attempt to codegen invalid function type");
        return NULL;
    }

    //TODO: push back owner type

    vector<Type*> params;
    for(int i = 0; i < astfty->params.size(); i++)
    {
        Type *llty = codegenType(astfty->params[i]);
        params.push_back(llty);
    }

    FunctionType *llfty = FunctionType::get(codegenType(astfty->ret), params, astfty->vararg);
    return llfty;
}

llvm::Type *IRCodegenContext::codegenType(ASTType *ty)
{
    llvm::Type *llvmty = NULL;
    if(ty->cgType) return (Type*) ty->cgType;

    // XXX: type resolution is below; should be in some other pass, not codegen
    if(ty->kind == TYPE_UNKNOWN || ty->kind == TYPE_UNKNOWN_USER)
    {
        Identifier* id = lookup(ty->getName());
        Declaration* decl = id->getDeclaration();
        if(TypeDeclaration* tdecl = dynamic_cast<TypeDeclaration*>(decl))
        {
            ty = tdecl->getDeclaredType();
            if(!tdecl->getDeclaredType())
            {
                emit_message(msg::FATAL, "error, invalid type");
            }
        } else {
            emit_message(msg::FATAL, "error, invalid type");
            return NULL;
        }
        //TODO
        if(id->isUndeclared()) {
            emit_message(msg::ERROR, string("undeclared type'") +
                    id->getName() + string("' in scope"));
            return NULL;
        }
    }

    ASTType *tmp;
    switch(ty->kind)
    {
        case TYPE_BOOL:
            llvmty = Type::getInt1Ty(context);
            break;
        case TYPE_CHAR:
        case TYPE_UCHAR:
            llvmty = Type::getInt8Ty(context);
            break;
        case TYPE_SHORT:
        case TYPE_USHORT:
            llvmty = Type::getInt16Ty(context);
            break;
        case TYPE_INT:
        case TYPE_UINT:
            llvmty = Type::getInt32Ty(context);
            break;
        case TYPE_LONG:
        case TYPE_ULONG:
            llvmty = Type::getInt64Ty(context);
            break;
        case TYPE_FLOAT:
            llvmty = Type::getFloatTy(context);
            break;
        case TYPE_DOUBLE:
            llvmty = Type::getDoubleTy(context);
            break;
        case TYPE_VOID:
            llvmty = Type::getVoidTy(context);
            break;
        case TYPE_POINTER:
            tmp = ty->getReferencedTy();
            if(tmp->kind == TYPE_VOID) tmp = ASTType::getCharTy();
            llvmty = codegenType(tmp)->getPointerTo();
            break;
        case TYPE_USER:
            llvmty = codegenUserType(ty);
            if(ty->isReference())
                llvmty = llvmty->getPointerTo();
            break;
        case TYPE_TUPLE:
            llvmty = codegenTupleType(ty);
            break;
        case TYPE_ARRAY:
            llvmty = codegenArrayType(ty);
            break;
        case TYPE_DYNAMIC_ARRAY:
            llvmty = codegenArrayType(ty);
            break;
        case TYPE_FUNCTION:
            llvmty = codegenFunctionType(ty);
            break;
        default:
            emit_message(msg::FAILURE, "type not handled", currentLoc);
    }

    ty->cgType = llvmty;

    return (llvm::Type *) llvmty;
}

llvm::Value *IRCodegenContext::codegenValue(ASTValue *value)
{
    if(!value->cgValue) {
        //TODO
        emit_message(msg::FAILURE, "AST Value failed to generate");
    }

    if(value->isLValue()) {
        return ir->CreateAlignedLoad(codegenLValue(value), 4);
    }

    return (llvm::Value *) value->cgValue;
}

llvm::Value *IRCodegenContext::codegenLValue(ASTValue *value)
{
    if(!value->isLValue() && !value->isReference())
    {
        emit_message(msg::FATAL, "rvalue used in lvalue context!");
        return NULL;
    }
    if(!value->cgValue) {
        //TODO
        emit_message(msg::FAILURE, "AST Value failed to generate");
    }

    if(value->isLValue() && value->isReference()) {
        return ir->CreateAlignedLoad(value->cgValue, 4);
    }

    return (llvm::Value*) value->cgValue;
}

llvm::Value *IRCodegenContext::codegenRefValue(ASTValue *value) {
    if(!value->cgValue) {
        //TODO
        emit_message(msg::FAILURE, "AST Value failed to generate");
    }

    if(!value->isReference()) {
        emit_message(msg::FAILURE, "Attempting to get reference value of non reference");
    }

    return (llvm::Value*) value->cgValue;
}

ASTValue *IRCodegenContext::storeValue(ASTValue *dest, ASTValue *val)
{
    Value *llval = 0;
    if(dest->isReference()) {
        llval = ir->CreateStore(codegenValue(val), codegenRefValue(dest));
    } else {
        llval = ir->CreateStore(codegenValue(val), codegenLValue(dest));
    }

    return new ASTValue(dest->type, llval);
}

ASTValue *IRCodegenContext::getStringValue(std::string str) {
    Constant *strConstant = ConstantDataArray::getString(context, str);
    GlobalVariable *GV = new GlobalVariable(*module, strConstant->getType(), true,
            GlobalValue::PrivateLinkage, strConstant);
    vector<Value *> gep;
    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
    Constant *val = ConstantExpr::getInBoundsGetElementPtr(GV, gep);

    //TODO: make this an ARRAY (when arrays are added), so that it has an associated length

    return new ASTValue(ASTType::getCharTy()->getPointerTy(), val);
}

ASTValue *IRCodegenContext::getFloatValue(ASTType *t, float i){
    //TODO: constant cache -256-255
    return new ASTValue(t, ConstantFP::get(codegenType(t), i));
}

ASTValue *IRCodegenContext::getIntValue(ASTType *t, int i){
    //TODO: constant cache -256-255
    return new ASTValue(t, ConstantInt::get(codegenType(t), i));
}

ASTValue *IRCodegenContext::loadValue(ASTValue *lval)
{
    assert_message(lval->isLValue(), msg::FAILURE, "attempted to load RValue (must be LValue)");
    ASTValue *loaded = new ASTValue(lval->type, codegenValue(lval));
    return loaded;
}

ASTValue *IRCodegenContext::getThis() {
    Identifier *thisId = getScope()->lookupInScope("this");
    return codegenIdentifier(thisId);
}

ASTValue *IRCodegenContext::getVTable(ASTValue *instance) {
    return getMember(instance, "vtable");
}

ASTValue *IRCodegenContext::vtableLookup(ASTValue *instance, std::string func) {
    ASTUserType *userty = instance->getType()->userType();
    if(!userty) emit_message(msg::ERROR, "virtual function lookup only valid for class");

    //TODO: function index
    ASTValue *vtable = getVTable(instance);

    std::vector<Value *> gep;
    //gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), userty->getVTableIndex(func))); // specific function
    Value *llval = ir->CreateLoad(ir->CreateGEP(codegenValue(vtable), gep));

    Identifier *id = userty->getScope()->lookup(func);
    FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(id->getDeclaration());
    ASTType *fty = fdecl->getType();

    return new ASTValue(fty, ir->CreatePointerCast(llval, codegenType(fty)->getPointerTo()));
}

ASTValue *IRCodegenContext::createTypeInfo(ASTType *ty) {
    if(!ty->isClass()){
        emit_message(msg::ERROR, "can only get type info for class");
    }

    std::vector<Constant *> members; // type info constant members
    std::vector<Constant *> arrayList;
    ASTType *vfty = ASTType::getVoidFunctionTy()->getPointerTy();

    UserTypeDeclaration *utdecl = ty->getDeclaration()->userTypeDeclaration();
    if(ClassDeclaration *cdecl = utdecl->classDeclaration()) {
        for(int i = 0; i < cdecl->vtable.size(); i++){
            FunctionDeclaration *fdecl = cdecl->vtable[i];
            if(!fdecl) emit_message(msg::FAILURE, "invalid function identifier");
            FunctionType *fty = (FunctionType*) codegenType(fdecl->getType());
            Constant *func = module->getOrInsertFunction(fdecl->getMangledName(), fty);

            arrayList.push_back((Function*)
                    ir->CreatePointerCast(func, codegenType(vfty)));
        }
    }

    int sz = utdecl->classDeclaration() ? utdecl->classDeclaration()->vtable.size() : 0;
    ArrayType *vtableTy = ArrayType::get(codegenType(vfty), sz);
    Constant *vtable = ConstantArray::get(
                vtableTy,
                arrayList
                );

    members.push_back(vtable);

    GlobalVariable *gv = new GlobalVariable(*module, vtableTy, true, GlobalValue::PrivateLinkage, vtable);
    gv->setName("TypeInfo_" + ty->getName());
    return new ASTValue(vfty, gv, true);
}

// BINOP .
ASTValue *IRCodegenContext::getMember(ASTValue *val, std::string member) {
    if(val->getType()->isPointer())
        val = getValueOf(val);
    ASTUserType *userty = val->getType()->userType();

    if(!userty){
        emit_message(msg::FAILURE, "cannot get member in non-usertype");
    }

    Identifier *id = userty->getScope()->lookupInScope(member);

    // identifier is either in base or does not exist, recurse to base
    if(!id){
        // zeroeth member is base class
        std::vector<Value*> gep;
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        Value *llval = codegenLValue(val);
        llval = ir->CreateInBoundsGEP(llval, gep);
        ASTValue base(userty->getBaseType(), llval, true, false);
        return getMember(&base, member);
    }

    if(id->isFunction()){
        //TODO: static vs nonvirtual vs virtual function calls
        ASTValue *ret = 0;
        if(val->getType()->isClass()) {
            ret = vtableLookup(val, member);
        } else {
            ret = codegenIdentifier(id);
        }
        ret->setOwner(val);
        return ret;
    } else if(id->isVariable()){
        // member lookup
        int index = userty->getMemberIndex(member);
        if(userty->getBaseType()) index++; // skip over base class index
        ASTType *mtype = id->getType();
        std::vector<Value*> gep;
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context),
                                    index));
        Value *llval;
        llval = ir->CreateInBoundsGEP(codegenLValue(val), gep);
        llval = ir->CreatePointerCast(llval, codegenType(mtype->getPointerTy()));
        ASTValue *ret = new ASTValue(mtype, llval, true);
        ret->setOwner(val);
        return ret;
    } else {
        emit_message(msg::ERROR, "unknown member in user type");
        return NULL;
    }
}

// UNOP ^
ASTValue *IRCodegenContext::getValueOf(ASTValue *ptr, bool lvalue){
    assert_message(ptr->getType()->isPointer(), msg::FAILURE, "attempt to dereference non pointer type");
    return new ASTValue(ptr->getType()->getReferencedTy(), codegenValue(ptr), lvalue);
}

// UNOP &
ASTValue *IRCodegenContext::getAddressOf(ASTValue *lval){
    assert_message(lval->isLValue(), msg::FAILURE, "attempt to get address of non-LValue value");
    return new ASTValue(lval->getType()->getPointerTy(), codegenLValue(lval), false);
}

// BINOP +
ASTValue *IRCodegenContext::opAddValues(ASTValue *a, ASTValue *b){
    assert_message(a->getType() == b->getType(), msg::FAILURE, "values must be same type for addition");
    if(a->getType()->isFloating()){
        return new ASTValue(a->getType(), ir->CreateFAdd(codegenValue(a), codegenValue(b)));
    }
    return new ASTValue(a->getType(), ir->CreateAdd(codegenValue(a), codegenValue(b)));
}

// BINOP -
ASTValue *IRCodegenContext::opSubValues(ASTValue *a, ASTValue *b){
    assert_message(a->getType() == b->getType(), msg::FAILURE, "values must be same type for subtraction");
    if(a->getType()->isFloating()){
        return new ASTValue(a->getType(), ir->CreateFSub(codegenValue(a), codegenValue(b)));
    }
    return new ASTValue(a->getType(), ir->CreateSub(codegenValue(a), codegenValue(b)));
}

// BINOP *
ASTValue *IRCodegenContext::opMulValues(ASTValue *a, ASTValue *b){ // *
    assert_message(a->getType() == b->getType(), msg::FAILURE, "values must be same type for multiplication");
    if(a->getType()->isFloating()){
        return new ASTValue(a->getType(), ir->CreateFMul(codegenValue(a), codegenValue(b)));
    }
    //TODO: sign?
    return new ASTValue(a->getType(), ir->CreateMul(codegenValue(a), codegenValue(b)));
}

// BINOP /
ASTValue *IRCodegenContext::opDivValues(ASTValue *a, ASTValue *b){ // /
    assert_message(a->getType() == b->getType(), msg::FAILURE, "values must be same type for division");
    if(a->getType()->isFloating()){
        return new ASTValue(a->getType(), ir->CreateFDiv(codegenValue(a), codegenValue(b)));
    }

    if(a->getType()->isSigned()){
        return new ASTValue(a->getType(), ir->CreateSDiv(codegenValue(a), codegenValue(b)));
    }

    // unsigned int div
    return new ASTValue(a->getType(), ir->CreateUDiv(codegenValue(a), codegenValue(b)));
}

// BINOP %
ASTValue *IRCodegenContext::opModValue(ASTValue *a, ASTValue *b){ // %
    assert_message(a->getType() == b->getType(), msg::FAILURE, "values must be same type for modulus");
    if(a->getType()->isFloating()){
        return new ASTValue(a->getType(), ir->CreateFRem(codegenValue(a), codegenValue(b)));
    }

    if(a->getType()->isSigned()){
        return new ASTValue(a->getType(), ir->CreateSRem(codegenValue(a), codegenValue(b)));
    }

    // unsigned remainder
    return new ASTValue(a->getType(), ir->CreateURem(codegenValue(a), codegenValue(b)));
}

// BINOP <<
ASTValue *IRCodegenContext::opShlValue(ASTValue *a, ASTValue *b){ // <<
    return new ASTValue(a->getType(), ir->CreateShl(codegenValue(a), codegenValue(b)));
}

// BINOP >>
ASTValue *IRCodegenContext::opShrValue(ASTValue *a, ASTValue *b){ // >>
    return new ASTValue(a->getType(), ir->CreateLShr(codegenValue(a), codegenValue(b)));
}

// BINOP **
ASTValue *IRCodegenContext::opPowValue(ASTValue *a, ASTValue *b){ // **
    emit_message(msg::UNIMPLEMENTED, "unimplemented power operator");
}

// does not create IR Value for codegen (unless global).
// values should be created at declaration
ASTValue *IRCodegenContext::codegenIdentifier(Identifier *id)
{
    if(id->isVariable())
    {
        if(id->getScope()->getUnit() != unit->unit) // id not declared in current TU because imported
        {
            if(unit->globals.count(id->getName()))
            {
                return unit->globals[id->getName()];
            } else
            {
                GlobalVariable* GV =
                (GlobalVariable*) module->getOrInsertGlobal(id->getMangledName(),
                        codegenType(id->getType()));
                assert(GV);
                IRValue irval = IRValue(new ASTValue(id->getType(), GV, true), GV);
                unit->globals[id->getName()] = irval;
                return irval;
            }
            emit_message(msg::FAILURE, "failed to codegen identifier");
        }

        // is a member of 'this'
        if(id->isTypeMember()){
            //TODO: do not handle extensions in codegen
            if(getScope()->table->extensionEnabled("implicit_this")){
                // get 'this' from current function
                // OR static look up, if static member
                return getMember(getThis(), id->getName());
            } else if(!id->getValue()){
                emit_message(msg::ERROR, "identifier not found in scope. did you mean '." + id->getName() + "'?");
            }
        }

        return id->getValue(); // else declared in current TU, so we are good
    } else if(id->isFunction())
    {
        FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(id->getDeclaration());
        if(!fdecl) emit_message(msg::FAILURE, "invalid function identifier");
        FunctionType *fty = (FunctionType*) codegenType(fdecl->getType());
        Constant *func = module->getOrInsertFunction(fdecl->getMangledName(), fty);
        id->setValue(new ASTValue(fdecl->getType(), func));
    } else if(id->isStruct())
    {
        id->setValue(new ASTValue(id->getDeclaredType(), NULL));
    } else if(id->isExpression())
    {
        id->setValue(codegenExpression(id->getExpression()));
    } else if(id->isLabel())
    {
        id->setValue(new ASTValue(NULL, BasicBlock::Create(context,
                        id->getName(), ir->GetInsertBlock()->getParent())));
    }

    return id->getValue();
}

ASTValue *IRCodegenContext::codegenExpression(Expression *exp)
{
    if(NumericExpression *nexp = exp->numericExpression())
    {
        llvm::Value *llvmval;
        ASTType *ty;
        switch(nexp->type)
        {
            case NumericExpression::INT:
                if(nexp->astType->isPointer())
                {
                    if(!nexp->intValue)
                    {
                        llvmval = ConstantPointerNull::get((PointerType*) codegenType(nexp->astType));
                        ty = nexp->astType;
                    }
                } else
                {
                    llvmval = ConstantInt::get(codegenType(nexp->astType), nexp->intValue);
                    ty = nexp->astType;
                }
                return new ASTValue(ty, llvmval); //TODO: assign
            case NumericExpression::DOUBLE:
                llvmval = ConstantFP::get(codegenType(nexp->astType), nexp->floatValue);
                ty = nexp->astType;
                return new ASTValue(ty, llvmval); //TODO: assign
        }
    }
    else if(StringExpression *sexp = exp->stringExpression())
    {
        return getStringValue(sexp->string);
    }
    else if(PostfixExpression *pexp = exp->postfixExpression())
    {
        return codegenPostfixExpression(pexp);
    }
    else if(UnaryExpression *uexp = exp->unaryExpression())
    {
        return codegenUnaryExpression(uexp);
    }
    else if(BinaryExpression *bexp = exp->binaryExpression())
    {
        return codegenBinaryExpression(bexp);
    } else if(CallExpression *cexp = exp->callExpression())
    {
        return codegenCallExpression(cexp);
    } else if(IdentifierExpression *iexp = exp->identifierExpression())
    {
        return codegenIdentifier(iexp->id);
    } else if(ImportExpression *iexp = exp->importExpression())
    {
        //TODO: should it return something? probably. Some sort of const package ptr or something...
        return NULL;
    } else if(CastExpression *cexp = exp->castExpression())
    {
        return promoteType(codegenExpression(cexp->expression), cexp->type);
    } else if(TupleExpression *texp = dynamic_cast<TupleExpression*>(exp))
    {
        return codegenTupleExpression(texp);
    } else if(NewExpression *nexp = dynamic_cast<NewExpression*>(exp))
    {
        return codegenNewExpression(nexp);
    } else if(DeleteExpression *dexp = dynamic_cast<DeleteExpression*>(exp))
    {
        return codegenDeleteExpression(dexp);
    } else if(UseExpression *uexp = exp->useExpression())
    {
        return NULL;
    }

    emit_message(msg::FAILURE, "bad expression?", exp->loc);
    return NULL; //TODO
}

ASTValue *IRCodegenContext::codegenTupleExpression(TupleExpression *exp, ASTType *ty)
{
    //XXX codegen tuple
    std::vector<ASTValue*> vals;
    std::vector<ASTType*> types;
    bool lvalue = true;
    ASTCompositeType* compty = 0;
    if(ty)
        ty->compositeType();

    for(int i = 0; i < exp->members.size(); i++)
    {
        ASTValue *val = codegenExpression(exp->members[i]);

        if(compty)
            val = promoteType(val, compty->getMemberType(i));

        vals.push_back(val);
        types.push_back(val->getType());

        if(!val->isLValue()) lvalue = false;
    }

    ASTTupleType *tupty = new ASTTupleType(types);

    if(lvalue)
    {
        emit_message(msg::UNIMPLEMENTED, "lvalue tuple unimplemented", exp->loc);
        return NULL;
    }
    else
    {
        std::vector<Constant*> llvals;
        for(int i = 0; i < vals.size(); i++)
        {
            llvals.push_back((Constant*) codegenValue(vals[i]));
        }

        StructType *llty = (StructType*) codegenType(tupty);

        GlobalVariable *GV = new GlobalVariable(*module, llty, true,
                GlobalValue::PrivateLinkage,
                ConstantStruct::get(llty, llvals));

        //vector<Value *> gep;
        //gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        //gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        //Constant *val = ConstantExpr::getInBoundsGetElementPtr(GV, gep);
        return new ASTValue(tupty, GV, true);
    }
}

ASTValue *IRCodegenContext::codegenNewExpression(NewExpression *exp)
{
    if(exp->type->kind == TYPE_DYNAMIC_ARRAY)
    {
        emit_message(msg::ERROR, "cannot created unsized array. meaningless alloaction", exp->loc);
        return NULL;
    }

    ASTType *ty = exp->type;
    vector<Value*> llargs;
    llargs.push_back(ConstantInt::get(codegenType(ASTType::getULongTy()),
                exp->type->getSize()));
    vector<Type*> llargty;
    llargty.push_back(codegenType(ASTType::getULongTy()));
    FunctionType *fty = FunctionType::get(codegenType(ASTType::getVoidTy()->getPointerTy()),
                llargty, false);
    Function *mallocFunc = (Function*) module->getOrInsertFunction("malloc", fty);
    Value *value;
    value = ir->CreateCall(mallocFunc, llargs);
    ASTValue *val = NULL;
    if(exp->type->isArray())
    {
        ASTType *arrty = exp->type->getReferencedTy();
        ty = arrty->getArrayTy();
        Value *ptr = ir->CreateBitCast(value,
                codegenType(arrty)->getPointerTo());
        Value *sz = ConstantInt::get(codegenType(ASTType::getULongTy()), exp->type->length());
        value = ir->CreateAlloca(codegenType(ty));
        ir->CreateStore(ptr, ir->CreateStructGEP(value, 0));
        ir->CreateStore(sz, ir->CreateStructGEP(value, 1));
        value = ir->CreateLoad(value);
        //TODO: create a 'create array' function
    } else {
        if(exp->type->isReference()){
            value = ir->CreateBitCast(value, codegenType(exp->type));
        } else {
            value = ir->CreateBitCast(value, codegenType(exp->type)->getPointerTo());
        }
    }

    if(ty->isReference()){
        val = new ASTValue(ty, value, false, true);
    } else {
        val = new ASTValue(ty->getPointerTy(), value);
    }

    if(ty->isClass()) {
         // no default value, and allocated class. set VTable, in case
        ASTUserType *uty = ty->userType();
        if(FunctionDeclaration *fdecl = uty->getDefaultConstructor()){
            std::vector<ASTValue*> args;
            args.push_back(val);
            codegenCall(codegenIdentifier(fdecl->identifier), args);
        }
        //TODO call default constructor
        //XXX temp below. set vtable of new class
        // TODO: also do if type is pointer to class (called through 'new')
        ASTValue *vtable = getVTable(val);

        codegenType(uty); // XXX to create typeinfo. So we are able to load it below... hacky

        //the pass through identifier is an ugly hack to get typeinfo in case of duplicate ASTType types, eww
        Value *tival = codegenLValue(uty->getDeclaration()->classDeclaration()->typeinfo);

        vector<Value *> gep;
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        tival = ir->CreateGEP(tival, gep); // GEP to class's vtable, so we can store to it

        ir->CreateStore(tival, codegenLValue(vtable));
    }

    return val;
}

ASTValue *IRCodegenContext::codegenDeleteExpression(DeleteExpression *exp)
{
    vector<Value*> llargs;
    vector<Type*> llargty;
    ASTValue *val = codegenExpression(exp->expression);

    if(val->type->isArray() && val->type->kind == TYPE_DYNAMIC_ARRAY)
    {
        //TODO: duplicate of ".ptr". make a function for this
        std::vector<Value*> gep;
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        Value *llval = ir->CreateInBoundsGEP(codegenLValue(val), gep);
        val = new ASTValue(val->type->getReferencedTy()->getPointerTy(), llval, true);
    }

    val = promoteType(val, ASTType::getCharTy()->getPointerTy());
    llargs.push_back(codegenValue(val));
    llargty.push_back(codegenType(ASTType::getVoidTy()->getPointerTy()));

    FunctionType *fty = FunctionType::get(codegenType(ASTType::getVoidTy()), llargty, false);
    Function *freeFunc = (Function*) module->getOrInsertFunction("free", fty);

    Value *value = ir->CreateCall(freeFunc, llargs);
    //TODO: call deallocator function
    //return new ASTValue(exp->type, value);
    return NULL;
}

void IRCodegenContext::codegenElseStatement(ElseStatement *stmt)
{
    if(!stmt->body)
    {
        emit_message(msg::ERROR, "else keyword expects body", stmt->loc);
        return;
    }

    codegenStatement(stmt->body);
}

void IRCodegenContext::codegenIfStatement(IfStatement *stmt)
{
    ASTValue *cond = codegenExpression(stmt->condition);

    ASTValue *icond = promoteType(cond, ASTType::getBoolTy());
    llvm::BasicBlock *ontrue = BasicBlock::Create(context, "true",
            ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *onfalse = BasicBlock::Create(context, "false",
            ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *endif = BasicBlock::Create(context, "endif",
            ir->GetInsertBlock()->getParent());
    ir->CreateCondBr(codegenValue(icond), ontrue, onfalse);

    ir->SetInsertPoint(ontrue);
    codegenStatement(stmt->body);
    if(!isTerminated())
        ir->CreateBr(endif);
    setTerminated(false);

    ir->SetInsertPoint(onfalse);
    if(stmt->elsebr) codegenStatement(stmt->elsebr);
    if(!isTerminated())
        ir->CreateBr(endif);
    setTerminated(false);

    ir->SetInsertPoint(endif);
    setTerminated(false);

    return;
}

void IRCodegenContext::codegenLoopStatement(LoopStatement *stmt)
{
    llvm::BasicBlock *loopBB = BasicBlock::Create(context, "loop_condition",
        ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *ontrue = BasicBlock::Create(context, "loop_true",
        ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *onfalse = BasicBlock::Create(context, "loop_false",
        ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *loopupdate = BasicBlock::Create(context, "loop_update",
        ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *loopend = BasicBlock::Create(context, "loop_end",
        ir->GetInsertBlock()->getParent());

    if(ForStatement *fstmt = stmt->forStatement())
    {
        if(fstmt->decl) codegenStatement(fstmt->decl);
    }

    ir->CreateBr(loopBB);
    ir->SetInsertPoint(loopBB);

    if(stmt->condition)
    {
        ASTValue *cond = codegenExpression(stmt->condition);
        ASTValue *icond = promoteType(cond, ASTType::getBoolTy());
        ir->CreateCondBr(codegenValue(icond), ontrue, onfalse);
    } else ir->CreateBr(ontrue);

    getScope()->breakLabel = loopend;
    getScope()->continueLabel = loopupdate;

    if(!loopupdate) getScope()->continueLabel = loopBB;

    ir->SetInsertPoint(ontrue);
    if(stmt->body) codegenStatement(stmt->body);
    ir->CreateBr(loopupdate);
    ir->SetInsertPoint(loopupdate);
    if(stmt->update) codegenStatement(stmt->update);
    ir->CreateBr(loopBB);

    ir->SetInsertPoint(onfalse);
    //TODO: scoping might be weird. ifScope is on stack
    if(stmt->elsebr) codegenStatement(stmt->elsebr);
    ir->CreateBr(loopend);

    ir->SetInsertPoint(loopend);
    return;
}

void IRCodegenContext::codegenSwitchStatement(SwitchStatement *stmt)
{
    BasicBlock *switch_default = BasicBlock::Create(context, "switch_default",
                                         ir->GetInsertBlock()->getParent());

    BasicBlock *switch_end = BasicBlock::Create(context, "switch_end",
                                         ir->GetInsertBlock()->getParent());

    BasicBlock *preSwitch = ir->GetInsertBlock();

    ASTValue *cond = codegenExpression(stmt->condition);
    SwitchInst *sinst = ir->CreateSwitch(codegenValue(cond), switch_default);
    //TODO: set cases

    getScope()->switchStmt = stmt;
    getScope()->breakLabel = switch_end;

    //setTerminated(true);
    ir->SetInsertPoint(switch_default);
    if(stmt->body) codegenStatement(stmt->body);

    for(int i = 0; i < getScope()->cases.size(); i++)
    {
        IRSwitchCase *cs = getScope()->cases[i];
        if(!cs->irCase->getType()->isInteger())
        {
            emit_message(msg::ERROR, "case value can currently only be constant integer values",
                    cs->astCase->loc);
        }
        ASTValue *caseValue = promoteType(cs->irCase, cond->type);
        sinst->addCase((llvm::ConstantInt*) codegenValue(caseValue), cs->irBlock);
    }

    if(!isTerminated())
        ir->CreateBr(switch_end);
    ir->SetInsertPoint(switch_end);
    setTerminated(false);

    return;
}

ASTValue *IRCodegenContext::codegenCall(ASTValue *func, std::vector<ASTValue *> args) {
    vector<Value*> llargs;

    ASTFunctionType *astfty = dynamic_cast<ASTFunctionType*>(func->getType());
    if(!astfty) {
        emit_message(msg::ERROR, "invalid value used in function call context");
        if(func->getType()){
            emit_message(msg::ERROR, "value is of type \"" +
                    func->getType()->getName() + "\"");
        }
        return NULL;
    }
    ASTType *rtype = astfty->getReturnType();

    for(int i = 0; i < args.size(); i++){
        llargs.push_back(codegenValue(args[i]));
    }

    llvm::Value *value = ir->CreateCall(codegenValue(func), llargs);
    return new ASTValue(rtype, value);
}

ASTValue *IRCodegenContext::codegenCallExpression(CallExpression *exp)
{
    ASTValue *func = codegenExpression(exp->function);
    if(!func) {
        emit_message(msg::ERROR, "unknown expression used as function", exp->loc);
        return NULL;
    }

    ASTFunctionType *astfty = NULL;
    FunctionDeclaration *fdecl = NULL;
    ASTType *rtype = NULL;

    if(func->getType()->isPointer()){ // dereference function pointer
        func = getValueOf(func, false);
    }
    astfty = dynamic_cast<ASTFunctionType*>(func->getType());
    if(!astfty) {
        emit_message(msg::ERROR, "invalid value used in function call context", exp->loc);
        if(func->getType()){
            emit_message(msg::ERROR, "value is of type \"" +
                    func->getType()->getName() + "\"", exp->loc);
        }
        return NULL;
    }
    rtype = astfty->getReturnType();

    // allow default values if declaration is known
    if(IdentifierExpression *iexp = dynamic_cast<IdentifierExpression*>(exp->function)){
        fdecl = dynamic_cast<FunctionDeclaration*>(iexp->id->getDeclaration());
    }


    std::vector<ASTValue *> args;
    // allow for uniform function call syntax
    int nargs = 0;
    if(func->getOwner()){
        ASTValue *ownerVal = NULL;
        //TODO: figure out correct type for owner
        if(func->getOwner()->getType()->isPointer()){
            ownerVal = getAddressOf(func->getOwner());
        } else {
            ownerVal = getAddressOf(func->getOwner());
            //ownerVal = functionValue->getOwner();
        }
        //TODO: cast to correct type if needed
        args.push_back(ownerVal);
        nargs++;
    }

    for(int i = 0; i < exp->args.size() || i < astfty->params.size(); i++)
    {
        ASTValue *val = NULL;

        if(i < exp->args.size())
            val = codegenExpression(exp->args[i]);
        else if(fdecl && fdecl->parameters[i]->value)
        {
            val = codegenExpression(fdecl->parameters[i]->value);
        } else
        {
            break;
        }

        //TODO: nargs is messy. used to move param forward if implicit 'this'
        if(astfty->params.size() > i && astfty->params[i+nargs]){
            val = promoteType(val, astfty->params[i+nargs]);
        }

        else if(astfty->vararg)
        {
            if(val->getType()->isFloating())
                val = promoteType(val, ASTType::getDoubleTy());
            else if(val->getType()->isInteger() && val->getType()->isSigned() &&
                    val->getType()->getSize() < ASTType::getIntTy()->getSize())
                val = promoteType(val, ASTType::getIntTy());
            else if(val->getType()->isInteger() &&
                    val->getType()->getSize() < ASTType::getIntTy()->getSize())
                val = promoteType(val, ASTType::getUIntTy());
        }

        if(!val)
        {
            emit_message(msg::ERROR, "invalid arguement provided for function", exp->loc);
        }

        //cargs.push_back(val);
        args.push_back(val);
    }

    if(args.size() != exp->args.size()){
        /*
        emit_message(msg::ERROR,
                "invalid number of arguments provided for function call", exp->loc);
                */
        //TODO: add 'this' member as expected argument to methods
        //return NULL;
    }

    return codegenCall(func, args);
}

ASTValue *IRCodegenContext::codegenUnaryExpression(UnaryExpression *exp)
{
    //TODO: messy
    // use operator lowering
    if(exp->op == tok::dot){
        return getMember(getThis(), exp->lhs->identifierExpression()->id->getName());
    }

    ASTValue *lhs = codegenExpression(exp->lhs); // expression after unary op: eg in !a, lhs=a

    ASTValue *val;
    Value *llval = 0;
    switch(exp->op)
    {
        case tok::plusplus:
            if(!lhs->isLValue()) {
                emit_message(msg::ERROR, "can only incrememt LValue", exp->loc);
                return NULL;
            }

            if(lhs->getType()->isFloating())
            {
                val = opAddValues(lhs, getFloatValue(lhs->getType(), 1.0f));
            } else
            {
                val = opAddValues(lhs, getIntValue(lhs->getType(), 1));
            }

            storeValue(lhs, val);
            return val;
        case tok::minusminus:
            if(!lhs->isLValue()) {
                emit_message(msg::ERROR, "can only decrement LValue", exp->loc);
                return NULL;
            }

            if(lhs->getType()->isFloating())
            {
                val = opSubValues(lhs, getFloatValue(lhs->getType(), 1.0f));
            } else
            {
                val = opSubValues(lhs, getIntValue(lhs->getType(), 1));
            }

            storeValue(lhs, val);
            return val;
        case tok::plus:
            return lhs;
        case tok::minus:
            if(!lhs->getType()->isSigned()){
                emit_message(msg::UNIMPLEMENTED,
                        "conversion to signed value using '-' unary op",
                        exp->loc);
                return NULL;
            }
            val = new ASTValue(lhs->getType(), ir->CreateNeg(codegenValue(lhs)));
            return val;
        case tok::tilde:
            emit_message(msg::UNIMPLEMENTED, "unimplemented unary codegen (~)", exp->loc);
            return NULL;
        case tok::bang:
            val = promoteType(lhs, ASTType::getBoolTy());
            return new ASTValue(ASTType::getBoolTy(), ir->CreateNot(codegenValue(val)));
            return val;
        case tok::caret:
            if(!lhs->getType()->isPointer()) {
                emit_message(msg::ERROR, "attempt to dereference non-pointer type", exp->loc);
                return NULL;
            }
            return getValueOf(lhs);
        case tok::amp:
            if(!lhs->isLValue()){
                emit_message(msg::ERROR, "attempt to take reference of non-LValue", exp->loc);
                return NULL;
            }
            return getAddressOf(lhs);
        default:
            emit_message(msg::UNIMPLEMENTED, "unimplemented unary codegen", exp->loc);
    }
    return NULL;
}

ASTValue *IRCodegenContext::codegenPostfixExpression(PostfixExpression *exp)
{
    if(CallExpression *cexp = exp->callExpression())
    {
        return codegenCallExpression(cexp);
    } else if(IndexExpression *iexp = exp->indexExpression())
    {
        ASTValue *arr = codegenExpression(iexp->lhs);
        ASTValue *ind = codegenExpression(iexp->index);
        if(arr->getType()->kind == TYPE_DYNAMIC_ARRAY)
        {
            ASTType *indexedType = arr->getType()->getReferencedTy();
            Value *val = ir->CreateStructGEP(codegenLValue(arr), 0);
            val = ir->CreateLoad(val);
            val = ir->CreateInBoundsGEP(val, codegenValue(ind));
            val = ir->CreateBitCast(val, codegenType(indexedType->getPointerTy()));
            return new ASTValue(indexedType, val, true);
        }  else if(arr->getType()->kind == TYPE_ARRAY)
        {
            ASTType *indexedType = arr->getType()->getReferencedTy();
            Value *val = ir->CreateInBoundsGEP(codegenLValue(arr), codegenValue(ind));
            return new ASTValue(indexedType, val, true);
        } else if(arr->getType()->isPointer())
        {
            //TODO: index array
            ASTType *indexedType = arr->getType()->getReferencedTy();
            Value *val = ir->CreateInBoundsGEP(codegenValue(arr), codegenValue(ind));
            return new ASTValue(indexedType, val, true);
        } else if(arr->getType()->kind == TYPE_TUPLE)
        {
            if(ConstantInt *llci = dynamic_cast<ConstantInt*>(codegenValue(ind)))
            {
                ASTTupleType *tupty = (ASTTupleType*) dynamic_cast<ASTTupleType*>(arr->getType());
                unsigned long long index = llci->getZExtValue();
                if(tupty->types.size() <= index)
                {
                    emit_message(msg::ERROR, "invalid tuple index", exp->loc);
                    return NULL;
                }
                ASTType *type = tupty->types[index];
                Value *val = ir->CreateStructGEP(codegenLValue(arr), index);
                return new ASTValue(type, val, true);
            } else
            {
                emit_message(msg::ERROR, "tuples can only be indexed with\
                        constant integers, due to static typing", exp->loc);
                return NULL;
            }
        } else {
            emit_message(msg::ERROR, "attempt to index non-pointer/array type", exp->loc);
            return NULL;
        }
    } else if(PostfixOpExpression *e = dynamic_cast<PostfixOpExpression*>(exp))
    {
        ASTValue *old;
        ASTValue *val;
        ASTValue *lhs = codegenExpression(e->lhs);
        Value *llval = 0;

        switch(e->op)
        {
            case tok::plusplus:
            old = loadValue(lhs);
            if(lhs->getType()->isFloating())
            {
                val = opAddValues(lhs, getFloatValue(lhs->getType(), 1.0f));
            } else
            {
                val = opAddValues(lhs, getIntValue(lhs->getType(), 1));
            }

            old = loadValue(lhs);
            storeValue(lhs, val);
            return old;
            case tok::minusminus:
            if(lhs->getType()->isFloating())
            {
                val = opSubValues(lhs, getFloatValue(lhs->getType(), 1.0f));
            } else
            {
                val = opSubValues(lhs, getIntValue(lhs->getType(), 1));
            }
            old = loadValue(lhs);
            storeValue(lhs, val);
            return old;
        }
    } else if(DotExpression *dexp = dynamic_cast<DotExpression*>(exp))
    {
            ASTValue *lhs;

            // if left hand side represents a type
            if(ASTType *declty = dexp->lhs->getDeclaredType())
            {
                if(dexp->rhs == "sizeof")
                {
                    return new ASTValue(ASTType::getULongTy(),
                            ConstantInt::get(Type::getInt64Ty(context),
                                declty->getSize()));
                } else if(dexp->rhs == "offsetof")
                {
                    emit_message(msg::UNIMPLEMENTED,
                            "offsetof attribute not yet implemented", dexp->loc);
                }
                //TODO: static members
                emit_message(msg::ERROR, "unknown attribute '" + dexp->rhs + "' of struct '" +
                        declty->getName() + "'", dexp->loc);
                return NULL;
            }

            lhs = codegenExpression(dexp->lhs);

            if(lhs->getType()->isPointer())
                lhs = getValueOf(lhs);

            //TODO: allow indexing types other than userType and array?
            if(!lhs->getType()->userType() && !lhs->getType()->isArray()) {
                emit_message(msg::ERROR, "can only index struct or array type", dexp->loc);
                return NULL;
            }

            if(ASTUserType *userty = lhs->getType()->userType()){
                Identifier *id = userty->getDeclaration()->lookup(dexp->rhs);
                ASTValue *ret = NULL;

                if(!id || id->isUndeclared()) {
                    // if not in user type, search current scope
                    // this allows for uniform function syntax
                    id = getScope()->lookup(dexp->rhs);
                    if(!id || id->isUndeclared()){
                        emit_message(msg::ERROR, "no member found in user type", dexp->loc);
                        return NULL;
                    }
                }

                if(id->isVariable()) {
                    return getMember(lhs, dexp->rhs);
                } else if(id->isFunction()) {
                    //TODO: currently codegenIdentifier caches, might break with 'owner'
                    //TODO: codegenidentifier might not work here. it does not declare
                    ASTValue *ret;
                    if(id->isTypeMember())
                        ret = getMember(lhs, dexp->rhs);
                    else ret = codegenIdentifier(id);
                    ret->setOwner(lhs);
                    return ret;
                } else {
                    emit_message(msg::ERROR, "invalid identifier type in user type", dexp->loc);
                }
                return ret;
            }

            else if(lhs->getType()->kind == TYPE_DYNAMIC_ARRAY)
            {
                ASTArrayType *arrty = dynamic_cast<ASTArrayType*>(lhs->getType());
                if(!arrty){
                    emit_message(msg::FATAL, "dot exp on array, not actually an array?", dexp->loc);
                    return NULL;
                }
                ASTType *ty = arrty->getReferencedTy();

                if(dexp->rhs == "ptr")
                {
                    std::vector<Value*> gep;
                    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
                    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
                    Value *llval = ir->CreateInBoundsGEP(codegenLValue(lhs), gep);
                    return new ASTValue(ty->getPointerTy(), llval, true);
                }

                if(dexp->rhs == "size")
                {
                    std::vector<Value*> gep;
                    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
                    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 1));
                    Value *llval = ir->CreateInBoundsGEP(codegenLValue(lhs), gep);
                    return new ASTValue(ASTType::getULongTy(), llval, true);
                }
            } else if(lhs->getType()->kind == TYPE_ARRAY)
            {
                ASTArrayType *arrty = dynamic_cast<ASTArrayType*>(lhs->getType());
                ASTType *ty = arrty->getReferencedTy();

                if(dexp->rhs == "ptr")
                {
                    return new ASTValue(ty->getPointerTy(),
                            ir->CreateInBoundsGEP(codegenLValue(lhs), 0), true);
                }

                if(dexp->rhs == "size")
                {
                    return new ASTValue(ASTType::getULongTy(),
                            ConstantInt::get(Type::getInt64Ty(context), arrty->length()));
                }
            }
        emit_message(msg::ERROR, "unknown dot expression", exp->loc);
        return NULL;
    }

    emit_message(msg::UNIMPLEMENTED, "postfix codegen unimpl", exp->loc);
    return NULL;
}

ASTValue *IRCodegenContext::promoteInt(ASTValue *val, ASTType *toType)
{
    if(toType->isBool())
    {
        ASTValue zero(val->getType(), ConstantInt::get(codegenType(val->getType()), 0));
        return new ASTValue(ASTType::getBoolTy(), ir->CreateICmpNE(codegenValue(val),
                    codegenValue(&zero)));
    }

    if(toType->isInteger())
    {
        return new ASTValue(toType, ir->CreateIntCast(codegenValue(val),
                    codegenType(toType), false)); //TODO: signedness
    }
    if(toType->isPointer())
    {
        return new ASTValue(toType, ir->CreateIntToPtr(codegenValue(val), codegenType(toType)));
    }

    if(toType->isFloating())
    {
        if(val->type->isSigned())
        return new ASTValue(toType, ir->CreateSIToFP(codegenValue(val),
                    codegenType(toType)), false);
        return new ASTValue(toType, ir->CreateUIToFP(codegenValue(val),
                    codegenType(toType)), false);
    }
}

ASTValue *IRCodegenContext::promoteFloat(ASTValue *val, ASTType *toType)
{
    if(toType->isFloating())
    {
        return new ASTValue(toType, ir->CreateFPCast(codegenValue(val), codegenType(toType)));
    } else if(toType->isInteger())
    {
        return new ASTValue(toType, ir->CreateFPToUI(codegenValue(val),
                    codegenType(toType)));
    }
}

ASTValue *IRCodegenContext::promotePointer(ASTValue *val, ASTType *toType)
{
    if(toType->isPointer())
    {
        return new ASTValue(toType,
                ir->CreatePointerCast(codegenValue(val), codegenType(toType)));
    }

    if(toType->isBool())
    {
        Value *i = ir->CreatePtrToInt(codegenValue(val),
                codegenType(ASTType::getULongTy()));

        return new ASTValue(ASTType::getBoolTy(),
                ir->CreateICmpNE(i,
                    ConstantInt::get(codegenType(ASTType::getULongTy()), 0)
                    ));
    }

    if(toType->isInteger())
    {
        return new ASTValue(toType, ir->CreateIntCast(codegenValue(val),
                    codegenType(toType), false));
    }

    if(toType->isReference()) {
        return new ASTValue(toType,
                ir->CreatePointerCast(codegenValue(val), codegenType(toType)),
                false); //XXX might be a bit cludgy
    }
}

ASTValue *IRCodegenContext::promoteTuple(ASTValue *val, ASTType *toType)
{
    if(toType->isStruct())
    {
        if(((ASTTupleType*) val->type)->types.size() ==
                toType->length()) //TODO proper test
        {
            Value *toPtr = ir->CreateBitCast(codegenLValue(val),
                    codegenType(toType)->getPointerTo());
            return new ASTValue(toType, toPtr, true);
        } else
        {
            emit_message(msg::ERROR, "cannot convert tuple to struct");
            return NULL;
        }
    } else if(toType->kind == TYPE_TUPLE)
    {
        if(((ASTTupleType*) val->type)->types.size() ==
                ((ASTTupleType*) toType)->types.size())
        {
            Value *toPtr;
            if(val->isLValue())
            {
                toPtr = ir->CreateBitCast(codegenLValue(val),
                      codegenType(toType)->getPointerTo());
            } else
            {
                toPtr = ir->CreateAlloca(codegenType(val->type));
                ir->CreateStore(codegenValue(val), toPtr);
                toPtr = ir->CreateBitCast(toPtr,
                      codegenType(toType)->getPointerTo());
            }

            return new ASTValue(toType, toPtr, true);
        } else
        {
            emit_message(msg::ERROR, "cannot convert tuple to incompatible tuple");
            return NULL;
        }
    } else if(toType->kind == TYPE_ARRAY)
    {
        Value *toPtr;
        if(val->isLValue())
        {
            toPtr = ir->CreateBitCast(codegenLValue(val),
                    codegenType(toType)->getPointerTo());
        } else
        {
            emit_message(msg::FAILURE, "unimplemented RValue bitcast");
        }

        return new ASTValue(toType, toPtr, true);
    } else if(toType->kind == TYPE_DYNAMIC_ARRAY)
    {
        Value *toPtr;
        Value *toSize;
        AllocaInst *arr;
        if(val->isLValue())
        {
            toPtr = ir->CreateBitCast(codegenLValue(val),
                    codegenType(toType->getReferencedTy())->getPointerTo());
            ASTTupleType *tupty = (ASTTupleType*) val->type;
            toSize = ConstantInt::get(codegenType(ASTType::getULongTy()), tupty->length());

            arr = ir->CreateAlloca(codegenType(toType));
            ir->CreateStore(toPtr, ir->CreateStructGEP(arr, 0));
            ir->CreateStore(toSize, ir->CreateStructGEP(arr, 1));
        } else
        {
            emit_message(msg::FAILURE, "unimplemented RValue bitcast");
        }

        return new ASTValue(toType, arr, true);
    }
}

ASTValue *IRCodegenContext::promoteArray(ASTValue *val, ASTType *toType)
{
    if(val->type->kind == TYPE_ARRAY)
    {
        if(toType->kind == TYPE_POINTER)
        {
            ASTArrayType *arrty = (ASTArrayType*) val->type;
            if(arrty->arrayOf != toType->getReferencedTy())
            {
                emit_message(msg::ERROR, "invalid convesion from array to invalid pointer type");
                return NULL;
            }
            Value *ptr = codegenLValue(val);
            ptr = ir->CreateBitCast(ptr, codegenType(toType));
            return new ASTValue(toType, ptr, false); //TODO: should be lvalue, but that causes it to load incorrectly
        }
    } else if(val->type->kind == TYPE_DYNAMIC_ARRAY)
    {

    }

    return val; //TODO
}


ASTValue *IRCodegenContext::promoteType(ASTValue *val, ASTType *toType)
{
    if(val->type != toType)
    {
        if(val->type->isInteger())
        {
            return promoteInt(val, toType);
        } else if(val->type->isFloating())
        {
            return promoteFloat(val, toType);
        }
        else if(val->type->isPointer())
        {
            return promotePointer(val, toType);
        }
        else if(val->type->kind == TYPE_TUPLE)
        {
            return promoteTuple(val, toType);
        } if(val->type->isArray())
        {
            return promoteArray(val, toType);
        } else if(val->type->isReference()) {
            if(toType->isPointer()) {
                return new ASTValue(toType,
                        ir->CreatePointerCast(codegenLValue(val),
                            codegenType(toType)));
            }
        }
    }
    return val; // no conversion? failed converson?
}

//XXX is op required?
//XXX The way i do this with pointers is kinda silly, also, leaky?
void IRCodegenContext::codegenResolveBinaryTypes(ASTValue **v1, ASTValue **v2, unsigned op)
{
    if((*v1)->type != (*v2)->type)
    {
        if((*v1)->type->isStruct() || (*v2)->type->isStruct())
        {
            // TODO: loc
            emit_message(msg::UNIMPLEMENTED, "cannot convert structs (yet)");
        }
        if((*v2)->type->getPriority() > (*v1)->type->getPriority())
            *v1 = promoteType(*v1, (*v2)->type);
        else
            *v2 = promoteType(*v2, (*v1)->type);
    }
}

//TODO: should take astvalue's?
ASTValue *IRCodegenContext::codegenAssign(Expression *lhs, Expression *rhs, bool convert)
{
    /*
    if(!lhs->isLValue())
    {
        // XXX work around. if lhs is unknown, isLValue will fail on expression
        ASTValue *vlhs = codegenExpression(lhs);
        if(!vlhs->isLValue())
        {
            emit_message(msg::ERROR, "assignment requires lvalue", lhs->loc);
            return NULL;
        }
    }*/

    // codegen tuple assignment
    if(TupleExpression *tlhs = lhs->tupleExpression())
    {
        if(TupleExpression *trhs = rhs->tupleExpression())
        {
            if(tlhs->members.size() > trhs->members.size())
            {
                emit_message(msg::ERROR, "tuple assignment requires compatible tuple types", lhs->loc);
                return NULL;
            }
            for(int i = 0; i < tlhs->members.size(); i++)
            {
                codegenAssign(tlhs->members[i], trhs->members[i]);
            }
        } else {
            emit_message(msg::ERROR, "tuple assignment requires a tuple on rhs", lhs->loc);
            return NULL;
        }
        return codegenExpression(rhs);
    }

    ASTValue *vlhs = codegenExpression(lhs);
    ASTValue *vrhs = codegenExpression(rhs);

    //if(vrhs->type != vlhs->type)
    {
        vrhs = promoteType(vrhs, vlhs->getType());
        if(vrhs->type->coercesTo(vlhs->type) || (convert && vrhs->type->castsTo(vlhs->type)))
        {
            vrhs = promoteType(vrhs, vlhs->getType());
        }
        else
        {
            emit_message(msg::ERROR, "cannot assign value of type '" + vrhs->type->getName() +
                    "' to type '" + vlhs->type->getName() + "'", lhs->loc);
            return NULL;
        }
    }

    storeValue(vlhs, vrhs);
    return vrhs;
}

ASTValue *IRCodegenContext::codegenBinaryExpression(BinaryExpression *exp)
{
    //TODO: bit messy
    if(exp->op == tok::dot)
    {
        emit_message(msg::FAILURE, "this should not be a binop", exp->loc);
    } else if(exp->op == tok::colon) //cast op
    {
        ASTValue *rhs = codegenExpression(exp->rhs);
        if(IdentifierExpression *iexp = dynamic_cast<IdentifierExpression*>(exp->lhs))
        {
            ASTType *ty = iexp->identifier()->getDeclaredType();
            return promoteType(rhs, ty);
        } else emit_message(msg::ERROR, "need to cast to type", exp->loc);
    }

    //XXX temp. shortcut to allow LValue tuples
    if(exp->op == tok::equal || exp->op == tok::colonequal)
            return codegenAssign(exp->lhs, exp->rhs, exp->op == tok::colonequal);

    ASTValue *lhs = codegenExpression(exp->lhs);
    ASTValue *rhs = codegenExpression(exp->rhs);

    if(!lhs || !rhs){
        emit_message(msg::FAILURE, "could not codegen expression in binary op", exp->loc);
        return NULL;
    }

    if(!isAssignOp((tok::TokenKind) exp->op)) //XXX messy
        codegenResolveBinaryTypes(&lhs, &rhs, exp->op);
    else if(lhs->type->kind == TYPE_ARRAY)
    {
        emit_message(msg::ERROR, "cannot assign to statically defined array", exp->loc);
    }

    llvm::Value *val;
    ASTValue *retValue = NULL;
#define lhs_val codegenValue(lhs)
#define rhs_val codegenValue(rhs)

    ASTType *TYPE = lhs->getType();
    switch(exp->op)
    {
        //ASSIGN
        case tok::equal:
        case tok::colonequal:
            return codegenAssign(exp->lhs, exp->rhs, exp->op == tok::colonequal);

        // I dont know, do something with a comma eventually
        case tok::comma:
                emit_message(msg::UNIMPLEMENTED, "comma operator not yet implemented", exp->loc);
                return NULL;

        // LOGIC OPS
        case tok::barbar:
        case tok::kw_or:
            TYPE = ASTType::getBoolTy();
            lhs = promoteType(lhs, TYPE);
            rhs = promoteType(rhs, TYPE);
            val = ir->CreateOr(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(TYPE, val);
        case tok::ampamp:
        case tok::kw_and:
            TYPE = ASTType::getBoolTy();
            lhs = promoteType(lhs, TYPE);
            rhs = promoteType(rhs, TYPE);
            val = ir->CreateAnd(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(TYPE, val);

        // BITWISE OPS
        case tok::bar:
        case tok::barequal:
            val = ir->CreateOr(codegenValue(lhs), codegenValue(rhs));
            retValue = new ASTValue(TYPE, val);
            break;

        case tok::caret:
        case tok::caretequal:
            val = ir->CreateXor(codegenValue(lhs), codegenValue(rhs));
            retValue = new ASTValue(TYPE, val);
            break;

        case tok::amp:
        case tok::ampequal:
            val = ir->CreateAnd(codegenValue(lhs), codegenValue(rhs));
            retValue = new ASTValue(TYPE, val);
            break;

        // COMPARE OPS
        case tok::equalequal:
                if(TYPE->isFloating())
                    val = ir->CreateFCmp(CmpInst::FCMP_OEQ, lhs_val, rhs_val);
                else // sign not required, irrelivant for equality
                    val = ir->CreateICmp(CmpInst::ICMP_EQ, lhs_val, rhs_val);
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::bangequal:
                if(TYPE->isFloating())
                    val = ir->CreateFCmp(CmpInst::FCMP_ONE, lhs_val, rhs_val);
                else // sign not required, irrelivant for equality
                    val = ir->CreateICmp(CmpInst::ICMP_NE, lhs_val, rhs_val);
                return new ASTValue(ASTType::getBoolTy(), val);

        case tok::less:
                if(TYPE->isFloating())
                    val = ir->CreateFCmpOLT(lhs_val, rhs_val);
                else if(TYPE->isSigned())
                    val = ir->CreateICmpSLT(lhs_val, rhs_val);
                else
                    val = ir->CreateICmpULT(lhs_val, rhs_val);
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::lessequal:
                if(TYPE->isFloating())
                    val = ir->CreateFCmpOLE(lhs_val, rhs_val);
                else if(TYPE->isSigned())
                    val = ir->CreateICmpSLE(lhs_val, rhs_val);
                else
                    val = ir->CreateICmpULE(lhs_val, rhs_val);
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::greater:
                if(TYPE->isFloating())
                    val = ir->CreateFCmpOGT(lhs_val, rhs_val);
                else if(TYPE->isSigned())
                    val = ir->CreateICmpSGT(lhs_val, rhs_val);
                else
                    val = ir->CreateICmpUGT(lhs_val, rhs_val);
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::greaterequal:
                if(TYPE->isFloating())
                    val = ir->CreateFCmpOGE(lhs_val, rhs_val);
                else if(TYPE->isSigned())
                    val = ir->CreateICmpSGE(lhs_val, rhs_val);
                else
                    val = ir->CreateICmpUGE(lhs_val, rhs_val);
                return new ASTValue(ASTType::getBoolTy(), val);

        // ARITHMETIC OPS
        case tok::plus:
        case tok::plusequal:
            retValue = opAddValues(lhs, rhs);
            break;

        case tok::minus:
        case tok::minusequal:
            retValue = opSubValues(lhs, rhs);
            break;

        case tok::star:
        case tok::starequal:
            retValue = opMulValues(lhs, rhs);
            break;

        case tok::slash:
        case tok::slashequal:
            retValue = opDivValues(lhs, rhs);
            break;

        case tok::percent:
        case tok::percentequal:
            retValue = opModValue(lhs, rhs);
            break;

        case tok::lessless:
            retValue = opShlValue(lhs, rhs);
            break;

        case tok::greatergreater:
            retValue = opShrValue(lhs, rhs);
            break;

        case tok::starstar:
            retValue = opPowValue(lhs, rhs);
        default:
            emit_message(msg::UNIMPLEMENTED, "unimplemented operator", exp->loc);
            return NULL; //XXX: null val
    }

    if(isAssignOp((tok::TokenKind) exp->op)) //XXX messy
    {
        if(rhs->type->coercesTo(lhs->type))
        {
            retValue = promoteType(retValue, TYPE); //TODO: merge with decl assign
        } else if(rhs->type->castsTo(lhs->type) && exp->op == tok::colonequal) // cast equal
        {
            retValue = promoteType(retValue, TYPE);
        } else
        {
            emit_message(msg::ERROR, "cannot assign value of type '" + rhs->type->getName() +
                    "' to type '" + lhs->type->getName() + "'", exp->loc);
            return NULL;
        }
        storeValue(lhs, retValue);
    }

    return retValue;

#undef lhs_val
#undef rhs_val
}

void IRCodegenContext::codegenReturnStatement(ReturnStatement *exp)
{
    if(exp->expression)
    {
        ASTValue *value = codegenExpression(exp->expression);
        ASTValue *v = promoteType(value, currentFunction.retVal->getType());
        storeValue(currentFunction.retVal, v);
    }

    ir->CreateBr(currentFunction.exit);
    setTerminated(true);
}

void IRCodegenContext::codegenStatement(Statement *stmt)
{
    if(!stmt) return;
    dwarfStopPoint(stmt->loc);
    if(Expression *estmt = dynamic_cast<Expression*>(stmt))
    {
        codegenExpression(estmt);
    } else if (DeclarationStatement *dstmt = dynamic_cast<DeclarationStatement*>(stmt))
    {
        codegenDeclaration(dstmt->declaration);
    } else if (ReturnStatement *rstmt = dynamic_cast<ReturnStatement*>(stmt))
    {
        codegenReturnStatement(rstmt);
    } else if(CaseStatement *cstmt = dynamic_cast<CaseStatement*>(stmt))
    {
        BasicBlock *caseBB = BasicBlock::Create(context, "case", ir->GetInsertBlock()->getParent());

        if(!isTerminated())
            ir->CreateBr(getScope()->getBreak());

        setTerminated(false);

        ir->SetInsertPoint(caseBB);

        for(int i = 0; i < cstmt->values.size(); i++)
        {
            Expression *value = cstmt->values[i];
            if(!value->isConstant())
            {
                emit_message(msg::ERROR, "case value must be a constant", cstmt->loc);
            }
            getScope()->addCase(value, codegenExpression(value), caseBB);
        }

    } else if(LabelStatement *lstmt = dynamic_cast<LabelStatement*>(stmt))
    {
        ASTValue *lbl = codegenIdentifier(lstmt->identifier);
        llvm::BasicBlock *BB = (llvm::BasicBlock*) lbl->cgValue;
        if(!isTerminated())
            ir->CreateBr(BB);
        ir->SetInsertPoint(BB);
    } else if(GotoStatement *gstmt = dynamic_cast<GotoStatement*>(stmt))
    {
        ASTValue *lbl = codegenIdentifier(gstmt->identifier);
        llvm::BasicBlock *BB = (llvm::BasicBlock*) lbl->cgValue;
        ir->CreateBr(BB);
       // post GOTO block
        BasicBlock *PG = BasicBlock::Create(context, "", ir->GetInsertBlock()->getParent());
        ir->SetInsertPoint(PG);
        setTerminated(true);
    } else if(BreakStatement *bstmt = dynamic_cast<BreakStatement*>(stmt))
    {
        BasicBlock *br = getScope()->getBreak();
        if(!br){
            emit_message(msg::ERROR, "break doesnt make sense here!", stmt->loc);
            return;
        }
        ir->CreateBr(br);
        ir->SetInsertPoint(BasicBlock::Create(context, "", ir->GetInsertBlock()->getParent()));
    } else if(ContinueStatement *cstmt = dynamic_cast<ContinueStatement*>(stmt))
    {
        BasicBlock *cont = getScope()->getContinue();
        if(!cont){
            emit_message(msg::ERROR, "continue doesnt make sense here!", stmt->loc);
            return;
        }

        ir->CreateBr(cont);
        ir->SetInsertPoint(BasicBlock::Create(context, "", ir->GetInsertBlock()->getParent()));
    } else if(CompoundStatement *cstmt = stmt->compoundStatement())
    {
        for(int i = 0; i < cstmt->statements.size(); i++)
        {
            if(!isTerminated() || (dynamic_cast<LabelStatement*>(cstmt->statements[i]) ||
                              dynamic_cast<CaseStatement*>(cstmt->statements[i])))
                codegenStatement(cstmt->statements[i]);
        }
        return;
    } else if(BlockStatement *bstmt = stmt->blockStatement())
    {
        ASTValue *value = NULL;
        pushScope(new IRScope(bstmt->scope, unit->debug->createScope(getScope()->debug, bstmt->loc)));
        if(IfStatement *istmt = stmt->ifStatement())
        {
            codegenIfStatement(istmt);
        } else if(LoopStatement *lstmt = stmt->loopStatement())
        {
            codegenLoopStatement(lstmt);
        } else if(SwitchStatement *sstmt = stmt->switchStatement())
        {
            codegenSwitchStatement(sstmt);
        } else if(ElseStatement *estmt = stmt->elseStatement())
        {
            codegenElseStatement(estmt);
        }
        popScope();
    } else emit_message(msg::FAILURE, "i dont know what kind of statmeent this isssss", stmt->loc);
}

void IRCodegenContext::codegenVariableDeclaration(VariableDeclaration *vdecl) {
    dwarfStopPoint(vdecl->loc);
    ASTType *vty = vdecl->type;

    //XXX merge with 'new expression'?

    ASTValue *defaultValue = 0;
    //XXX note that we are storing the alloca(pointer) to the variable in the CGValue
    if(vdecl->value)
    {
        TupleExpression *texp = dynamic_cast<TupleExpression*>(vdecl->value);
        if(vty && vty->isComposite() && texp)
        {
            defaultValue = codegenTupleExpression(texp, vty);
        } else { // not composite vty
            defaultValue = codegenExpression(vdecl->value);
        }
    }

    if(vty->kind == TYPE_DYNAMIC)
    {
        if(!defaultValue)
        {
            emit_message(msg::FAILURE,
                    "failure to codegen dynamic 'var' type default expression", vdecl->loc);
        }

        vty = defaultValue->getType();
        vdecl->type = vty;
    } else if(defaultValue)
    {
        //TODO: promote type is borkd
        defaultValue = promoteType(defaultValue, vty);
    }

    ///////////////
    //XXX work around for undeclared struct
    Identifier *id = NULL;
    ASTUserType *userty = vty->userType();
    if(userty && vty->isUnknown())
    {
        emit_message(msg::WARNING, "unknown type should be resolved in codegen" + id->getName(), vdecl->loc);
        if(userty->identifier->isUndeclared())
        {
            id = lookup(userty->identifier->getName());
            if(id->isUndeclared()){
                emit_message(msg::ERROR, "undeclared struct: " + id->getName(), vdecl->loc);
                return;
            }
            vty = id->getDeclaredType();
        }
    }
    ///////////////

    Type *llty = codegenType(vty);

    AllocaInst *llvmDecl = ir->CreateAlloca(llty, 0, vdecl->getName());

    llvmDecl->setAlignment(8);
    ASTValue *idValue = new ASTValue(vty, llvmDecl, true, vty->isReference());

    if(defaultValue) {
        defaultValue = promoteType(defaultValue, vty);
        storeValue(idValue, defaultValue);

        Instruction *vinst = unit->debug->createVariable(vdecl->getName(),
                idValue, ir->GetInsertBlock(), vdecl->loc);
        vinst->setDebugLoc(llvm::DebugLoc::get(vdecl->loc.line, vdecl->loc.ch, diScope()));
        //TODO: maybe create a LValue field in CGValue?
    } else if(vty->isClass()) {
        //TODO: store null if no value
    }

    vdecl->identifier->setValue(idValue);
}

void IRCodegenContext::codegenFunctionDeclaration(FunctionDeclaration *fdecl) {
    IRFunction backup = currentFunction;
    currentFunction = IRFunction();

    FunctionType *fty = (FunctionType*) codegenType(fdecl->getType());
    Function *func;
    func = (Function*) module->getOrInsertFunction(fdecl->getMangledName(), fty);

    if(fdecl->body)
    {
        func->setLinkage(Function::ExternalLinkage);
        BasicBlock *BB = BasicBlock::Create(context, "entry", func);
        BasicBlock *exitBB = BasicBlock::Create(context, "exit", func);
        currentFunction.exit = exitBB;

        currentFunction.retVal = NULL;
        if(func->getReturnType() != Type::getVoidTy(context))
        {
            currentFunction.retVal = new ASTValue(fdecl->getReturnType(),
                    new AllocaInst(codegenType(fdecl->getReturnType()),
                        0, "ret", BB), true);
        }

        ir->SetInsertPoint(BB);

        dwarfStopPoint(fdecl->loc);
        unit->debug->createFunction(fdecl);
        pushScope(new IRScope(fdecl->scope, fdecl->diSubprogram));
        dwarfStopPoint(fdecl->loc);

        ASTFunctionType *astfty = fdecl->getType()->functionType();
        int idx = 0;

        Function::arg_iterator AI = func->arg_begin();
        if(fdecl->owner) {
            AI->setName("this");
            lookup("this")->setValue(new ASTValue(fdecl->owner, AI, false, true));
            AI++;
        }

        for(; AI != func->arg_end(); AI++, idx++)
        {
            if(astfty->params.size() < idx){
                    emit_message(msg::FAILURE,
                            "argument counts dont seem to match up...", fdecl->loc);
                    return;
            }

            AI->setName(fdecl->parameters[idx]->getName());
            AllocaInst *alloc = new AllocaInst(codegenType(fdecl->parameters[idx]->getType()),
                                               0, fdecl->parameters[idx]->getName(), BB);
            alloc->setAlignment(8);
            ASTValue *alloca = new ASTValue(fdecl->parameters[idx]->getType(), alloc, true);

            if(fdecl->parameters[idx]->getType()->isReference()) {
                new StoreInst(AI, codegenRefValue(alloca), BB);
            } else {
                new StoreInst(AI, codegenLValue(alloca), BB);
            }
            // i think we arent using IRBuilder here so we can insert at top of BB

            fdecl->parameters[idx]->getIdentifier()->setValue(alloca);

            //register debug params
            //XXX hacky with Instruction, and setDebugLoc manually
            Instruction *ainst = unit->debug->createVariable(fdecl->parameters[idx]->getName(),
                                                      alloca, BB, fdecl->loc, idx+1);
            ainst->setDebugLoc(llvm::DebugLoc::get(fdecl->loc.line, fdecl->loc.ch, diScope()));
            //TODO: register value to scope
        }

        codegenStatement(fdecl->body);

        if(!isTerminated())
            ir->CreateBr(currentFunction.exit);

        if(!currentFunction.retVal) // returns void
        {
            ir->SetInsertPoint(currentFunction.exit);
            ir->CreateRetVoid();
        } else
        {
            ir->SetInsertPoint(currentFunction.exit);
            ASTValue *astRet = loadValue(currentFunction.retVal);
            ir->CreateRet(codegenValue(astRet));
        }

        popScope();
    } else { // function has no body; external linkage (weak external if actually declared 'extern')
        if(fdecl->isExternal())
            func->setLinkage(Function::ExternalWeakLinkage);
        func->setLinkage(Function::ExternalLinkage);
    }

    setTerminated(false);
    currentFunction = backup;
}

void IRCodegenContext::codegenUserTypeDeclaration(UserTypeDeclaration *utdecl) {
    //TODO: should this be near codegen type?
    //define type interface. define functions in type
    ASTScope::iterator end = utdecl->getScope()->end();
    for(ASTScope::iterator it = utdecl->getScope()->begin(); it != end; it++){
        if(it->getDeclaration()->functionDeclaration()){
            codegenDeclaration(it->getDeclaration());
        }
    }
}


void IRCodegenContext::codegenDeclaration(Declaration *decl)
{
    if(FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(decl)) {
        codegenFunctionDeclaration(fdecl);
    } else if(VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(decl)) {
        codegenVariableDeclaration(vdecl);
    } else if(UserTypeDeclaration *utdecl = dynamic_cast<UserTypeDeclaration*>(decl)) {
        codegenUserTypeDeclaration(utdecl);
    }
    ///NOTE (for the most part) type declarations are not Codegen'd like values, accesible across Modules
}

void IRCodegenContext::codegenIncludeUnit(IRTranslationUnit *current, TranslationUnit *inc)
{
    //XXX should be 'import'?
}

void IRCodegenContext::codegenTranslationUnit(IRTranslationUnit *u)
{
    this->unit = u;
    this->module = this->unit->module;

    pushScope(u->getScope());

    for(int i = 0; i < unit->unit->importUnits.size(); i++) //TODO: import symbols.
    {
        codegenIncludeUnit(this->unit, unit->unit->importUnits[i]);
    }

    // alloc globals before codegen'ing functions
    // iterate over globals
    ASTScope::iterator end = unit->getScope()->end();
    for(ASTScope::iterator it = unit->getScope()->begin(); it != end; it++)
    {
        Identifier *id = *it;
        VariableDeclaration *vdecl = id->getDeclaration()->variableDeclaration();
        if(id->isVariable())
        {
            ASTType *idTy = id->getType();

            //TODO: correct type for global storage (esspecially pointers?)
            ASTValue *idValue = 0;
            if(vdecl->value)
                idValue = codegenExpression(vdecl->value);

            if(idTy->kind == TYPE_DYNAMIC)
            {
                if(!idValue)
                    emit_message(msg::FAILURE,
                            "attempt to codegen dynamically typed \
                            variable without properly assigned value", vdecl->loc);
                idTy = idValue->getType();
                vdecl->type = idTy;
            }

            llvm::Constant* gValue;
            if(vdecl->qualifier.external)
            {
                gValue = NULL;
            } else if(vdecl->value)
            {
                gValue = (llvm::Constant*) codegenValue(
                            promoteType(codegenExpression(vdecl->value), idTy)
                            );
            } else
            {
                gValue = (llvm::Constant*) llvm::Constant::getNullValue(codegenType(idTy));
            }

            GlobalVariable *llvmval = (GlobalVariable*)
                    module->getOrInsertGlobal(id->getName(), codegenType(idTy));

            GlobalValue::LinkageTypes linkage = vdecl->qualifier.external ?
                GlobalValue::ExternalLinkage : GlobalValue::ExternalLinkage;
            llvmval->setLinkage(linkage);
            llvmval->setInitializer(gValue);

            ASTValue *gv = new ASTValue(idTy, llvmval, true);
            id->setValue(gv);

            dwarfStopPoint(vdecl->loc);
            unit->debug->createGlobal(vdecl, gv);

        } else if(id->isFunction())
        {
            //TODO: declare func?
        }
    }

    // iterate over unit, get functions and type interface (functions)
    end = unit->getScope()->end();
    for(ASTScope::iterator it = unit->getScope()->begin(); it != end; it++){
        if(it->getDeclaration()->functionDeclaration() ||
                it->getDeclaration()->typeDeclaration()){
            codegenDeclaration(it->getDeclaration());
        }
    }

    unit->debug->finalize();
    popScope();
}

void IRCodegenContext::codegenPackage(Package *p)
{
    if(p->isTranslationUnit()) // leaf in package tree
    {
        std::string err;
        IRTranslationUnit *unit = new IRTranslationUnit(this, (TranslationUnit*) p);
        p->cgValue = 0;
        codegenTranslationUnit(unit);

        // XXX debug, output all modules
        std::string outputll = config.tempName + "/" + unit->unit->getName() + ".ll";
        raw_fd_ostream output(outputll.c_str(), err);
        unit->module->print(output, 0);

        linker.linkInModule(unit->module, (unsigned) Linker::DestroySource, &err);

    } else // generate all leaves ...
    {
        for(int i = 0; i < p->children.size(); i++)
        {
            codegenPackage(p->children[i]);
        }
    }
}

#include <fcntl.h>
#include <unistd.h>

std::string IRCodegenContext::codegenAST(AST *ast, WLConfig config)
{
    this->ast = ast;
    this->config = config;
    codegenPackage(ast->getRootPackage());
    if(currentErrorLevel() > msg::WARNING)
    {
        emit_message(msg::OUTPUT, "compilation ended with errors");
        return "";
    }

    createIdentMetadata(linker.getModule());
    //linker.getModule()->MaterializeAll();

    if(verifyModule(*linker.getModule(), PrintMessageAction))
    {
        emit_message(msg::OUTPUT, "failed to compile source code");
        //return;
    } else
    {
        emit_message(msg::OUTPUT, "successfully compiled source");
    }

    std::string err;
    std::string outputll;
    std::string outputo;

    if(config.emitllvm)
    {
        outputll = "output.ll";
    } else
    {
        outputll = config.tempName + "/output.ll";
    }

    if(config.link)
    {
        outputo = config.tempName + "/output.o";
    } else
    {
        outputo = "output.o";
    }

    raw_fd_ostream output(outputll.c_str(), err);
    linker.getModule()->print(output, 0);
    output.close();

    std::string llccmd = "llc " + outputll + " --filetype=obj -O0 -o " + outputo;

    if(!config.emitllvm)
    {
        int syserr = system(llccmd.c_str());
        if(syserr)
        {
            emit_message(msg::FATAL, std::string("system command failed...") + llccmd.c_str());
        }
    } else
    {
        return "";
    }

    return outputo;
}

void IRCodegen(AST *ast, WLConfig config)
{
    IRCodegenContext context;
    context.codegenAST(ast, config);
}
