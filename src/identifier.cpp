#include "ast.hpp"
#include "message.hpp"

#include <assert.h>

Identifier::Identifier(ASTScope *ta, std::string s, IDType t) :
    table(ta), kind(t), name(s), declaration(NULL), astValue(NULL), ref(NULL), isMangled(false), expression(0), astType(NULL)
{
    mangled = "";
}

void Identifier::addDeclaration(Declaration *decl, IDType ty)
{
    FunctionDeclaration *fdeclaration = dynamic_cast<FunctionDeclaration*>(declaration);
    FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(decl);
    if(fdecl && fdeclaration){
        if(fdecl->getType()->is(fdeclaration->getType())) {
            if(fdecl->body && fdeclaration->body) {
                emit_message(msg::ERROR, "redeclaration of function with identical parameters", decl->loc);
            }
            return;
        }
        fdecl->nextoverload = fdeclaration; // push decl into front of overload linked list
        declaration = decl;
        return;
    }

    // if already declared, but not a function; or if a function with a body and new decl is
    // not a function; then there is a conflict
    if((!fdeclaration && declaration) || (fdeclaration && fdeclaration->body)){
        emit_message(msg::FATAL, "redefinition of " + getName() +
                " originally defined at " + declaration->loc.toString(), decl->loc);
    }
    declaration = decl;
    kind = ty;
}

ASTType *Identifier::getType()
{
    if(kind == ID_EXPRESSION){
        return expression->getType();
    }

    if(declaration)
        return declaration->getType();

    return NULL;
}

Declaration *Identifier::getDeclaration() {
    if(kind == ID_EXPRESSION) {
        if(IdentifierExpression *iexp = expression->identifierExpression()) {
            return iexp->getDeclaration();
        }
    }
    return declaration;
}

ASTType *Identifier::getDeclaredType()
{
    //XXX special case for c typedef
    if(kind == ID_TYPE) return astType;

    //TODO: class
    if(kind != ID_USER && kind != ID_UNKNOWN) {
        if(kind == ID_EXPRESSION && expression && expression->typeExpression()) {
            return expression->typeExpression()->getDeclaredType();
        }
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
