#include "ast.hpp"
#include "message.hpp"

#include <assert.h>

Identifier::Identifier(ASTScope *ta, std::string s, IDType t) :
    table(ta), type(t), name(s), declaration(NULL), astValue(NULL), isMangled(false), expression(0)
{
    mangled = "";
}

void Identifier::setDeclaration(Declaration *decl, IDType ty)
{
    FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(declaration);
    if(!fdecl || (fdecl && fdecl->body))
        if(declaration)
        {
            emit_message(msg::FATAL, "redefinition of " + getName() +
                    " originally defined at " + declaration->loc.toString(), decl->loc);
        }
    declaration = decl;
    type = ty;
}

ASTType *Identifier::getType()
{
    if(type == ID_EXPRESSION){
        return expression->getType();
    }

    return declaration->getType();
}

ASTType *Identifier::getDeclaredType()
{
    //TODO: class
    if(type != ID_USER && type != ID_UNKNOWN) {
        //emit_message(msg::FAILURE, "this doesnt look like a type");
        return NULL;
    }
    if(!astType)
    {
        astType = new ASTUserType(this);
    }
    return astType;
}

void Identifier::setDeclaredType(ASTType *ty)
{
    astType = ty;
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

std::string Identifier::getMangledName() {
    if(noMangle) return getName();

    if(!isMangled) {
        mangled = table->getMangledName() + "$" + getName();
        isMangled = true;
    }
    return mangled;
}

bool Identifier::isStruct() { return dynamic_cast<StructDeclaration*>(declaration); }
bool Identifier::isUnion() { return dynamic_cast<UnionDeclaration*>(declaration); }
bool Identifier::isClass() { return dynamic_cast<ClassDeclaration*>(declaration); }
