
#include "ast.hpp"
#include "token.hpp"
#include "irCodegenContext.hpp"

#include "message.hpp"

#include <fstream>
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace llvm;

void IRCodegenContext::dwarfStopPoint(int ln)
{
    llvm::DebugLoc loc = llvm::DebugLoc::get(ln, 1, diScope());
    assert_message(!loc.isUnknown(), msg::FAILURE, "unknown debug location");
    ir->SetCurrentDebugLocation(loc);
}

void IRCodegenContext::dwarfStopPoint(SourceLocation l)
{
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
        ty->cgType = sty;
    }
    else ty->cgType = Type::getInt8Ty(context); //allow fwd declared types, TODO: cleaner
    return(llvm::Type*) ty->cgType;
}

llvm::Type *IRCodegenContext::codegenType(ASTType *ty)
{
    if(!ty->cgType)
    {
        llvm::Type *llvmty = NULL;
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
                llvmty = codegenType(ty->getReferencedTy())->getPointerTo();
                break;
            case TYPE_STRUCT:
                llvmty = codegenStructType(ty);
                //TODO
                break;
            default:
                emit_message(msg::FAILURE, "type not handled");
        }
        ty->cgType = llvmty;
    }

    //XXX workaround for weird struct stuff (struct being declared without name)... very silly
    //if(ty->type == TYPE_STRUCT)
        //module->addTypeName(((StructTypeInfo*)ty->info)->identifier->getName(),(StructType*)ty->cgType);
        //((StructType*)ty->cgType)->setName(((StructTypeInfo*)ty->info)->identifier->getName());

    return (llvm::Type *) ty->cgType;
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
    assert_message(value->isLValue(), msg::FATAL, "rvalue used in lvalue context!");
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

ASTValue *IRCodegenContext::codegenExpression(Expression *exp)
{
    if(BlockExpression *bexp = exp->blockExpression())
    {
        for(int i = 0; i < bexp->statements.size(); i++)
        {
            codegenStatement(bexp->statements[i]);
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
        if(iexp->identifier()->isVariable())
        {
            //ASTValue *ptrvalue = iexp->identifier()->value; //TODO: load value?
            return iexp->identifier()->getValue();

        //} else if(FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(iexp->identifier()->getDeclaration()))
        } else if(iexp->identifier()->isFunction())
        {
            //Value *llvmfunc = module->getFunction(iexp->identifier()->getName());
            Value *llvmfunc = (Value*) ((FunctionDeclaration*) iexp->identifier()->declaration)->cgValue;
            //TODO: proper function CGType (llvmfunc->getFunctionType())
            return new ASTValue(NULL, llvmfunc); 
        } else if(iexp->identifier()->isStruct())
        {

        }
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
    }
    emit_message(msg::FAILURE, "bad expression?", exp->loc);
    return NULL; //TODO
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
    ir->CreateBr(onfalse);

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

    vector<ASTValue*> cargs;
    vector<Value*> llargs;
    for(int i = 0; i < exp->args.size(); i++)
    {
        ASTValue *val = codegenExpression(exp->args[i]);
        if(!ftype->vararg || ftype->parameters[i].first)
            val = promoteType(val, ftype->parameters[i].first);
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
    switch(exp->op)
    {
        case tok::plusplus:
            if(!lhs->isLValue()) {
                emit_message(msg::ERROR, "can only incrememt LValue", exp->loc);
                return NULL; 
            }

            val = new ASTValue(lhs->getType(),
                    ir->CreateAdd(codegenValue(lhs),
                        ConstantInt::get(codegenType(lhs->getType()), 1)));
            storeValue(lhs, val);
            return val;
        case tok::minusminus:
            if(!lhs->isLValue()) {
                emit_message(msg::ERROR, "can only decrement LValue", exp->loc);
                return NULL; 
            }
            val = new ASTValue(lhs->getType(),
                    ir->CreateSub(codegenValue(lhs),
                        ConstantInt::get(codegenType(lhs->getType()), 1)));
            storeValue(lhs, val);
            return val;
        case tok::plus:
            emit_message(msg::UNIMPLEMENTED, "unimplemented unary codegen (+)", exp->loc);
            return NULL;
        case tok::minus:
            emit_message(msg::UNIMPLEMENTED, "unimplemented unary codegen (-)", exp->loc);
            return NULL;
        case tok::tilde:
            emit_message(msg::UNIMPLEMENTED, "unimplemented unary codegen (~)", exp->loc);
            return NULL;
        case tok::bang:
            val = promoteType(lhs, ASTType::getBoolTy());
            return new ASTValue(ASTType::getBoolTy(), ir->CreateNot(codegenValue(val))); 
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
        if(arr->getType()->isArray())
        {
            //TODO
            emit_message(msg::UNIMPLEMENTED, 
                    "indexing arrays not implemented. try pointers", exp->loc);
        } else if(arr->getType()->isPointer())
        {
            //TODO: index array
            ASTType *indexedType = arr->getType()->getReferencedTy(); 
            Value *val = ir->CreateInBoundsGEP(codegenValue(arr), codegenValue(ind));
            return new ASTValue(indexedType, val, true);
        } else {
            emit_message(msg::ERROR, "attempt to index non-pointer/array type", exp->loc);
            return NULL;
        }
    } else if(PostfixOpExpression *e = dynamic_cast<PostfixOpExpression*>(exp))
    {
        ASTValue *old;
        ASTValue *val;
        ASTValue *lhs = codegenExpression(e->lhs);
        switch(e->op)
        {
            case tok::plusplus:
            old = loadValue(lhs);
            val = new ASTValue(lhs->getType(),
                    ir->CreateAdd(codegenValue(lhs),
                        ConstantInt::get(codegenType(lhs->getType()), 1)));
            storeValue(lhs, val);
            return old;
            case tok::minusminus:
            old = loadValue(lhs);
            val = new ASTValue(lhs->getType(),
                    ir->CreateSub(codegenValue(lhs),
                        ConstantInt::get(codegenType(lhs->getType()), 1)));
            storeValue(lhs, val);
            return old;
        }
    } else if(DotExpression *dexp = dynamic_cast<DotExpression*>(exp))
    {
            ASTValue *lhs = codegenExpression(dexp->lhs);

            if(lhs->getType()->isPointer()) lhs = new ASTValue(lhs->getType()->getReferencedTy(), 
                    codegenValue(lhs), true);
            if(!lhs->getType()->isStruct()) {
                emit_message(msg::ERROR, "can only index struct type", dexp->loc);
                return NULL;
            }
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
            emit_message(msg::ERROR, "member not in struct", dexp->loc);
        return NULL;
    }

    emit_message(msg::UNIMPLEMENTED, "postfix codegen unimpl", exp->loc);
    return NULL;
}


ASTValue *IRCodegenContext::promoteType(ASTValue *val, ASTType *toType)
{
    if(val->type != toType)
    {
        if(val->type->isInteger())
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
                return new ASTValue(toType, ir->CreateUIToFP(codegenValue(val),
                            codegenType(toType)), false);
                return new ASTValue(toType, ir->CreateSIToFP(codegenValue(val), 
                            codegenType(toType)), false);
            }
        } else if(val->type->isFloating())
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
        if(val->type->isPointer())
        {
            if(toType->isPointer())
            {
                return new ASTValue(toType, 
                        ir->CreatePointerCast(codegenValue(val), codegenType(toType)));
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
        if((*v1)->type->size() > (*v2)->type->size()) *v2 = promoteType(*v2, (*v1)->type);
        else if((*v2)->type->size() > (*v1)->type->size()) *v1 = promoteType(*v1, (*v2)->type);
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
    if(exp->op != tok::equal) //XXX messy
        codegenResolveBinaryTypes(&lhs, &rhs, exp->op);

    llvm::Value *val;
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
            rhs = promoteType(rhs, TYPE); //TODO: merge with decl assign
            storeValue(lhs, rhs);
            return rhs;

        // I dont know, do something with a comma eventually
        case tok::comma:
                emit_message(msg::UNIMPLEMENTED, "unimpl binop", exp->loc);
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
            val = ir->CreateOr(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(TYPE, val);
        case tok::caret:
            val = ir->CreateXor(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(TYPE, val);
        case tok::amp:
            val = ir->CreateAnd(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(TYPE, val);

        // COMPARE OPS
        case tok::equalequal:
                if(TYPE->isFloating())
                    val = ir->CreateFCmp(CmpInst::FCMP_OEQ, lhs_val, rhs_val);
                else // sign not required, irrelivant for equality
                    val = ir->CreateICmp(CmpInst::ICMP_EQ, lhs_val, rhs_val);
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
            if(TYPE->isFloating())
                val = ir->CreateFAdd(lhs_val, rhs_val);
            else
                val = ir->CreateAdd(lhs_val, rhs_val);
            return new ASTValue(TYPE, val); //TODO: proper typing (for all below too)

        case tok::minus:
            if(TYPE->isFloating())
                val = ir->CreateFSub(lhs_val, rhs_val);
            else
                val = ir->CreateSub(lhs_val, rhs_val);
            return new ASTValue(TYPE, val);

        case tok::star:
            if(TYPE->isFloating())
                val = ir->CreateFMul(lhs_val, rhs_val);
            else //TODO: signed?
                val = ir->CreateMul(lhs_val, rhs_val);
            return new ASTValue(TYPE, val);

        case tok::slash:
            if(TYPE->isFloating())
                val = ir->CreateFDiv(lhs_val, rhs_val);
            else if(TYPE->isSigned())
                val = ir->CreateSDiv(lhs_val, rhs_val);
            else
                val = ir->CreateUDiv(lhs_val, rhs_val);
            return new ASTValue(TYPE, val);

        case tok::percent:
            if(TYPE->isFloating())
                val = ir->CreateFRem(lhs_val, rhs_val);
            else if(TYPE->isSigned())
                val = ir->CreateSRem(lhs_val, rhs_val);
            else
                val = ir->CreateURem(lhs_val, rhs_val);
            return new ASTValue(TYPE, val);

        case tok::starstar:
        default:
            emit_message(msg::UNIMPLEMENTED, "unimplemented operator", exp->loc);
            return NULL; //XXX: null val
    }

#undef lhs_val
#undef rhs_val
}

void IRCodegenContext::codegenReturnStatement(ReturnStatement *exp)
{
    if(exp->expression)
    {
        ASTValue *value = codegenExpression(exp->expression);
        ir->CreateRet(codegenValue(value));
        //TODO: cast to proper type
        return;
    }
    ir->CreateRetVoid();
    //return value;
}

void IRCodegenContext::codegenStatement(Statement *stmt)
{
    if(!stmt) return;
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
        if(!lstmt->identifier->getValue())
            lstmt->identifier->setValue(new ASTValue(NULL, BasicBlock::Create(context, 
                            lstmt->identifier->getName(), ir->GetInsertBlock()->getParent())));
        llvm::BasicBlock *BB = (llvm::BasicBlock*) lstmt->identifier->getValue()->cgValue;
        ir->CreateBr(BB);
        ir->SetInsertPoint(BB);
        lstmt->identifier->setValue(new ASTValue(NULL, BB)); //TODO: cg value?
    } else if(GotoStatement *gstmt = dynamic_cast<GotoStatement*>(stmt))
    {
        if(!gstmt->identifier->getValue())
            gstmt->identifier->setValue(new ASTValue(NULL, BasicBlock::Create(context, 
                            gstmt->identifier->getName(), ir->GetInsertBlock()->getParent())));
        llvm::BasicBlock *BB = (llvm::BasicBlock*) gstmt->identifier->getValue()->cgValue;
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
        //FunctionType *fty = codegenFunctionPrototype(fdecl->prototype);
        //Function *func = Function::Create(fty, Function::ExternalLinkage, fdecl->getName(), module);
        Function *func = module->getFunction(fdecl->getName());
        func->addFnAttr("no-frame-pointer-elim", "true");
        if(fdecl->body)
        {
            BasicBlock *BB = BasicBlock::Create(context, "entry", func);
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
                alloc->setAlignment(4);
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

            if(func->getReturnType() == Type::getVoidTy(context))
            {
                ir->CreateRetVoid();
            }

            popScope();
        }
    } else if(VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(decl))
    {
        dwarfStopPoint(vdecl->loc);
        ASTType *vty = vdecl->type;
        AllocaInst *llvmDecl = ir->CreateAlloca(codegenType(vty), 0, vdecl->getName());
        llvmDecl->setAlignment(4);

        //XXX note that we are storing the alloca(pointer) to the variable in the CGValue
        ASTValue *idValue = new ASTValue(vty, llvmDecl, true); 
        if(vdecl->value)
        {
            ASTValue *defaultValue = codegenExpression(vdecl->value);
            //promoteType(defaultValue, vty);
            //ir->CreateStore(codegenValue(defaultValue), llvmDecl);
            defaultValue = promoteType(defaultValue, vty);
            storeValue(idValue, defaultValue);

            Instruction *vinst = debug->createVariable(vdecl->getName(), 
                    idValue, ir->GetInsertBlock(), vdecl->loc);
            vinst->setDebugLoc(llvm::DebugLoc::get(decl->loc.line, decl->loc.ch, diScope()));
            //TODO: maybe create a LValue field in CGValue?
        }
        vdecl->identifier->setValue(idValue);
    } else if(StructDeclaration *sdecl = dynamic_cast<StructDeclaration*>(decl))
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

void IRCodegenContext::codegenIncludeUnit(TranslationUnit *current, TranslationUnit *inc)
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
            GlobalValue::LinkageTypes linkage = GlobalValue::WeakAnyLinkage;
            GlobalVariable *llvmval = new GlobalVariable(*(Module*)current->cgValue,
                    codegenType(idTy),
                    false,
                    linkage,
                    (llvm::Constant*) NULL,
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
                fdecl->getName(), (Module*)current->cgValue);
    }
}

void IRCodegenContext::codegenTranslationUnit(TranslationUnit *u)
{
    this->unit = u; //TODO: revert to old tunit once done?
    this->module = (Module*) u->cgValue;
    this->debug = new IRDebug(this, u);

    pushScope(unit->scope, debug->diUnit); //TODO: debug
    //if(u->cgValue) return (llvm::Module*) u->cgValue; //XXX already codegend

    for(int i = 0; i < unit->imports.size(); i++) //TODO: import symbols.
    {
        codegenIncludeUnit(u, unit->importUnits[i]);
    }

    /*
    for(int i = 0; i < unit->types.size(); i++) //XXX what about recursive types?
    {
        Declaration *decl = unit->types[i];
        codegenDeclaration(decl);
    }*/

    // alloc globals before codegen'ing functions
    for(int i = 0; i < unit->globals.size(); i++)
    {
        Identifier *id = unit->globals[i]->identifier;
        if(id->isVariable())
        {
            ASTType *idTy = id->getType();
            //llvm::Value *llvmval = module->getOrInsertGlobal(id->getName(), codegenType(idTy));
            GlobalValue::LinkageTypes linkage = unit->globals[i]->external ? GlobalValue::ExternalWeakLinkage : GlobalValue::WeakAnyLinkage;
            GlobalVariable *llvmval = new GlobalVariable(*module,
                    codegenType(idTy),
                    false,
                    linkage,
                    (llvm::Constant*) (unit->globals[i]->value ? codegenValue(codegenExpression(unit->globals[i]->value)) : 0),
                    id->getName()); //TODO: proper global insertion

            ASTValue *gv = new ASTValue(idTy, llvmval, true);
            id->setValue(gv);

            dwarfStopPoint(unit->globals[i]->loc);
            debug->createGlobal(unit->globals[i], gv);

        } else if(id->isFunction())
        {
            //TODO: declare func
        }
    }

    // declare functions prototypes
    for(int i = 0; i < unit->functions.size(); i++)
    {
        FunctionDeclaration *fdecl = unit->functions[i];
        FunctionType *fty = codegenFunctionPrototype(fdecl->prototype);
        if(fdecl->body)
        fdecl->cgValue = Function::Create(fty, Function::ExternalLinkage, fdecl->getName(), module);
        else
        fdecl->cgValue = Function::Create(fty, Function::ExternalWeakLinkage, fdecl->getName(), module);
    }
        // codegen function bodys
    for(int i = 0; i < unit->functions.size(); i++)
        codegenDeclaration(unit->functions[i]);

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
        p->cgValue = m;
        codegenTranslationUnit((TranslationUnit *) p);
        linker.linkInModule(m, (unsigned) Linker::DestroySource, &err);
    } else // generate all leaves ...
    {
        for(int i = 0; i < p->children.size(); i++)
        {
            codegenPackage(p->children[i]);
        }
    }
}

void IRCodegenContext::codegenAST(AST *ast)
{
    codegenPackage(ast->getRootPackage());
    if(currentErrorLevel() > msg::WARNING)
    {
        emit_message(msg::OUTPUT, "compilation ended with errors");
        return;
    }

    createIdentMetadata(linker.getModule());
    linker.getModule()->MaterializeAll();
    linker.getModule()->dump();
}

void IRCodegen(AST *ast)
{
    IRCodegenContext context;
    context.codegenAST(ast);
}
