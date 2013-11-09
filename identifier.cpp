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
    if(TypeDeclaration *tdcl = dynamic_cast<TypeDeclaration*>(declaration))
    {
        return tdcl->type;
    }
}

ASTBasicType *Identifier::getBasicType() 
{ 
    if(ASTType *ty = getType())
    {
        return dynamic_cast<ASTBasicType *>(ty);
    }
    return NULL;
}
