#ifndef _IRCODEGENCONTEXT_HPP
#define _IRCODEGENCONTEXT_HPP

#include "codegenContext.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/DIBuilder.h>
#include <llvm/DebugInfo.h>
#include <llvm/Linker.h>
#include <stack>

#define CGSTR "wlc 0.1 - Jan 2014"

void IRCodegen(AST *ast);

class IRTranslationUnit
{
    public:
    TranslationUnit *self;
    llvm::Module *module;
    llvm::DIBuilder di; //debug info
    llvm::DICompileUnit dunit;
    IRTranslationUnit(TranslationUnit *u, llvm::Module *m) : 
        self(u), module(m), 
        di(*module) 
    {
        di.createCompileUnit(0, u->filenm, "DIR", CGSTR, false, "", 0);

        // add debug for basic types
        // TODO: do something with this??
        di.createBasicType("char", 8, 8, llvm::dwarf::DW_ATE_signed_char);
        di.createBasicType("uchar", 8, 8, llvm::dwarf::DW_ATE_unsigned_char);
        di.createBasicType("int8", 8, 8, llvm::dwarf::DW_ATE_signed);
        di.createBasicType("uint8", 8, 8, llvm::dwarf::DW_ATE_unsigned);
        di.createBasicType("int16", 16, 16, llvm::dwarf::DW_ATE_signed);
        di.createBasicType("uint16", 16, 16, llvm::dwarf::DW_ATE_unsigned);
        di.createBasicType("int32", 32, 32, llvm::dwarf::DW_ATE_signed);
        di.createBasicType("uint32", 32, 32, llvm::dwarf::DW_ATE_unsigned);
        di.createBasicType("int64", 64, 64, llvm::dwarf::DW_ATE_signed);
        di.createBasicType("uint64", 64, 64, llvm::dwarf::DW_ATE_unsigned);
        di.createBasicType("float32", 32, 32, llvm::dwarf::DW_ATE_float);
        di.createBasicType("float64", 64, 64, llvm::dwarf::DW_ATE_float);
    }
};

class IRValue
{
    llvm::Value *irvalue;
    ASTValue *astvalue;
    llvm::DIDescriptor debug;
    IRValue(ASTValue &v) {}
    llvm::DIDescriptor getDebug() { return debug; }
    void setDebug(llvm::DIDescriptor d) { debug = d; }
};

class IRType
{
    llvm::Type *irtype;
    ASTType *asttype;
    llvm::DIType debug;
    IRType(ASTType *t) {}
    ASTType *astType() { return asttype; }
    llvm::DIType getDebug() { return debug; }
    void setDebug(llvm::DIType d) { debug = d; }
}; 

class IRCodegenContext : public CodegenContext
{
    public:
    llvm::LLVMContext &context;
    llvm::IRBuilder<> *ir;
    llvm::DIBuilder *di;
    llvm::Module *module;
    llvm::BasicBlock *breakLabel;
    llvm::BasicBlock *continueLabel;
    llvm::Linker linker;
    TranslationUnit *unit;

    IRCodegenContext() : context(llvm::getGlobalContext()), 
    ir(new llvm::IRBuilder<>(context)), 
    di(NULL), module(NULL), linker(new llvm::Module("", context)) {}

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
    void codegenTranslationUnit(IRTranslationUnit *unit);
    void codegenIncludeUnit(IRTranslationUnit *current, TranslationUnit *inc);
    void codegenPackage(Package *p);
};

#endif
