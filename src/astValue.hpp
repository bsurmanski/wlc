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
    ASTType *type;
    llvm::Value *cgValue; //XXX
    llvm::DIVariable debug; //XXX
    ASTType *getType() { return type; }
    ASTValue(ASTType *ty, void *cgv = NULL, bool lv = false) : type(ty),
        cgValue((llvm::Value*) cgv), lValue(lv) {}
    bool isLValue() { return lValue; }
};

struct TupleValue : public ASTValue
{
    TupleExpression *tuple;
    TupleValue(TupleExpression *texp) : ASTValue(texp->getType(), NULL, texp->isLValue()),
    tuple(texp)
    {}
};

#endif
