#include "ast.hpp"
#include "message.hpp"

#include <assert.h>

Identifier::Identifier(ASTScope *ta, std::string s, IDType t) :
    table(ta), kind(t), name(s), declaration(NULL), astValue(NULL), isMangled(false), expression(0), astType(NULL)
{
    mangled = "";
}

void Identifier::addDeclaration(Declaration *decl, IDType ty)
{
    FunctionDeclaration *fdeclaration = dynamic_cast<FunctionDeclaration*>(declaration);
    FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(decl);
    if(fdecl && fdeclaration && fdecl->body){
        fdecl->nextoverload = fdeclaration; // push decl into front of overload linked list
        declaration = decl;
        return;
    }

    if(!fdeclaration || (fdeclaration && fdeclaration->body)){
        if(declaration)
        {
            emit_message(msg::FATAL, "redefinition of " + getName() +
                    " originally defined at " + declaration->loc.toString(), decl->loc);
        }
    }
    declaration = decl;
    kind = ty;
}

ASTType *Identifier::getType()
{
    if(kind == ID_EXPRESSION){
        return expression->getType();
    }

    return declaration->getType();
}

ASTType *Identifier::getDeclaredType()
{
    //TODO: class
    if(kind != ID_USER && kind != ID_UNKNOWN) {
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
    if(ty->kind == TYPE_UNKNOWN)
        emit_message(msg::WARNING, "SETTING UNKNOWN DECL ON IDENT");
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
    if(!isMangled) {
        mangled = table->getMangledName() + "$" + getName();
        isMangled = true;
    }
    return mangled;
}

bool Identifier::isStruct() { return dynamic_cast<StructDeclaration*>(declaration); }
bool Identifier::isUnion() { return dynamic_cast<UnionDeclaration*>(declaration); }
bool Identifier::isClass() { return dynamic_cast<ClassDeclaration*>(declaration); }
bool Identifier::isInterface() { return dynamic_cast<InterfaceDeclaration*>(declaration); }

bool Identifier::isTypeMember() { return getScope()->isUserTypeScope(); }
ASTType *Identifier::getMemberOwner(){ return getScope()->getUserTypeScope(); }
