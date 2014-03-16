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

void IRCodegen(AST *ast, WLConfig cfg);

struct IRFunction
{
    FunctionDeclaration *declaration;
    ASTValue *retVal;
    llvm::BasicBlock *exit;
    bool terminated;

    IRFunction(): declaration(NULL), retVal(NULL), terminated(false) {}
    IRFunction(FunctionDeclaration *decl) : declaration(decl), terminated(false) {}
};

struct IRSwitchCase
{
    Expression *astCase;
    llvm::Value *irCase;
    llvm::BasicBlock *irBlock;
};

struct IRCodegenContext;
struct IRLoop
{
    llvm::BasicBlock *end; // where to go after break
    llvm::BasicBlock *udp; // where to go after continue

    IRLoop(IRCodegenContext *c, LoopExpression *exp);
};

struct IRScope
{
    llvm::BasicBlock *breakLabel;
    llvm::BasicBlock *continueLabel;
    llvm::DIDescriptor debug;
    SymbolTable *table;

    //switch
    SwitchExpression *switchExp;
    std::vector<IRSwitchCase*> cases;

    llvm::DIDescriptor getDebug() { return debug; }

    bool contains(std::string s) { return table->contains(s); }
    Identifier *getInScope(std::string s) { return table->getInScope(s); }
    Identifier *get(std::string s) { return table->get(s); }
    Identifier *lookup(std::string s, bool imports = true) { return table->lookup(s, imports); }

    IRScope(SymbolTable *tbl, llvm::DIDescriptor dbg) : table(tbl), debug(dbg) {}
};

struct IRTranslationUnit
{
    TranslationUnit *unit;
    IRScope *scope;
    llvm::Module *module;


    std::map<std::string, llvm::Type*> types;

    std::vector<VariableDeclaration*>& getGlobals() { return unit->globals; }
    std::vector<FunctionDeclaration*>& getFunctions() { return unit->functions; }


    //IRScope *getScope() {
    //    if(!scope)
    //        scope = new IRScope(unit->getScope(), );
    //    return scope;
    //}
    //operator TranslationUnit*() { return unit; }
    IRTranslationUnit(TranslationUnit *u) : unit(u), scope(NULL) {}
};

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
    IRFunction currentFunction;
    IRTranslationUnit *unit;
    IRDebug *debug;
    std::stack<IRScope*> scope;

    IRCodegenContext() : context(llvm::getGlobalContext()),
    ir(new llvm::IRBuilder<>(context)),
     module(NULL), linker(new llvm::Module("", context)) {}

    llvm::LLVMContext& getLLVMContext() { return context; }

    std::string codegenAST(AST *ast, WLConfig param);
    protected:

    // codegen type
    llvm::Type *codegenArrayType(ASTType *ty);
    llvm::Type *codegenStructType(ASTType *ty);
    llvm::Type *codegenUnionType(ASTType *ty);
    llvm::Type *codegenTupleType(ASTType *ty);
    llvm::Type *codegenType(ASTType *ty);

    // codegen value
    llvm::Value *codegenValue(ASTValue *v);
    llvm::Value *codegenLValue(ASTValue *v);

    //debug
    llvm::DIDescriptor diScope(){ return scope.top()->debug; }

    public:
    //scope
    void pushScope(IRScope *sc) { scope.push(sc);}
    IRScope *popScope() { IRScope *tbl = scope.top(); scope.pop(); return tbl;}
    IRScope *getScope() { return scope.top(); }
    Identifier *lookup(std::string str){ return scope.top()->lookup(str); }
    Identifier *getInScope(std::string str) { return scope.top()->getInScope(str); }

    protected:

    void dwarfStopPoint(int ln);
    void dwarfStopPoint(SourceLocation l);

    ASTValue *loadValue(ASTValue *val);
    ASTValue *storeValue(ASTValue *dest, ASTValue *val);

    // expression
    ASTValue *codegenExpression(Expression *exp);
    ASTValue *codegenTupleExpression(TupleExpression *exp, ASTType *ty = 0);

    ASTValue *codegenNewExpression(NewExpression *exp);
    ASTValue *codegenDeleteExpression(DeleteExpression *exp);

    ASTValue *codegenIdentifier(Identifier *id);
    ASTValue *codegenIfExpression(IfExpression *exp);
    ASTValue *codegenLoopExpression(LoopExpression *exp);
    ASTValue *codegenSwitchExpression(SwitchExpression *exp);
    ASTValue *codegenCallExpression(CallExpression *exp);
    ASTValue *codegenPostfixExpression(PostfixExpression *exp);
    ASTValue *codegenUnaryExpression(UnaryExpression *exp);
    ASTValue *codegenBinaryExpression(BinaryExpression *exp);

    // promote
    ASTValue *promoteType(ASTValue *val, ASTType *type);
    ASTValue *promoteInt(ASTValue *val, ASTType *type);
    ASTValue *promoteFloat(ASTValue *val, ASTType *type);
    ASTValue *promotePointer(ASTValue *val, ASTType *type);
    ASTValue *promoteTuple(ASTValue *val, ASTType *type);
    ASTValue *promoteArray(ASTValue *val, ASTType *type);
    void codegenResolveBinaryTypes(ASTValue **v1, ASTValue **v2, unsigned op);

    void codegenImport(ImportExpression *e);

    // codegen statement
    void codegenReturnStatement(ReturnStatement *exp);
    void codegenStatement(Statement *stmt);

    // codegen declaration
    llvm::FunctionType *codegenFunctionPrototype(FunctionPrototype *proto);
    void codegenDeclaration(Declaration *decl);

    // codegen etc
    void codegenTranslationUnit(IRTranslationUnit *unit);
    void codegenIncludeUnit(IRTranslationUnit *current, TranslationUnit *inc);
    void codegenPackage(Package *p);
};

#endif
