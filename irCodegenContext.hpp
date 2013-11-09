#ifndef _IRCODEGENCONTEXT_HPP
#define _IRCODEGENCONTEXT_HPP

#include "codegenContext.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <stack>

struct CGType
{
    ASTQualType qual;
    llvm::Type *type;
    CGType(ASTQualType qu, llvm::Type *ty) : qual(qu), type(ty){}
    CGType() {}
};

struct CGValue
{
    CGType type;
    llvm::Value *value;
    CGValue(CGType ty, llvm::Value *v = NULL) : type(ty), value(v) {}
    CGValue() {}
    llvm::Type *llvmTy() { return type.type; }
    ASTQualType qualTy() { return type.qual; }
};

void IRCodegen(AST *ast);

class IRCodegenContext : public CodegenContext
{
    public:
    llvm::LLVMContext &context;
    llvm::IRBuilder<> builder;
    llvm::Module *module;

    IRCodegenContext() : context(llvm::getGlobalContext()), builder(context), module(NULL) {}

    void codegenAST(AST *ast);
    protected:
    std::stack<SymbolTable*> scope;

    void pushScope(SymbolTable* tbl) { scope.push(tbl); }
    SymbolTable *popScope() { SymbolTable *tbl = scope.top(); scope.pop(); return tbl;}
    SymbolTable *getScope() { return scope.top(); }

    CGType codegenType(ASTType *ty); 
    CGType codegenType(ASTQualType ty); //diferent name from above
    CGValue codegenExpression(Expression *exp);
    CGValue codegenBinaryExpression(BinaryExpression *exp);
    void codegenResolveBinaryTypes(CGValue &v1, CGValue &v2, unsigned op);
    CGValue codegenUnaryExpression(UnaryExpression *exp);
    void codegenReturnStatement(ReturnStatement *exp);
    void codegenStatement(Statement *stmt);
    llvm::FunctionType *codegenFunctionPrototype(FunctionPrototype *proto);
    void codegenDeclaration(Declaration *decl);
    llvm::Module *codegenTranslationUnit(TranslationUnit *unit);
};

#endif
