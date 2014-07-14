#ifndef _IRCODEGENCONTEXT_HPP
#define _IRCODEGENCONTEXT_HPP

#include "codegenContext.hpp"
#include "config.hpp"
#include "message.hpp"
#include "astScope.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/DIBuilder.h>
#include <llvm/DebugInfo.h>
#include <llvm/Linker.h>
#include <stack>

#include "irDebug.hpp"

struct IRCodegenContext;

void IRCodegen(AST *ast, WLConfig cfg);

struct IRFunction
{
    ASTValue *retVal;
    llvm::BasicBlock *exit;
    bool terminated;

    IRFunction(): retVal(NULL), exit(NULL), terminated(false) {}
};

struct IRSwitchCase
{
    Expression *astCase;
    ASTValue *irCase;
    llvm::BasicBlock *irBlock;
    IRSwitchCase(Expression *e, ASTValue *i, llvm::BasicBlock *ir) :
        astCase(e), irCase(i), irBlock(ir) {}
};

struct IRValue;
struct IRType
{
    ASTType *type;
    IRValue *vtable; // vtable if class type
    llvm::Type *llvmType;
    IRType() : type(0), llvmType(0) {}
    IRType(ASTType *ty, llvm::Type *llty) : type(ty), llvmType(llty), vtable(0) {}

    operator ASTType*() const { return type; }
    operator llvm::Type*() const { return llvmType; }
};

struct IRValue
{
    ASTValue *value;
    llvm::Value *llvmValue;
    IRValue() : value(0), llvmValue(0) {}
    IRValue(ASTValue *val, llvm::Value *llval) : value(val), llvmValue(llval) {}

    operator ASTValue*() const { return value; }
    operator llvm::Value*() const { return llvmValue; }
};

struct IRScope
{
    llvm::BasicBlock *breakLabel;
    llvm::BasicBlock *continueLabel;
    llvm::DIDescriptor debug;
    ASTScope *table;
    IRScope *parent;

    ASTScope::iterator begin() { return table->begin(); }
    ASTScope::iterator end() { return table->end(); }

    //switch
    SwitchStatement *switchStmt;
    std::vector<IRSwitchCase*> cases;

    llvm::DIDescriptor getDebug() { return debug; }

    llvm::BasicBlock *getBreak() {
        if(!breakLabel && parent)
            return parent->getBreak();
        return breakLabel;
    }

    llvm::BasicBlock *getContinue() {
        if(!continueLabel && parent)
            return parent->getContinue();
        return continueLabel;
    }

    void addCase(Expression *cs, ASTValue *ircs, llvm::BasicBlock *bl)
    {
        if(switchStmt)
        {
            cases.push_back(new IRSwitchCase(cs, ircs, bl));
        } else if(parent)
        {
            parent->addCase(cs, ircs, bl);
        } else
        {
            emit_message(msg::ERROR, "'case' label must be in switch scope");
        }
    }

    IRScope *getParent() { return parent; }

    bool contains(std::string s) { return table->contains(s); }
    Identifier *getInScope(std::string s) { return table->getInScope(s); }
    Identifier *get(std::string s) { return table->get(s); }
    Identifier *lookup(std::string s, bool imports = true) { return table->lookup(s, imports); }
    Identifier *lookupInScope(std::string s) { return table->lookupInScope(s); }

    IRScope(ASTScope *tbl, llvm::DIDescriptor dbg) : table(tbl), debug(dbg),
        breakLabel(0), continueLabel(0), parent(0) {}
};

class IRDebug;
struct IRTranslationUnit
{
    TranslationUnit *unit;
    IRScope *scope;
    IRCodegenContext *context;
    IRDebug *debug;
    llvm::Module *module;


    std::map<std::string, IRType> types;
    std::map<std::string, IRValue> globals;
    std::map<std::string, Identifier*> symbols; //TODO

    IRScope* getScope() { return scope; }

    IRTranslationUnit(IRCodegenContext *c, TranslationUnit *u);
};

class IRCodegenContext : public CodegenContext
{
    public:
    llvm::LLVMContext &context;
    llvm::IRBuilder<> *ir;
    llvm::Module *module;
    llvm::Linker linker;
    IRFunction currentFunction;
    IRTranslationUnit *unit;
    IRScope *scope;
    AST *ast;
    bool terminated;
    WLConfig config;

    IRCodegenContext() : context(llvm::getGlobalContext()),
    ir(new llvm::IRBuilder<>(context)),
     module(NULL), linker(new llvm::Module("", context)), terminated(false), ast(0) {}

    llvm::LLVMContext& getLLVMContext() { return context; }

    std::string codegenAST(AST *ast, WLConfig param);
    protected:

    bool isTerminated() { return terminated; }
    bool setTerminated(bool b) { terminated = b; }

    // codegen type
    llvm::Type *codegenArrayType(ASTType *ty);
    llvm::Type *codegenUserType(ASTType *ty);
    llvm::Type *codegenStructType(ASTType *ty);
    llvm::Type *codegenUnionType(ASTType *ty);
    llvm::Type *codegenClassType(ASTType *ty);
    llvm::Type *codegenTupleType(ASTType *ty);
    llvm::Type *codegenFunctionType(ASTType *ty);
    llvm::Type *codegenType(ASTType *ty);

    // codegen value
    llvm::Value *codegenValue(ASTValue *v);
    llvm::Value *codegenLValue(ASTValue *v);
    llvm::Value *codegenRefValue(ASTValue *v);

    //debug
    llvm::DIDescriptor diScope(){ return scope->debug; }

    public:
    //scope
    void pushScope(IRScope *sc) { sc->parent = scope; scope = sc;}
    IRScope *popScope() { IRScope *tbl = scope; scope = scope->parent; return tbl;}
    IRScope *getScope() { return scope; }
    Identifier *lookup(std::string str){ return scope->lookup(str); }
    Identifier *getInScope(std::string str) { return scope->getInScope(str); }

    protected:

    void dwarfStopPoint(int ln);
    void dwarfStopPoint(SourceLocation l);

    ASTValue *indexValue(ASTValue *val, int i);

    ASTValue *loadValue(ASTValue *val);
    ASTValue *storeValue(ASTValue *dest, ASTValue *val);

    ASTValue *getStringValue(std::string str);
    ASTValue *getFloatValue(ASTType *t, float i);
    ASTValue *getIntValue(ASTType *t, int i);

    // vtable
    ASTValue *getThis();
    ASTValue *getVTable(ASTValue *instance);
    ASTValue *vtableLookup(ASTValue *instance, std::string func);
    ASTValue *createTypeInfo(ASTType *ty);

    // ops
    ASTValue *getMember(ASTValue *val, std::string member); // .
    ASTValue *getValueOf(ASTValue *ptr, bool lval=true);    // ^
    ASTValue *getAddressOf(ASTValue *lval); // &
    ASTValue *opAddValues(ASTValue *a, ASTValue *b); // +
    ASTValue *opSubValues(ASTValue *a, ASTValue *b); // -
    ASTValue *opMulValues(ASTValue *a, ASTValue *b); // *
    ASTValue *opDivValues(ASTValue *a, ASTValue *b); // /
    ASTValue *opModValue(ASTValue *a, ASTValue *b); // %
    ASTValue *opShlValue(ASTValue *a, ASTValue *b); // <<
    ASTValue *opShrValue(ASTValue *a, ASTValue *b); // >>
    ASTValue *opPowValue(ASTValue *a, ASTValue *b); // **

    //TODO: add compare ops

    // expression
    ASTValue *codegenExpression(Expression *exp);
    ASTValue *codegenTupleExpression(TupleExpression *exp, ASTType *ty = 0);

    ASTValue *codegenNewExpression(NewExpression *exp);
    ASTValue *codegenDeleteExpression(DeleteExpression *exp);

    ASTValue *codegenIdentifier(Identifier *id);
    ASTValue *codegenCall(ASTValue *func, std::vector<ASTValue *> args);
    ASTValue *codegenCallExpression(CallExpression *exp);
    ASTValue *codegenPostfixExpression(PostfixExpression *exp);
    ASTValue *codegenUnaryExpression(UnaryExpression *exp);
    ASTValue *codegenBinaryExpression(BinaryExpression *exp);
    ASTValue *codegenAssign(Expression *lhs, Expression *rhs, bool convert=false);

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

    void codegenIfStatement(IfStatement *stmt);
    void codegenElseStatement(ElseStatement *stmt);
    void codegenLoopStatement(LoopStatement *stmt);
    void codegenSwitchStatement(SwitchStatement *stmt);

    // codegen declaration
    void codegenDeclaration(Declaration *decl);
    void codegenVariableDeclaration(VariableDeclaration *vdecl);
    void codegenFunctionDeclaration(FunctionDeclaration *fdecl);
    void codegenUserTypeDeclaration(UserTypeDeclaration *utdecl);

    // codegen etc
    void codegenTranslationUnit(IRTranslationUnit *unit);
    void codegenIncludeUnit(IRTranslationUnit *current, TranslationUnit *inc);
    void codegenPackage(Package *p);
};

#endif
