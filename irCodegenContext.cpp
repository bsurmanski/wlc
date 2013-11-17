
#include "ast.hpp"
#include "token.hpp"
#include "irCodegenContext.hpp"

#include <iostream>

using namespace std;
using namespace llvm;


llvm::Type *IRCodegenContext::codegenType(ASTType *ty)
{
    if(!ty->cgType)
    {
        llvm::Type *llvmty = NULL;
        switch(ty->type)
        {
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

    return (llvm::Value *) value->cgValue;
}

ASTValue *IRCodegenContext::loadValue(ASTValue *value)
{
    ASTValue *loaded = new ASTValue(value->type->getPointerTy(), builder.CreateLoad(codegenValue(value)));
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
        //TODO: IRBuilder doesn't seem to do globals
        return new ASTValue(ASTType::getCharTy()->getPointerTy(), builder.CreateGlobalStringPtr(sexp->string));
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
        if(!iexp->identifier()->getDeclaration()) iexp->id = getScope()->lookup(iexp->id->name);
        if(VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(iexp->identifier()->getDeclaration()))
        {
            //ASTValue *ptrvalue = iexp->identifier()->value; //TODO: load value?
            return loadValue(iexp->identifier()->value);
            
        } else if(FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(iexp->identifier()->getDeclaration()))
        {
            Function *llvmfunc = module->getFunction(fdecl->getName());
            return new ASTValue(NULL, llvmfunc); //TODO: proper function CGType (llvmfunc->getFunctionType())
        }
    }
    //assert(false && "bad expression?");
    return NULL; //TODO
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
        cargs.push_back(codegenExpression(exp->args[i]));
        llargs.push_back(codegenValue(cargs[i]));
    }
    llvm::Value *value = builder.CreateCall(codegenValue(func), llargs);
    return NULL; //TODO
}

ASTValue *IRCodegenContext::codegenUnaryExpression(UnaryExpression *exp)
{
    ASTValue *lhs = codegenExpression(exp->lhs);

    switch(exp->op)
    {

    }
    return NULL;
}

void IRCodegenContext::codegenResolveBinaryTypes(ASTValue &v1, ASTValue &v2, unsigned op)
{
    /*
    if(ASTBasicType *bty1 = dynamic_cast<ASTBasicType*>(v1.qualTy()))
    {
        ASTBasicType *bty2 = dynamic_cast<ASTBasicType*>(v2.qualTy());
        if(bty1->size() > bty2->size())
        {
            v2.type = v1.type;
            v2.value = builder.CreateSExt(v2.value, v1.llvmTy()); //TODO: signedness
        } else if(bty1->size() < bty2->size())
        {
            v1.type = v2.type;
            v1.value = builder.CreateSExt(v1.value, v2.llvmTy());
        }
    }
    */
    //TODO: fix for ASTValue
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
            storeValue(iexp->identifier()->value, rhs);
        } else assert(false && "LHS must be LValue");

        return ret;
    }

    ASTValue *lhs = codegenExpression(exp->lhs);
    ASTValue *rhs = codegenExpression(exp->rhs);
    llvm::Value *val;

    Value *lhs_val = codegenValue(lhs);
    Value *rhs_val = codegenValue(rhs);

    //codegenResolveBinaryTypes(lhs, rhs, exp->op);
    //TODO: resolve types

    switch(exp->op)
    {
        case tok::comma:
        case tok::barbar:
        case tok::kw_or:
        case tok::ampamp:
        case tok::kw_and:
        case tok::bar:
        case tok::caret:
        case tok::amp:
        case tok::equalequal:
        case tok::less:
        case tok::lessequal:
        case tok::greater:
        case tok::greaterequal:
                assert(false && "unimpl binop");
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

        default:
            return NULL; //XXX: null val
    }
}

void IRCodegenContext::codegenReturnStatement(ReturnStatement *exp)
{
    ASTValue *value = codegenExpression(exp->expression);
    builder.CreateRet(codegenValue(value));
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
            codegenStatement(fdecl->body);

            if(func->getReturnType() == Type::getVoidTy(context))
            {
                builder.CreateRetVoid();
            }
        }
    } else if(VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(decl))
    {
        ASTType *vty = vdecl->type;
        Value *llvmDecl = builder.CreateAlloca(codegenType(vty), 0, vdecl->getName());
        
        if(vdecl->value)
        {
            ASTValue *defaultValue = codegenExpression(vdecl->value);
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
