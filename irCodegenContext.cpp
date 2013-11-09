
#include "ast.hpp"
#include "token.hpp"
#include "irCodegenContext.hpp"

#include <iostream>

using namespace std;
using namespace llvm;


CGType IRCodegenContext::codegenType(ASTType *ty)
{
    if(ASTBasicType *bty = dynamic_cast<ASTBasicType*>(ty))
    {
        Type *llvmty = NULL;
        switch(bty->sz)
        {
            case 1:
                llvmty = Type::getInt8Ty(context);
                break;
            case 2:
                llvmty = Type::getInt16Ty(context);
                break;
            case 4:
                llvmty = Type::getInt32Ty(context);
                break;
            case 8:
                llvmty = Type::getInt64Ty(context);
                break;
        }
        return CGType(ASTQualType(bty->identifier), llvmty);
    }

    if(ASTPointerType *pty = dynamic_cast<ASTPointerType*>(ty))
    {
        Type *llvmty = NULL; //XXX codegenType(pty->ptrTo)->getPointerTo();
        return CGType(ASTQualType(NULL), llvmty); //XXX
    }
}

CGType IRCodegenContext::codegenType(ASTQualType ty)
{
    if(TypeDeclaration *tdcl = dynamic_cast<TypeDeclaration*>(ty.identifier->declaration))
    {
        return codegenType(tdcl->type);
    }

    assert(false && "unimpl type");
    //TODO: struct
}

CGValue IRCodegenContext::codegenExpression(Expression *exp)
{
    if(BlockExpression *bexp = dynamic_cast<BlockExpression*>(exp))
    {
        for(int i = 0; i < bexp->statements.size(); i++)
        {
            codegenStatement(bexp->statements[i]);
        }
    } 
    else if(NumericExpression *nexp = dynamic_cast<NumericExpression*>(exp))
    {
        switch(nexp->type)
        {
            case NumericExpression::INT:
                Value *llvmval = ConstantInt::get(Type::getInt32Ty(context), nexp->intValue);
                //return CGValue(ASTQualType(), llvmval);//TODO
                CGType ity = CGType(getScope()->lookup("int"), Type::getInt32Ty(context));
                return CGValue(ity, llvmval);
                //TODO: other types
        }
    }
    else if(UnaryExpression *uexp = dynamic_cast<UnaryExpression *>(exp))
    {
        return codegenUnaryExpression(uexp); 
    }
    else if(BinaryExpression *bexp = dynamic_cast<BinaryExpression*>(exp))
    {
        return codegenBinaryExpression(bexp);
    }
    //assert(false && "bad expression?");
    return CGValue();
}

CGValue IRCodegenContext::codegenUnaryExpression(UnaryExpression *exp)
{
    CGValue lhs = codegenExpression(exp->lhs);

    switch(exp->op)
    {

    }
}

void IRCodegenContext::codegenResolveBinaryTypes(CGValue &v1, CGValue &v2, unsigned op)
{
    //if(ASTBasicType *bty1 = dynamic_cast v1.qual
    if(ASTBasicType *bty1 = dynamic_cast<ASTBasicType*>(v1.qualTy().getBaseType()))
    {
        ASTBasicType *bty2 = dynamic_cast<ASTBasicType*>(v2.qualTy().getBaseType());
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
}

CGValue IRCodegenContext::codegenBinaryExpression(BinaryExpression *exp)
{
    CGValue lhs = codegenExpression(exp->lhs);
    CGValue rhs = codegenExpression(exp->rhs);
    llvm::Value *val;

    codegenResolveBinaryTypes(lhs, rhs, exp->op);

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
            val = builder.CreateAdd(lhs.value, rhs.value);
            return CGValue(lhs.type, val); //TODO: proper typing (for all below too)

        case tok::minus:
            val = builder.CreateSub(lhs.value, rhs.value);
            return CGValue(lhs.type, val);

        case tok::star:
            val = builder.CreateMul(lhs.value, rhs.value);
            return CGValue(lhs.type, val);

        case tok::slash:
            val = builder.CreateUDiv(lhs.value, rhs.value); //TODO: typed div
            return CGValue(lhs.type, val);

        case tok::percent:
            val = builder.CreateURem(lhs.value, rhs.value);
            return CGValue(lhs.type, val);



        default:
            return CGValue(); //XXX: null val
    }
}

void IRCodegenContext::codegenReturnStatement(ReturnStatement *exp)
{
    CGValue value = codegenExpression(exp->expression);
    builder.CreateRet(value.value);
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
    CGType rty = codegenType(proto->returnType);
    vector<Type*> params;
    for(int i = 0; i < proto->parameters.size(); i++)
    {
        params.push_back(codegenType(proto->parameters[i].first).type);
    }

    FunctionType *fty = FunctionType::get(rty.type, params, false); // XXX return, args, varargs 
    return fty;
}

void IRCodegenContext::codegenDeclaration(Declaration *decl)
{
    if(FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(decl))
    {
        FunctionType *fty = codegenFunctionPrototype(fdecl->prototype);
        Function *func = Function::Create(fty, Function::ExternalLinkage, fdecl->getName(), module);
        if(fdecl->body)
        {
            BasicBlock *BB = BasicBlock::Create(context, "entry", func);
            builder.SetInsertPoint(BB);
            codegenStatement(fdecl->body);

            if(fty->getReturnType() == Type::getVoidTy(context))
            {
                builder.CreateRetVoid();
            }
        }
    }
}

Module *IRCodegenContext::codegenTranslationUnit(TranslationUnit *unit)
{
    pushScope(unit->scope);
    module = new Module(unit->getName(), context);
    for(int i = 0; i < unit->statements.size(); i++)
    {
        codegenStatement(unit->statements[i]);
    }
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
