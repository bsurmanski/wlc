#ifndef _ASTVALUE_HPP
#define _ASTVALUE_HPP

#include "ast.hpp"

struct ASTValueInfo
{
    virtual ~ASTValueInfo(){}
};

struct ASTValue
{
    // value in which this value is indexing or owned by.
    // eg: if current value represents 'mc.func()', owner will be 'mc'
    ASTValue *owner;
    llvm::Value *value;
    llvm::DIVariable debug; //XXX

    ASTValue() : value(0), owner(0) {}
    ASTValue(llvm::Value *val) : value(val), owner(0) {}

    void setOwner(ASTValue *v) { owner = v; }
    ASTValue *getOwner() { return owner; }

    virtual ASTType *getType() = 0;
    virtual bool isConst() { return false; } // TODO
    virtual bool isLValue() = 0;
    virtual bool isReference() = 0;

    virtual llvm::Value *codegenValue() { return NULL; }
    virtual llvm::Value *codegenLValue() { return NULL; }
    virtual llvm::Value *codegenRefValue() { return NULL; }
};

struct ASTBasicValue : ASTValue
{
    ASTType *type;
    bool lValue;
    bool reference;

    ASTBasicValue(ASTType *ty, llvm::Value *val, bool lv=false, bool ref=false) :
        ASTValue(val), type(ty), lValue(lv), reference(ref) {}

    virtual ASTType *getType() { return type; }
    virtual bool isLValue() { return lValue; }
    virtual bool isReference() { return reference; }
};

// TODO: use polymorphic values(?)

/*
struct TupleValue : public ASTValue
{
    TupleExpression *tuple;
    TupleValue(TupleExpression *texp) : ASTValue(texp->getType(), NULL, texp->isLValue()),
    tuple(texp)
    {}
};
*/

struct FunctionValue : public ASTValue {
    FunctionDeclaration *declaration;
    FunctionValue(FunctionDeclaration *fdecl=NULL) : declaration(fdecl) {

    }

    virtual ASTType *getType() { return declaration->getType(); }
    virtual bool isLValue() { return false; }
    virtual bool isReference() { return false; }
    bool isOverloaded() { return declaration->isOverloaded(); }
    FunctionDeclaration *getDeclaration() { return declaration; }

    FunctionValue *getNextOverload() {
        if(declaration->getNextOverload()) {
            FunctionValue *overload = new FunctionValue(declaration->getNextOverload());
            overload->setOwner(owner);
            return overload;
        }
        return NULL;
    }
};

struct MethodValue : public FunctionValue {
    ASTValue *instance; // is this the same as 'owner'?

    MethodValue(ASTValue *inst, FunctionDeclaration *fdecl) : FunctionValue(fdecl), instance(inst) {}
    ASTValue *getInstance() { return instance; }
};

#endif
