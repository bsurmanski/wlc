#include "ast.hpp"

#include <assert.h>

void Identifier::setDeclaration(Declaration *decl, IDType ty)
{
    assert(!declaration && "redefinition");
    declaration = decl;
    type = ty;
}

ASTType *Identifier::getType()
{
    if(VariableDeclaration *vdcl = dynamic_cast<VariableDeclaration*>(declaration))
    {
        return vdcl->type;
    }
}

ASTType *Identifier::declaredType()
{
    if(TypeDeclaration *tdcl = dynamic_cast<TypeDeclaration*>(declaration))
    {
        return tdcl->type;
    }
}

ASTValue *Identifier::getReference()
{
    return ref;
}

ASTValue *Identifier::getValue()
{
    return value; //TODO: do something... 
}

void Identifier::setValue(ASTValue *val)
{
    value = val;
}
