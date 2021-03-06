#ifndef _CODEGENCONTEXT_HPP
#define _CODEGENCONTEXT_HPP

#include "ast.hpp"
#include "astValue.hpp"

class CodegenContext
{
    protected:
    CodegenContext(){}

    public:
    virtual ASTValue *codegenExpression(Expression *exp) = 0;
};

#endif
