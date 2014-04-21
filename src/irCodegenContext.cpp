
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

SourceLocation currentLoc;

std::string IRFunction::getName(bool mangle)
{
    if(mangle){
        return declaration->identifier->getMangledName();
    } else
    {
        return declaration->getName();
    }
}

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

llvm::Type *IRCodegenContext::codegenStructType(ASTType *ty)
{
    if(!ty->isStruct()) {
        emit_message(msg::FAILURE, "unknown struct type");
    }

    StructTypeInfo *sti = (StructTypeInfo*) ty->info;

    if(StructType *sty = module->getTypeByName(sti->identifier->getName()))
    {
        return sty;
    }

    if(unit->types.count(sti->identifier->getName())){
        Type *llty = unit->types[sti->identifier->getName()];
        return llty;
    }

    StructType *sty = StructType::create(context);
    sty->setName(sti->identifier->getName());
    unit->types[sti->identifier->getName()] = IRType(ty, sty);

    std::vector<Type*> structVec;
    for(int i = 0; i < sti->members.size(); i++)
    {
        if(VariableDeclaration *vd = dynamic_cast<VariableDeclaration*>(sti->members[i]))
        {
            structVec.push_back(codegenType(vd->type));
        } else
            emit_message(msg::UNIMPLEMENTED, "this cant be declared in a struct (yet?)");
    }
    if(sti->members.size())
    {
        sty->setBody(structVec);
    }

    debug->createStructType(ty);
    return sty;
}

llvm::Type *IRCodegenContext::codegenUnionType(ASTType *ty)
{
    if(!ty->isUnion()) {
        emit_message(msg::FAILURE, "unknown union type");
    }

    UnionTypeInfo *sti = (UnionTypeInfo*) ty->info;

    unsigned align = 0;
    unsigned size = 0;
    ASTType *alignedType = 0;
    std::vector<Type*> unionVec;
    for(int i = 0; i < sti->members.size(); i++)
    {
        if(VariableDeclaration *vd = dynamic_cast<VariableDeclaration*>(sti->members[i]))
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

    if(sti->members.size())
    {
        unionVec.push_back(codegenType(alignedType));
        if(size - alignedType->getSize())
            unionVec.push_back(ArrayType::get(Type::getInt8Ty(context),
                        size - alignedType->getSize()));
        StructType *sty = StructType::create(context, sti->identifier->getName());
        sty->setBody(unionVec);
        ty->cgType = sty;
    }
    else ty->cgType = Type::getInt8Ty(context); //allow fwd declared types, TODO: cleaner

    debug->createUnionType(ty);
    return(llvm::Type*) ty->cgType;
}

llvm::Type *IRCodegenContext::codegenClassType(ASTType *ty)
{
    ClassTypeInfo *cti = (ClassTypeInfo*) ty->info;
    //TODO: generate class type
    return NULL;
}

llvm::Type *IRCodegenContext::codegenTupleType(ASTType *ty)
{
    TupleTypeInfo *tti = dynamic_cast<TupleTypeInfo*>(ty->info);
    if(ty->kind != TYPE_TUPLE || !tti)
    {
        emit_message(msg::FAILURE, "invalid tuple type codegen");
        return NULL;
    }

    if(ty->cgType) return ty->cgType;

    if(!tti->types.size())
    {
        emit_message(msg::ERROR, "invalid 0-tuple");
        return NULL;
    }

    std::vector<Type*> tupleVec;
    for(int i = 0; i < tti->types.size(); i++)
    {
        tupleVec.push_back(codegenType(tti->types[i]));
    }

    StructType *tty = StructType::create(context);
    tty->setBody(tupleVec);
    ty->cgType = tty;
    debug->createType(ty); // TODO
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
    ArrayTypeInfo *ati = dynamic_cast<ArrayTypeInfo*>(ty->info);
    if(!ati) {
        emit_message(msg::FAILURE, "attempt to codegen invalid array type");
    }

    if(ty->cgType) return ty->cgType;

    if(ati->isDynamic())
    {
        //Identifier *id = ast->getRuntimeUnit()->getScope()->lookup("DynamicArray");
        //ASTType *arrayty = id->getDeclaredType();
        //ty->cgType = codegenType(arrayty);

        vector<Type*> members;
        members.push_back(codegenType(ati->arrayOf->getPointerTy()));
        members.push_back(codegenType(ASTType::getLongTy()));
        StructType *aty = StructType::create(context, ty->getName());
        aty->setBody(members);
        ty->cgType = aty;

        debug->createDynamicArrayType(ty);
    } else
    {
        llvm::ArrayType *aty = ArrayType::get(codegenType(ati->arrayOf), ati->length());
        ty->cgType = aty;

        debug->createArrayType(ty);
    }

    return (llvm::Type*) ty->cgType;
}

llvm::Type *IRCodegenContext::codegenFunctionType(ASTType *ty) {
    FunctionTypeInfo *fti = dynamic_cast<FunctionTypeInfo*>(ty->info);
    if(!fti) {
        emit_message(msg::FAILURE, "attempt to codegen invalid function type");
        return NULL;
    }

    vector<Type*> params;
    for(int i = 0; i < fti->params.size(); i++)
    {
        params.push_back(codegenType(fti->params[i]));
    }

    FunctionType *fty = FunctionType::get(codegenType(fti->ret), params, fti->vararg);
    return fty;
}

llvm::Type *IRCodegenContext::codegenType(ASTType *ty)
{
    llvm::Type *llvmty = NULL;
    if(ty->cgType) return (Type*) ty->cgType;
    //if(!ty->cgType)
    {
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
            case TYPE_STRUCT:
                llvmty = codegenStructType(ty);
                break;
            case TYPE_UNION:
                llvmty = codegenUnionType(ty);
                break;
            case TYPE_CLASS:
                llvmty = codegenClassType(ty);
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
    }

    ty->cgType = llvmty;

    return (llvm::Type *) llvmty;
}

ASTValue *IRCodegenContext::indexValue(ASTValue *val, int i)
{
    CompositeTypeInfo *cti = dynamic_cast<CompositeTypeInfo*>(val->type->info);
    if(!cti)
    {
        emit_message(msg::FAILURE, "cannot index non-composite type");
        return NULL;
    }

    std::vector<Value*> gep;
    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
    gep.push_back(ConstantInt::get(Type::getInt32Ty(context), i));
    Value *v = codegenLValue(val);
    Value *llval = ir->CreateInBoundsGEP(v, gep);
    llval = ir->CreateBitCast(llval, codegenType(cti->getContainedType(i)->getPointerTy()));
    return new ASTValue(cti->getContainedType(i), llval, true);
}

llvm::Value *IRCodegenContext::codegenValue(ASTValue *value)
{
    if(!value->cgValue){
        //TODO
        emit_message(msg::FAILURE, "AST Value failed to generate");
    }

    if(value->isLValue())
    {
        return ir->CreateAlignedLoad(codegenLValue(value), 4);
    }

    return (llvm::Value *) value->cgValue;
}

llvm::Value *IRCodegenContext::codegenLValue(ASTValue *value)
{
    if(!value->isLValue())
    {
        emit_message(msg::FATAL, "rvalue used in lvalue context!");
        return NULL;
    }
    if(!value->cgValue) {
        //TODO
        emit_message(msg::FAILURE, "AST Value failed to generate");
    }

    return (llvm::Value*) value->cgValue;
}

ASTValue *IRCodegenContext::storeValue(ASTValue *dest, ASTValue *val)
{
    ASTValue *stored = new ASTValue(dest->type,
            ir->CreateStore(codegenValue(val), codegenLValue(dest)));
    return stored;
}

ASTValue *IRCodegenContext::loadValue(ASTValue *lval)
{
    assert_message(lval->isLValue(), msg::FAILURE, "attempted to load RValue (must be LValue");
    ASTValue *loaded = new ASTValue(lval->type, codegenValue(lval));
    return loaded;
}

ASTValue *IRCodegenContext::codegenIdentifier(Identifier *id)
{
    ///XXX work around for forward declarations of variables in parent scopes
    if(id->isUndeclared())
    {
        //id = lookup(iexp->id->getName());
        if(id->isUndeclared()) {
            emit_message(msg::ERROR, string("undeclared variable '") +
                    id->getName() + string("' in scope"));
            return NULL;
        }
    }

    if(id->isVariable())
    {
        if(id->getScope()->getUnit() != unit->unit) // id not declared in current TU
        {
            if(unit->globals.count(id->getName()))
            {
                return unit->globals[id->getName()];
            } else
            {
                Declaration *decl = id->getDeclaration();
                GlobalValue::LinkageTypes linkage = decl->isExternal() ?
                    GlobalValue::ExternalLinkage : GlobalValue::ExternalWeakLinkage;
                GlobalVariable *GV = new GlobalVariable(*module,
                        codegenType(id->getType()),
                        false,
                        linkage,
                        NULL,
                        id->getName());
                IRValue irval = IRValue(new ASTValue(id->getType(), GV, true), GV);
                unit->globals[id->getName()] = irval;
                return irval;
            }
            emit_message(msg::FAILURE, "failed to codegen identifier");
        }

        return id->getValue(); // else declared in current TU, so we are good
    } else if(id->isFunction())
    {
        Value *llvmfunc = unit->functions[id->getName()];
        FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(id->getDeclaration());
        if(!fdecl) emit_message(msg::FAILURE, "invalid function identifier");
        id->setValue(new ASTValue(fdecl->prototype, llvmfunc));
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
    if(CompoundExpression *cexp = exp->compoundExpression())
    {
        for(int i = 0; i < cexp->statements.size(); i++)
        {
            if(!isTerminated() || (dynamic_cast<LabelStatement*>(cexp->statements[i]) ||
                              dynamic_cast<CaseStatement*>(cexp->statements[i])))
                codegenStatement(cexp->statements[i]);
        }
        return NULL; //TODO: value?
    }
    else if(NumericExpression *nexp = exp->numericExpression())
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
        Constant *strConstant = ConstantDataArray::getString(context, sexp->string);
        GlobalVariable *GV = new GlobalVariable(*module, strConstant->getType(), true,
                GlobalValue::PrivateLinkage, strConstant);
        vector<Value *> gep;
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        Constant *val = ConstantExpr::getInBoundsGetElementPtr(GV, gep);

        //TODO: make this an ARRAY (when arrays are added), so that it has an associated length

        return new ASTValue(ASTType::getCharTy()->getPointerTy(), val);
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
        ///XXX work around for forward declarations of variables in parent scopes
        if(iexp->identifier()->isUndeclared())
        {
            iexp->id = lookup(iexp->id->getName());
            //TODO
            if(iexp->id->isUndeclared()) {
                emit_message(msg::ERROR, string("undeclared variable '") +
                        iexp->id->getName() + string("' in scope"), iexp->loc);
                return NULL;
            }
        }
        return codegenIdentifier(iexp->id);
    } else if(BlockExpression *bexp = exp->blockExpression())
    {
        ASTValue *value = NULL;
        pushScope(new IRScope(bexp->scope, debug->createScope(getScope()->debug, bexp->loc)));
        if(IfExpression *iexp = exp->ifExpression())
        {
            value = codegenIfExpression(iexp);
        } else if(LoopExpression *lexp = exp->loopExpression())
        {
            value = codegenLoopExpression(lexp);
        } else if(SwitchExpression *sexp = exp->switchExpression())
        {
            value = codegenSwitchExpression(sexp);
        } else if(ElseExpression *eexp = exp->elseExpression())
        {
            value = codegenElseExpression(eexp);
        }
        popScope();
        return value;
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
    CompositeTypeInfo *cti = 0;
    if(ty) cti = dynamic_cast<CompositeTypeInfo*>(ty->info);

    for(int i = 0; i < exp->members.size(); i++)
    {
        ASTValue *val = codegenExpression(exp->members[i]);

        if(cti)
            val = promoteType(val, cti->getContainedType(i));

        vals.push_back(val);
        types.push_back(val->getType());

        if(!val->isLValue()) lvalue = false;
    }

    TupleTypeInfo *tti = new TupleTypeInfo(types);
    ASTType *tty = new ASTType();
    tty->setTypeInfo(tti, TYPE_TUPLE);

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

        StructType *llty = (StructType*) codegenType(tty);

        GlobalVariable *GV = new GlobalVariable(*module, llty, true,
                GlobalValue::PrivateLinkage,
                ConstantStruct::get(llty, llvals));

        //vector<Value *> gep;
        //gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        //gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        //Constant *val = ConstantExpr::getInBoundsGetElementPtr(GV, gep);
        return new ASTValue(tty, GV, true);
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
    Function *mallocFunc = module->getFunction("malloc");
    Value *value;
    value = ir->CreateCall(mallocFunc, llargs);
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
    } else
        value = ir->CreateBitCast(value, codegenType(exp->type)->getPointerTo());
    return new ASTValue(ty, value);
}

ASTValue *IRCodegenContext::codegenDeleteExpression(DeleteExpression *exp)
{
    vector<Value*> llargs;
    ASTValue *val = codegenExpression(exp->expression);

    if(val->type->isArray() && val->type->kind == TYPE_DYNAMIC_ARRAY)
    {
        //TODO: duplicate of ".ptr". make a function for this
        std::vector<Value*> gep;
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
        Value *llval = ir->CreateInBoundsGEP(codegenLValue(val), gep);
        val = new ASTValue(val->type->getReferencedTy(), llval, true);
    }

    val = promoteType(val, ASTType::getCharTy()->getPointerTy());
    llargs.push_back(codegenValue(val));
    Function *freeFunc = module->getFunction("free");

    Value *value = ir->CreateCall(freeFunc, llargs);
    //TODO: call deallocator function
    //return new ASTValue(exp->type, value);
    return NULL;
}

ASTValue *IRCodegenContext::codegenElseExpression(ElseExpression *exp)
{
    if(!exp->body)
    {
        emit_message(msg::ERROR, "else keyword expects body", exp->loc);
        return NULL;
    }

    codegenStatement(exp->body);
    return NULL; //TODO: value?
}

ASTValue *IRCodegenContext::codegenIfExpression(IfExpression *exp)
{
    ASTValue *cond = codegenExpression(exp->condition);

    ASTValue *icond = promoteType(cond, ASTType::getBoolTy());
    llvm::BasicBlock *ontrue = BasicBlock::Create(context, "true",
            ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *onfalse = BasicBlock::Create(context, "false",
            ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *endif = BasicBlock::Create(context, "endif",
            ir->GetInsertBlock()->getParent());
    ir->CreateCondBr(codegenValue(icond), ontrue, onfalse);

    ir->SetInsertPoint(ontrue);
    codegenStatement(exp->body);
    if(!isTerminated())
        ir->CreateBr(endif);
    setTerminated(false);

    ir->SetInsertPoint(onfalse);
    if(exp->elsebr) codegenExpression(exp->elsebr);
    if(!isTerminated())
        ir->CreateBr(endif);
    setTerminated(false);

    ir->SetInsertPoint(endif);
    setTerminated(false);

    return NULL;
}

ASTValue *IRCodegenContext::codegenLoopExpression(LoopExpression *exp)
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

    if(ForExpression *fexp = exp->forExpression())
    {
        if(fexp->decl) codegenStatement(fexp->decl);
    }

    ir->CreateBr(loopBB);
    ir->SetInsertPoint(loopBB);

    if(exp->condition)
    {
        ASTValue *cond = codegenExpression(exp->condition);
        ASTValue *icond = promoteType(cond, ASTType::getBoolTy());
        ir->CreateCondBr(codegenValue(icond), ontrue, onfalse);
    } else ir->CreateBr(ontrue);

    getScope()->breakLabel = loopend;
    getScope()->continueLabel = loopupdate;

    if(!loopupdate) getScope()->continueLabel = loopBB;

    ir->SetInsertPoint(ontrue);
    if(exp->body) codegenStatement(exp->body);
    ir->CreateBr(loopupdate);
    ir->SetInsertPoint(loopupdate);
    if(exp->update) codegenStatement(exp->update);
    ir->CreateBr(loopBB);

    ir->SetInsertPoint(onfalse);
    if(exp->elsebr) codegenExpression(exp->elsebr); //TODO: scoping might be weird. ifScope is on stack
    ir->CreateBr(loopend);

    ir->SetInsertPoint(loopend);
    return NULL;
}

ASTValue *IRCodegenContext::codegenSwitchExpression(SwitchExpression *exp)
{
    BasicBlock *switch_default = BasicBlock::Create(context, "switch_default",
                                         ir->GetInsertBlock()->getParent());

    BasicBlock *switch_end = BasicBlock::Create(context, "switch_end",
                                         ir->GetInsertBlock()->getParent());

    BasicBlock *preSwitch = ir->GetInsertBlock();

    ASTValue *cond = codegenExpression(exp->condition);
    SwitchInst *sinst = ir->CreateSwitch(codegenValue(cond), switch_default);
    //TODO: set cases

    getScope()->switchExp = exp;
    getScope()->breakLabel = switch_end;

    //setTerminated(true);
    ir->SetInsertPoint(switch_default);
    if(exp->body) codegenStatement(exp->body);

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

    return NULL; //TODO:
}

ASTValue *IRCodegenContext::codegenCallExpression(CallExpression *exp)
{
    ASTValue *func = codegenExpression(exp->function);
    if(!func) {
        emit_message(msg::ERROR, "unknown expression", exp->loc);
        return NULL;
    }

    //assert(!func.llvmTy()->isFunctionTy() && "not callable!");
    //FunctionType *fty = (FunctionType*) func.llvmTy();
    //TODO: once proper type passing is done, check if callable above

    FunctionTypeInfo *fti = NULL;
    ASTType *rtype = NULL;
    //TODO: very messy, should be able to get return value of function type
    if(IdentifierExpression *iexp = dynamic_cast<IdentifierExpression*>(exp->function))
    {
        // XXX messy, not always true
        if(FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(iexp->id->declaration))
        {
            fti = dynamic_cast<FunctionTypeInfo*>(fdecl->prototype->info);
            rtype = fti->ret;
            //ftype = fdecl->prototype;
        } else if(VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(iexp->id->declaration))
        {
            ASTType *pty = vdecl->getType();
            fti = dynamic_cast<FunctionTypeInfo*>(pty->getReferencedTy()->info);
            rtype = fti->ret;
        }
    } else emit_message(msg::FAILURE, "unknown function type?", exp->loc);

    if((fti->params.size() != exp->args.size()) && !fti->vararg)
    {
        emit_message(msg::ERROR, "invalid number of arguments provided for function call", exp->loc);
    }

    vector<ASTValue*> cargs;
    vector<Value*> llargs;
    for(int i = 0; i < exp->args.size(); i++)
    {
        ASTValue *val = codegenExpression(exp->args[i]);
        if(!fti->vararg || (fti->params.size() > i && fti->params[i]))
            val = promoteType(val, fti->params[i]);

        else if(fti->vararg)
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

        cargs.push_back(val);
        llargs.push_back(codegenValue(cargs[i]));
    }
    llvm::Value *value = ir->CreateCall(codegenValue(func), llargs);
    return new ASTValue(rtype, value);
}

ASTValue *IRCodegenContext::codegenUnaryExpression(UnaryExpression *exp)
{
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
                llval = ConstantFP::get(codegenType(lhs->getType()), 1.0f);
                llval = ir->CreateFAdd(codegenValue(lhs), llval);
            } else
            {
                llval = ConstantInt::get(codegenType(lhs->getType()), 1);
                llval = ir->CreateAdd(codegenValue(lhs), llval);
            }

            val = new ASTValue(lhs->getType(), llval);
            storeValue(lhs, val);
            return val;
        case tok::minusminus:
            if(!lhs->isLValue()) {
                emit_message(msg::ERROR, "can only decrement LValue", exp->loc);
                return NULL;
            }

            if(lhs->getType()->isFloating())
            {
                llval = ConstantFP::get(codegenType(lhs->getType()), 1.0f);
                llval = ir->CreateFSub(codegenValue(lhs), llval);
            } else
            {
                llval = ConstantInt::get(codegenType(lhs->getType()), 1);
                llval = ir->CreateSub(codegenValue(lhs), llval);
            }

            val = new ASTValue(lhs->getType(), llval);
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
            return new ASTValue(lhs->getType()->getReferencedTy(), codegenValue(lhs), true);
        case tok::amp:
            if(!lhs->isLValue()){
                emit_message(msg::ERROR, "attempt to take reference of non-LValue", exp->loc);
                return NULL;
            }
            return new ASTValue(lhs->getType()->getPointerTy(), codegenLValue(lhs), false);
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
                TupleTypeInfo *tti = (TupleTypeInfo*) arr->getType()->info;
                unsigned long long index = llci->getZExtValue();
                if(tti->types.size() <= index)
                {
                    emit_message(msg::ERROR, "invalid tuple index", exp->loc);
                    return NULL;
                }
                ASTType *type = tti->types[index];
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
                llval = ConstantFP::get(codegenType(lhs->getType()), 1.0f);
                llval = ir->CreateFAdd(codegenValue(lhs), llval);
            } else
            {
                llval = ConstantInt::get(codegenType(lhs->getType()), 1);
                llval = ir->CreateAdd(codegenValue(lhs), llval);
            }

            val = new ASTValue(lhs->getType(), llval);

            storeValue(lhs, val);
            return old;
            case tok::minusminus:
            if(lhs->getType()->isFloating())
            {
                llval = ConstantFP::get(codegenType(lhs->getType()), 1.0f);
                llval = ir->CreateFSub(codegenValue(lhs), llval);
            } else
            {
                llval = ConstantInt::get(codegenType(lhs->getType()), 1);
                llval = ir->CreateSub(codegenValue(lhs), llval);
            }
            old = loadValue(lhs);
            val = new ASTValue(lhs->getType(), llval);

            storeValue(lhs, val);
            return old;
        }
    } else if(DotExpression *dexp = dynamic_cast<DotExpression*>(exp))
    {

            ASTValue *lhs; // = codegenExpression(dexp->lhs);
            if(TypeExpression *tyexp = dexp->lhs->typeExpression())
            {
                if(dexp->rhs == "sizeof")
                {
                    return new ASTValue(ASTType::getULongTy(),
                            ConstantInt::get(Type::getInt64Ty(context),
                                tyexp->type->getSize()));
                } else if(dexp->rhs == "offsetof")
                {
                    emit_message(msg::UNIMPLEMENTED,
                            "offsetof attribute not yet implemented", dexp->loc);
                }
                emit_message(msg::ERROR, "unknown attribute '" + dexp->rhs + "' of struct '" +
                        tyexp->type->getName() + "'", dexp->loc);
                return NULL;
            }

            lhs = codegenExpression(dexp->lhs);

            //TODO: duplicate of above. resolve 'MyStruct.sizeof' lhs to TypeExpression
            if(!lhs->cgValue)
            {
                if(dexp->rhs == "sizeof")
                {
                    return new ASTValue(ASTType::getULongTy(),
                            ConstantInt::get(Type::getInt64Ty(context),
                                lhs->getType()->getSize()));
                } else if(dexp->rhs == "offsetof")
                {
                    emit_message(msg::UNIMPLEMENTED,
                            "offsetof attribute not yet implemented", dexp->loc);
                }
                emit_message(msg::ERROR, "unknown attribute '" + dexp->rhs + "' of struct '" +
                        lhs->getType()->getName() + "'", dexp->loc);
                return NULL;
            }

            if(lhs->getType()->isPointer()) lhs = new ASTValue(lhs->getType()->getReferencedTy(),
                    codegenValue(lhs), true);
            if(!lhs->getType()->isStruct() && !lhs->getType()->isArray() &&
                    !lhs->getType()->isUnion()) {
                emit_message(msg::ERROR, "can only index struct or array type", dexp->loc);
                return NULL;
            }

            if(lhs->getType()->isStruct())
            {
                HetrogenTypeInfo *hti = (HetrogenTypeInfo*) lhs->getType()->getTypeInfo();

                int i = hti->getMemberIndex(dexp->rhs);
                if(i < 0)
                {
                    emit_message(msg::ERROR, "member not in struct: " + hti->getName() + "." +
                            dexp->rhs,
                            dexp->loc);
                }
                return indexValue(lhs, i);
            } else if (lhs->getType()->isUnion())
            {
                HetrogenTypeInfo *hti = (HetrogenTypeInfo*) lhs->getType()->getTypeInfo();
                Declaration *mdecl = hti->getMemberDeclaration(dexp->rhs);
                if(!mdecl)
                {
                    emit_message(msg::ERROR, "member not in struct: " + hti->getName() + "." +
                            dexp->rhs,
                            dexp->loc);
                }
                ASTType *ty = mdecl->getType();
                std::vector<Value*> gep;
                gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
                gep.push_back(ConstantInt::get(Type::getInt32Ty(context),
                                            hti->getMemberIndex(dexp->rhs)));
                Value *llval = ir->CreateInBoundsGEP(codegenLValue(lhs), gep);
                return new ASTValue(ty, llval, true);
            }else if(lhs->getType()->kind == TYPE_DYNAMIC_ARRAY)
            {
                ArrayTypeInfo *ati = dynamic_cast<ArrayTypeInfo*>(lhs->getType()->info);
                if(!ati){
                    emit_message(msg::FATAL, "dot exp on array, not actually an array?", dexp->loc);
                    return NULL;
                }
                ASTType *ty = ati->getReferenceTy();

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
                ArrayTypeInfo *ati = dynamic_cast<ArrayTypeInfo*>(lhs->getType()->info);
                ASTType *ty = ati->getReferenceTy();

                if(dexp->rhs == "ptr")
                {
                    return new ASTValue(ty->getPointerTy(),
                            ir->CreateInBoundsGEP(codegenLValue(lhs), 0), true);
                }

                if(dexp->rhs == "size")
                {
                    return new ASTValue(ASTType::getULongTy(),
                            ConstantInt::get(Type::getInt64Ty(context), ati->length()));
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
        return new ASTValue(toType, ir->CreatePointerCast(codegenValue(val), codegenType(toType)));
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
}

ASTValue *IRCodegenContext::promoteTuple(ASTValue *val, ASTType *toType)
{
    if(toType->kind == TYPE_STRUCT)
    {
        if(((TupleTypeInfo*) val->type->info)->types.size() ==
                ((StructTypeInfo*) toType->info)->members.size()) //TODO proper test
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
        if(((TupleTypeInfo*) val->type->info)->types.size() ==
                ((TupleTypeInfo*) toType->info)->types.size())
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
            TupleTypeInfo *tti = (TupleTypeInfo*) val->type->info;
            toSize = ConstantInt::get(codegenType(ASTType::getULongTy()), tti->length());

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
            ArrayTypeInfo *ati = (ArrayTypeInfo*) val->type->info;
            if(ati->arrayOf != toType->getReferencedTy())
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
    if(!lhs->isLValue())
    {
        // XXX work around. if lhs is unknown, isLValue will fail on expression
        ASTValue *vlhs = codegenExpression(lhs);
        if(!vlhs->isLValue())
        {
            emit_message(msg::ERROR, "assignment requires lvalue", lhs->loc);
            return NULL;
        }
    }

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
            if(TYPE->isFloating())
                val = ir->CreateFAdd(lhs_val, rhs_val);
            else
                val = ir->CreateAdd(lhs_val, rhs_val);
            retValue = new ASTValue(TYPE, val); //TODO: proper typing (for all below too)
            break;

        case tok::minus:
        case tok::minusequal:
            if(TYPE->isFloating())
                val = ir->CreateFSub(lhs_val, rhs_val);
            else
                val = ir->CreateSub(lhs_val, rhs_val);
            retValue = new ASTValue(TYPE, val);
            break;

        case tok::star:
        case tok::starequal:
            if(TYPE->isFloating())
                val = ir->CreateFMul(lhs_val, rhs_val);
            else //TODO: signed?
                val = ir->CreateMul(lhs_val, rhs_val);
            retValue = new ASTValue(TYPE, val);
            break;

        case tok::slash:
        case tok::slashequal:
            if(TYPE->isFloating())
                val = ir->CreateFDiv(lhs_val, rhs_val);
            else if(TYPE->isSigned())
                val = ir->CreateSDiv(lhs_val, rhs_val);
            else
                val = ir->CreateUDiv(lhs_val, rhs_val);
            retValue = new ASTValue(TYPE, val);
            break;

        case tok::percent:
        case tok::percentequal:
            if(TYPE->isFloating())
                val = ir->CreateFRem(lhs_val, rhs_val);
            else if(TYPE->isSigned())
                val = ir->CreateSRem(lhs_val, rhs_val);
            else
                val = ir->CreateURem(lhs_val, rhs_val);
            retValue = new ASTValue(TYPE, val);
            break;

        case tok::lessless:
            val = ir->CreateShl(lhs_val, rhs_val);
            return new ASTValue(TYPE, val);

        case tok::greatergreater:
            val = ir->CreateLShr(lhs_val, rhs_val);
            return new ASTValue(TYPE, val);

        case tok::starstar:
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
    if(ExpressionStatement *estmt = dynamic_cast<ExpressionStatement*>(stmt))
    {
        codegenExpression(estmt->expression);
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
        //lstmt->identifier->setValue(new ASTValue(NULL, BB)); //TODO: cg value?
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
    } else emit_message(msg::FAILURE, "i dont know what kind of statmeent this isssss", stmt->loc);

}

void IRCodegenContext::codegenDeclaration(Declaration *decl)
{
    if(FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(decl))
    {
        IRFunction backup = currentFunction;
        currentFunction = IRFunction(fdecl);
        // create on declaration(?) No, then other references my be invalid
        Function *func = module->getFunction(currentFunction.getName());
        if(fdecl->body)
        {
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

            dwarfStopPoint(decl->loc);
            debug->createFunction(fdecl);
            pushScope(new IRScope(fdecl->scope, fdecl->diSubprogram));
            dwarfStopPoint(decl->loc);

            FunctionTypeInfo *fti = dynamic_cast<FunctionTypeInfo*>(fdecl->prototype->info);
            int idx = 0;
            for(Function::arg_iterator AI = func->arg_begin(); AI != func->arg_end(); AI++, idx++)
            {
                if(fti->params.size() < idx){
                        emit_message(msg::FAILURE,
                                "argument counts dont seem to match up...", decl->loc);
                        return;
                }
                //pair<ASTType*, std::string> param_i = fdecl->prototype->parameters[idx];
                AI->setName(fdecl->paramNames[idx]);
                AllocaInst *alloc = new AllocaInst(codegenType(fti->params[idx]),
                                                   0, fdecl->paramNames[idx], BB);
                    //ir->CreateAlloca(codegenType(param_i.first), 0, param_i.second);
                alloc->setAlignment(8);
                ASTValue *alloca = new ASTValue(fti->params[idx], alloc, true);
                new StoreInst(AI, codegenLValue(alloca), BB);
                //ir->CreateStore(AI, codegenLValue(alloca));

                Identifier *id = getInScope(fdecl->paramNames[idx]);
                id->setDeclaration(NULL, Identifier::ID_VARIABLE);
                id->setValue(alloca);

                //register debug params
                //XXX hacky with Instruction, and setDebugLoc manually
                Instruction *ainst = debug->createVariable(fdecl->paramNames[idx],
                                                          alloca, BB, decl->loc, idx+1);
                ainst->setDebugLoc(llvm::DebugLoc::get(decl->loc.line, decl->loc.ch, diScope()));
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
                //ir->CreateBr(currentFunction.exit);
                ir->SetInsertPoint(currentFunction.exit);
                ASTValue *astRet = loadValue(currentFunction.retVal);
                ir->CreateRet(codegenValue(astRet));
            }

            popScope();
        }
        setTerminated(false);
        currentFunction = backup;
    } else if(VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(decl))
    {
        dwarfStopPoint(vdecl->loc);
        ASTType *vty = vdecl->type;

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
            defaultValue = promoteType(defaultValue, vty);
        }

        //XXX work around for undeclared struct
        Identifier *id = NULL;
        if(NamedUnknownInfo *usi = dynamic_cast<NamedUnknownInfo*>(vty->info))
        {
            if(usi->identifier->isUndeclared())
            {
                id = lookup(usi->identifier->getName());
                if(id->isUndeclared()){
                    emit_message(msg::ERROR, "undeclared struct: " + id->getName(), vdecl->loc);
                    return;
                }
                vty = id->getDeclaredType();
            }
        }

        AllocaInst *llvmDecl = ir->CreateAlloca(codegenType(vty), 0, vdecl->getName());
        llvmDecl->setAlignment(8);
        ASTValue *idValue = new ASTValue(vty, llvmDecl, true);

        if(defaultValue)
        {
            defaultValue = promoteType(defaultValue, vty);
            storeValue(idValue, defaultValue);

            Instruction *vinst = debug->createVariable(vdecl->getName(),
                    idValue, ir->GetInsertBlock(), vdecl->loc);
            vinst->setDebugLoc(llvm::DebugLoc::get(decl->loc.line, decl->loc.ch, diScope()));
            //TODO: maybe create a LValue field in CGValue?
        }
        vdecl->identifier->setValue(idValue);

        /*
        if(ArrayDeclaration *adecl = dynamic_cast<ArrayDeclaration*>(decl))
        {
            if(DynamicArrayTypeInfo *dti = dynamic_cast<DynamicArrayTypeInfo*>(adecl->type->info))
            {
                NumericExpression *nsz = dynamic_cast<NumericExpression *>(adecl->sz);

                // create static array and store in array 'struct'
                if( nsz && nsz->type == NumericExpression::INT)
                {
                    // array has a size
                    Value *llvmSz = ConstantInt::get(codegenType(ASTType::getULongTy()),
                            nsz->intValue);
                    AllocaInst *staticArray = ir->CreateAlloca(
                            codegenType(adecl->getType()->info->getReferenceTy()), llvmSz);


                    ir->CreateStore(staticArray, ir->CreateStructGEP(llvmDecl, 0));
                    ir->CreateStore(llvmSz, ir->CreateStructGEP(llvmDecl, 1));

                } else {
                    emit_message(msg::FATAL,
                            "array declaration size must be constant integer expression (for now?)",
                            adecl->loc);
                    return;
                }
            }
        } */
    } else if(TypeDeclaration *tdecl = dynamic_cast<TypeDeclaration*>(decl))
    {
        //codegenType(tdecl->getType()); // make sure types are present in Module
    }
    ///NOTE type declarations are not Codegen'd like values, accesible across Modules
}

//XXX should be 'import'?
void IRCodegenContext::codegenIncludeUnit(IRTranslationUnit *current, TranslationUnit *inc)
{
    // alloc globals before codegen'ing functions
    /*
    for(int i = 0; i < inc->globals.size(); i++)
    {
        Identifier *id = inc->globals[i]->identifier;
        if(id->isVariable() && !id->getValue())
        {
            ASTType *idTy = id->getType();
            //llvm::Value *llvmval = module->getOrInsertGlobal(id->getName(), codegenType(idTy));
            GlobalValue::LinkageTypes linkage = inc->globals[i]->external ?
                GlobalValue::ExternalLinkage : GlobalValue::ExternalWeakLinkage;
            linkage = GlobalValue::WeakAnyLinkage;
            GlobalVariable *llvmval = new GlobalVariable(*current->module,
                    codegenType(idTy),
                    false,
                    linkage,
                    NULL,
                    id->getName()); //TODO: proper global insertion

            ASTValue *gv = new ASTValue(idTy, llvmval, true);
            id->setValue(gv); //TODO: is id declared across modules? should value only be set in local module?
        } else if(id->isFunction())
        {
            //TODO: declare func
        }
    }*/

    // declare functions prototypes
    for(int i = 0; i < inc->functions.size(); i++)
    {
        FunctionDeclaration *fdecl = inc->functions[i];

        if(current->functions.count(fdecl->getName())) continue;

        FunctionType *fty = (FunctionType*) codegenType(fdecl->prototype);
        Function *func = Function::Create(fty, Function::ExternalWeakLinkage,
                fdecl->getName(), current->module);

        current->functions[fdecl->getName()] = func;
    }
}

void IRCodegenContext::codegenTranslationUnit(IRTranslationUnit *u)
{
    this->unit = u;
    this->module = this->unit->module;
    this->debug = new IRDebug(this, u);

    pushScope(new IRScope(unit->unit->getScope(), debug->getCompileUnit()));

    for(int i = 0; i < unit->unit->imports.size(); i++) //TODO: import symbols.
    {
        codegenIncludeUnit(this->unit, unit->unit->importUnits[i]);
    }

    // alloc globals before codegen'ing functions
    std::vector<VariableDeclaration*>& globals = unit->getGlobals();
    for(int i = 0; i < globals.size(); i++)
    {
        Identifier *id = globals[i]->identifier;
        if(id->isVariable())
        {
            ASTType *idTy = id->getType();
            //llvm::Value *llvmval = module->getOrInsertGlobal(id->getName(), codegenType(idTy));
            GlobalValue::LinkageTypes linkage = globals[i]->external ?
                GlobalValue::ExternalLinkage : GlobalValue::ExternalLinkage;

            //TODO: correct type for global storage (esspecially pointers?)
            ASTValue *idValue = 0;
            if(globals[i]->value)
                idValue = codegenExpression(globals[i]->value);

            if(idTy->kind == TYPE_DYNAMIC)
            {
                if(!idValue)
                    emit_message(msg::FAILURE,
                            "attempt to codegen dynamically typed \
                            variable without properly assigned value", globals[i]->loc);
                idTy = idValue->getType();
                globals[i]->type = idTy;
            }

            llvm::Constant* gValue;
            if(globals[i]->external)
            {
                gValue = NULL;
            } else if(globals[i]->value)
            {
                gValue = (llvm::Constant*) codegenValue(
                            promoteType(codegenExpression(globals[i]->value), idTy)
                            );
            } else
            {
                gValue = (llvm::Constant*) llvm::Constant::getNullValue(codegenType(idTy));
            }

            GlobalVariable *llvmval = new GlobalVariable(*module,
                    codegenType(idTy),
                    false,
                    linkage,
                    gValue,
                    id->getName()); //TODO: proper global insertion

            ASTValue *gv = new ASTValue(idTy, llvmval, true);
            id->setValue(gv);

            dwarfStopPoint(globals[i]->loc);
            debug->createGlobal(globals[i], gv);

        } else if(id->isFunction())
        {
            //TODO: declare func?
        }
    }

    // declare functions prototypes
    std::vector<FunctionDeclaration*>& functions = unit->getFunctions();
    for(int i = 0; i < functions.size(); i++)
    {
        FunctionDeclaration *fdecl = functions[i];
        FunctionType *fty = (FunctionType*) codegenType(fdecl->prototype);
        Function *func;

        if(unit->functions.count(fdecl->getName())) continue; // XXX duplicate?

        //cout << "CG: " << fdecl->getName() << " : " << fdecl->identifier->getMangledName() << endl;
        func = Function::Create(fty, Function::ExternalLinkage, fdecl->getName(), module);

        unit->functions[fdecl->getName()] = func;
    }
        // codegen function bodys
    for(int i = 0; i < functions.size(); i++)
        codegenDeclaration(functions[i]);

    popScope();

    delete debug;
}

void IRCodegenContext::codegenPackage(Package *p)
{
    if(p->isTranslationUnit()) // leaf in package tree
    {
        std::string err;
        Module *m = new Module("", context);
        IRTranslationUnit *unit = new IRTranslationUnit((TranslationUnit*) p);
        unit->module = m;
        p->cgValue = 0;
        codegenTranslationUnit(unit);

        // XXX debug, output all modules
        std::string outputll = config.tempName + "/" + unit->unit->getName() + ".ll";
        raw_fd_ostream output(outputll.c_str(), err);
        m->print(output, 0);

        linker.linkInModule(m, (unsigned) Linker::DestroySource, &err);

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
