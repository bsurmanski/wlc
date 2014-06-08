#ifndef _ASTVALUE_HPP
#define _ASTVALUE_HPP

#include "ast.hpp"

struct ASTValueInfo
{
    virtual ~ASTValueInfo(){}
};

struct ASTValue
{
    bool lValue;
    // value in which this value is indexing or owned by.
    // eg: if current value represents 'mc.func()', owner will be 'mc'
    ASTValue *owner;
    ASTType *type;
    llvm::Value *cgValue; //XXX
    llvm::DIVariable debug; //XXX
    ASTType *getType() { return type; }
    ASTValue(ASTType *ty, void *cgv = NULL, bool lv = false) : type(ty),
        cgValue((llvm::Value*) cgv), lValue(lv), owner(0) {}

    ASTValue(ASTValue *own, ASTType *ty, void *cgv = NULL, bool lv = false) : type(ty),
        cgValue((llvm::Value*) cgv), lValue(lv), owner(own) {}

    void setOwner(ASTValue *v) { owner = v; }
    ASTValue *getOwner() { return owner; }
    bool isLValue() { return lValue; }
};

// TODO: use polymorphic values

struct TupleValue : public ASTValue
{
    TupleExpression *tuple;
    TupleValue(TupleExpression *texp) : ASTValue(texp->getType(), NULL, texp->isLValue()),
    tuple(texp)
    {}
};

#endif
