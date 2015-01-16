#include "ast.hpp"
#include "token.hpp"
#include "irCodegenContext.hpp"

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 5
#define LLVM_35
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Linker/Linker.h>
#elif LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 4
#define LLVM_34
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#endif

#include "message.hpp"

#include <fstream>
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace llvm;

IRTranslationUnit::IRTranslationUnit(IRCodegenContext *c, ModuleDeclaration *mod) :
    context(c), mdecl(mod) {
    llvmModule = new Module("", context->context);
    debug = new IRDebug(context, this);
    scope = new IRScope(mdecl->getScope(), debug->getCompileUnit());
}

SourceLocation currentLoc;

/*
void IRCodegenContext::dwarfStopPoint(int ln)
{
    llvm::DebugLoc loc = llvm::DebugLoc::get(ln, 1, diScope());
    assert_message(!loc.isUnknown(), msg::FAILURE, "unknown debug location", location);
    //ir->SetCurrentDebugLocation(loc);
} */

void IRCodegenContext::dwarfStopPoint(SourceLocation l)
{
    currentLoc = l;
    llvm::DebugLoc loc = llvm::DebugLoc::get(l.line, l.ch, diScope());
    assert_message(!loc.isUnknown(), msg::FAILURE, "unknown debug location", location);
    ir->SetCurrentDebugLocation(loc);
}

llvm::Function *IRCodegenContext::getLLVMMalloc() {
    vector<Type*> llargty;
    llargty.push_back(codegenType(ASTType::getULongTy()));
    FunctionType *fty = FunctionType::get(codegenType(ASTType::getVoidTy()->getPointerTy()),
                llargty, false);
    Function *mallocFunc = (Function*) module->getOrInsertFunction("malloc", fty);

    return mallocFunc;
}

llvm::Function *IRCodegenContext::getLLVMMemcpy() {
    vector<Type*> llargty;
    llargty.push_back(codegenType(ASTType::getVoidTy()->getPointerTy()));
    llargty.push_back(codegenType(ASTType::getVoidTy()->getPointerTy()));
    llargty.push_back(codegenType(ASTType::getULongTy()));
    FunctionType *fty = FunctionType::get(codegenType(ASTType::getVoidTy()->getPointerTy()),
                llargty, false);
    Function *memcpyFunc = (Function*) module->getOrInsertFunction("memcpy", fty);

    return memcpyFunc;
}

/*
 * interface
 */

ASTValue *IRCodegenContext::codegenInterfaceVTable(InterfaceVTable *vt) {
    if(vt->cgvalue) return vt->cgvalue;

    std::vector<Constant *> members; // type info constant members
    std::vector<Constant *> arrayList;
    ASTType *vfty = ASTType::getVoidFunctionTy()->getPointerTy()->getPointerTy();

    for(int i = 0; i < vt->vtable.size(); i++){
        FunctionDeclaration *fdecl = vt->vtable[i];
        if(!fdecl) emit_message(msg::FAILURE, "invalid function identifier");
        FunctionType *fty = (FunctionType*) codegenType(fdecl->getType());
        Constant *func = module->getOrInsertFunction(fdecl->getMangledName(), fty);

        arrayList.push_back((Function*)
                ir->CreatePointerCast(func, codegenType(vfty)));
    }

    ArrayType *vtableTy = ArrayType::get(codegenType(vfty), vt->vtable.size());
    Constant *vtable = ConstantArray::get(
                vtableTy,
                arrayList
                );

    members.push_back(vtable);

    GlobalVariable *gv = new GlobalVariable(*module, vtableTy, true, GlobalValue::PrivateLinkage, vtable);
    gv->setName("InterfaceVT_" + vt->interface->getMangledName() + "$" + vt->sourceType->getMangledName());
    vt->cgvalue = new ASTBasicValue(vfty, gv, true);
    return vt->cgvalue;
}

ASTValue *IRCodegenContext::getInterfaceSelf(ASTValue *iface) {
    Value *v = codegenLValue(iface);

    return new ASTBasicValue(ASTType::getVoidTy()->getPointerTy(), ir->CreateStructGEP(v, 0), true);
}

ASTValue *IRCodegenContext::getInterfaceVTable(ASTValue *iface) {
    Value *v = codegenLValue(iface);

    return new ASTBasicValue(ASTType::getVoidTy()->getPointerTy()->getPointerTy(), ir->CreateStructGEP(v, 1), true);
}

llvm::Type *IRCodegenContext::codegenUserType(ASTType *ty)
{
    ASTUserType *userty = ty->asUserType();
    if(!userty) {
        emit_message(msg::FAILURE, "CODEGEN: invalid user type", location);
    }

    if(userty->identifier->isUndeclared()){ //XXX superfluous test
        emit_message(msg::WARNING, "CODEGEN: type should be resolved by now", location);
        userty->identifier = lookup(ty->getName());
        if(userty->identifier->isUndeclared()){
            emit_message(msg::ERROR, "CODEGEN: undeclared struct: " + userty->getName());
            return NULL;
        }
    }

    if(StructType *sty = module->getTypeByName(ty->getMangledName())) {
        return sty;
    }

    if(unit->types.count(ty->getMangledName())) {
        Type *llty = unit->types[ty->getMangledName()];
        return llty;
    }

    // if interface create an interface struct
    // will still need to populate the interface members though
    // XXX seems a bit messy
    if(ty->isInterface()) {
        InterfaceDeclaration *idecl = ty->getDeclaration()->interfaceDeclaration();

        std::map<std::string, InterfaceVTable*>::iterator it = idecl->vtables.begin();
        while(it != idecl->vtables.end()) {
            //codegen interface vtable
            InterfaceVTable *vt = it->second;
            codegenInterfaceVTable(vt);
            printf("codegen'ing iface %s\n", vt->sourceType->getName().c_str());
            it++;
        }

        vector<Type*> members;
        members.push_back(codegenType(ASTType::getVoidTy()->getPointerTy())); //*this*
        members.push_back(codegenType(ASTType::getVoidFunctionTy()->getPointerTy()->getPointerTy())); //*vtable*
        StructType *sty = StructType::create(context, ty->getMangledName());
        sty->setBody(members);
        return sty; //XXX create debug info
    }


    StructType *sty = StructType::create(context);
    sty->setName(ty->getMangledName());
    unit->types[ty->getMangledName()] = IRType(ty, sty);

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
    for(int i = 0; i < userty->length(); i++) {
        if(VariableDeclaration *vd = dynamic_cast<VariableDeclaration*>(userty->getMember(i)))
        {
            structVec.push_back(codegenType(vd->type));
        } else {
            emit_message(msg::UNIMPLEMENTED, "this cant be declared in a struct (yet?)", location);
        }
    }

    if(!userty->isOpaque()) {
        sty->setBody(structVec);
    }

    unit->debug->createUserType(ty);

    if(ty->isClass()) {
        userty->getDeclaration()->classDeclaration()->typeinfo = createTypeInfo(ty);
    }

    return sty;
}

llvm::Type *IRCodegenContext::codegenStructType(ASTType *ty) {
    return codegenUserType(ty);
}

//TODO: fix union types. codegen is incorrect (calls 'codegen usertype')
llvm::Type *IRCodegenContext::codegenUnionType(ASTType *ty) {
    //return codegenUserType(ty);

    // XXX useless below
    if(!ty->isUnion()) {
        emit_message(msg::FAILURE, "unknown union type", location);
    }

    ASTUserType *userty = ty->asUserType();

    unsigned align = 0;
    unsigned size = 0;
    ASTType *alignedType = 0;
    std::vector<Type*> unionVec;
    for(int i = 0; i < userty->length(); i++) {
        if(VariableDeclaration *vd = dynamic_cast<VariableDeclaration*>(userty->getMember(i))) {
            if(vd->getType()->getAlign() > align) {
                alignedType = vd->type;
                align = vd->getType()->getAlign();
            }

            if(vd->getType()->getSize() > size) {
                size = vd->getType()->getSize();
            }
            //unionVec.push_back(codegenType(vd->type));
        } else
            emit_message(msg::UNIMPLEMENTED, "this cant be declared in a union (yet?)", location);
    }

    if(!userty->isOpaque()) {
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
    if(ty->getKind() != TYPE_TUPLE || !tupty) {
        emit_message(msg::FAILURE, "CODEGEN: invalid tuple type codegen", location);
        return NULL;
    }

    if(ty->cgType) return ty->cgType;

    if(!tupty->types.size()) {
        emit_message(msg::FAILURE, "CODEGEN: 0-tuple found in codegen", location);
        return NULL;
    }

    std::vector<Type*> tupleVec;
    for(int i = 0; i < tupty->types.size(); i++)
    {
        tupleVec.push_back(codegenType(tupty->types[i]));
    }

    StructType *tty = StructType::get(context, tupleVec);
    //tty->setBody(tupleVec);
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
        emit_message(msg::FAILURE, "attempt to codegen invalid array type", location);
    }

    //if(ty->cgType) return ty->cgType;

    Type *ret = NULL;
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

        unit->debug->createDynamicArrayType(ty);

        ret = aty;
    } else
    {
        llvm::ArrayType *aty = ArrayType::get(codegenType(arrty->arrayOf), arrty->length());
        unit->debug->createArrayType(ty);

        ret = aty;
    }

    return ret;
}

llvm::Type *IRCodegenContext::codegenFunctionType(ASTType *ty) {
    ASTFunctionType *astfty = ty->asFunctionType();
    if(!astfty) {
        emit_message(msg::FAILURE, "attempt to codegen invalid function type", location);
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

    ASTType *tmp;
    switch(ty->getKind())
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
            tmp = ty->getPointerElementTy();
            if(tmp->getKind() == TYPE_VOID) tmp = ASTType::getCharTy();
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
    // XXX bit messy this.
    if(MethodValue *mvalue = dynamic_cast<MethodValue*>(value)) {
        return codegenMethod(mvalue);
    }

    if(FunctionValue *fvalue = dynamic_cast<FunctionValue*>(value)) {
        return codegenFunction(fvalue);
    }

    if(TupleValue *tvalue = dynamic_cast<TupleValue*>(value)) {
        //return ir->CreateLoad(codegenTuple(tvalue));
        return codegenTuple(tvalue);
    }

    if(!value->value) {
        //TODO
        emit_message(msg::FAILURE, "AST Value failed to generate", location);
    }

    if(value->isLValue()) {
        if(value->isReference()) {
            return codegenLValue(value);
        }
        return ir->CreateAlignedLoad(codegenLValue(value), 4);
    }

    return (llvm::Value *) value->value;
}

llvm::Value *IRCodegenContext::codegenLValue(ASTValue *value)
{
    //XXX duplicate messy code
    if(TupleValue *tvalue = dynamic_cast<TupleValue*>(value)) {
        return codegenTuple(tvalue);
    }

    if(!value->isLValue() && !value->isReference())
    {
        emit_message(msg::FATAL, "rvalue used in lvalue context!", location);
        return NULL;
    }
    if(!value->value) {
        //TODO
        emit_message(msg::FAILURE, "AST Value failed to generate", location);
    }

    if(value->isLValue() && value->isReference()) {
        return ir->CreateAlignedLoad(value->value, 4);
    }

    return (llvm::Value*) value->value;
}

llvm::Value *IRCodegenContext::codegenRefValue(ASTValue *value) {
    if(!value->value) {
        //TODO
        emit_message(msg::FAILURE, "AST Value failed to generate", location);
    }

    if(!value->isReference()) {
        emit_message(msg::FAILURE, "Attempting to get reference value of non reference", location);
    }

    return (llvm::Value*) value->value;
}

// SCOPE
void IRCodegenContext::enterScope(IRScope *sc) {
    sc->parent = scope;
    scope = sc;
}

IRScope *IRCodegenContext::endScope() {
    if(scope->table->isLocalScope()) { // only release if we are in code block (not global)
        ASTScope::iterator it = scope->begin();
        for(; it != scope->end(); it++) {
            Identifier *id = *it;
            if(id->isVariable() && id->getType()->isReleasable() && !id->getDeclaration()->isWeak()) {
                if(id->getName() != "this") { //XXX messy. dont release 'this'
                    releaseObject(codegenIdentifier(id));
                }
            }
        }
    }
    return scope;
}

IRScope *IRCodegenContext::exitScope() {
    IRScope *sc = scope;
    if(!isTerminated()) endScope();
    scope = scope->parent;
    return sc;
}

/*
void IRCodegenContext::copyMemory(ASTValue *dest, ASTValue *src, ASTValue *sz) {
    std::vector<Type*> argty;
    argty.push_back(Type::getInt8Ty(context)->getPointerTo()); //dst
    argty.push_back(Type::getInt8Ty(context)->getPointerTo()); //src
    argty.push_back(Type::getInt64Ty(context)); //sz
    argty.push_back(Type::getInt32Ty(context)); // align
    argty.push_back(Type::getInt1Ty(context)); // volatile
    Function *memcpy_intr = Intrinsic::getDeclaration(module, Intrinsic::memcpy, argty);

    std::vector<Value*> vals;
    // XXX populate vals
    ir->CreateCall(memcpy_intr, vals);
} */

void IRCodegenContext::storeValue(ASTValue *dest, ASTValue *val)
{
    if(dest->getType()->isSArray()) {
        if(!val->getType()->is(dest->getType())) {
            emit_message(msg::ERROR, "CG: array store must be same type", location);
        }

        /*
        unsigned len = dest->getType()->length();
        for(int i = 0; i < len; i++) {
            storeValue(
                    opIndex(dest, getIntValue(ASTType::getLongTy(), i)),
                    opIndex(val, getIntValue(ASTType::getLongTy(), i)));
        }

        //XXX use memcpy?
        return;
        */
    }

    if(dest->isReference()) {
        // store return value if needed?
        ir->CreateStore(codegenValue(val), codegenRefValue(dest));
    } else {
        ir->CreateStore(codegenValue(val), codegenLValue(dest));
    }

    // if storing a value that is stack allocated, this value should not be freed
    if(val->isNoFree() && dynamic_cast<ASTBasicValue*>(dest)) {
        ((ASTBasicValue*)dest)->setNoFree(true);
    }
}

ASTValue *IRCodegenContext::getStringValue(std::string str) {
    // plus one for null
    ASTType *strty = ASTType::getStringTy(str.length() + 1);

    Constant *strConstant = ConstantDataArray::getString(context, str);
    GlobalVariable *GV = new GlobalVariable(*module, strConstant->getType(), true,
            GlobalValue::PrivateLinkage, strConstant);

    //XXX string dedup?

    ASTBasicValue *ret = new ASTBasicValue(strty, GV, true);
    ret->setConstant(true);
    return ret;
}

ASTValue *IRCodegenContext::getFloatValue(ASTType *t, float i){
    //TODO: constant cache -256-255
    ASTBasicValue *val = new ASTBasicValue(t, ConstantFP::get(codegenType(t), i));
    val->setConstant(true);
    return val;
}

ASTValue *IRCodegenContext::getIntValue(ASTType *t, int i){
    //TODO: constant cache -256-255
    ASTBasicValue *val = new ASTBasicValue(t, ConstantInt::get(codegenType(t), i));
    val->setConstant(true);
    return val;
}

ASTValue *IRCodegenContext::indexValue(ASTValue *val, int i) {
    ASTCompositeType *compty = val->getType()->asCompositeType();

    if(!compty) {
        emit_message(msg::ERROR, "attempt to index non-composite type");
        return NULL;
    }

    //TODO: handle if val is LValue
    Value *v = ir->CreateExtractValue(codegenValue(val), i);
    return new ASTBasicValue(compty->getMemberType(i), v);
}

ASTValue *IRCodegenContext::loadValue(ASTValue *lval)
{
    assert_message(lval->isLValue(), msg::FAILURE, "attempted to load RValue (must be LValue)", location);
    ASTValue *loaded = new ASTBasicValue(lval->getType(), codegenValue(lval));
    return loaded;
}

ASTValue *IRCodegenContext::getThis() {
    Identifier *thisId = getScope()->lookup("this", false);
    if(!thisId) {
        emit_message(msg::ERROR, "attempt to access 'this' in non-class function", location);
        return NULL;
    }
    return codegenIdentifier(thisId);
}

ASTValue *IRCodegenContext::getVTable(ASTValue *instance) {
    return getMember(instance, "vtable");
}

llvm::Value *IRCodegenContext::codegenMethod(MethodValue *method) {
    ASTUserType *userty = method->getInstance()->getType()->asUserType();
    if(!userty) emit_message(msg::ERROR, "virtual function lookup only valid for class", location);

    ASTValue *vtable = getVTable(method->getInstance());

    std::vector<Value *> gep;
    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), method->getDeclaration()->getVTableIndex()));
    Value *llval = ir->CreateLoad(ir->CreateGEP(codegenValue(vtable), gep));

    ASTType *fty = method->getDeclaration()->getType();
    return ir->CreatePointerCast(llval, codegenType(fty)->getPointerTo());
}

llvm::Value *IRCodegenContext::codegenFunction(FunctionValue *function) {
    Constant *fconst= module->getOrInsertFunction(
                function->getDeclaration()->getMangledName(),
                (FunctionType*) codegenType(function->getDeclaration()->getType())
            );
    return fconst;
}

llvm::Value *IRCodegenContext::codegenTuple(TupleValue *tuple, ASTType *target) {
    ASTCompositeType* ty;
    if(!target) target = tuple->getType();
    ty = dynamic_cast<ASTCompositeType*>(target);

    std::vector<ASTValue*> vals;
    std::vector<ASTType*> types;
    bool isConst = true;

    // cannot simply allocate dynamic array type. create static array instead. Then it is
    // simple to convert static array (later on) to dynamic array by bounding it in the relevent struct.
    bool dyarray = false;
    ASTDynamicArrayType *dyarrayTy = NULL;
    if(ASTDynamicArrayType *daty = dynamic_cast<ASTDynamicArrayType*>(ty)) {
        dyarrayTy = daty;
        ty = (ASTCompositeType*) daty->getMemberType(0)->getArrayTy(tuple->values.size());
        dyarray = true;
    }

    for(int i = 0; i < tuple->values.size(); i++)
    {
        //if(i >= ty->length()) emit_message(msg::FAILURE, "unexpected tuple expression length");
        ASTValue *val = tuple->values[i];
        val = promoteType(val, ty->getMemberType(i));

        vals.push_back(val);
        types.push_back(val->getType());

        if(!val->isConstant()) {
            isConst = false;
        }
    }

    Type *llty = codegenType(ty);

    // tuple is a constant; can optimize tuple as constant global
    if(isConst) {
        std::vector<Constant*> llvals;
        for(int i = 0; i < vals.size(); i++)
        {
            llvals.push_back((Constant*) codegenValue(vals[i]));
        }

        Constant *llconst = NULL;
        // create constant global value for array. fetch pointer
        if(dynamic_cast<ASTArrayType*>(ty)) {
            llconst = ConstantArray::get((ArrayType*)llty, llvals);
        } else {
            llconst = ConstantStruct::get((StructType*)llty, llvals);
        }

        if(dyarray) {
            GlobalVariable *GV = new GlobalVariable(*module, llty, true,
                    GlobalValue::PrivateLinkage,
                    llconst);
            GV->setConstant(false);

            std::vector<Constant*> gep;
            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
            llconst = ConstantExpr::getGetElementPtr(GV, gep);

            std::vector<Constant*> vals;
            vals.push_back(llconst);
            vals.push_back(ConstantInt::get(codegenType(ASTType::getLongTy()), tuple->values.size()));
            Constant *st = ConstantStruct::get((StructType*) codegenType(dyarrayTy), vals);
            return st;
        } else {
            //GlobalVariable *DGV = new GlobalVariable(*module, codegenType(ty), true,
            //        GlobalValue::PrivateLinkage, llconst);
        }

        return llconst;
    } else {
        // alloca, and store all values into a struct type

        //XXX will fail on dynamic arrays
        Value *val = ir->CreateAlloca(llty);
        for(int i = 0; i < vals.size(); i++) {
            ir->CreateStore(codegenValue(vals[i]), ir->CreateStructGEP(val, i)); //XXX const gep
        }

        return ir->CreateLoad(val);
    }
}

ASTValue *IRCodegenContext::createTypeInfo(ASTType *ty) {
    if(!ty->isClass()){
        emit_message(msg::ERROR, "can only get type info for class");
    }

    std::vector<Constant *> members; // type info constant members
    std::vector<Constant *> arrayList;
    ASTType *vfty = ASTType::getVoidFunctionTy()->getPointerTy();

    UserTypeDeclaration *utdecl = ty->getDeclaration()->userTypeDeclaration();
    if(ClassDeclaration *cldecl = utdecl->classDeclaration()) {
        for(int i = 0; i < cldecl->vtable.size(); i++) {
            FunctionDeclaration *fdecl = cldecl->vtable[i];
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
    return new ASTBasicValue(vfty, gv, true);
}

void IRCodegenContext::retainObject(ASTValue *val) {
    //XXX error if not class?
    if(val->getType()->isClass()) {
        BasicBlock *retainBB = BasicBlock::Create(context, "retain", ir->GetInsertBlock()->getParent());
        BasicBlock *endretainBB = BasicBlock::Create(context, "endretain", ir->GetInsertBlock()->getParent());
        ASTBasicValue null = ASTBasicValue(val->getType(), ConstantPointerNull::get((PointerType*) codegenType(val->getType())));
        ASTValue *notNull = opNEqValue(val, &null);
        ir->CreateCondBr(codegenValue(notNull), retainBB, endretainBB);
        ir->SetInsertPoint(retainBB);
        ASTValue *v = getMember(val, "refcount");
        storeValue(v, opIncValue(v));
        ir->CreateBr(endretainBB);
        ir->SetInsertPoint(endretainBB);
    }
}

void IRCodegenContext::releaseObject(ASTValue *val) {
    //XXX error if not class?
    if(val->getType()->isClass()) {
        BasicBlock *beginBr = BasicBlock::Create(context, "decref", ir->GetInsertBlock()->getParent());
        BasicBlock *deconstructBr = BasicBlock::Create(context, "del", ir->GetInsertBlock()->getParent());
        BasicBlock *afterBr = BasicBlock::Create(context, "endref", ir->GetInsertBlock()->getParent());

        //TODO: statically null check if able?
        ASTBasicValue null = ASTBasicValue(val->getType(), ConstantPointerNull::get((PointerType*) codegenType(val->getType())));
        ASTValue *isNull = opNEqValue(val, &null);
        ir->CreateCondBr(codegenValue(isNull), beginBr, afterBr);
        ir->SetInsertPoint(beginBr);
        ASTValue *refcount = getMember(val, "refcount");
        storeValue(refcount, opDecValue(refcount));
        ASTValue *zero = getIntValue(ASTType::getLongTy(), 0);
        ASTValue *isZero = opLEValue(refcount, zero);

        ir->CreateCondBr(codegenValue(isZero), deconstructBr, afterBr); //TODO: expect no deconstruct
        ir->SetInsertPoint(deconstructBr);
        codegenDelete(val); //delete object
        ir->CreateBr(afterBr); // jump to after block once we're done with destructor conditional
        ir->SetInsertPoint(afterBr);
    }
}

// BINOP .
ASTValue *IRCodegenContext::getMember(ASTValue *val, std::string member) {
    // we are trying to get a base that does not exist

    if(!val->getType()) {
        emit_message(msg::ERROR, "member '" + member + "' not found in class");
    }

    if(val->getType()->isPointer())
        val = getValueOf(val);
    ASTUserType *userty = val->getType()->asUserType();

    if(val->getType() && !userty){
        emit_message(msg::FAILURE, "cannot get member in non-usertype");
    }

    ASTType *orig = val->getType();
    std::vector<Value*> gep;
    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));

    Identifier *id = NULL;
    while(!id && userty) {
        id = userty->getScope()->lookupInScope(member);

        // identifier is either in base or does not exist, recurse to base
        if(!id) {
            // zeroeth member is base class
            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
            userty = dynamic_cast<ASTUserType*>(userty->getBaseType());
        }
    }

    if(!id) {
        emit_message(msg::FAILURE, "cannot get member '" + member + "' in type '" + orig->getName() + "'");
        return NULL;
    }

    if(id->isFunction()){
        //TODO: static vs nonvirtual vs virtual function calls
        ASTValue *ret = 0;
        if(val->getType()->isClass()) {
            ASTUserType *userty = val->getType()->asUserType();
            Identifier *id = userty->getScope()->lookup(member);
            FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(id->getDeclaration());
            ret = new MethodValue(val, fdecl);
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
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context),
                                    index));

        Value *llval;
        if(mtype->isSArray()) {
            gep.push_back(ConstantInt::get(Type::getInt32Ty(context),
                                        0));
            llval = ir->CreateInBoundsGEP(codegenLValue(val), gep);
        } else {
            if(id->getDeclaration()->isStatic()) {
                return id->getValue();
            } else {
                llval = ir->CreateInBoundsGEP(codegenLValue(val), gep);
                llval = ir->CreatePointerCast(llval, codegenType(mtype->getPointerTy()));
            }
        }

        ASTBasicValue *ret = new ASTBasicValue(mtype, llval, true, mtype->isReference());
        ret->setOwner(val);
        ret->setWeak(id->getDeclaration()->isWeak());
        //TODO set other qualifiers
        return ret;
    } else {
        emit_message(msg::ERROR, "unknown member in user type");
        return NULL;
    }
}

// UNOP ^
ASTValue *IRCodegenContext::getValueOf(ASTValue *ptr, bool lvalue){
    assert_message(ptr->getType()->isPointer(), msg::FAILURE, "attempt to dereference non pointer type");
    return new ASTBasicValue(ptr->getType()->getPointerElementTy(), codegenValue(ptr), lvalue);
}

// UNOP &
ASTValue *IRCodegenContext::getAddressOf(ASTValue *lval){
    assert_message(lval->isLValue() || lval->isReference(), msg::FAILURE, "attempt to get address of non-LValue value");
    return new ASTBasicValue(lval->getType()->getPointerTy(), codegenLValue(lval), false);
}

// BINOP +
ASTValue *IRCodegenContext::opAddValues(ASTValue *a, ASTValue *b){
    assert_message(a->getType() == b->getType(), msg::FAILURE, "values must be same type for addition");
    if(a->getType()->isFloating()){
        return new ASTBasicValue(a->getType(), ir->CreateFAdd(codegenValue(a), codegenValue(b)));
    }
    return new ASTBasicValue(a->getType(), ir->CreateAdd(codegenValue(a), codegenValue(b)));
}

// BINOP -
ASTValue *IRCodegenContext::opSubValues(ASTValue *a, ASTValue *b){
    assert_message(a->getType() == b->getType(), msg::FAILURE, "values must be same type for subtraction");
    if(a->getType()->isFloating()){
        return new ASTBasicValue(a->getType(), ir->CreateFSub(codegenValue(a), codegenValue(b)));
    }
    return new ASTBasicValue(a->getType(), ir->CreateSub(codegenValue(a), codegenValue(b)));
}

// BINOP *
ASTValue *IRCodegenContext::opMulValues(ASTValue *a, ASTValue *b){ // *
    assert_message(a->getType() == b->getType(), msg::FAILURE, "values must be same type for multiplication");
    if(a->getType()->isFloating()){
        return new ASTBasicValue(a->getType(), ir->CreateFMul(codegenValue(a), codegenValue(b)));
    }
    //TODO: sign?
    return new ASTBasicValue(a->getType(), ir->CreateMul(codegenValue(a), codegenValue(b)));
}

// BINOP /
ASTValue *IRCodegenContext::opDivValues(ASTValue *a, ASTValue *b){ // /
    assert_message(a->getType() == b->getType(), msg::FAILURE, "values must be same type for division");
    if(a->getType()->isFloating()){
        return new ASTBasicValue(a->getType(), ir->CreateFDiv(codegenValue(a), codegenValue(b)));
    }

    if(a->getType()->isSigned()){
        return new ASTBasicValue(a->getType(), ir->CreateSDiv(codegenValue(a), codegenValue(b)));
    }

    // unsigned int div
    return new ASTBasicValue(a->getType(), ir->CreateUDiv(codegenValue(a), codegenValue(b)));
}

// BINOP %
ASTValue *IRCodegenContext::opModValue(ASTValue *a, ASTValue *b){ // %
    assert_message(a->getType() == b->getType(), msg::FAILURE, "values must be same type for modulus");
    if(a->getType()->isFloating()){
        return new ASTBasicValue(a->getType(), ir->CreateFRem(codegenValue(a), codegenValue(b)));
    }

    if(a->getType()->isSigned()){
        return new ASTBasicValue(a->getType(), ir->CreateSRem(codegenValue(a), codegenValue(b)));
    }

    // unsigned remainder
    return new ASTBasicValue(a->getType(), ir->CreateURem(codegenValue(a), codegenValue(b)));
}

// BINOP <<
ASTValue *IRCodegenContext::opShlValue(ASTValue *a, ASTValue *b){ // <<
    return new ASTBasicValue(a->getType(), ir->CreateShl(codegenValue(a), codegenValue(b)));
}

// BINOP >>
ASTValue *IRCodegenContext::opShrValue(ASTValue *a, ASTValue *b){ // >>
    return new ASTBasicValue(a->getType(), ir->CreateLShr(codegenValue(a), codegenValue(b)));
}

// BINOP **
ASTValue *IRCodegenContext::opPowValue(ASTValue *a, ASTValue *b){ // **
    emit_message(msg::UNIMPLEMENTED, "unimplemented power operator");
	return NULL;
}

// or, ||, logical or
ASTValue *IRCodegenContext::opLOrValue(Expression *a, Expression *b) {
    ASTType *TYPE = ASTType::getBoolTy();
    ASTValue *aval = promoteType(a->getValue(this), TYPE);

    BasicBlock *beforeblock = ir->GetInsertBlock();
    BasicBlock *ortrue = BasicBlock::Create(context, "orend", ir->GetInsertBlock()->getParent());
    BasicBlock *orfalse = BasicBlock::Create(context, "orfalse", ir->GetInsertBlock()->getParent());
    Value *val = codegenValue(aval);
    ir->CreateCondBr(val, ortrue, orfalse);
    ir->SetInsertPoint(orfalse);
    ASTValue *bval = promoteType(b->getValue(this), TYPE);
    Value *falseVal = codegenValue(bval);
    BasicBlock *falsePHIBlock = ir->GetInsertBlock(); //we need to recheck the block we are in because b might be a logical expression that creates a new basic block
    ir->CreateBr(ortrue);
    ir->SetInsertPoint(ortrue);

    PHINode *phiNode = ir->CreatePHI(codegenType(TYPE), 2);
    phiNode->addIncoming(falseVal, falsePHIBlock);
    phiNode->addIncoming(codegenValue(getIntValue(TYPE, 1)), beforeblock);
    val = phiNode;
    return new ASTBasicValue(TYPE, val);
}

// and, &&, logical and
ASTValue *IRCodegenContext::opLAndValue(Expression *a, Expression *b) {
    // same deal as logical Or above
    ASTType *TYPE = ASTType::getBoolTy();
    ASTValue *aval = promoteType(a->getValue(this), TYPE);

    BasicBlock *beforeblock = ir->GetInsertBlock();
    BasicBlock *andend = BasicBlock::Create(context, "andend", ir->GetInsertBlock()->getParent());
    BasicBlock *andtrue = BasicBlock::Create(context, "andtrue", ir->GetInsertBlock()->getParent());
    Value *val = codegenValue(aval);
    ir->CreateCondBr(val, andtrue, andend);
    ir->SetInsertPoint(andtrue);
    ASTValue *bval = promoteType(b->getValue(this), TYPE);
    Value *trueVal = codegenValue(bval);
    BasicBlock *truePHIBlock = ir->GetInsertBlock(); //we need to recheck the block we are in because b might be a logical expression that creates a new basic block
    ir->CreateBr(andend);
    ir->SetInsertPoint(andend);

    PHINode *phiNode = ir->CreatePHI(codegenType(TYPE), 2);
    phiNode->addIncoming(trueVal, truePHIBlock);
    phiNode->addIncoming(codegenValue(getIntValue(TYPE, 0)), beforeblock);
    val = phiNode;
    return new ASTBasicValue(TYPE, val);
}

// UNOP ++
ASTValue *IRCodegenContext::opIncValue(ASTValue *a) {
    if(a->getType()->isFloating()) {
        return opAddValues(a, getFloatValue(a->getType(), 1.0f));
    } else {
        return opAddValues(a, getIntValue(a->getType(), 1));
    }
}

// UNOP --
ASTValue *IRCodegenContext::opDecValue(ASTValue *a) {
    if(a->getType()->isFloating()) {
        return opSubValues(a, getFloatValue(a->getType(), 1.0f));
    } else {
        return opSubValues(a, getIntValue(a->getType(), 1));
    }
}

//XXX expects a and b are same type and are floating or integer
ASTValue *IRCodegenContext::opEqValue(ASTValue *a, ASTValue *b)
{
    llvm::Value *val = NULL;
    if(a->getType()->isFloating()) {
        val = ir->CreateFCmp(CmpInst::FCMP_OEQ, codegenValue(a), codegenValue(b));
    } else { // sign not required, irrelivant for equality
        val = ir->CreateICmp(CmpInst::ICMP_EQ, codegenValue(a), codegenValue(b));
    }
    return new ASTBasicValue(ASTType::getBoolTy(), val);
}

//XXX expects a and b are same type and are floating or integer
ASTValue *IRCodegenContext::opNEqValue(ASTValue *a, ASTValue *b)
{
    llvm::Value *val = NULL;
    if(a->getType()->isFloating()) {
        val = ir->CreateFCmp(CmpInst::FCMP_ONE, codegenValue(a), codegenValue(b));
    } else { // sign not required, irrelivant for equality
        val = ir->CreateICmp(CmpInst::ICMP_NE, codegenValue(a), codegenValue(b));
    }
    return new ASTBasicValue(ASTType::getBoolTy(), val);
}

//XXX expects a and b are same type and are floating or integer
ASTValue *IRCodegenContext::opLTValue(ASTValue *a, ASTValue *b){ // <
    if(a->getType()->isFloating()) {
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateFCmpOLT(codegenValue(a), codegenValue(b)));
    } else if(a->getType()->isSigned()){
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateICmpSLT(codegenValue(a), codegenValue(b)));
    } else {
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateICmpULT(codegenValue(a), codegenValue(b)));
    }
}

//XXX expects a and b are same type and are floating or integer
ASTValue *IRCodegenContext::opGTValue(ASTValue *a, ASTValue *b){ // >
    if(a->getType()->isFloating()) {
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateFCmpOGT(codegenValue(a), codegenValue(b)));
    } else if(a->getType()->isSigned()){
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateICmpSGT(codegenValue(a), codegenValue(b)));
    } else {
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateICmpUGT(codegenValue(a), codegenValue(b)));
    }
}

//XXX expects a and b are same type and are floating or integer
ASTValue *IRCodegenContext::opLEValue(ASTValue *a, ASTValue *b){ // <=
    if(a->getType()->isFloating()) {
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateFCmpOLE(codegenValue(a), codegenValue(b)));
    } else if(a->getType()->isSigned()){
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateICmpSLE(codegenValue(a), codegenValue(b)));
    } else {
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateICmpULE(codegenValue(a), codegenValue(b)));
    }
}

//XXX expects a and b are same type and are floating or integer
ASTValue *IRCodegenContext::opGEValue(ASTValue *a, ASTValue *b){  // >=
    if(a->getType()->isFloating()) {
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateFCmpOGE(codegenValue(a), codegenValue(b)));
    } else if(a->getType()->isSigned()){
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateICmpSGE(codegenValue(a), codegenValue(b)));
    } else {
        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateICmpUGE(codegenValue(a), codegenValue(b)));
    }
}

ASTValue *IRCodegenContext::opIndexDArray(ASTValue *arr, ASTValue *idx) {
    ASTType *indexedType = arr->getType()->getPointerElementTy();
    Value *val = ir->CreateStructGEP(codegenLValue(arr), 0);
    val = ir->CreateLoad(val);
    val = ir->CreateInBoundsGEP(val, codegenValue(idx));
    val = ir->CreateBitCast(val, codegenType(indexedType->getPointerTy()));
    return new ASTBasicValue(indexedType, val, true);
}

ASTValue *IRCodegenContext::opIndexSArray(ASTValue *arr, ASTValue *idx) {
    ASTType *indexedType = arr->getType()->getPointerElementTy();

    std::vector<Value*> gep;
    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
    gep.push_back(codegenValue(idx));

    Value *val = ir->CreateInBoundsGEP(codegenLValue(arr), gep);
    return new ASTBasicValue(indexedType, val, true);
}

ASTValue *IRCodegenContext::opIndexPointer(ASTValue *ptr, ASTValue *idx) {
    //TODO: index array
    ASTType *indexedType = ptr->getType()->getPointerElementTy();
    Value *val = ir->CreateInBoundsGEP(codegenValue(ptr), codegenValue(idx));
    return new ASTBasicValue(indexedType, val, true);
}

// b must be constant int
ASTValue *IRCodegenContext::opIndexTuple(ASTValue *tup, ASTValue *idx) {
// apple version of LLVM libs does not have typeinfo for ConstantInt
// TODO: find a better way to check for ConstantInt type
#if defined __APPLE__ || defined WIN32
    if(ConstantInt *llci = static_cast<ConstantInt*>(codegenValue(idx)))
#else
    if(ConstantInt *llci = dynamic_cast<ConstantInt*>(codegenValue(idx)))
#endif
    {
        ASTTupleType *tupty = (ASTTupleType*) dynamic_cast<ASTTupleType*>(tup->getType());
        unsigned long long index = llci->getZExtValue();
        if(tupty->types.size() <= index)
        {
            emit_message(msg::ERROR, "invalid tuple index");
            return NULL;
        }

        if(TupleValue *tupval = dynamic_cast<TupleValue*>(tup)) {
            // tuple is an immidiate. [1, 2, 3]
            return tupval->values[index];
        } else {
            // tuple is likely a declaration; "[int, int] myvar"
            ASTType *type = tupty->types[index];
            Value *val = ir->CreateStructGEP(codegenLValue(tup), index);
            return new ASTBasicValue(type, val, true);
            //emit_message(msg::FAILURE, "tuple found that is not tuple value", location);
        }

    } else
    {
        emit_message(msg::ERROR, "tuples can only be indexed with\
                constant integers, due to static typing");
        return NULL;
    }
}

ASTValue *IRCodegenContext::opIndex(ASTValue *a, ASTValue *b) {
    if(a->getType()->getKind() == TYPE_DYNAMIC_ARRAY) {
        return opIndexDArray(a,b);
    } else if(a->getType()->getKind() == TYPE_ARRAY) {
        return opIndexSArray(a,b);
    } else if(a->getType()->getKind() == TYPE_POINTER) {
        return opIndexPointer(a,b);
    } else if(a->getType()->getKind() == TYPE_TUPLE) {
        return opIndexTuple(a,b);
    } else {
        emit_message(msg::ERROR, "attempt to index non-pointer/array type");
        return NULL;
    }
}

ASTValue *IRCodegenContext::opAlloca(ASTType *ty) {
    return new ASTBasicValue(ty, ir->CreateAlloca(codegenType(ty), 0), true);
}

ASTValue *IRCodegenContext::getArrayPointer(ASTValue *arr) {
    ASTArrayType *arrty = dynamic_cast<ASTArrayType*>(arr->getType());
    ASTType *elemty = arrty->getPointerElementTy();
    if(arr->getType()->getKind() == TYPE_DYNAMIC_ARRAY) {
        std::vector<Value*> gep;
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        Value *llval = ir->CreateInBoundsGEP(codegenLValue(arr), gep);
        return new ASTBasicValue(elemty->getPointerTy(), llval, true);
    } else if(arr->getType()->getKind() == TYPE_ARRAY) {
        std::vector<Value*> gep;
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        Value *ptr = ir->CreateInBoundsGEP(codegenLValue(arr), gep);
        return new ASTBasicValue(elemty->getPointerTy(), ptr); //XXX static array ptr is immutable(?)
    } else {
        emit_message(msg::ERROR, "invalid .ptr on non-array type");
        return NULL;
    }
}

ASTValue *IRCodegenContext::getArraySize(ASTValue *arr) {
    if(arr->getType()->getKind() == TYPE_DYNAMIC_ARRAY) {
        std::vector<Value*> gep;
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 1));
        Value *llval = ir->CreateInBoundsGEP(codegenLValue(arr), gep);
        return new ASTBasicValue(ASTType::getULongTy(), llval, true);
    } else if(arr->getType()->getKind() == TYPE_ARRAY) {
        return new ASTBasicValue(ASTType::getULongTy(),
                ConstantInt::get(Type::getInt64Ty(context), arr->getType()->length()));
    } else {
        //TODO: allow getArraySize call on tuple?
        emit_message(msg::ERROR, "invalid .size on non-array type");
    }
    return NULL;
}


// does not create IR Value for codegen (unless global).
// values should be created at declaration
ASTValue *IRCodegenContext::codegenIdentifier(Identifier *id)
{
    if(id->isVariable())
    {
        if(id->getScope()->getModule() != unit->mdecl) // id not declared in current TU because imported
        {
            if(unit->globals.count(id->getMangledName()))
            {
                return unit->globals[id->getMangledName()];
            } else if(id->getDeclaration()->qualifier.isConst) {
                VariableDeclaration *vdecl = (VariableDeclaration*) id->getDeclaration();
                ASTBasicValue *val = dynamic_cast<ASTBasicValue*>(promoteType(codegenExpression(vdecl->value), vdecl->getType()));
                val->setConstant(true);
                return val;
            } else
            {
                GlobalVariable* GV =
                (GlobalVariable*) module->getOrInsertGlobal(id->getMangledName(),
                        codegenType(id->getType()));
                assert(GV);
                IRValue irval = IRValue(new ASTBasicValue(id->getType(), GV, true), GV);
                unit->globals[id->getMangledName()] = irval;
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
                emit_message(msg::ERROR, "CG: identifier not found in scope. did you mean '." + id->getName() + "'?");
            }
        }

        return id->getValue(); // else declared in current TU, so we are good
    } else if(id->isFunction())
    {
        FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(id->getDeclaration());
        if(!fdecl) emit_message(msg::FAILURE, "invalid function identifier");
        id->setValue(new FunctionValue(fdecl));
    } else if(id->isUserType())
    {
        id->setValue(new TypeValue(id->getDeclaredType()));
    } else if(id->isExpression())
    {
        id->setValue(codegenExpression(id->getExpression()));
    } else if(id->isLabel())
    {
        if(!id->getValue()) {
        id->setValue(new ASTBasicValue(NULL, BasicBlock::Create(context,
                        id->getName(), ir->GetInsertBlock()->getParent())));
        }
    }

    return id->getValue();
}

ASTValue *IRCodegenContext::codegenExpression(Expression *exp)
{
    location = exp->loc;
    if(IntExpression *iexp = exp->intExpression())
    {
        llvm::Value *llvmval = NULL;
        ASTType *ty = NULL;
        ASTBasicValue *ret = NULL;

        if(iexp->astType->isPointer())
        {
            if(!iexp->value)
            {
                llvmval = ConstantPointerNull::get((PointerType*) codegenType(iexp->astType));
                ty = iexp->astType;
            }
        } else
        {
            llvmval = ConstantInt::get(codegenType(iexp->astType), iexp->value);
            ty = iexp->astType;
        }
        ret = new ASTBasicValue(ty, llvmval); //TODO: assign
        ret->setConstant(true);
        return ret;
    }
    else if(FloatExpression *fexp = exp->floatExpression()) {
        llvm::Value *llvmval = ConstantFP::get(codegenType(fexp->astType), fexp->value);
        ASTType *ty = fexp->astType;
        ASTBasicValue *ret = new ASTBasicValue(ty, llvmval); //TODO: assign
        ret->setConstant(true);
        return ret;
    }
    else if(StringExpression *sexp = exp->stringExpression())
    {
        return getStringValue(sexp->string); //TODO: const
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
    } else if(exp->importExpression())
    {
        //TODO: should it return something? probably. Some sort of const package ptr or something...
        return NULL;
    } else if(CastExpression *cexp = exp->castExpression())
    {
        return promoteType(cexp->expression->getValue(this), cexp->type, true);
    } else if(TupleExpression *texp = dynamic_cast<TupleExpression*>(exp))
    {
        return codegenTupleExpression(texp);
    } else if(NewExpression *nexp = dynamic_cast<NewExpression*>(exp))
    {
        return codegenNewExpression(nexp);
    } else if(IdOpExpression *dexp = dynamic_cast<IdOpExpression*>(exp))
    {
        return codegenIdOpExpression(dexp);
    } else if(exp->useExpression())
    {
        // XXX do something with the UseExpression? make Use a Statement?
        return NULL;
    } else if(AllocExpression *aexp = exp->allocExpression()) {
        if(HeapAllocExpression *haexp = dynamic_cast<HeapAllocExpression*>(aexp)) {
            return codegenHeapAllocExpression(haexp);
        } else if(StackAllocExpression *saexp = dynamic_cast<StackAllocExpression*>(aexp)) {
            return codegenStackAllocExpression(saexp);
        }
    } else if(FunctionExpression *fexp = dynamic_cast<FunctionExpression*>(exp)) {
        emit_message(msg::WARNING, "CG: FunctionExpression should be handled by call CG");
        if(fexp->fpointer) return fexp->fpointer->getValue(this);
        if(fexp->overload) {
            return new FunctionValue(fexp->overload);
        }
    } else if(DotPtrExpression *dpexp = dynamic_cast<DotPtrExpression*>(exp)) {
        if(dpexp->lhs->getType()->isArray()) {
            return getArrayPointer(codegenExpression(dpexp->lhs));
        } else if(dpexp->lhs->getType()->isInterface()) {
            return getInterfaceSelf(codegenExpression(dpexp->lhs));
        }
    }

    emit_message(msg::FAILURE, "CG: bad expression?", exp->loc);
    return NULL; //TODO
}

ASTValue *IRCodegenContext::codegenTupleExpression(TupleExpression *exp, ASTCompositeType *ty)
{
    // this does not fully codegen the tuple. Tuple constants are implicitly convertible to
    // arrays (static or dynamic), structs, or tuple varibles. as such, it is best to codegen
    // the individual values, hold on to them, and construct the expected type when needed.
    std::vector<ASTValue*> vals;
    for(int i = 0; i < exp->members.size(); i++) {
        vals.push_back(codegenExpression(exp->members[i]));
    }

    return new TupleValue(vals);
}

ASTValue *IRCodegenContext::codegenHeapAlloc(ASTType *ty) {
    Value *asz = NULL; //array size
    Value *size = NULL; //size in bytes (amount to alloc)

    //XXX a bit messy here
    // manually calculate array size
    // TODO: only manually calculate size if exp->type->size is not constant
    if(ty->isArray()) {
        ASTStaticArrayType *arrayTy = dynamic_cast<ASTStaticArrayType*>(ty);
        ASTValue *sz = promoteType(codegenExpression(arrayTy->size), ASTType::getLongTy());
        asz = codegenValue(sz);
        sz = opMulValues(sz, getIntValue(ASTType::getLongTy(), arrayTy->arrayOf->getSize()));
        size = codegenValue(sz);
    } else {
        asz = ConstantInt::get(codegenType(ASTType::getULongTy()), ty->length());
        size = ConstantInt::get(codegenType(ASTType::getULongTy()), ty->getSize());
    }

    vector<Value*> llargs;
    llargs.push_back(size);
    Value *value = ir->CreateCall(getLLVMMalloc(), llargs);

    ASTValue *val = NULL;
    if(ty->isArray()) {
        ASTType *arrty = ty->getPointerElementTy();
        ty = arrty->getArrayTy();
        Value *ptr = ir->CreateBitCast(value, codegenType(arrty)->getPointerTo());
        value = ir->CreateAlloca(codegenType(ty));
        ir->CreateStore(ptr, ir->CreateStructGEP(value, 0));
        ir->CreateStore(asz, ir->CreateStructGEP(value, 1));
        //TODO: create a 'create array' function
    } else {
        if(ty->isReference()) {
            value = ir->CreateBitCast(value, codegenType(ty));
        } else {
            value = ir->CreateBitCast(value, codegenType(ty)->getPointerTo());
        }
    }

    if(ty->isReference()){
        val = new ASTBasicValue(ty, value, false, true);
    } else if(ty->isArray()) {
        val = new ASTBasicValue(ty, value, true);
    } else {
        val = new ASTBasicValue(ty, value, false, true);
    }

    return val;
}

ASTValue *IRCodegenContext::codegenStackAlloc(ASTType *ty) {
// XXX deal with arrays and weird other types?
    ASTValue *ret = NULL;
    ASTUserType *uty = ty->asUserType();
    Value *alloc = ir->CreateAlloca(codegenType(uty));

    if(uty->isReference()) {
        // allocate room for value on stack store in reference
        Value *stackAlloca = ir->CreateAlloca(codegenType(uty)->getPointerElementType());
        if(uty->isClass()) ir->CreateStore(ConstantAggregateZero::get(codegenType(uty)->getPointerElementType()), stackAlloca);
        ir->CreateStore(stackAlloca, alloc);
    }

    ret = new ASTBasicValue(uty, alloc, true, uty->isReference());
    ((ASTBasicValue*) ret)->setNoFree(true);

    return ret;
}

ASTValue *IRCodegenContext::codegenHeapAllocExpression(HeapAllocExpression *aexp) {
    return codegenHeapAlloc(aexp->type);
}

ASTValue *IRCodegenContext::codegenStackAllocExpression(StackAllocExpression *aexp) {
    return codegenStackAlloc(aexp->type);
}

ASTValue *IRCodegenContext::codegenNewExpression(NewExpression *exp)
{
    if(exp->type->getKind() == TYPE_DYNAMIC_ARRAY) {
        emit_message(msg::ERROR, "CODEGEN: cannot created unsized array. meaningless alloaction", exp->loc);
        return NULL;
    }

    ASTValue *ret = NULL;
    ASTValue *this_val = NULL;

    if(exp->alloc == NewExpression::STACK) {
        ret = codegenStackAlloc(exp->type);
        this_val = getAddressOf(ret);
    } else { // heap alloc
        ret = codegenHeapAlloc(exp->type);
        this_val = ret;
    }

    ASTType *ty = exp->type;
    if(ty->isPointer() && ty->getPointerElementTy()->isStruct()) {
        ty = ty->getPointerElementTy();
    }

    if(ty->isUserType()) {
         // no default value, and allocated class. set VTable, in case
        ASTUserType *uty = ty->asUserType();

        // if class, store vtable and refcount
        if(ty->isClass()) {
            ir->CreateStore(ConstantAggregateZero::get(codegenType(uty)->getPointerElementType()), codegenValue(this_val));
            ASTValue *vtable = getVTable(this_val);

            codegenType(uty); // XXX to create typeinfo. So we are able to load it below... hacky

            //the pass through identifier is an ugly hack to get typeinfo in case of duplicate ASTType types, eww
            Value *tival = codegenLValue(uty->getDeclaration()->classDeclaration()->typeinfo);

            // store vtable
            vector<Value *> gep;
            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
            tival = ir->CreateGEP(tival, gep); // GEP to class's vtable, so we can store to it

            ir->CreateStore(tival, codegenLValue(vtable));
        }

        //TODO: call parent class constructor
        if(exp->function) {
            std::vector<ASTValue*> args;
            args.push_back(this_val); // push 'this'
            std::list<Expression*>::iterator it = exp->args.begin();

            while(it != exp->args.end()) {
                args.push_back(codegenExpression(*it));
                it++;
            }

            if(FunctionExpression *fexp = exp->function->functionExpression()) {
                if(!fexp->overload) {
                    emit_message(msg::ERROR, "CG: invalid function in 'new' expression (fpointer?)");
                }

            codegenCall(new FunctionValue(fexp->overload), args);
            } else {
                emit_message(msg::ERROR, "CG: invalid function in 'new' expression");
            }
        }
    }

    return ret;
}


void IRCodegenContext::codegenDelete(ASTValue *val) {
    vector<Value*> llargs;
    vector<Type*> llargty;
    if(val->getType()->isArray() && val->getType()->getKind() == TYPE_DYNAMIC_ARRAY)
    {
        //TODO: duplicate of ".ptr". make a function for this?
        std::vector<Value*> gep;
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        Value *llval = ir->CreateInBoundsGEP(codegenLValue(val), gep);
        val = new ASTBasicValue(val->getType()->getPointerElementTy()->getPointerTy(), llval, true);
    }

    if(val->getType()->isUserType()) {
        ASTUserType *uty = val->getType()->asUserType();
        do {
            //TODO: make destructor virtual
            //TODO: make destructor for struct type work
            FunctionDeclaration *dtor = uty->getDestructor();
            if(dtor) {
                std::vector<ASTValue*> args;
                ASTValue *func = codegenIdentifier(dtor->identifier);
                args.push_back(promoteType(val, uty));
                codegenCall(func, args);
            }
        } while((uty = dynamic_cast<ASTUserType*>(uty->getBaseType())));
    }

    // only call free if object was not stack allocated
    if(!val->isNoFree()) {
        ASTValue *charval = promoteType(val, ASTType::getCharTy()->getPointerTy());
        llargs.push_back(codegenValue(charval));
        llargty.push_back(codegenType(ASTType::getVoidTy()->getPointerTy()));

        FunctionType *fty = FunctionType::get(codegenType(ASTType::getVoidTy()), llargty, false);
        Function *freeFunc = (Function*) module->getOrInsertFunction("free", fty);

        ir->CreateCall(freeFunc, llargs);
    }

    storeValue(val, new ASTBasicValue(val->getType(),
                ConstantPointerNull::get((llvm::PointerType*) codegenType(val->getType()))));
}

ASTValue *IRCodegenContext::codegenIdOpExpression(IdOpExpression *exp)
{
    ASTValue *val = codegenExpression(exp->expression);
    if(exp->isDelete()) {
        codegenDelete(val);
    } else if(exp->isRetain()) {
        retainObject(val);
    } else if(exp->isRelease()) {
        releaseObject(val);
    }

    //XXX return a value?
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
    if(!isTerminated()) // ifstmt body can end in unconditional return; if not, create Branch to endif
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
        ASTValue *caseValue = promoteType(cs->irCase, cond->getType());
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
        emit_message(msg::ERROR, "CG: invalid value used in function call context");
        if(func->getType()){
            emit_message(msg::ERROR, "CG: value is of type \"" +
                    func->getType()->getName() + "\"");
        }
        return NULL;
    }

    ASTType *rtype = astfty->getReturnType();

    for(int i = 0; i < args.size(); i++) {
        llargs.push_back(codegenValue(args[i]));
    }

    llvm::Value *value = ir->CreateCall(codegenValue(func), llargs);
    return new ASTBasicValue(rtype, value, false, rtype->isReference());
}

ASTValue *IRCodegenContext::codegenCallExpression(CallExpression *exp)
{
    ASTValue *ret = NULL;
    std::vector<ASTValue *> args; //provided arguments

    if(!exp->resolvedFunction) {
        emit_message(msg::ERROR, "CG: unknown expression used as function", exp->loc);
        return NULL;
    }

    std::list<Expression*>::iterator it = exp->args.begin();
    while(it != exp->args.end()) {
        ASTValue *argval = codegenExpression(*it);
        if(!argval) emit_message(msg::ERROR, "CG: null value for argument", exp->loc);
        args.push_back(argval);
        it++;
    }

    ASTValue *func = NULL;
    if(exp->resolvedFunction->fpointer) {
        func = codegenExpression(exp->resolvedFunction->fpointer);
    } else {
        assert(exp->resolvedFunction->overload);

        FunctionDeclaration *oload = exp->resolvedFunction->overload;

        // if function is virtual, we need to load the proper value from the vtable
        // of the first argument ('this')
        if(oload->isVirtual()) {
            ASTValue *vtable = NULL;
            ASTValue *self = NULL;

            // if we are calling an interface method, pass interface *this* instead of interface
            if(oload->owner->isInterface()) {
                self = getInterfaceSelf(args[0]);
                vtable = getInterfaceVTable(args[0]);
            } else {
                self = args[0];
                vtable = getVTable(args[0]);
            }

            args[0] = self;

            std::vector<Value *> gep;
            if(oload->getVTableIndex() == -1)
                emit_message(msg::FAILURE, "CG: invalid vtable index (-1)");
            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), oload->getVTableIndex()));
            Value *llval = ir->CreateLoad(ir->CreateGEP(codegenValue(vtable), gep));
            func = new ASTBasicValue(oload->getType(), ir->CreatePointerCast(llval, codegenType(oload->getType())->getPointerTo()));
        } else {
            func = new FunctionValue(oload);
        }
    }

    //XXX should not be needed? should be dereferenced in validate?
    if(func->getType()->isPointer()){ // dereference function pointer
        emit_message(msg::WARNING, "CG: function pointer should be dereferenced in validate?", exp->loc);
        func = getValueOf(func, false);
    }

    if(!func) {
        emit_message(msg::ERROR, "CG: no valid function overload found for call", exp->loc);
        return NULL;
    }

    //XXX messy. Steal first argument as return, because this is a
    // stack constructor (eg MyStruct st = MyStruct(1, 2, 3))
    if(exp->isConstructor) {
        ret = args[0];
        codegenCall(func, args);
    } else {
        ret = codegenCall(func, args);
        ((ASTBasicValue*) ret)->setNoFree(true);
    }
    return ret;
}

ASTValue *IRCodegenContext::codegenUnaryExpression(UnaryExpression *exp)
{
    ASTValue *lhs = exp->lhs->getValue(this); // expression after unary op: eg in !a, lhs=a

    ASTValue *val;
    switch(exp->op) {
        case tok::plusplus:
            if(!lhs->isLValue()) {
                emit_message(msg::ERROR, "CODEGEN: can only incrememt LValue", exp->loc);
                return NULL;
            }

            val = opIncValue(lhs);
            storeValue(lhs, val);
            return val;
        case tok::minusminus:
            if(!lhs->isLValue()) {
                emit_message(msg::ERROR, "CODEGEN: can only decrement LValue", exp->loc);
                return NULL;
            }

            val = opDecValue(lhs);
            storeValue(lhs, val);
            return val;
        case tok::plus:
            return lhs;
        case tok::minus:
            if(lhs->getType()->isFloating()) {
                return new ASTBasicValue(lhs->getType(), ir->CreateFNeg(codegenValue(lhs)));
            }
            if(!lhs->getType()->isSigned()){
                emit_message(msg::UNIMPLEMENTED,
                        "CODEGEN: conversion to signed value using '-' unary op",
                        exp->loc);
                return NULL;
            }
            val = new ASTBasicValue(lhs->getType(), ir->CreateNeg(codegenValue(lhs)));
            return val;
        case tok::tilde:
            emit_message(msg::UNIMPLEMENTED, "CODEGEN: unimplemented unary codegen (~)", exp->loc);
            return NULL;
        case tok::bang:
            val = promoteType(lhs, ASTType::getBoolTy());
            return new ASTBasicValue(ASTType::getBoolTy(), ir->CreateNot(codegenValue(val)));
            return val;
        case tok::caret:
            if(!lhs->getType()->isPointer()) {
                emit_message(msg::ERROR, "CODEGEN: attempt to dereference non-pointer type", exp->loc);
                return NULL;
            }
            return getValueOf(lhs);
        case tok::amp:
            if(!lhs->isLValue()){
                emit_message(msg::ERROR, "CODEGEN: attempt to take reference of non-LValue", exp->loc);
                return NULL;
            }
            return getAddressOf(lhs);
        default:
            emit_message(msg::UNIMPLEMENTED, "CODEGEN: unimplemented unary codegen", exp->loc);
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
        ASTValue *arr = iexp->lhs->getValue(this);
        ASTValue *ind = iexp->index->getValue(this);
        return opIndex(arr, ind);
    } else if(PostfixOpExpression *e = dynamic_cast<PostfixOpExpression*>(exp))
    {
        ASTValue *old;
        ASTValue *val;
        ASTValue *lhs = e->lhs->getValue(this);

        switch(e->op)
        {
            case tok::plusplus:
            old = loadValue(lhs);
            val = opIncValue(lhs);
            old = loadValue(lhs);
            storeValue(lhs, val);
            return old;
            case tok::minusminus:
            old = loadValue(lhs);
            val = opDecValue(lhs);
            storeValue(lhs, val);
            return old;
        }
    } else if(DotExpression *dexp = dynamic_cast<DotExpression*>(exp))
    {
            ASTValue *lhs;

            // if left hand side represents a type
            if(ASTType *declty = dexp->lhs->getDeclaredType())
            {
                //TODO: this should be lowered before CG
                emit_message(msg::ERROR, "CG: unknown attribute '" + dexp->rhs + "' of struct '" +
                        declty->getName() + "'", dexp->loc);
                return NULL;
            }

            lhs = dexp->lhs->getValue(this);

            if(lhs->getType()->isPointer()) {
                lhs = getValueOf(lhs);
                //TODO: 'this' for structs is a pointer; handle case
                //emit_message(msg::ERROR, "CODEGEN: invalid dot on pointer type", location);
            }

            //TODO: allow indexing types other than userType and array?
            if(!lhs->getType()->asUserType() && !lhs->getType()->isArray()) {
                emit_message(msg::ERROR, "CG: dot expression only applicable to userType or array type", dexp->loc);
                return NULL;
            }

            if(ASTUserType *userty = lhs->getType()->asUserType()) {
                Identifier *id = userty->getDeclaration()->lookup(dexp->rhs);
                ASTValue *ret = NULL;

                if(!id || id->isUndeclared()) {
                    // if not in user type, search current scope
                    // this allows for uniform function syntax
                    id = getScope()->lookup(dexp->rhs);
                    if(!id || id->isUndeclared()){
                        emit_message(msg::ERROR, "CG: no member found in user type", dexp->loc);
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
                    emit_message(msg::ERROR, "CG: invalid identifier type in user type", dexp->loc);
                }
                return ret;
            }

            else if(lhs->getType()->getKind() == TYPE_DYNAMIC_ARRAY ||
                    lhs->getType()->getKind() == TYPE_ARRAY)
            {
                ASTArrayType *arrty = dynamic_cast<ASTArrayType*>(lhs->getType());
                if(!arrty){
                    emit_message(msg::FATAL, "dot exp on array, not actually an array?", dexp->loc);
                    return NULL;
                }

                //XXX REMOVE
                if(dexp->rhs == "ptr")
                {
                    emit_message(msg::FAILURE, "CG: .ptr should be codegen'd elsewhere");
                    return getArrayPointer(lhs);
                }

                if(dexp->rhs == "size")
                {
                    return getArraySize(lhs);
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
    ASTBasicValue *ret = NULL;
    if(toType->isBool())
    {
        ASTBasicValue zero(val->getType(), ConstantInt::get(codegenType(val->getType()), 0));
        ret = new ASTBasicValue(ASTType::getBoolTy(), ir->CreateICmpNE(codegenValue(val),
                    codegenValue(&zero)));
    } else if(toType->isInteger())
    {
        ret = new ASTBasicValue(toType, ir->CreateIntCast(codegenValue(val),
                    codegenType(toType), val->getType()->isSigned()));
    } else if(toType->isPointer())
    {
        ret = new ASTBasicValue(toType, ir->CreateIntToPtr(codegenValue(val), codegenType(toType)));
    } else if(toType->isFloating())
    {
        if(val->getType()->isSigned()) {
            ret = new ASTBasicValue(toType, ir->CreateSIToFP(codegenValue(val),
                        codegenType(toType)), false);
        } else {
            ret = new ASTBasicValue(toType, ir->CreateUIToFP(codegenValue(val),
                        codegenType(toType)), false);
        }
    }
    if(val->isConstant()) ret->setConstant(true);
    return ret;
}

ASTValue *IRCodegenContext::promoteFloat(ASTValue *val, ASTType *toType) {
    if(toType->isFloating())
    {
        return new ASTBasicValue(toType, ir->CreateFPCast(codegenValue(val), codegenType(toType)));
    } else if(toType->isInteger())
    {
        return new ASTBasicValue(toType, ir->CreateFPToUI(codegenValue(val),
                    codegenType(toType)));
    }
    return NULL;
}

ASTValue *IRCodegenContext::promotePointer(ASTValue *val, ASTType *toType) {
    if(toType->isPointer())
    {
        return new ASTBasicValue(toType,
                ir->CreatePointerCast(codegenValue(val), codegenType(toType)));
    }

    if(toType->isBool())
    {
        Value *i = ir->CreatePtrToInt(codegenValue(val),
                codegenType(ASTType::getULongTy()));

        return new ASTBasicValue(ASTType::getBoolTy(),
                ir->CreateICmpNE(i,
                    ConstantInt::get(codegenType(ASTType::getULongTy()), 0)
                    ));
    }

    if(toType->isInteger())
    {
        return new ASTBasicValue(toType, ir->CreateIntCast(codegenValue(val),
                    codegenType(toType), false));
    }

    if(toType->isReference()) {
        return new ASTBasicValue(toType,
                ir->CreatePointerCast(codegenValue(val), codegenType(toType)), false, true); //XXX might be a bit cludgy
    }
    return NULL;
}

ASTValue *IRCodegenContext::promoteTuple(ASTValue *val, ASTType *toType) {
    TupleValue *tupval = dynamic_cast<TupleValue*>(val);
    ASTCompositeType *compty = dynamic_cast<ASTCompositeType*>(toType);
    if(!tupval) emit_message(msg::FAILURE, "promoting invalid value to tuple");
    if(!compty) emit_message(msg::FAILURE, "promoting tuple to invalid type");
    tupval->setType(compty);
    return tupval;
}

ASTValue *IRCodegenContext::promoteArray(ASTValue *val, ASTType *toType) {
    if(val->getType()->getKind() == TYPE_ARRAY)
    {
        ASTStaticArrayType *sarrty = val->getType()->asSArray();
        if(toType->getKind() == TYPE_POINTER)
        {
            if(sarrty->arrayOf != toType->getPointerElementTy())
            {
                emit_message(msg::ERROR, "invalid convesion from array to invalid pointer type");
                return NULL;
            }
            Value *ptr = codegenLValue(val);

            ASTBasicValue *ret = NULL;
            if(val->isConstant()) {
                //XXX make sure that ptr is constant
                ptr = ConstantExpr::getPointerCast((Constant*) ptr, codegenType(toType));
                ret = new ASTBasicValue(toType, ptr, false); //TODO: should be lvalue, but that causes it to load incorrectly
                ret->setConstant(true);
            } else {
                ptr = ir->CreateBitCast(ptr, codegenType(toType));
                ret = new ASTBasicValue(toType, ptr, false); //TODO: should be lvalue, but that causes it to load incorrectly
            }
            return ret;
        } else if(toType->getKind() == TYPE_DYNAMIC_ARRAY) {
            ASTArrayType *arrty = (ASTArrayType*) val->getType();
            if(arrty->arrayOf != toType->getPointerElementTy())
            {
                emit_message(msg::ERROR, "CG: invalid convesion from array to incompatible array type");
                return NULL;
            }

            // XXX will this work on non-constants? probably not...
            Constant *ptr = (Constant*) codegenValue(getArrayPointer(val));
            //Constant *ptr = ConstantExpr::getInBoundsGetElementPtr((Constant*) codegenLValue(val), ConstantInt::get(Type::getInt32Ty(context), 0));
            Constant *sz = ConstantInt::get(codegenType(ASTType::getLongTy()), sarrty->length());

            std::vector<Constant*> sarrMembers;
            sarrMembers.push_back(ptr);
            sarrMembers.push_back(sz);

            Constant *toStruct = ConstantStruct::get((StructType*) codegenType(toType), sarrMembers);

            ASTBasicValue *ret = new ASTBasicValue(toType, toStruct); //TODO: should be lvalue, but that causes it to load incorrectly
            ret->setConstant(true);
            return ret;
        }
    } else if(val->getType()->getKind() == TYPE_DYNAMIC_ARRAY)
    {
        if(toType->getKind() == TYPE_POINTER) {
            return indexValue(val, 0);
        }
        return NULL;
    }

    return val; //TODO
}


ASTValue *IRCodegenContext::promoteType(ASTValue *val, ASTType *toType, bool isExplicit)
{
    ASTValue *ret = NULL;
    if(val->getType()->is(toType)) {
        ret = val;
    } else if(val->getType()->isFunctionType()) {
        if(toType->isPointer()) {
            ret = val; //XXX TODO: make sure target is function pointer type
        }
    } else {
        //convert reference value to non-reference type
        //currently, reference types are simply pointers, so just turn it into an LValue and we're set

        //TODO: if val->dereferencedTy == toType
        if(val->isReference()) {
                //(toType->getReferenceTy()->is(val->getType()))) {
                 //getValueOf(val)->getType()->is(toType))) {
            if(toType->getReferenceTy()->is(val->getType())) {
                ret = new ASTBasicValue(toType, codegenLValue(val), true, false);
            } else if(val->getType()->getPointerTy()->is(toType)) {
                ret = new ASTBasicValue(toType, codegenLValue(val), false, false);
            }
        }

        if(!ret) {
            if(val->getType()->isInteger()) {
                ret = promoteInt(val, toType);
            }
            else if(val->getType()->isFloating()) {
                ret = promoteFloat(val, toType);
            }
            else if(val->getType()->isPointer()) {
                ret = promotePointer(val, toType);
            }
            else if(val->getType()->getKind() == TYPE_TUPLE) {
                ret = promoteTuple(val, toType);
            } else if(val->getType()->isArray()) {
                ret = promoteArray(val, toType);
            } else if(val->getType()->isReference()) {
                if(toType->isPointer()) {
                    ret = new ASTBasicValue(toType,
                            ir->CreatePointerCast(codegenLValue(val),
                                codegenType(toType)));
                }  else if(toType->isBool()) {
                    // class/reference to boolean
                    Value *i = ir->CreatePtrToInt(codegenLValue(val),
                            codegenType(ASTType::getULongTy()));

                    return new ASTBasicValue(ASTType::getBoolTy(),
                            ir->CreateICmpNE(i,
                                ConstantInt::get(codegenType(ASTType::getULongTy()), 0)
                                ));
                }else if(val->getType()->extends(toType)) {
                    ret = new ASTBasicValue(toType,
                            ir->CreatePointerCast(codegenLValue(val),
                                codegenType(toType)), false, true);
                } else if(isExplicit && toType->isClass()) {
                    //TODO: make dynamic cast instead of reinterpret cast
                    ret = new ASTBasicValue(toType,
                            ir->CreatePointerCast(codegenLValue(val),
                                codegenType(toType)), false, true);
                } else if(val->getType()->isClass() && toType->isInterface()) {
                    Value *ptr = ir->CreatePointerCast(codegenValue(val), codegenType(ASTType::getVoidTy()->getPointerTy()));
                    InterfaceDeclaration *idecl = toType->getDeclaration()->interfaceDeclaration();
                    InterfaceVTable *ivt = idecl->getVTable(val->getType()->getMangledName());
                    ASTValue *vtable = codegenInterfaceVTable(ivt);

                    Value *interface = ir->CreateAlloca(codegenType(toType));

                    ir->CreateStore(ptr, ir->CreateStructGEP(interface, 0));
                    ir->CreateStore(ir->CreatePointerCast(codegenLValue(vtable),
                                codegenType(ASTType::getVoidFunctionTy()->getPointerTy()->getPointerTy())),
                            ir->CreateStructGEP(interface, 1));

                    ret = new ASTBasicValue(toType, interface, true);
                }
            } else if(val->getType()->isInterface()) {
                return val; //XXX CHEATING HERE
            }
        }
    }

 // no conversion? failed converson?
    if(!ret || !ret->getType()) {
        char err[1024]; //XXX may overflow if type names are too long
        sprintf(err, "CODEGEN: cannot convert value of type '%s' to type '%s'", val->getType()->getName().c_str(), toType->getName().c_str());
        emit_message(msg::ERROR, std::string(err), location);
    }

    return ret;
}

//XXX is op required?
//XXX The way i do this with pointers is kinda silly, also, leaky?
void IRCodegenContext::codegenResolveBinaryTypes(ASTValue **v1, ASTValue **v2, unsigned op)
{
    if((*v1)->getType() != (*v2)->getType())
    {
        if((*v1)->getType()->isStruct() || (*v2)->getType()->isStruct())
        {
            // TODO: loc
            emit_message(msg::UNIMPLEMENTED, "cannot convert structs (yet)", location);
        }
        if((*v2)->getType()->getPriority() > (*v1)->getType()->getPriority())
            *v1 = promoteType(*v1, (*v2)->getType());
        else
            *v2 = promoteType(*v2, (*v1)->getType());
    }
}

ASTValue *IRCodegenContext::codegenAssign(ASTValue *lhs, ASTValue *rhs, bool convert)
{
    // codegen tuple assignment
    if(TupleValue *tlhs = dynamic_cast<TupleValue*>(lhs))
    {
        if(TupleValue *trhs = dynamic_cast<TupleValue*>(rhs))
        {
            if(tlhs->values.size() > trhs->values.size())
            {
                emit_message(msg::ERROR, "tuple assignment requires compatible tuple types", location);
                return NULL;
            }
            for(int i = 0; i < tlhs->values.size(); i++)
            {
                codegenAssign(tlhs->values[i], trhs->values[i]);
            }
        } else {
            //TODO: messy; this is here because a function call returns a BasicValue and not a TupleValue
            if(rhs->getType()->isTuple()) {
                for(int i = 0; i < tlhs->values.size(); i++)
                {
                    codegenAssign(tlhs->values[i], indexValue(rhs, i));
                }
            } else {
                emit_message(msg::ERROR, "tuple assignment requires a tuple on rhs", location);
                return NULL;
            }
        }
        return rhs;
    }

    if(lhs->isConstant()) {
        emit_message(msg::ERROR, "CG: cannot assign to constant value", location);
        return NULL;
    }

    if(!lhs->isWeak()) {
        if(lhs->getType()->isReleasable()) {
            releaseObject(lhs);
        }

        // check for LValue is because we can't retain 'null'
        if(rhs->getType()->isRetainable()) {
            retainObject(rhs);
        }
    }

    storeValue(lhs, rhs);
    return rhs;
}

ASTValue *IRCodegenContext::codegenBinaryExpression(BinaryExpression *exp)
{
    ASTValue *lhs = NULL;
    ASTValue *rhs = NULL;

    //TODO: messy. this is here to avoid codegenExpression below (duplicating LHS, RHS values)
    if(exp->op.kind == tok::kw_or || exp->op.kind == tok::barbar) {
        return opLOrValue(exp->lhs, exp->rhs);
    }

    //XXX same as above
    if(exp->op.kind == tok::kw_and || exp->op.kind == tok::ampamp) {
        return opLAndValue(exp->lhs, exp->rhs);
    }

    lhs = exp->lhs->getValue(this);
    rhs = exp->rhs->getValue(this);
    //XXX temp. shortcut to allow LValue tuples
    if(exp->op.kind == tok::equal || exp->op.kind == tok::colonequal)
            return codegenAssign(lhs, rhs, exp->op.kind == tok::colonequal);


    if(!lhs || !rhs){
        emit_message(msg::FAILURE, "could not codegen expression in binary op", exp->loc);
        return NULL;
    }

    if(exp->op.isCompoundAssignOp()) {
        emit_message(msg::ERROR, "compound assign should be lowered", exp->loc);
    }

    if(!exp->op.isAssignOp() && !exp->op.isLogicalOp()){ //XXX messy
        codegenResolveBinaryTypes(&lhs, &rhs, exp->op.kind);
    }
    else if(lhs->getType()->isSArray())
    {
        emit_message(msg::ERROR, "cannot assign to statically defined array", exp->loc);
    }

    llvm::Value *val;
    ASTValue *retValue = NULL;
#define lhs_val codegenValue(lhs)
#define rhs_val codegenValue(rhs)

    ASTType *TYPE = lhs->getType();
    switch(exp->op.kind)
    {
        //ASSIGN
        case tok::equal:
        case tok::colonequal:
            return codegenAssign(lhs, rhs, exp->op.kind == tok::colonequal);

        // I dont know, do something with a comma eventually
        case tok::comma:
                emit_message(msg::UNIMPLEMENTED, "comma operator not yet implemented", exp->loc);
                return NULL;

        // LOGIC OPS
        case tok::barbar:
        case tok::kw_or:
            return opLOrValue(exp->lhs, exp->rhs); //XXX not used right now, bypassed near top of function (before expression codegen)
        case tok::ampamp:
        case tok::kw_and:
            return opLAndValue(exp->lhs, exp->rhs); //XXX ditto

        // BITWISE OPS
        case tok::bar:
            val = ir->CreateOr(codegenValue(lhs), codegenValue(rhs));
            retValue = new ASTBasicValue(TYPE, val);
            break;

        case tok::caret:
            val = ir->CreateXor(codegenValue(lhs), codegenValue(rhs));
            retValue = new ASTBasicValue(TYPE, val);
            break;

        case tok::amp:
            val = ir->CreateAnd(codegenValue(lhs), codegenValue(rhs));
            retValue = new ASTBasicValue(TYPE, val);
            break;

        // COMPARE OPS
        case tok::equalequal:
                return opEqValue(lhs, rhs);
        case tok::bangequal:
                return opNEqValue(lhs, rhs);
        case tok::less:
                return opLTValue(lhs, rhs);
        case tok::lessequal:
                return opLEValue(lhs, rhs);
        case tok::greater:
                return opGTValue(lhs, rhs);
        case tok::greaterequal:
                return opGEValue(lhs, rhs);
        // ARITHMETIC OPS
        case tok::plus:
            retValue = opAddValues(lhs, rhs);
            break;

        case tok::minus:
            retValue = opSubValues(lhs, rhs);
            break;

        case tok::star:
            retValue = opMulValues(lhs, rhs);
            break;

        case tok::slash:
            retValue = opDivValues(lhs, rhs);
            break;

        case tok::percent:
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

    return retValue;

#undef lhs_val
#undef rhs_val
}

void IRCodegenContext::codegenReturnStatement(ReturnStatement *exp)
{
    if(exp->expression) {
        ASTValue *value = exp->expression->getValue(this);
        retainObject(value); // retain return value
        ASTValue *v = promoteType(value, currentFunction.retVal->getType());
        storeValue(currentFunction.retVal, v);
    }

    endScope();
    ir->CreateBr(currentFunction.exit);
    setTerminated(true);
}

void IRCodegenContext::codegenStatement(Statement *stmt)
{
    if(!stmt) {
        emit_message(msg::WARNING, "null statement found in codegen", location);
        return;
    }

    location = stmt->loc;
    dwarfStopPoint(stmt->loc);
    if(Expression *estmt = dynamic_cast<Expression*>(stmt))
    {
        estmt->getValue(this);
    } else if (Declaration *dstmt = dynamic_cast<Declaration*>(stmt))
    {
        codegenDeclaration(dstmt);
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
            ASTValue *val = value->getValue(this);
            if(!val->isConstant()) {
                emit_message(msg::ERROR, "case value must be a constant (err, found in codegen)", cstmt->loc);
            }
            getScope()->addCase(value, val, caseBB);
        }

    } else if(LabelStatement *lstmt = dynamic_cast<LabelStatement*>(stmt))
    {
        ASTValue *lbl = codegenIdentifier(lstmt->identifier);
        llvm::BasicBlock *BB = (llvm::BasicBlock*) lbl->value;
        if(!isTerminated())
            ir->CreateBr(BB);
        setTerminated(false);
        ir->SetInsertPoint(BB);
    } else if(GotoStatement *gstmt = dynamic_cast<GotoStatement*>(stmt))
    {
        ASTValue *lbl = codegenIdentifier(gstmt->identifier);
        llvm::BasicBlock *BB = (llvm::BasicBlock*) lbl->value;
        ir->CreateBr(BB);
       // post GOTO block
        //BasicBlock *PG = BasicBlock::Create(context, "", ir->GetInsertBlock()->getParent());
        //ir->SetInsertPoint(PG);
        //TODO: end scope?
        setTerminated(true);
    } else if(dynamic_cast<BreakStatement*>(stmt))
    {
        //XXX do something with BreakStatement?
        BasicBlock *br = getScope()->getBreak();
        if(!br){
            emit_message(msg::ERROR, "break doesnt make sense here!", stmt->loc);
            return;
        }
        ir->CreateBr(br);
        ir->SetInsertPoint(BasicBlock::Create(context, "", ir->GetInsertBlock()->getParent()));
    } else if(dynamic_cast<ContinueStatement*>(stmt))
    {
        //XXX do something with continueStatement?
        BasicBlock *cont = getScope()->getContinue();
        if(!cont){
            emit_message(msg::ERROR, "continue doesnt make sense here!", stmt->loc);
            return;
        }

        ir->CreateBr(cont);
        ir->SetInsertPoint(BasicBlock::Create(context, "", ir->GetInsertBlock()->getParent()));
    } else if(CompoundStatement *cstmt = stmt->compoundStatement())
    {
        enterScope(new IRScope(cstmt->getScope(), unit->debug->createScope(getScope()->debug, cstmt->loc)));
        for(int i = 0; i < cstmt->statements.size(); i++)
        {
            if(!isTerminated() || (dynamic_cast<LabelStatement*>(cstmt->statements[i]) ||
                              dynamic_cast<CaseStatement*>(cstmt->statements[i])))
                codegenStatement(cstmt->statements[i]);
        }
        exitScope();
    } else if(BlockStatement *bstmt = stmt->blockStatement())
    {
        enterScope(new IRScope(bstmt->scope, unit->debug->createScope(getScope()->debug, bstmt->loc)));
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
        exitScope();
    } else emit_message(msg::FAILURE, "codegen doesn't know what kind of statement this is", stmt->loc);
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
            defaultValue = codegenTupleExpression(texp, dynamic_cast<ASTCompositeType*>(vty));
        } else { // not composite vty
            defaultValue = vdecl->value->getValue(this);
        }
    }

    //XXX remove block
    if(vty->getKind() == TYPE_DYNAMIC) {
        emit_message(msg::WARNING, "dynamic type found in codegen", vdecl->loc);
        if(!defaultValue) {
            emit_message(msg::FAILURE,
                    "failure to codegen dynamic 'var' type default expression", vdecl->loc);
        }

        vty = defaultValue->getType();
        vdecl->type = vty;
    }

    ///////////////
    //XXX work around for undeclared struct
    Identifier *id = NULL;
    ASTUserType *userty = vty->asUserType();
    if(userty && vty->isUnknown())
    {
        emit_message(msg::WARNING, "unknown type should be resolved in codegen" + id->getName(), vdecl->loc);
        if(userty->identifier->isUndeclared())
        {
            id = lookup(userty->identifier->getName());
            if(id->isUndeclared()) {
                emit_message(msg::ERROR, "undeclared struct: " + id->getName(), vdecl->loc);
                return;
            }
            vty = id->getDeclaredType();
        }
    }
    ///////////////

    ASTBasicValue *idValue = NULL;
    if(vdecl->qualifier.isConst) {
        defaultValue = promoteType(defaultValue, vty);
        idValue = new ASTBasicValue(vty, codegenValue(defaultValue), false);
        idValue->setConstant(true);
    } else if(vdecl->isStatic()) {
        // create as global
        idValue = (ASTBasicValue*) codegenGlobal(vdecl);
    } else {

        Type *llty = codegenType(vty);

        AllocaInst *llvmDecl = ir->CreateAlloca(llty, 0, vdecl->getName());
        llvmDecl->setAlignment(8);

        Value *llval = llvmDecl;

        idValue = new ASTBasicValue(vty, llval, true, vty->isReference());
        idValue->setWeak(vdecl->isWeak());

        if(defaultValue) {
            if(defaultValue->isConstant()) {
                int len = defaultValue->getType()->length();
                if(idValue->getType()->isArray()) { //XXX bit of a hack to prevent modification of constant arrays
                    if(idValue->getType()->isDArray()) { //allocate space for the dynamic array data to go
                        ASTValue *arrptr = getArrayPointer(idValue);
                        ASTValue *alloca = opAlloca(arrptr->getType()->getPointerElementTy()->getArrayTy(len));
                        alloca = promoteType(alloca, arrptr->getType());
                        storeValue(arrptr, alloca);

                        ASTValue *arrsz = getArraySize(idValue);
                        storeValue(arrsz, getIntValue(ASTType::getLongTy(), len));
                    }

                    for(int i = 0; i < len; i++) {
                        //TODO: replace with memcpy
                        storeValue(
                                opIndex(idValue, getIntValue(ASTType::getLongTy(), i)),
                                promoteType(
                                    opIndex(defaultValue, getIntValue(ASTType::getLongTy(), i)),
                                    idValue->getType()->getPointerElementTy()));

                    }
                } else {
                    defaultValue = promoteType(defaultValue, vty);
                    storeValue(idValue, defaultValue);
                }
            } else {
                //TODO: should not need promoteType. defaultValue should be coerced in validate
                defaultValue = promoteType(defaultValue, vty);
                storeValue(idValue, defaultValue);
            }

            if(vty->isRetainable() && !idValue->isWeak()) {
                retainObject(defaultValue);
            }

			//TODO: fix debug information for variable creation
#ifndef WIN32
            Instruction *vinst = unit->debug->createVariable(vdecl->getName(),
                    idValue, ir->GetInsertBlock(), vdecl->loc);
            vinst->setDebugLoc(llvm::DebugLoc::get(vdecl->loc.line, vdecl->loc.ch, diScope()));
#endif
            //TODO: maybe create a LValue field in CGValue?
        } else if(vty->isClass()) {
            // store null to class if no value
            ASTBasicValue null = ASTBasicValue(vty, ConstantPointerNull::get((PointerType*) codegenType(vty)));
            storeValue(idValue, &null);
        }
    }

    vdecl->identifier->setValue(idValue);
}

void IRCodegenContext::codegenFunctionDeclaration(FunctionDeclaration *fdecl) {
    IRFunction backup = currentFunction;
    currentFunction = IRFunction();

    FunctionType *fty = (FunctionType*) codegenType(fdecl->getType());
    Function *func;
    func = (Function*) module->getOrInsertFunction(fdecl->getMangledName(), fty);

    if(fdecl->body) {
        dwarfStopPoint(fdecl->loc);
        unit->debug->createFunction(fdecl, func);

        func->setLinkage(Function::ExternalLinkage);
        BasicBlock *BB = BasicBlock::Create(context, "entry", func);
        BasicBlock *exitBB = BasicBlock::Create(context, "exit", func);
        currentFunction.exit = exitBB;
        ir->SetInsertPoint(BB);

        currentFunction.retVal = NULL;
        if(func->getReturnType() != Type::getVoidTy(context))
        {
            currentFunction.retVal = new ASTBasicValue(fdecl->getReturnType(),
                    ir->CreateAlloca(codegenType(fdecl->getReturnType()),
                        0, "ret"), true);
        }


        enterScope(new IRScope(fdecl->scope, fdecl->diSubprogram));
        dwarfStopPoint(fdecl->loc);

        ASTFunctionType *astfty = fdecl->getType()->asFunctionType();
        int idx = 0;

        Function::arg_iterator AI = func->arg_begin();
        if(fdecl->owner) {
            AI->setName("this");
            ASTValue *thisval = new ASTBasicValue(fdecl->owner->getReferenceTy(), AI, false, true);
            lookupInScope("this")->setValue(thisval);
#ifndef WIN32 // currently broken on windows
            Instruction *ainst = unit->debug->createVariable("this",
                                                      thisval, ir->GetInsertBlock(), fdecl->loc, 1);
            ainst->setDebugLoc(llvm::DebugLoc::get(fdecl->loc.line, fdecl->loc.ch, diScope()));
#endif
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
            AllocaInst *alloc = ir->CreateAlloca(codegenType(fdecl->parameters[idx]->getType()),
                                               0, fdecl->parameters[idx]->getName());
            alloc->setAlignment(8);
            ASTBasicValue *alloca = new ASTBasicValue(fdecl->parameters[idx]->getType(), alloc, true, fdecl->parameters[idx]->getType()->isReference());
            alloca->setWeak(fdecl->parameters[idx]->isWeak()); //XXX weak reference

            // i think we arent using IRBuilder here so we can insert at top of BB
            if(fdecl->parameters[idx]->getType()->isReference()) {
                ir->CreateStore(AI, codegenRefValue(alloca));
            } else {
                ir->CreateStore(AI, codegenLValue(alloca));
            }

            fdecl->parameters[idx]->getIdentifier()->setValue(alloca);

            //register debug params
            //XXX hacky with Instruction, and setDebugLoc manually
			//TODO: fix createVariable for function parameter. (errors because we are ignoring 'this' right now)
#ifndef WIN32
			Instruction *ainst = unit->debug->createVariable(fdecl->parameters[idx]->getName(),
                                                      alloca, ir->GetInsertBlock(), fdecl->loc, idx+1);
            ainst->setDebugLoc(llvm::DebugLoc::get(fdecl->loc.line, fdecl->loc.ch, diScope()));
#endif
            //TODO: register value to scope

            // retain class parameters (retain after assigning the passed value above)
            // parameters will be released on scope exit
            if(alloca->getType()->isClass() && !alloca->isWeak()) {
                retainObject(alloca);
            }
        }


        codegenStatement(fdecl->body);

        if(!isTerminated())
            ir->CreateBr(currentFunction.exit);

        ir->SetInsertPoint(currentFunction.exit);
        setTerminated(false);
        exitScope();
        if(!currentFunction.retVal) // returns void
        {
            ir->CreateRetVoid();
        } else
        {
            ASTValue *astRet = loadValue(currentFunction.retVal);
            ir->CreateRet(codegenValue(astRet));
        }

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
    for(ASTScope::iterator it = utdecl->getScope()->begin(); it != end; it++) {
        if(it->getDeclaration()->variableDeclaration() && it->getDeclaration()->isStatic()) {
            codegenDeclaration(it->getDeclaration());
        }
    }

    // codegen methods; interfaces should not have methods codegen'd, they are just placeholder declarations
    if(!utdecl->interfaceDeclaration()) {
        for(ASTScope::iterator it = utdecl->getScope()->begin(); it != end; it++) {
            if(it->getDeclaration()->functionDeclaration()) {
                codegenDeclaration(it->getDeclaration());
            }
        }
    }

    /*
    for(int i = 0; i < utdecl->methods.size(); i++) {
            codegenDeclaration(utdecl->methods[i]);
    }

    for(int i = 0; i < utdecl->staticMembers.size(); i++) {
            codegenDeclaration(utdecl->staticMembers[i]);
    }
    */
}


void IRCodegenContext::codegenDeclaration(Declaration *decl)
{
    location = decl->loc;
    if(FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(decl)) {
        codegenFunctionDeclaration(fdecl);
        if(fdecl->getNextOverload()){ //codegen all function overloads
            codegenFunctionDeclaration(fdecl->getNextOverload());
        }
    } else if(VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(decl)) {
        codegenVariableDeclaration(vdecl);
    } else if(UserTypeDeclaration *utdecl = dynamic_cast<UserTypeDeclaration*>(decl)) {
        codegenUserTypeDeclaration(utdecl);
    }
    ///NOTE (for the most part) type declarations are not Codegen'd like values, accesible across Modules
}

void IRCodegenContext::codegenInclude(IRTranslationUnit *current, ModuleDeclaration *inc)
{
    //XXX should be 'import'?
}

ASTValue *IRCodegenContext::codegenGlobal(VariableDeclaration *vdecl) {
    Identifier *id = vdecl->getIdentifier();
    ASTType *idTy = id->getType();

    //TODO: correct type for global storage (esspecially pointers?)
    ASTValue *idValue = 0;
    if(vdecl->value)
        idValue = codegenExpression(vdecl->value);

    if(idTy->getKind() == TYPE_DYNAMIC)
    {
        emit_message(msg::WARNING, "CG: dynamic type", location);
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
        if(!vdecl->value->isConstant()) {
            emit_message(msg::ERROR, "CG: global value initializer must be constant", vdecl->value->loc);
        }

        ASTValue *defaultValue = codegenExpression(vdecl->value);
        gValue = (llvm::Constant*) codegenValue(promoteType(defaultValue, idTy));
    } else
    {
        gValue = (llvm::Constant*) llvm::Constant::getNullValue(codegenType(idTy));
    }

    GlobalVariable *llvmval = NULL;
    if(!vdecl->isStatic()) {
        llvmval = (GlobalVariable*)
                module->getOrInsertGlobal(id->getMangledName(), codegenType(idTy));

        GlobalValue::LinkageTypes linkage = vdecl->qualifier.external ?
            GlobalValue::ExternalLinkage : GlobalValue::ExternalLinkage;
        llvmval->setLinkage(linkage);
        llvmval->setInitializer(gValue);
    } else {
        llvmval = new GlobalVariable(*module, codegenType(vdecl->getType()), vdecl->isConstant(),
                GlobalValue::InternalLinkage, gValue);
        llvmval->setName(id->getMangledName());
    }

    ASTBasicValue *globalValue = NULL;
    dwarfStopPoint(vdecl->loc);
    if(vdecl->isConstant()) {
        globalValue = new ASTBasicValue(idTy, gValue);
        globalValue->setConstant(true);
        //TODO create global debug
    } else {
        globalValue = new ASTBasicValue(idTy, llvmval, true, idTy->isReference());
        if(!vdecl->isStatic())
            unit->debug->createGlobal(vdecl, globalValue);
    }

    return globalValue;
}

void IRCodegenContext::codegenTranslationUnit(IRTranslationUnit *u)
{
    this->unit = u;
    this->module = this->unit->llvmModule;

    enterScope(u->getScope());

    ASTScope::iterator it;
    for(it = unit->mdecl->importScope->begin(); it != unit->mdecl->importScope->end(); it++) //TODO: import symbols.
    {
        Identifier *mod_id = *it;
        codegenInclude(this->unit, mod_id->getDeclaration()->moduleDeclaration());
    }

    // alloc globals before codegen'ing functions
    // iterate over globals
    ASTScope::iterator end = unit->getScope()->end();
    for(ASTScope::iterator it = unit->getScope()->begin(); it != end; it++) {
        Identifier *id = *it;
        VariableDeclaration *vdecl = id->getDeclaration()->variableDeclaration();
        if(id->isVariable()) {
            ASTValue *gval = codegenGlobal(vdecl);
            id->setValue(gval);
        } else if (id->isFunction()) {
            //declare function?
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
    exitScope();
}

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 6
void printModule(Module *m, std::string filenm) {
	std::error_code err;
    raw_fd_ostream output(filenm.c_str(), err, llvm::sys::fs::OpenFlags::F_Text);
    m->print(output, 0);
    output.close();
}
#elif LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 5
void printModule(Module *m, std::string filenm) {
	std::string err;
	raw_fd_ostream output(filenm.c_str(), err, llvm::sys::fs::OpenFlags::F_Text);
	m->print(output, 0);
	output.close();
}
#elif LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 4
void printModule(Module *m, std::string filenm) {
    std::string err;
    raw_fd_ostream output(filenm.c_str(), err);
    m->print(output, 0);
    output.close();
}
#else
#error invalid LLVM version
#endif

void IRCodegenContext::codegenPackage(PackageDeclaration *p)
{
    if(p->moduleDeclaration()) // leaf in package tree
    {
        std::string err;
        IRTranslationUnit *unit = new IRTranslationUnit(this, p->moduleDeclaration());
        //p->cgValue = 0;
        codegenTranslationUnit(unit);

        // XXX debug, output all modules
        std::string outputll = config.tempName + "/" + unit->mdecl->getName() + ".ll";
        printModule(unit->llvmModule, outputll);

        linker.linkInModule(unit->llvmModule, (unsigned) Linker::DestroySource, &err);

    } else // generate all leaves ...
    {
        for(int i = 0; i < p->children.size(); i++)
        {
            codegenPackage(p->children[i]);
        }
    }
}

#include <fcntl.h>
#ifdef WIN32
#else
#include <unistd.h>
#endif

#ifdef LLVM_35
bool checkModule(Module *m) {
    if(verifyModule(*m))
    {
        emit_message(msg::OUTPUT, "failed to compile source code");
        return false;
    } else
    {
        emit_message(msg::OUTPUT, "successfully compiled source");
        return true;
    }
}
#endif

#ifdef LLVM_34
bool checkModule(Module *m) {
    if(verifyModule(*m, PrintMessageAction))
    {
        emit_message(msg::OUTPUT, "failed to compile source code");
        return false;
    } else
    {
        emit_message(msg::OUTPUT, "successfully compiled source");
        return true;
    }
}
#endif

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

    checkModule(linker.getModule());

    std::string err;
    std::string outputll;
    std::string outputo;

    if(config.emitllvm)
    {
        outputll = "output.ll";
    } else
    {
        outputll = config.tempName + "output.ll";
    }

    if(config.link)
    {
        outputo = config.tempName + "output.o";
    } else
    {
        outputo = "output.o";
    }

    printModule(linker.getModule(), outputll);

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
