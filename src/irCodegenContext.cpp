
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
    llvm::Type* llvmty = NULL;

    if(unit->types.count(sti->identifier->getName()))
        return unit->types[sti->identifier->getName()];

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
        StructType *sty = StructType::create(context, sti->identifier->getName());
        sty->setBody(structVec);
        llvmty = sty;
    }
    else llvmty = Type::getInt8Ty(context); //allow fwd declared types, TODO: cleaner

    debug->createStructType(ty);

    unit->types[sti->identifier->getName()] = llvmty;
    return(llvm::Type*) llvmty;
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
            if(vd->getType()->align() > align)
            {
                alignedType = vd->type;
                align = vd->getType()->align();
            }

            if(vd->getType()->size() > size)
            {
                size = vd->getType()->size();
            }
            //unionVec.push_back(codegenType(vd->type));
        } else
            emit_message(msg::UNIMPLEMENTED, "this cant be declared in a union (yet?)");
    }

    if(sti->members.size())
    {
        unionVec.push_back(codegenType(alignedType));
        if(size - alignedType->size())
            unionVec.push_back(ArrayType::get(Type::getInt8Ty(context), size - alignedType->size()));
        StructType *sty = StructType::create(context, sti->identifier->getName());
        sty->setBody(unionVec);
        ty->cgType = sty;
    }
    else ty->cgType = Type::getInt8Ty(context); //allow fwd declared types, TODO: cleaner

    debug->createUnionType(ty);
    return(llvm::Type*) ty->cgType;
}

llvm::Type *IRCodegenContext::codegenTupleType(ASTType *ty)
{
    TupleTypeInfo *tti = dynamic_cast<TupleTypeInfo*>(ty->info);
    if(ty->type != TYPE_TUPLE || !tti)
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
        vector<Type*> members;
        members.push_back(codegenType(ati->arrayOf->getPointerTy()));
        members.push_back(codegenType(ASTType::getLongTy()));
        StructType *aty = StructType::create(context, ty->getName());
        aty->setBody(members);
        ty->cgType = aty;

        debug->createDynamicArrayType(ty);
    } else
    {
        llvm::ArrayType *aty = ArrayType::get(codegenType(ati->arrayOf), ati->size);
        ty->cgType = aty;

        debug->createArrayType(ty);
    }

    return (llvm::Type*) ty->cgType;
}

llvm::Type *IRCodegenContext::codegenType(ASTType *ty)
{
    llvm::Type *llvmty = NULL;
    //if(!ty->cgType)
    {
        if(ty->type == TYPE_UNKNOWN || ty->type == TYPE_UNKNOWN_USER)
        {
            Identifier* id = lookup(ty->getName());
            Declaration* decl = id->getDeclaration();
            if(TypeDeclaration* tdecl = dynamic_cast<TypeDeclaration*>(decl))
            {
                ty = tdecl->type;
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
        switch(ty->type)
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
                if(tmp->type == TYPE_VOID) tmp = ASTType::getCharTy();
                llvmty = codegenType(tmp)->getPointerTo();
                break;
            case TYPE_STRUCT:
                llvmty = codegenStructType(ty);
                break;
            case TYPE_UNION:
                llvmty = codegenUnionType(ty);
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
            default:
                emit_message(msg::FAILURE, "type not handled", currentLoc);
        }
    }

    return (llvm::Type *) llvmty;
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
    if(id->getValue()) return id->getValue();

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
        emit_message(msg::FAILURE, "failed to codegen identifier");
        return id->getValue(); //TODO

    //} else if(FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(id->getDeclaration()))
    } else if(id->isFunction())
    {
        //Value *llvmfunc = module->getFunction(id->getName());
        Value *llvmfunc = (Value*) ((FunctionDeclaration*) id->declaration)->cgValue;
        //TODO: proper function CGType (llvmfunc->getFunctionType())
        id->setValue(new ASTValue(NULL, llvmfunc));
    } else if(id->isStruct())
    {
        id->setValue(new ASTValue(id->declaredType(), NULL));
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
    if(BlockExpression *bexp = exp->blockExpression())
    {
        bool terminated = false;
        for(int i = 0; i < bexp->statements.size(); i++)
        {
            if(terminated && dynamic_cast<LabelStatement*>(bexp->statements[i])) terminated = false;

            if(!terminated)
                codegenStatement(bexp->statements[i]);

            if(dynamic_cast<GotoStatement*>(bexp->statements[i]) ||
                    dynamic_cast<ReturnStatement*>(bexp->statements[i]))
                terminated = true;
        }
        currentFunction.terminated = terminated;
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
    } else if(IfExpression *iexp = exp->ifExpression())
    {
        return codegenIfExpression(iexp);
    } else if(WhileExpression *wexp = exp->whileExpression())
    {
        return codegenWhileExpression(wexp);
    } else if(ForExpression *fexp = exp->forExpression())
    {
        return codegenForExpression(fexp);
    }else if(ImportExpression *iexp = exp->importExpression())
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
    if(exp->type->type == TYPE_DYNAMIC_ARRAY)
    {
        emit_message(msg::ERROR, "cannot created unsized array. meaningless alloaction", exp->loc);
        return NULL;
    }

    ASTType *ty = exp->type;
    vector<Value*> llargs;
    llargs.push_back(ConstantInt::get(codegenType(ASTType::getULongTy()), exp->type->size()));
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

    if(val->type->isArray() && val->type->type == TYPE_DYNAMIC_ARRAY)
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

ASTValue *IRCodegenContext::codegenIfExpression(IfExpression *exp)
{
    ASTValue *cond = codegenExpression(exp->condition);
    //ASTValue *zero = new ASTValue(ASTType::getIntTy(), ConstantInt::get(Type::getInt32Ty(context), 0));
    //codegenResolveBinaryTypes(&cond, &zero, 0);
    //ASTValue *icond = new ASTValue(ASTType::getBoolTy(), ir->CreateICmpNE(codegenValue(cond), codegenValue(zero)));
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
    if(!dynamic_cast<ReturnStatement*>(exp->body))
        ir->CreateBr(endif);

    ir->SetInsertPoint(onfalse);
    if(exp->elsebranch) codegenStatement(exp->elsebranch);
    ir->CreateBr(endif);

    ir->SetInsertPoint(endif);
    return NULL;
}

ASTValue *IRCodegenContext::codegenWhileExpression(WhileExpression *exp)
{
    llvm::BasicBlock *whileBB = BasicBlock::Create(context, "while_condition",
            ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *ontrue = BasicBlock::Create(context, "while_true",
            ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *onfalse = BasicBlock::Create(context, "while_false",
            ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *endwhile = BasicBlock::Create(context, "endwhile",
            ir->GetInsertBlock()->getParent());

    ir->CreateBr(whileBB);
    ir->SetInsertPoint(whileBB);
    ASTValue *cond = codegenExpression(exp->condition);
    ASTValue *icond = promoteType(cond, ASTType::getBoolTy());

    /*if(icond->getType() != ASTType::getBoolTy())
    {
        ASTValue *zero = new ASTValue(ASTType::getIntTy(), ConstantInt::get(Type::getInt32Ty(context), 0));
        codegenResolveBinaryTypes(&cond, &zero, 0);
        icond = new ASTValue(ASTType::getBoolTy(), ir->CreateICmpNE(codegenValue(cond), codegenValue(zero)));
    }*/
    ir->CreateCondBr(codegenValue(icond), ontrue, onfalse);

    this->breakLabel = endwhile;
    ir->SetInsertPoint(ontrue);
    codegenStatement(exp->body);
    ir->CreateBr(whileBB);

    ir->SetInsertPoint(onfalse);
    if(exp->elsebranch) codegenStatement(exp->elsebranch);
    ir->CreateBr(endwhile);

    //TODO: break label should be set to whatever it was previously, to allow for nested while
    this->breakLabel = NULL;

    ir->SetInsertPoint(endwhile);
    return NULL;
}

ASTValue *IRCodegenContext::codegenForExpression(ForExpression *exp)
{
    llvm::BasicBlock *forBB = BasicBlock::Create(context, "for_condition",
            ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *ontrue = BasicBlock::Create(context, "for_true",
            ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *onfalse = BasicBlock::Create(context, "for_false",
            ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *forupdate = BasicBlock::Create(context, "forupdate",
            ir->GetInsertBlock()->getParent());
    llvm::BasicBlock *endfor = BasicBlock::Create(context, "endfor",
            ir->GetInsertBlock()->getParent());

    if(exp->decl) codegenStatement(exp->decl);

    ir->CreateBr(forBB);
    ir->SetInsertPoint(forBB);

    if(exp->condition)
    {
        ASTValue *cond = codegenExpression(exp->condition);
        ASTValue *icond = promoteType(cond, ASTType::getBoolTy());
        ir->CreateCondBr(codegenValue(icond), ontrue, onfalse);
    } else ir->CreateBr(ontrue);

    BasicBlock *OLDBREAK = this->breakLabel; // TODO: ugly
    BasicBlock *OLDCONTINUE = this->continueLabel; //TODO: still ugly
    this->breakLabel = endfor;
    this->continueLabel = forupdate;
    ir->SetInsertPoint(ontrue);
    if(exp->body) codegenStatement(exp->body);
    ir->CreateBr(forupdate);
    ir->SetInsertPoint(forupdate);
    if(exp->update) codegenStatement(exp->update);
    ir->CreateBr(forBB);

    ir->SetInsertPoint(onfalse);
    if(exp->elsebranch) codegenStatement(exp->elsebranch);
    ir->CreateBr(endfor);
    this->breakLabel = OLDBREAK;
    this->continueLabel = OLDCONTINUE;

    ir->SetInsertPoint(endfor);
    return NULL;
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

    FunctionPrototype *ftype = NULL;
    ASTType *rtype = NULL;
    //TODO: very messy, should be able to get return value of function type
    if(IdentifierExpression *iexp = dynamic_cast<IdentifierExpression*>(exp->function))
    {
        // XXX messy, not always true
        FunctionDeclaration *fdecl = (FunctionDeclaration*) iexp->id->declaration;
        if(fdecl)
        {
            rtype = fdecl->prototype->returnType;
            ftype = fdecl->prototype;
        }
    } else emit_message(msg::FAILURE, "unknown function type?", exp->loc);

    if((ftype->parameters.size() != exp->args.size()) && !ftype->vararg)
    {
        emit_message(msg::ERROR, "invalid number of arguments provided for function call", exp->loc);
    }

    vector<ASTValue*> cargs;
    vector<Value*> llargs;
    for(int i = 0; i < exp->args.size(); i++)
    {
        ASTValue *val = codegenExpression(exp->args[i]);
        if(!ftype->vararg || (ftype->parameters.size() > i && ftype->parameters[i].first))
            val = promoteType(val, ftype->parameters[i].first);

        else if(ftype->vararg)
        {
            if(val->getType()->isFloating())
                val = promoteType(val, ASTType::getDoubleTy());
            else if(val->getType()->isInteger() && val->getType()->isSigned() &&
                    val->getType()->size() < ASTType::getIntTy()->size())
                val = promoteType(val, ASTType::getIntTy());
            else if(val->getType()->isInteger() &&
                    val->getType()->size() < ASTType::getIntTy()->size())
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
        if(arr->getType()->type == TYPE_DYNAMIC_ARRAY)
        {
            ASTType *indexedType = arr->getType()->getReferencedTy();
            Value *val = ir->CreateStructGEP(codegenLValue(arr), 0);
            val = ir->CreateLoad(val);
            val = ir->CreateInBoundsGEP(val, codegenValue(ind));
            return new ASTValue(indexedType, val, true);
        }  else if(arr->getType()->type == TYPE_ARRAY)
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
        } else if(arr->getType()->type == TYPE_TUPLE)
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
                                tyexp->type->size()));
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
                                lhs->getType()->size()));
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
            if(!lhs->getType()->isStruct() && !lhs->getType()->isArray() && !lhs->getType()->isUnion()) {
                emit_message(msg::ERROR, "can only index struct or array type", dexp->loc);
                return NULL;
            }

            if(lhs->getType()->isStruct())
            {
                StructTypeInfo *sti = (StructTypeInfo*) lhs->getType()->getTypeInfo();
                int offset = 0;
                for(int i = 0; i < sti->members.size(); i++)
                {
                    // XXX better way to compare equality?
                    if(sti->members[i]->identifier->getName() == dexp->rhs)
                    {
                        if(VariableDeclaration *vdecl =
                                dynamic_cast<VariableDeclaration*>(sti->members[i]))
                        {
                            //TODO proper struct GEP

                            ASTType *ty = vdecl->type;
                            std::vector<Value*> gep;
                            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
                            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), offset));
                            Value *llval = ir->CreateInBoundsGEP(codegenLValue(lhs), gep);
                            return new ASTValue(ty, llval, true);
                        } else emit_message(msg::UNIMPLEMENTED,
                                "this should not be in a struct (right now)", sti->members[i]->loc);
                    }
                    offset++;
                }
                emit_message(msg::ERROR, "member not in struct: " + sti->getName() + "." + dexp->rhs,
                        dexp->loc);
            } else if(lhs->getType()->isUnion())
            {
                UnionTypeInfo *sti = (UnionTypeInfo*) lhs->getType()->getTypeInfo();
                for(int i = 0; i < sti->members.size(); i++)
                {
                    // XXX better way to compare equality?
                    if(sti->members[i]->identifier->getName() == dexp->rhs)
                    {
                        if(VariableDeclaration *vdecl =
                                dynamic_cast<VariableDeclaration*>(sti->members[i]))
                        {
                            //TODO proper struct GEP

                            ASTType *ty = vdecl->type;
                            std::vector<Value*> gep;
                            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
                            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
                            Value *llval = ir->CreateInBoundsGEP(codegenLValue(lhs), gep);
                            llval = ir->CreateBitCast(llval, codegenType(ty)->getPointerTo());
                            return new ASTValue(ty, llval, true);
                        } else emit_message(msg::UNIMPLEMENTED,
                                "this should not be in a union (right now)", sti->members[i]->loc);
                    }
                }
                emit_message(msg::ERROR, "member not in struct", dexp->loc);
            } else if(lhs->getType()->type == TYPE_DYNAMIC_ARRAY)
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
            } else if(lhs->getType()->type == TYPE_ARRAY)
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
                            ConstantInt::get(Type::getInt64Ty(context), ati->size));
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
    if(toType->type == TYPE_STRUCT)
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
    } else if(toType->type == TYPE_TUPLE)
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
    } else if(toType->type == TYPE_ARRAY)
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
    } else if(toType->type == TYPE_DYNAMIC_ARRAY)
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
    if(val->type->type == TYPE_ARRAY)
    {
        if(toType->type == TYPE_POINTER)
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
    } else if(val->type->type == TYPE_DYNAMIC_ARRAY)
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
        else if(val->type->type == TYPE_TUPLE)
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
        if((*v2)->type->size() > (*v1)->type->size()) *v1 = promoteType(*v1, (*v2)->type);
        else *v2 = promoteType(*v2, (*v1)->type);
    }
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
            ASTType *ty = iexp->identifier()->declaredType();
            return promoteType(rhs, ty);
        } else emit_message(msg::ERROR, "need to cast to type", exp->loc);
    }

    ASTValue *lhs = codegenExpression(exp->lhs);
    ASTValue *rhs = codegenExpression(exp->rhs);
    if(!isAssignOp((tok::TokenKind) exp->op)) //XXX messy
        codegenResolveBinaryTypes(&lhs, &rhs, exp->op);
    else if(lhs->type->type == TYPE_ARRAY)
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
            if(!lhs->isLValue()){
                emit_message(msg::ERROR, "LHS must be LValue", exp->loc);
                return NULL;
            }
            retValue = rhs; //TODO: merge with decl assign
            break;

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
        retValue = promoteType(retValue, TYPE); //TODO: merge with decl assign
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
    } else if(LabelStatement *lstmt = dynamic_cast<LabelStatement*>(stmt))
    {
        ASTValue *lbl = codegenIdentifier(lstmt->identifier);
        llvm::BasicBlock *BB = (llvm::BasicBlock*) lbl->cgValue;
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
    } else if(BreakStatement *bstmt = dynamic_cast<BreakStatement*>(stmt))
    {
        if(!breakLabel){
            emit_message(msg::ERROR, "break doesnt make sense here!", stmt->loc);
            return;
        }
        ir->CreateBr(breakLabel);
        ir->SetInsertPoint(BasicBlock::Create(context, "", ir->GetInsertBlock()->getParent()));
    } else if(ContinueStatement *cstmt = dynamic_cast<ContinueStatement*>(stmt))
    {
        if(!continueLabel){
            emit_message(msg::ERROR, "continue doesnt make sense here!", stmt->loc);
            return;
        }

        ir->CreateBr(continueLabel);
        ir->SetInsertPoint(BasicBlock::Create(context, "", ir->GetInsertBlock()->getParent()));
    } else emit_message(msg::FAILURE, "i dont know what kind of statmeent this isssss", stmt->loc);

}

FunctionType *IRCodegenContext::codegenFunctionPrototype(FunctionPrototype *proto)
{
    ASTType *rty = proto->returnType;
    vector<Type*> params;
    for(int i = 0; i < proto->parameters.size(); i++)
    {
        params.push_back(codegenType(proto->parameters[i].first));
    }
    // XXX return, args, varargs
    FunctionType *fty = FunctionType::get(codegenType(rty), params, proto->vararg);
    return fty;
}

void IRCodegenContext::codegenDeclaration(Declaration *decl)
{
    if(FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(decl))
    {
        IRFunction backup = currentFunction;
        currentFunction = IRFunction(fdecl);
        //FunctionType *fty = codegenFunctionPrototype(fdecl->prototype);
        //Function *func = Function::Create(fty, Function::ExternalLinkage, fdecl->getName(), module);
        Function *func = module->getFunction(fdecl->getName());
        func->addFnAttr("no-frame-pointer-elim", "true");
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
            pushScope(fdecl->scope, fdecl->diSubprogram);
            dwarfStopPoint(decl->loc);

            int idx = 0;
            for(Function::arg_iterator AI = func->arg_begin(); AI != func->arg_end(); AI++, idx++)
            {
                if(fdecl->prototype->parameters.size() < idx){
                        emit_message(msg::FAILURE,
                                "argument counts dont seem to match up...", decl->loc);
                        return;
                }
                pair<ASTType*, std::string> param_i = fdecl->prototype->parameters[idx];
                AI->setName(param_i.second);
                AllocaInst *alloc = new AllocaInst(codegenType(param_i.first), 0, param_i.second, BB);
                    //ir->CreateAlloca(codegenType(param_i.first), 0, param_i.second);
                alloc->setAlignment(8);
                ASTValue *alloca = new ASTValue(param_i.first, alloc, true);
                new StoreInst(AI, codegenLValue(alloca), BB);
                //ir->CreateStore(AI, codegenLValue(alloca));

                Identifier *id = getInScope(param_i.second);
                id->setDeclaration(NULL, Identifier::ID_VARIABLE);
                id->setValue(alloca);

                //register debug params
                //XXX hacky with Instruction, and setDebugLoc manually
                Instruction *ainst = debug->createVariable(param_i.second, alloca, BB, decl->loc, idx+1);
                ainst->setDebugLoc(llvm::DebugLoc::get(decl->loc.line, decl->loc.ch, diScope()));
                //TODO: register value to scope
            }


            codegenStatement(fdecl->body);

            if(dynamic_cast<GotoStatement*>(fdecl->body) ||
                    dynamic_cast<ReturnStatement*>(fdecl->body))
                currentFunction.terminated = true;

            if(!currentFunction.terminated)
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

        if(vty->type == TYPE_DYNAMIC)
        {
            if(!defaultValue)
            {
                emit_message(msg::FAILURE,
                        "failure to codegen dynamic 'var' type default expression", vdecl->loc);
            }

            vty = defaultValue->getType();
            vdecl->type = vty;
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
                vty = id->declaredType();
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

        if(ArrayDeclaration *adecl = dynamic_cast<ArrayDeclaration*>(decl))
        {
            if(adecl->sz)
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
        }

    } else if(StructUnionDeclaration *sdecl = dynamic_cast<StructUnionDeclaration*>(decl))
    {
        //XXX should be generated in the Declaration stuff of the package?
        /*
        std::vector<Type*> arr;
        arr.push_back(Type::getInt32Ty(context));
        arr.push_back(Type::getInt32Ty(context));
        StructType *st = StructType::get(context, arr);
        //sdecl->identifier->setType(st);
        printf("CG struct decl\n");
        */
    }
}

void IRCodegenContext::codegenIncludeUnit(IRTranslationUnit *current, TranslationUnit *inc)
{
    /*
    for(int i = 0; i < unit->types.size(); i++) //XXX what about recursive types?
    {
        Declaration *decl = unit->types[i];
        codegenDeclaration(decl);
    }*/

    // alloc globals before codegen'ing functions
    for(int i = 0; i < inc->globals.size(); i++)
    {
        Identifier *id = inc->globals[i]->identifier;
        if(id->isVariable() && !id->getValue())
        {
            ASTType *idTy = id->getType();
            //llvm::Value *llvmval = module->getOrInsertGlobal(id->getName(), codegenType(idTy));
            GlobalValue::LinkageTypes linkage = inc->globals[i]->external ?
                GlobalValue::ExternalLinkage : GlobalValue::WeakAnyLinkage;
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
    }

    // declare functions prototypes
    for(int i = 0; i < inc->functions.size(); i++)
    {
        FunctionDeclaration *fdecl = inc->functions[i];
        FunctionType *fty = codegenFunctionPrototype(fdecl->prototype);
        fdecl->cgValue = Function::Create(fty,
                Function::ExternalWeakLinkage,
                fdecl->getName(), current->module);
    }
}

void IRCodegenContext::codegenTranslationUnit(IRTranslationUnit *u)
{
    this->unit = u;
    this->module = this->unit->module;
    this->debug = new IRDebug(this, u);

    pushScope(unit->getScope(), debug->diUnit); //TODO: debug
    //if(u->cgValue) return (llvm::Module*) u->cgValue; //XXX already codegend

    for(int i = 0; i < unit->unit->imports.size(); i++) //TODO: import symbols.
    {
        codegenIncludeUnit(this->unit, unit->unit->importUnits[i]);
    }

    /*
    for(int i = 0; i < unit->types.size(); i++) //XXX what about recursive types?
    {
        Declaration *decl = unit->types[i];
        codegenDeclaration(decl);
    }*/

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
                GlobalValue::ExternalLinkage : GlobalValue::WeakAnyLinkage;

            //TODO: correct type for global storage (esspecially pointers?)
            ASTValue *idValue = 0;
            if(globals[i]->value)
                idValue = codegenExpression(globals[i]->value);

            if(idTy->type == TYPE_DYNAMIC)
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
            //TODO: declare func
        }
    }

    // declare functions prototypes
    std::vector<FunctionDeclaration*>& functions = unit->getFunctions();
    for(int i = 0; i < functions.size(); i++)
    {
        FunctionDeclaration *fdecl = functions[i];
        FunctionType *fty = codegenFunctionPrototype(fdecl->prototype);
        if(fdecl->body)
        fdecl->cgValue = Function::Create(fty, Function::ExternalLinkage, fdecl->getName(), module);
        else
        fdecl->cgValue = Function::Create(fty, Function::ExternalWeakLinkage, fdecl->getName(), module);
    }
        // codegen function bodys
    for(int i = 0; i < functions.size(); i++)
        codegenDeclaration(functions[i]);

    popScope();

    //fprintf(stderr, "~~~~~~");
    //((llvm::Module*) u->cgValue)->dump();
    //fprintf(stderr, "~~~~~~");
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
        linker.linkInModule(m, (unsigned) Linker::DestroySource, &err);
    } else // generate all leaves ...
    {
        for(int i = 0; i < p->children.size(); i++)
        {
            codegenPackage(p->children[i]);
        }
    }
}

void IRCodegenContext::codegenAST(AST *ast, WLConfig config)
{
    codegenPackage(ast->getRootPackage());
    if(currentErrorLevel() > msg::WARNING)
    {
        emit_message(msg::OUTPUT, "compilation ended with errors");
        return;
    }

    createIdentMetadata(linker.getModule());
    linker.getModule()->MaterializeAll();

    if(verifyModule(*linker.getModule(), PrintMessageAction))
    {
        emit_message(msg::OUTPUT, "failed to compile source code");
        //return;
    } else
    {
        emit_message(msg::OUTPUT, "successfully compiled source");
    }

    std::string err;
    raw_fd_ostream output("output.ll", err);

    linker.getModule()->print(output, 0);
}

void IRCodegen(AST *ast, WLConfig config)
{
    IRCodegenContext context;
    context.codegenAST(ast, config);
}
