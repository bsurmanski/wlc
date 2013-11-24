#ifndef _IRCODEGENCONTEXT_HPP
#define _IRCODEGENCONTEXT_HPP

#include "codegenContext.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <stack>

/*
struct CGType
{
    ASTType *type;
    llvm::Type *llvmType;
    CGType(ASTType *ty, llvm::Type *llvmty) : type(ty), llvmType(llvmty){}
    CGType() : type(NULL) {}
    ASTType *qualTy() { return type; }
    llvm::Type *llvmTy() { return llvmType; }
}; 

struct CGValue
{
    CGType *type;
    llvm::Value *value;
    CGValue(CGType *ty, llvm::Value *v = NULL) : type(ty), value(v) {}
    CGValue() : value(NULL) {}
    llvm::Value *llvmValue() { return value; }
    llvm::Type *llvmTy() { return type->llvmTy(); }
    ASTType *qualTy() { return type->qualTy(); }
};*/

void IRCodegen(AST *ast);

class IRCodegenContext : public CodegenContext
{
    public:
    llvm::LLVMContext &context;
    llvm::IRBuilder<> builder;
    llvm::Module *module;
    llvm::BasicBlock *breakLabel;

    IRCodegenContext() : context(llvm::getGlobalContext()), builder(context), module(NULL) {}

    void codegenAST(AST *ast);
    protected:
    std::stack<SymbolTable*> scope;

    llvm::Type *codegenType(ASTType *ty);
    llvm::Value *codegenValue(ASTValue *v);

    void pushScope(SymbolTable* tbl) { scope.push(tbl); }
    SymbolTable *popScope() { SymbolTable *tbl = scope.top(); scope.pop(); return tbl;}
    SymbolTable *getScope() { return scope.top(); }

    ASTValue *loadValue(ASTValue *val);
    ASTValue *storeValue(ASTValue *dest, ASTValue *val);

    ASTValue *codegenExpression(Expression *exp);
    ASTValue *codegenIfExpression(IfExpression *exp);
    ASTValue *codegenWhileExpression(WhileExpression *exp);
    ASTValue *codegenCallExpression(CallExpression *exp);
    ASTValue *codegenPostfixExpression(PostfixExpression *exp);
    ASTValue *codegenUnaryExpression(UnaryExpression *exp);
    ASTValue *codegenBinaryExpression(BinaryExpression *exp);
    void promoteType(ASTValue *val, ASTType *type);
    void codegenResolveBinaryTypes(ASTValue *v1, ASTValue *v2, unsigned op);
    void codegenReturnStatement(ReturnStatement *exp);
    void codegenStatement(Statement *stmt);
    llvm::FunctionType *codegenFunctionPrototype(FunctionPrototype *proto);
    void codegenDeclaration(Declaration *decl);
    llvm::Module *codegenTranslationUnit(TranslationUnit *unit);
};

#endif
