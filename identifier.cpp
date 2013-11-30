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
    assert(type == ID_STRUCT || type == ID_UNKNOWN && "this doesnt look like a type");
    if(!astType) astType = new ASTType();
    return astType;
}

ASTValue *Identifier::getReference()
{
    return ref;
}

ASTValue *Identifier::getValue()
{
    return astValue; //TODO: do something... 
}

void Identifier::setValue(ASTValue *val)
{
    astValue = val;
}
