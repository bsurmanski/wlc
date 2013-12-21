
#include "ast.hpp"
#include "token.hpp"
#include "irCodegenContext.hpp"

#include <fstream>
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace llvm;


llvm::Type *IRCodegenContext::codegenStructType(ASTType *ty)
{
    assert(ty->isStruct() && "must be struct");
    StructTypeInfo *sti = (StructTypeInfo*) ty->info;

    std::vector<Type*> structVec;
    for(int i = 0; i < sti->members.size(); i++)
    {
        if(VariableDeclaration *vd = dynamic_cast<VariableDeclaration*>(sti->members[i]))
        {
            structVec.push_back(codegenType(vd->type));
        } else { assert(false && "this cant be declared in a struct (yet?)"); }
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
                break;
            default:
                assert(false && "type not handled");
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
    if(!value->cgValue)
    {
        //TODO
        assert(false && "gen value...");
    }

    if(value->isLValue())
    {
        return builder.CreateAlignedLoad(codegenLValue(value), 4);
    }

    return (llvm::Value *) value->cgValue;
}

llvm::Value *IRCodegenContext::codegenLValue(ASTValue *value)
{
    assert(value->isLValue() && "rvalue used in lvalue context!");
    if(!value->cgValue)
    {
        //TODO
        assert(false && "gen value...");
    }

    return (llvm::Value*) value->cgValue;
}

ASTValue *IRCodegenContext::storeValue(ASTValue *dest, ASTValue *val)
{
    ASTValue *stored = new ASTValue(dest->type, 
            builder.CreateStore(codegenValue(val), codegenLValue(dest)));
    return stored;
}

ASTValue *IRCodegenContext::loadValue(ASTValue *lval)
{
    assert(lval->isLValue() && "must be lvalue to load");
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
                llvmval = ConstantInt::get(Type::getInt32Ty(context), nexp->intValue);
                ty = ASTType::getIntTy();
                return new ASTValue(ty, llvmval); //TODO: assign
            case NumericExpression::DOUBLE:
                llvmval = ConstantFP::get(Type::getDoubleTy(context), nexp->floatValue);
                ty = ASTType::getDoubleTy();
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
            iexp->id = getScope()->lookup(iexp->id->getName());
            //TODO
            assert(!iexp->id->isUndeclared() && "undeclared variable in this scope!");
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
    }
    assert(false && "bad expression?");
    return NULL; //TODO
}

ASTValue *IRCodegenContext::codegenIfExpression(IfExpression *exp)
{
    ASTValue *cond = codegenExpression(exp->condition);
    //ASTValue *zero = new ASTValue(ASTType::getIntTy(), ConstantInt::get(Type::getInt32Ty(context), 0));
    //codegenResolveBinaryTypes(&cond, &zero, 0);
    //ASTValue *icond = new ASTValue(ASTType::getBoolTy(), builder.CreateICmpNE(codegenValue(cond), codegenValue(zero)));
    ASTValue *icond = promoteType(cond, ASTType::getBoolTy());
    llvm::BasicBlock *ontrue = BasicBlock::Create(context, "true", 
            builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *onfalse = BasicBlock::Create(context, "false", 
            builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *endif = BasicBlock::Create(context, "endif", 
            builder.GetInsertBlock()->getParent());
    builder.CreateCondBr(codegenValue(icond), ontrue, onfalse);

    builder.SetInsertPoint(ontrue);
    codegenStatement(exp->body);
    builder.CreateBr(onfalse);

    builder.SetInsertPoint(onfalse);
    if(exp->elsebranch) codegenStatement(exp->elsebranch);
    builder.CreateBr(endif);

    builder.SetInsertPoint(endif);
    return NULL;
}

ASTValue *IRCodegenContext::codegenWhileExpression(WhileExpression *exp)
{
    llvm::BasicBlock *whileBB = BasicBlock::Create(context, "while_condition", 
            builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *ontrue = BasicBlock::Create(context, "while_true", 
            builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *onfalse = BasicBlock::Create(context, "while_false", 
            builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *endwhile = BasicBlock::Create(context, "endwhile", 
            builder.GetInsertBlock()->getParent());

    builder.CreateBr(whileBB);
    builder.SetInsertPoint(whileBB);
    ASTValue *cond = codegenExpression(exp->condition);
    ASTValue *icond = promoteType(cond, ASTType::getBoolTy());

    /*if(icond->getType() != ASTType::getBoolTy())
    {
        ASTValue *zero = new ASTValue(ASTType::getIntTy(), ConstantInt::get(Type::getInt32Ty(context), 0));
        codegenResolveBinaryTypes(&cond, &zero, 0);
        icond = new ASTValue(ASTType::getBoolTy(), builder.CreateICmpNE(codegenValue(cond), codegenValue(zero)));
    }*/
    builder.CreateCondBr(codegenValue(icond), ontrue, onfalse);

    this->breakLabel = endwhile;
    builder.SetInsertPoint(ontrue);
    codegenStatement(exp->body);
    builder.CreateBr(whileBB);

    builder.SetInsertPoint(onfalse);
    if(exp->elsebranch) codegenStatement(exp->elsebranch);
    builder.CreateBr(endwhile);

    //TODO: break label should be set to whatever it was previously, to allow for nested while
    this->breakLabel = NULL; 

    builder.SetInsertPoint(endwhile);
    return NULL;
}

ASTValue *IRCodegenContext::codegenForExpression(ForExpression *exp)
{
    llvm::BasicBlock *forBB = BasicBlock::Create(context, "for_condition", 
            builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *ontrue = BasicBlock::Create(context, "for_true", 
            builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *onfalse = BasicBlock::Create(context, "for_false", 
            builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *forupdate = BasicBlock::Create(context, "forupdate", 
            builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *endfor = BasicBlock::Create(context, "endfor", 
            builder.GetInsertBlock()->getParent());

    if(exp->decl) codegenStatement(exp->decl);

    builder.CreateBr(forBB);
    builder.SetInsertPoint(forBB);

    if(exp->condition)
    {
        ASTValue *cond = codegenExpression(exp->condition);
        ASTValue *icond = promoteType(cond, ASTType::getBoolTy());
        builder.CreateCondBr(codegenValue(icond), ontrue, onfalse);
    } else builder.CreateBr(ontrue);

    BasicBlock *OLDBREAK = this->breakLabel; // TODO: ugly
    BasicBlock *OLDCONTINUE = this->continueLabel; //TODO: still ugly
    this->breakLabel = endfor;
    this->continueLabel = forupdate;
    builder.SetInsertPoint(ontrue);
    if(exp->body) codegenStatement(exp->body);
    builder.CreateBr(forupdate);
    builder.SetInsertPoint(forupdate);
    if(exp->update) codegenStatement(exp->update);
    builder.CreateBr(forBB);

    builder.SetInsertPoint(onfalse);
    if(exp->elsebranch) codegenStatement(exp->elsebranch);
    builder.CreateBr(endfor);
    this->breakLabel = OLDBREAK;
    this->continueLabel = OLDCONTINUE;

    builder.SetInsertPoint(endfor);
    return NULL;
}

ASTValue *IRCodegenContext::codegenCallExpression(CallExpression *exp)
{
    ASTValue *func = codegenExpression(exp->function);
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
    }

    vector<ASTValue*> cargs;
    vector<Value*> llargs;
    for(int i = 0; i < exp->args.size(); i++)
    {
        ASTValue *val = codegenExpression(exp->args[i]);
        //XXX promote value?
        cargs.push_back(promoteType(val, ftype->parameters[i].first));
        llargs.push_back(codegenValue(cargs[i]));
    }
    llvm::Value *value = builder.CreateCall(codegenValue(func), llargs);
    return new ASTValue(rtype, value); //TODO
}

ASTValue *IRCodegenContext::codegenUnaryExpression(UnaryExpression *exp)
{
    ASTValue *lhs = codegenExpression(exp->lhs); // expression after unary op: eg in !a, lhs=a

    ASTValue *val;
    switch(exp->op)
    {
        case tok::plusplus:
            assert(lhs->isLValue() && "can only increment LValue");
            val = new ASTValue(lhs->getType(),
                    builder.CreateAdd(codegenValue(lhs),
                        ConstantInt::get(codegenType(lhs->getType()), 1)));
            storeValue(lhs, val);
            return val;
        case tok::minusminus:
            assert(lhs->isLValue() && "can only decrement LValue");
            val = new ASTValue(lhs->getType(),
                    builder.CreateSub(codegenValue(lhs),
                        ConstantInt::get(codegenType(lhs->getType()), 1)));
            storeValue(lhs, val);
            return val;
        case tok::plus:
        case tok::minus:
        case tok::tilde:
            assert(false && "unimpl unary codegen");
        case tok::bang:
            val = promoteType(lhs, ASTType::getBoolTy());
            return new ASTValue(ASTType::getBoolTy(), builder.CreateNot(codegenValue(val))); 
        case tok::caret:
            assert(lhs->getType()->isPointer() && "can only dereference pointer type");
            return new ASTValue(lhs->getType()->getReferencedTy(), codegenValue(lhs), true);
        case tok::amp:
            assert(lhs->isLValue() && "can only take reference of lvalue");
            return new ASTValue(lhs->getType()->getPointerTy(), codegenLValue(lhs), false);
        default:
            assert(false && "unary codegen unimpl");
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
            assert(false && "indexing array not impl. try pointers...");
        } else if(arr->getType()->isPointer())
        {
            //TODO: index array
            ASTType *indexedType = arr->getType()->getReferencedTy(); 
            Value *val = builder.CreateInBoundsGEP(codegenValue(arr), codegenValue(ind));
            val = builder.CreateLoad(val);
            return new ASTValue(indexedType, val);
        } else assert(false && "attempt to index non-pointer/array type");
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
                    builder.CreateAdd(codegenValue(lhs),
                        ConstantInt::get(codegenType(lhs->getType()), 1)));
            storeValue(lhs, val);
            return old;
            case tok::minusminus:
            old = loadValue(lhs);
            val = new ASTValue(lhs->getType(),
                    builder.CreateSub(codegenValue(lhs),
                        ConstantInt::get(codegenType(lhs->getType()), 1)));
            storeValue(lhs, val);
            return old;
        }
    }

    assert(false && "postfix codegen unimpl");
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
                return new ASTValue(ASTType::getBoolTy(), builder.CreateICmpNE(codegenValue(val),
                            codegenValue(&zero)));
            }

            if(toType->isInteger())
            {
                return new ASTValue(toType, builder.CreateIntCast(codegenValue(val), 
                            codegenType(toType), false)); //TODO: signedness
            }
            if(toType->isPointer())
            {
                return new ASTValue(toType, builder.CreatePointerCast(codegenValue(val), codegenType(toType)));
            }
        } else if(val->type->isFloating())
        {
            if(toType->isFloating())
            {
                return new ASTValue(toType, builder.CreateFPCast(codegenValue(val), codegenType(toType)));
            } else if(toType->isInteger())
            {
                return new ASTValue(toType, builder.CreateFPToUI(codegenValue(val), 
                            codegenType(toType)));
            }
        }
        if(val->type->isPointer())
        {
            if(toType->isPointer())
            {
                return new ASTValue(toType, 
                        builder.CreatePointerCast(codegenValue(val), codegenType(toType)));
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
            assert(false && "cannot convert structs (yet)");
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
            ASTValue *lhs = codegenExpression(exp->lhs);

            if(IdentifierExpression *rexp = dynamic_cast<IdentifierExpression*>(exp->rhs))
            {
                if(lhs->getType()->isPointer()) lhs = new ASTValue(lhs->getType()->getReferencedTy(), 
                        codegenValue(lhs), true);
                assert(lhs->getType()->isStruct() && "can only index struct!");
                StructTypeInfo *sti = (StructTypeInfo*) lhs->getType()->getTypeInfo();
                int offset = 0;
                for(int i = 0; i < sti->members.size(); i++)
                {
                    // XXX better way to compare equality?
                    if(sti->members[i]->identifier->getName() == rexp->identifier()->getName())
                    {
                        if(VariableDeclaration *vdecl = 
                                dynamic_cast<VariableDeclaration*>(sti->members[i]))
                        {
                            //TODO proper struct GEP

                            ASTType *ty = vdecl->type;
                            std::vector<Value*> gep;
                            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
                            gep.push_back(ConstantInt::get(Type::getInt32Ty(context), offset));
                            Value *llval = builder.CreateInBoundsGEP(codegenLValue(lhs), gep);
                            return new ASTValue(ty, llval, true);
                        } else assert(false && "this should not be in a struct (right now)");
                    }
                    offset++;
                }
            } else assert(false && "rhs must be identifier");
            assert(false && "member not in struct");
        return NULL;
    } else if(exp->op == tok::colon) //cast op
    {
        ASTValue *rhs = codegenExpression(exp->rhs);
        if(IdentifierExpression *iexp = dynamic_cast<IdentifierExpression*>(exp->lhs))
        {
            ASTType *ty = iexp->identifier()->declaredType();
            return promoteType(rhs, ty);
        } else assert(false && "need to cast to type");
    }

    ASTValue *lhs = codegenExpression(exp->lhs);
    ASTValue *rhs = codegenExpression(exp->rhs);
    if(!exp->op != tok::equal) //XXX
        codegenResolveBinaryTypes(&lhs, &rhs, exp->op);

    llvm::Value *val;
#define lhs_val codegenValue(lhs)
#define rhs_val codegenValue(rhs)

    ASTType *TYPE = lhs->getType();
    switch(exp->op)
    {
        //ASSIGN
        case tok::equal:
            if(!lhs->isLValue()) assert(false && "LHS must be LValue");
            rhs = promoteType(rhs, TYPE); //TODO: merge with decl assign
            storeValue(lhs, rhs);
            return rhs;

        // I dont know, do something with a comma eventually
        case tok::comma:
                assert(false && "unimpl binop");

        // LOGIC OPS
        case tok::barbar:
        case tok::kw_or:
            TYPE = ASTType::getBoolTy();
            lhs = promoteType(lhs, TYPE);
            rhs = promoteType(rhs, TYPE);
            val = builder.CreateOr(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(TYPE, val);
        case tok::ampamp:
        case tok::kw_and:
            TYPE = ASTType::getBoolTy();
            lhs = promoteType(lhs, TYPE);
            rhs = promoteType(rhs, TYPE);
            val = builder.CreateAnd(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(TYPE, val);

        // BITWISE OPS
        case tok::bar:
            val = builder.CreateOr(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(TYPE, val);
        case tok::caret:
            val = builder.CreateXor(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(TYPE, val);
        case tok::amp:
            val = builder.CreateAnd(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(TYPE, val);

        // COMPARE OPS
        case tok::equalequal:
                if(TYPE->isFloating())
                    val = builder.CreateFCmp(CmpInst::FCMP_OEQ, lhs_val, rhs_val);
                else // sign not required, irrelivant for equality
                    val = builder.CreateICmp(CmpInst::ICMP_EQ, lhs_val, rhs_val);
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::less:
                if(TYPE->isFloating())
                    val = builder.CreateFCmpOLT(lhs_val, rhs_val);
                else if(TYPE->isSigned())
                    val = builder.CreateICmpSLT(lhs_val, rhs_val);
                else
                    val = builder.CreateICmpULT(lhs_val, rhs_val);
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::lessequal:
                if(TYPE->isFloating())
                    val = builder.CreateFCmpOLE(lhs_val, rhs_val);
                else if(TYPE->isSigned())
                    val = builder.CreateICmpSLE(lhs_val, rhs_val);
                else
                    val = builder.CreateICmpULE(lhs_val, rhs_val);
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::greater:
                if(TYPE->isFloating())
                    val = builder.CreateFCmpOGT(lhs_val, rhs_val);
                else if(TYPE->isSigned())
                    val = builder.CreateICmpSGT(lhs_val, rhs_val);
                else
                    val = builder.CreateICmpUGT(lhs_val, rhs_val);
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::greaterequal:
                if(TYPE->isFloating())
                    val = builder.CreateFCmpOGE(lhs_val, rhs_val);
                else if(TYPE->isSigned())
                    val = builder.CreateICmpSGE(lhs_val, rhs_val);
                else
                    val = builder.CreateICmpUGE(lhs_val, rhs_val);
                return new ASTValue(ASTType::getBoolTy(), val);

        // ARITHMETIC OPS
        case tok::plus:
            if(TYPE->isFloating())
                val = builder.CreateFAdd(lhs_val, rhs_val);
            else
                val = builder.CreateAdd(lhs_val, rhs_val);
            return new ASTValue(TYPE, val); //TODO: proper typing (for all below too)

        case tok::minus:
            if(TYPE->isFloating())
                val = builder.CreateFSub(lhs_val, rhs_val);
            else
                val = builder.CreateSub(lhs_val, rhs_val);
            return new ASTValue(TYPE, val);

        case tok::star:
            if(TYPE->isFloating())
                val = builder.CreateFMul(lhs_val, rhs_val);
            else //TODO: signed?
                val = builder.CreateMul(lhs_val, rhs_val);
            return new ASTValue(TYPE, val);

        case tok::slash:
            if(TYPE->isFloating())
                val = builder.CreateFDiv(lhs_val, rhs_val);
            else if(TYPE->isSigned())
                val = builder.CreateSDiv(lhs_val, rhs_val);
            else
                val = builder.CreateUDiv(lhs_val, rhs_val);
            return new ASTValue(TYPE, val);

        case tok::percent:
            if(TYPE->isFloating())
                val = builder.CreateFRem(lhs_val, rhs_val);
            else if(TYPE->isSigned())
                val = builder.CreateSRem(lhs_val, rhs_val);
            else
                val = builder.CreateURem(lhs_val, rhs_val);
            return new ASTValue(TYPE, val);

        case tok::starstar:
        default:
            assert(false && "unimpl");
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
        builder.CreateRet(codegenValue(value));
        return;
    }
    builder.CreateRetVoid();
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
                            lstmt->identifier->getName(), builder.GetInsertBlock()->getParent())));
        llvm::BasicBlock *BB = (llvm::BasicBlock*) lstmt->identifier->getValue()->cgValue;
        builder.CreateBr(BB);
        builder.SetInsertPoint(BB);
        lstmt->identifier->setValue(new ASTValue(NULL, BB)); //TODO: cg value?
    } else if(GotoStatement *gstmt = dynamic_cast<GotoStatement*>(stmt))
    {
        if(!gstmt->identifier->getValue())
            gstmt->identifier->setValue(new ASTValue(NULL, BasicBlock::Create(context, 
                            gstmt->identifier->getName(), builder.GetInsertBlock()->getParent())));
        llvm::BasicBlock *BB = (llvm::BasicBlock*) gstmt->identifier->getValue()->cgValue;
        builder.CreateBr(BB);
       // post GOTO block 
        BasicBlock *PG = BasicBlock::Create(context, "", builder.GetInsertBlock()->getParent()); 
        builder.SetInsertPoint(PG);
    } else if(BreakStatement *bstmt = dynamic_cast<BreakStatement*>(stmt))
    {
        assert(breakLabel && "break doesnt make sense here!");
        builder.CreateBr(breakLabel);
        builder.SetInsertPoint(BasicBlock::Create(context, "", builder.GetInsertBlock()->getParent()));
    } else if(ContinueStatement *cstmt = dynamic_cast<ContinueStatement*>(stmt))
    {
        assert(continueLabel && "continue doesnt make sense here!");
        builder.CreateBr(continueLabel);
        builder.SetInsertPoint(BasicBlock::Create(context, "", builder.GetInsertBlock()->getParent()));
    } else assert(false && "i dont know what kind of statmeent this isssss");

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
        if(fdecl->body)
        {
            BasicBlock *BB = BasicBlock::Create(context, "entry", func);
            builder.SetInsertPoint(BB);

            pushScope(fdecl->scope);

            int idx = 0;
            for(Function::arg_iterator AI = func->arg_begin(); AI != func->arg_end(); AI++, idx++)
            {
                assert(fdecl->prototype->parameters.size() >= idx && 
                        "argument counts dont seem to match up...");
                pair<ASTType*, std::string> param_i = fdecl->prototype->parameters[idx];
                AI->setName(param_i.second);
                AllocaInst *alloc = builder.CreateAlloca(codegenType(param_i.first), 0, param_i.second);
                alloc->setAlignment(4);
                ASTValue *alloca = new ASTValue(param_i.first, alloc, true);
                builder.CreateStore(AI, codegenLValue(alloca));

                Identifier *id = getScope()->getInScope(param_i.second);
                id->setDeclaration(NULL, Identifier::ID_VARIABLE);
                id->setValue(alloca);
                //TODO: register value to scope
            }

            codegenStatement(fdecl->body);

            if(func->getReturnType() == Type::getVoidTy(context))
            {
                builder.CreateRetVoid();
            }

            popScope();
        }
    } else if(VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(decl))
    {
        ASTType *vty = vdecl->type;
        AllocaInst *llvmDecl = builder.CreateAlloca(codegenType(vty), 0, vdecl->getName());
        llvmDecl->setAlignment(4);

        //XXX note that we are storing the alloca(pointer) to the variable in the CGValue
        ASTValue *idValue = new ASTValue(vty, llvmDecl, true); 
        if(vdecl->value)
        {
            ASTValue *defaultValue = codegenExpression(vdecl->value);
            //promoteType(defaultValue, vty);
            //builder.CreateStore(codegenValue(defaultValue), llvmDecl);
            defaultValue = promoteType(defaultValue, vty);
            storeValue(idValue, defaultValue);
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
            GlobalVariable *llvmval = new GlobalVariable(*(Module*) current->cgValue,
                    codegenType(idTy),
                    false,
                    linkage,
                    (llvm::Constant*) NULL,
                    id->getName()); //TODO: proper global insertion
            id->setValue(new ASTValue(idTy, llvmval, true));
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
                fdecl->getName(), (Module*) current->cgValue);
    }
}

Module *IRCodegenContext::codegenTranslationUnit(TranslationUnit *u, bool declareOnly)
{
    this->unit = u; //TODO: revert to old tunit once done?
    pushScope(unit->scope);
    //if(u->cgValue) return (llvm::Module*) u->cgValue; //XXX already codegend
    if(!u->cgValue)
        u->cgValue = new Module(u->getName(), context);
    module = (llvm::Module*) u->cgValue;


    for(int i = 0; i < u->imports.size(); i++) //TODO: import symbols.
    {
        codegenIncludeUnit(u, u->importUnits[i]);
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

            id->setValue(new ASTValue(idTy, llvmval, true));
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
    return (llvm::Module*) u->cgValue;
}

void IRCodegenContext::codegenPackage(Package *p, bool declareOnly)
{
    if(p->isTranslationUnit()) // leaf in package tree
    {
        std::string err;
        linker.linkInModule(codegenTranslationUnit((TranslationUnit*) p, declareOnly), 
                (unsigned) Linker::DestroySource, &err);
    } else // generate all leaves ...
    {
        for(int i = 0; i < p->children.size(); i++)
        {
            codegenPackage(p->children[i], false);
        }
    }
}

void IRCodegenContext::codegenAST(AST *ast)
{
    codegenPackage(ast->getRootPackage(), false);
    linker.getModule()->dump();
}

void IRCodegen(AST *ast)
{
    IRCodegenContext context;
    context.codegenAST(ast);
}
