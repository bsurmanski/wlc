#include "ast.hpp"
#include "message.hpp"

#include <assert.h>

void Identifier::setDeclaration(Declaration *decl, IDType ty)
{
    FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(declaration);
    if(!fdecl || (fdecl && fdecl->body))
        if(declaration)
        {
            emit_message(msg::FATAL, "redefinition of " + getName() +
                    "originally defined at " + declaration->loc.toString(), decl->loc);
        }
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
    //TODO: class
    if(type != ID_STRUCT && type != ID_UNKNOWN && type != ID_UNION) {
        //emit_message(msg::FAILURE, "this doesnt look like a type");
        return NULL;
    }
    if(!astType)
    {
        astType = new ASTType();
        astType->info = new NamedUnknownInfo(this, NULL);
    }
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
