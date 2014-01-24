#ifndef _IRCODEGENCONTEXT_HPP
#define _IRCODEGENCONTEXT_HPP

#include "codegenContext.hpp"
#include "config.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/DIBuilder.h>
#include <llvm/DebugInfo.h>
#include <llvm/Linker.h>
#include <stack>

#include "irDebug.hpp"

#define CGSTR "wlc 0.1 - Jan 2014"

void IRCodegen(AST *ast, WLConfig cfg);

class IRDebug;
class IRCodegenContext : public CodegenContext
{
    public:
    llvm::LLVMContext &context;
    llvm::IRBuilder<> *ir;
    llvm::Module *module;
    llvm::BasicBlock *breakLabel;
    llvm::BasicBlock *continueLabel;
    llvm::Linker linker;
    TranslationUnit *unit;
    IRDebug *debug;

    IRCodegenContext() : context(llvm::getGlobalContext()), 
    ir(new llvm::IRBuilder<>(context)), 
     module(NULL), linker(new llvm::Module("", context)) {}

    llvm::LLVMContext& getLLVMContext() { return context; }

    void codegenAST(AST *ast, WLConfig param);
    protected:
    std::stack<SymbolTable*> scope;

    llvm::Type *codegenArrayType(ASTType *ty);
    llvm::Type *codegenStructType(ASTType *ty);
    llvm::Type *codegenType(ASTType *ty);
    llvm::Value *codegenValue(ASTValue *v);
    llvm::Value *codegenLValue(ASTValue *v);

    //debug
    llvm::DIDescriptor diScope(){ return scope.top()->debug; }

    public:
    //scope
    void pushScope(SymbolTable* tbl, llvm::DIDescriptor debug) { tbl->setDebug(debug); scope.push(tbl);}
    SymbolTable *popScope() { SymbolTable *tbl = scope.top(); scope.pop(); return tbl;}
    SymbolTable *getScope() { return scope.top(); }
    Identifier *lookup(std::string str){ return scope.top()->lookup(str); }
    Identifier *getInScope(std::string str) { return scope.top()->getInScope(str); }

    protected:

    void dwarfStopPoint(int ln);
    void dwarfStopPoint(SourceLocation l);

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
    void codegenTranslationUnit(TranslationUnit *unit);
    void codegenIncludeUnit(TranslationUnit *current, TranslationUnit *inc);
    void codegenPackage(Package *p);
};

#endif
