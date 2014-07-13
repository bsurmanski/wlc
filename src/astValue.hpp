#ifndef _ASTVALUE_HPP
#define _ASTVALUE_HPP

#include "ast.hpp"

struct ASTValueInfo
{
    virtual ~ASTValueInfo(){}
};

struct ASTValue
{
    bool reference; //XXX both reference *and* lValue? seems a bit cludgey
    bool lValue;
    // value in which this value is indexing or owned by.
    // eg: if current value represents 'mc.func()', owner will be 'mc'
    ASTValue *owner;
    ASTType *type;
    llvm::Value *cgValue; //XXX
    llvm::DIVariable debug; //XXX
    ASTType *getType() { return type; }

    /*
    ASTValue(ASTType *ty) : type(ty),
        cgValue((llvm::Value*) NULL), lValue(false), reference(ty->isReference()), owner(0) {}
        */

    ASTValue(ASTType *ty, void *cgv) : type(ty),
        cgValue((llvm::Value*) cgv), lValue(false), reference(ty->isReference()), owner(0) {}

    ASTValue(ASTType *ty, void *cgv, bool lv) : type(ty),
        cgValue((llvm::Value*) cgv), lValue(lv), reference(ty->isReference()), owner(0) {}

    ASTValue(ASTType *ty, void *cgv, bool lv, bool ref) : type(ty),
        cgValue((llvm::Value*) cgv), lValue(lv), reference(ref), owner(0) {}

    void setOwner(ASTValue *v) { owner = v; }
    ASTValue *getOwner() { return owner; }
    bool isLValue() { return lValue; }
    bool isReference() { return reference; }
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

/*
struct FunctionValue : public ASTValue {
    FunctionDeclaration *declaration;
    FunctionValue(ASTType *ty, void *cgv = NULL, bool lv = false, FunctionDeclaration *fdecl=NULL) : ASTValue(ty, cgv, lv) {

    }
};*/

#endif
