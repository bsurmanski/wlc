
#include "ast.hpp"
#include "token.hpp"
#include "irCodegenContext.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace llvm;


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
                llvmty = Type::getInt8Ty(context);
                break;
            case TYPE_SHORT:
                llvmty = Type::getInt16Ty(context);
                break;
            case TYPE_INT:
                llvmty = Type::getInt32Ty(context);
                break;
            case TYPE_LONG:
                llvmty = Type::getInt64Ty(context);
                break;
            case TYPE_VOID:
                llvmty = Type::getVoidTy(context);
                break;
            case TYPE_POINTER:
                llvmty = codegenType(ty->getReferencedTy())->getPointerTo();
                break;

            default:
                assert(false && "type not handled");
        }
        ty->cgType = llvmty;
    }

    return (llvm::Type *) ty->cgType;
}

llvm::Value *IRCodegenContext::codegenValue(ASTValue *value)
{
    if(!value->cgValue)
    {
    
        //TODO
    }
    if(value->type->isPointer())
    {
        printf("LOWERING PTR\n"); 
    }

    return (llvm::Value *) value->cgValue;
}

ASTValue *IRCodegenContext::loadValue(ASTValue *value)
{
    ASTValue *loaded = new ASTValue(value->type, builder.CreateLoad(codegenValue(value)));
    return loaded;
}

ASTValue *IRCodegenContext::storeValue(ASTValue *dest, ASTValue *val)
{
    ASTValue *stored = new ASTValue(dest->type->getPointerTy(), builder.CreateStore(codegenValue(val), codegenValue(dest)));
    return stored;
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
        switch(nexp->type)
        {
            case NumericExpression::INT:
                llvm::Value *llvmval = ConstantInt::get(Type::getInt32Ty(context), nexp->intValue);
                ASTType *ty = ASTType::getIntTy();
                return new ASTValue(ty, llvmval); //TODO: assign
        }
    } 
    else if(StringExpression *sexp = exp->stringExpression())
    {
        Constant *strConstant = ConstantDataArray::getString(context, sexp->string);
        GlobalVariable *GV = new GlobalVariable(*module, strConstant->getType(), true, GlobalValue::PrivateLinkage, strConstant);
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
            assert(!iexp->id->isUndeclared() && "undeclared variable in this scope!");
        }
        if(iexp->identifier()->isVariable())
        {
            //ASTValue *ptrvalue = iexp->identifier()->value; //TODO: load value?
            return loadValue(iexp->identifier()->getValue());
            
        //} else if(FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(iexp->identifier()->getDeclaration()))
        } else if(iexp->identifier()->isFunction())
        {
            Function *llvmfunc = module->getFunction(iexp->identifier()->getName());
            return new ASTValue(NULL, llvmfunc); //TODO: proper function CGType (llvmfunc->getFunctionType())
        }
    } else if(IfExpression *iexp = exp->ifExpression())
    {
        return codegenIfExpression(iexp);
    } else if(WhileExpression *wexp = exp->whileExpression())
    {
        return codegenWhileExpression(wexp);
    }
    assert(false && "bad expression?");
    return NULL; //TODO
}

ASTValue *IRCodegenContext::codegenIfExpression(IfExpression *exp)
{
    ASTValue *cond = codegenExpression(exp->condition);
    ASTValue *zero = new ASTValue(ASTType::getIntTy(), ConstantInt::get(Type::getInt32Ty(context), 0));
    codegenResolveBinaryTypes(cond, zero, 0);
    ASTValue *icond = new ASTValue(ASTType::getBoolTy(), builder.CreateICmpNE(codegenValue(cond), codegenValue(zero)));
    llvm::BasicBlock *ontrue = BasicBlock::Create(context, "true", builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *onfalse = BasicBlock::Create(context, "false", builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *endif = BasicBlock::Create(context, "endif", builder.GetInsertBlock()->getParent());
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
    llvm::BasicBlock *whileBB = BasicBlock::Create(context, "while_condition", builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *ontrue = BasicBlock::Create(context, "while_true", builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *onfalse = BasicBlock::Create(context, "while_false", builder.GetInsertBlock()->getParent());
    llvm::BasicBlock *endwhile = BasicBlock::Create(context, "endwhile", builder.GetInsertBlock()->getParent());

    builder.CreateBr(whileBB);
    builder.SetInsertPoint(whileBB);
    ASTValue *cond = codegenExpression(exp->condition);
    ASTValue *zero = new ASTValue(ASTType::getIntTy(), ConstantInt::get(Type::getInt32Ty(context), 0));
    codegenResolveBinaryTypes(cond, zero, 0);
    ASTValue *icond = new ASTValue(ASTType::getBoolTy(), builder.CreateICmpNE(codegenValue(cond), codegenValue(zero)));
    builder.CreateCondBr(codegenValue(icond), ontrue, onfalse);

    this->breakLabel = endwhile;
    builder.SetInsertPoint(ontrue);
    codegenStatement(exp->body);
    builder.CreateBr(whileBB);

    builder.SetInsertPoint(onfalse);
    if(exp->elsebranch) codegenStatement(exp->elsebranch);
    builder.CreateBr(endwhile);
    this->breakLabel = NULL; //TODO: break label should be set to whatever it was previously, to allow for nested while

    builder.SetInsertPoint(endwhile);
    return NULL;
}

ASTValue *IRCodegenContext::codegenCallExpression(CallExpression *exp)
{
    ASTValue *func = codegenExpression(exp->function);
    //assert(!func.llvmTy()->isFunctionTy() && "not callable!");
    //FunctionType *fty = (FunctionType*) func.llvmTy();
    //TODO: once proper type passing is done, check if callable above
    vector<ASTValue*> cargs;
    vector<Value*> llargs;
    for(int i = 0; i < exp->args.size(); i++)
    {
        ASTValue *val = codegenExpression(exp->args[i]);
        //promoteType(val, 
        cargs.push_back(val);
        llargs.push_back(codegenValue(cargs[i]));
    }
    llvm::Value *value = builder.CreateCall(codegenValue(func), llargs);
    return new ASTValue(NULL, value); //TODO
}

ASTValue *IRCodegenContext::codegenUnaryExpression(UnaryExpression *exp)
{
    ASTValue *lhs = codegenExpression(exp->lhs); // expression after unary op: eg in !a, lhs=a

    ASTValue *val;
    switch(exp->op)
    {
        case tok::plusplus:
        case tok::minusminus:
        case tok::plus:
        case tok::minus:
        case tok::bang:
        case tok::tilde:
        case tok::caret:
        case tok::amp:
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
        //TODO: index array
    }

    assert(false && "postfix codegen unimpl");
    return NULL;
}


void IRCodegenContext::promoteType(ASTValue *val, ASTType *toType)
{
    if(val->type->isInteger())
    {
        if(toType->isInteger())
        {
            val->cgValue = builder.CreateIntCast(codegenValue(val), codegenType(toType), false); //TODO: signedness
        }
    }
    if(val->type->isPointer())
    {
    }
}

//XXX is op required?
void IRCodegenContext::codegenResolveBinaryTypes(ASTValue *v1, ASTValue *v2, unsigned op)
{
    if(v1->type != v2->type)
    {
        if(v1->type->isStruct() || v2->type->isStruct())
        {
            assert(false && "cannot convert structs (yet)");
        }
        if(v1->type->size() > v2->type->size()) promoteType(v2, v1->type);
        else if(v2->type->size() > v1->type->size()) promoteType(v1, v2->type);
    }
}

ASTValue *IRCodegenContext::codegenBinaryExpression(BinaryExpression *exp)
{
    //TODO: bit messy
    if(exp->op == tok::equal)
    {
        ASTValue *ret = NULL;
        ASTValue *rhs = codegenExpression(exp->rhs);
        if(IdentifierExpression *iexp = dynamic_cast<IdentifierExpression*>(exp->lhs))
        {
            promoteType(rhs, iexp->identifier()->value->type);
            storeValue(iexp->identifier()->value, rhs);
        } else assert(false && "LHS must be LValue");

        return ret;
    }

    ASTValue *lhs = codegenExpression(exp->lhs);
    ASTValue *rhs = codegenExpression(exp->rhs);
    codegenResolveBinaryTypes(lhs, rhs, exp->op);

    llvm::Value *val;
    Value *lhs_val = codegenValue(lhs);
    Value *rhs_val = codegenValue(rhs);

    //codegenResolveBinaryTypes(lhs, rhs, exp->op);
    //TODO: resolve types

    switch(exp->op)
    {
        case tok::comma:
                assert(false && "unimpl binop");
        case tok::barbar:
        case tok::kw_or:
            promoteType(lhs, ASTType::getBoolTy());
            promoteType(rhs, ASTType::getBoolTy());
            val = builder.CreateOr(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(ASTType::getBoolTy(), val);
        case tok::ampamp:
        case tok::kw_and:
            promoteType(lhs, ASTType::getBoolTy());
            promoteType(rhs, ASTType::getBoolTy());
            val = builder.CreateAnd(codegenValue(lhs), codegenValue(rhs));
            return new ASTValue(ASTType::getBoolTy(), val);
        case tok::bar:
        case tok::caret:
        case tok::amp:
                assert(false && "unimpl binop");
        case tok::equalequal:
                val = builder.CreateICmpEQ(lhs_val, rhs_val); //TODO: sign, float
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::less:
                val = builder.CreateICmpULT(lhs_val, rhs_val); //TODO: sign, float
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::lessequal:
                val = builder.CreateICmpULE(lhs_val, rhs_val); //TODO: sign, float
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::greater:
                val = builder.CreateICmpUGT(lhs_val, rhs_val); //TODO: sign, float
                return new ASTValue(ASTType::getBoolTy(), val);
        case tok::greaterequal:
                val = builder.CreateICmpUGE(lhs_val, rhs_val); //TODO: sign, float
                return new ASTValue(ASTType::getBoolTy(), val);
            //TODO: all of these

        case tok::plus: 
            val = builder.CreateAdd(lhs_val, rhs_val);
            return new ASTValue(lhs->type, val); //TODO: proper typing (for all below too)

        case tok::minus:
            val = builder.CreateSub(lhs_val, rhs_val);
            return new ASTValue(lhs->type, val);

        case tok::star:
            val = builder.CreateMul(lhs_val, rhs_val);
            return new ASTValue(lhs->type, val);

        case tok::slash:
            val = builder.CreateUDiv(lhs_val, rhs_val); //TODO: typed div
            return new ASTValue(lhs->type, val);

        case tok::percent:
            val = builder.CreateURem(lhs_val, rhs_val);
            return new ASTValue(lhs->type, val);

        case tok::dot:
            //TODO: index into struct, GEP?

        default:
            assert(false && "unimpl");
            return NULL; //XXX: null val
    }
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
    if(ExpressionStatement *estmt = dynamic_cast<ExpressionStatement*>(stmt))
    {
        codegenExpression(estmt->expression);
    } else if (DeclarationStatement *dstmt = dynamic_cast<DeclarationStatement*>(stmt))
    {
        codegenDeclaration(dstmt->declaration);
    } else if (ReturnStatement *rstmt = dynamic_cast<ReturnStatement*>(stmt))
    {
        codegenReturnStatement(rstmt);
    }
}

FunctionType *IRCodegenContext::codegenFunctionPrototype(FunctionPrototype *proto)
{
    ASTType *rty = proto->returnType;
    vector<Type*> params;
    for(int i = 0; i < proto->parameters.size(); i++)
    {
        params.push_back(codegenType(proto->parameters[i].first));
    }

    FunctionType *fty = FunctionType::get(codegenType(rty), params, proto->vararg); // XXX return, args, varargs 
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
                assert(fdecl->prototype->parameters.size() >= idx && "argument counts dont seem to match up...");
                pair<ASTType*, std::string> param_i = fdecl->prototype->parameters[idx];
                AI->setName(param_i.second);
                ASTValue *alloca = new ASTValue(param_i.first, builder.CreateAlloca(codegenType(param_i.first), 0, param_i.second));
                builder.CreateStore(AI, codegenValue(alloca));

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
        Value *llvmDecl = builder.CreateAlloca(codegenType(vty), 0, vdecl->getName());
        
        if(vdecl->value)
        {
            ASTValue *defaultValue = codegenExpression(vdecl->value);
            promoteType(defaultValue, vty);
            builder.CreateStore(codegenValue(defaultValue), llvmDecl);
            ASTValue *idValue = new ASTValue(vty, llvmDecl); //XXX note that we are storing the alloca(pointer) to the variable in the CGValue
            vdecl->identifier->value = idValue;
            //TODO: maybe create a LValue field in CGValue?
        }
    }
}

Module *IRCodegenContext::codegenTranslationUnit(TranslationUnit *unit)
{
    pushScope(unit->scope);
    module = new Module(unit->getName(), context);


    for(int i = 0; i < unit->imports.size(); i++) //TODO: import symbols. XXX what about recursive dependancies?
    {}

    for(int i = 0; i < unit->types.size(); i++) //XXX what about recursive types?
    {}

    // alloc globals before codegen'ing functions
    for(int i = 0; i < unit->globals.size(); i++)
    {
        Identifier *id = unit->globals[i]->identifier; 
        if(id->isVariable()) 
        {
            ASTType *idTy = id->getType();
            //llvm::Value *llvmval = module->getOrInsertGlobal(id->getName(), codegenType(idTy));
            GlobalVariable *llvmval = new GlobalVariable(*module, 
                    codegenType(idTy), 
                    false,
                    GlobalVariable::CommonLinkage,
                    (llvm::Constant*) (unit->globals[i]->value ? codegenValue(codegenExpression(unit->globals[i]->value)) : 0),
                    id->getName()); //TODO: proper global insertion
            id->value = new ASTValue(idTy->getPointerTy(), llvmval);
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
        Function::Create(fty, Function::ExternalLinkage, fdecl->getName(), module);
    }

    // codegen function bodys
    for(int i = 0; i < unit->functions.size(); i++)
        codegenDeclaration(unit->functions[i]);

    module->dump();
    popScope();
    return module;
}

void IRCodegenContext::codegenAST(AST *ast)
{
    std::map<std::string, TranslationUnit*>::iterator it;
    for(it = ast->units.begin(); it != ast->units.end(); it++)
    {
        //TODO: something with module
        codegenTranslationUnit(it->second);
    }
}

void IRCodegen(AST *ast)
{
    IRCodegenContext context;
    context.codegenAST(ast);
}
