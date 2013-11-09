#include "ast.hpp"

#include <assert.h>

void Identifier::setDeclaration(Declaration *decl, IDType ty)
{
    assert(!declaration && "redefinition");
    declaration = decl;
    type = ty;
}
