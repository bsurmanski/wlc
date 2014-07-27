#ifndef _IRVALUE_HPP
#define _IRVALUE_HPP

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include "astType.hpp"

struct IRValue : ASTValue
{
    llvm::DIVariable debug;
    llvm::Value *llvmValue;

    virtual llvm::Value *codegenRef() { assert(false && "not an lvalue"); }
    virtual llvm::Value *codegenLValue() { assert(false && "not an lvalue"); }
    virtual llvm::Value *codegen() = 0;
    //operator llvm::Value*() const { return codegen(); }
};

struct IRBasicValue : public ASTBasicValue, public IRValue {
    llvm::Value *value;

    IRBasicValue(ASTType *ty, llvm::Value *val, bool lv=false, bool ref=false) :
        ASTBasicValue(ty,lv,ref), value(val) {
    }

    virtual llvm::Value *codegen() {
        return value;
    }
};

struct IRFunctionValue : public FunctionValue, public IRValue {
    virtual llvm::Value *codegen();
};

struct IRMethodValue : public MethodValue, public IRValue {
    virtual llvm::Value *codegen();
};

#endif
