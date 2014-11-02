#ifndef _ASTVALUE_HPP
#define _ASTVALUE_HPP

#include "ast.hpp"

struct ASTValueInfo
{
    virtual ~ASTValueInfo(){}
};

struct TypeValue;

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
    virtual bool isConstant() { return false; } // TODO
    virtual bool isWeak() { return false; } // XXX DITTO
    virtual bool isNoFree() { return false; } // if stack allocated; only destruct, no free
    virtual bool isLValue() = 0;
    virtual bool isReference() = 0;

    virtual llvm::Value *codegenValue() { return NULL; }
    virtual llvm::Value *codegenLValue() { return NULL; }
    virtual llvm::Value *codegenRefValue() { return NULL; }
    virtual bool isTypeValue() { return false; }
    virtual TypeValue *asTypeValue() { return NULL; }
};

struct ASTBasicValue : ASTValue
{
    ASTType *type;
    bool lValue;
    bool reference;
    bool weak;
    bool constant;

    bool nofree; // should not free on deallocate. (is stack allocated)

    ASTBasicValue(ASTType *ty, llvm::Value *val, bool lv=false, bool ref=false) :
        ASTValue(val), type(ty), lValue(lv), reference(ref), weak(false), constant(false), nofree(false) {}

    virtual ASTType *getType() { return type; }
    virtual bool isLValue() { return lValue; }
    virtual bool isReference() { return reference; }
    virtual bool isWeak() { return weak; }
    virtual void setWeak(bool b) { weak = b; }
    virtual bool isConstant() { return constant; }
    virtual void setConstant(bool b) { constant = b; }
    virtual void setNoFree(bool b) { nofree = b; }
    virtual bool isNoFree() { return nofree; }
};

struct TypeValue : ASTValue {
    ASTType *type;
    TypeValue(ASTType *ty) : ASTValue(NULL), type(ty) {}
    virtual ASTType *getType() { return NULL; } //XXX should return a 'TYPE' type
    virtual ASTType *getReferencedType() { return type; }
    virtual bool isLValue() { return false; }
    virtual bool isReference() { return false; }
    virtual bool isTypeValue() { return true; }
    virtual TypeValue *asTypeValue() { return this; }
};

struct TupleValue : public ASTValue
{
    //TupleExpression *tuple;
    ASTCompositeType *type;
    std::vector<ASTValue*> values;

    TupleValue(std::vector<ASTValue*> vals) : values(vals), ASTValue(NULL), type(NULL) {}

    virtual bool isLValue() {
        for(int i = 0; i < values.size(); i++) {
            if(!values[i]->isLValue())
                return false;
        }
        return true;
    }

    virtual bool isConstant() {
        for(int i = 0; i < values.size(); i++) {
            if(!values[i]->isConstant())
                return false;
        }
        return true;
    }

    void setType(ASTCompositeType *ty) {
        type = ty;
    }

    virtual ASTType *getType() {
        std::vector<ASTType*> tupty;

        if(type) return type;

        for(int i = 0; i < values.size(); i++) {
            tupty.push_back(values[i]->getType());
        }

        type = (ASTTupleType*) ASTType::getTupleTy(tupty);
        return type;
    }
    virtual bool isReference() { return false; }
};

struct FunctionValue : public ASTValue {
    FunctionDeclaration *declaration;
    FunctionValue(FunctionDeclaration *fdecl=NULL) : ASTValue(NULL), declaration(fdecl) {

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
