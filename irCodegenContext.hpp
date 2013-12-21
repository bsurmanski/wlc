#ifndef _IRCODEGENCONTEXT_HPP
#define _IRCODEGENCONTEXT_HPP

#include "codegenContext.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Linker.h>
#include <stack>

void IRCodegen(AST *ast);

class IRCodegenContext : public CodegenContext
{
    public:
    llvm::LLVMContext &context;
    llvm::IRBuilder<> builder;
    llvm::Module *module;
    llvm::BasicBlock *breakLabel;
    llvm::BasicBlock *continueLabel;
    llvm::Linker linker;
    TranslationUnit *unit;

    IRCodegenContext() : context(llvm::getGlobalContext()), builder(context), module(NULL), linker(new llvm::Module("", context)) {}

    void codegenAST(AST *ast);
    protected:
    std::stack<SymbolTable*> scope;

    llvm::Type *codegenStructType(ASTType *ty);
    llvm::Type *codegenType(ASTType *ty);
    llvm::Value *codegenValue(ASTValue *v);
    llvm::Value *codegenLValue(ASTValue *v);

    void pushScope(SymbolTable* tbl) { scope.push(tbl); }
    SymbolTable *popScope() { SymbolTable *tbl = scope.top(); scope.pop(); return tbl;}
    SymbolTable *getScope() { return scope.top(); }

    ASTValue *loadValue(ASTValue *val);
    ASTValue *storeValue(ASTValue *dest, ASTValue *val);

    ASTValue *codegenExpression(Expression *exp);
    ASTValue *codegenIfExpression(IfExpression *exp);
    ASTValue *codegenWhileExpression(WhileExpression *exp);
    ASTValue *codegenForExpression(ForExpression *exp);
    ASTValue *codegenCallExpression(CallExpression *exp);
    ASTValue *codegenPostfixExpression(PostfixExpression *exp);
    ASTValue *codegenUnaryExpression(UnaryExpression *exp);
    ASTValue *codegenBinaryExpression(BinaryExpression *exp);
    ASTValue *promoteType(ASTValue *val, ASTType *type);
    void codegenImport(ImportExpression *e);
    void codegenResolveBinaryTypes(ASTValue **v1, ASTValue **v2, unsigned op);
    void codegenReturnStatement(ReturnStatement *exp);
    void codegenStatement(Statement *stmt);
    llvm::FunctionType *codegenFunctionPrototype(FunctionPrototype *proto);
    void codegenDeclaration(Declaration *decl);
    llvm::Module *codegenTranslationUnit(TranslationUnit *unit, bool declare);
    void codegenIncludeUnit(TranslationUnit *current, TranslationUnit *inc);
    void codegenPackage(Package *p, bool declare);
};

#endif
