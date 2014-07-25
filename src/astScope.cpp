#include "astScope.hpp"
#include "astVisitor.hpp"
#include "ast.hpp"

#include <iostream>

ScopeIterator::ScopeIterator(ASTScope *sc, Type t, bool rec) :
    scope(sc), type(t), recurse(rec) {
    base = scope->symbols.begin();
}

ScopeIterator::ScopeIterator(ASTScope *sc, std::map<std::string, Identifier*>::iterator b,
        Type t, bool rec) : scope(sc), base(b), type(t), recurse(rec) {
            }

void ASTScope::addBuiltin()
{
    //XXX dont think this is needed
//#define BTYPE(X,SZ,SIGN) Identifier *id_##X = get(#X); id_##X->addDeclaration(new TypeDeclaration(id_##X, new ASTBasicType(id_##X,SZ,SIGN)), Identifier::ID_TYPE);
//#include "tokenkinds.def"
}

void ASTScope::accept(ASTVisitor *v){
    v->visitScope(this);
}

void ASTScope::dump()
{
    std::map<std::string, Identifier *>::iterator it = symbols.begin();
    for(; it != symbols.end(); it++)
    {
        std::cout << it->first << ": " << it->second->getName() << std::endl;
    }
}

bool ASTScope::contains(std::string str)
{
    return symbols.count(str) || (parent && parent->contains(str));
}

void ASTScope::addSibling(ASTScope *t)
{
    siblings.push_back(t);
}

Identifier *ASTScope::getInScope(std::string str)
{
    if(symbols.count(str))
    {
        return symbols[str];
    } else
    {
        Identifier *id = new Identifier(this, str);
        symbols[str] = id;
        return id;
    }
}

Identifier *ASTScope::get(std::string str)
{
    Identifier *id = lookup(str);

    if(!id)
    {
        id = new Identifier(this, str);
        symbols[str] = id;
    }

    return id;
}

Identifier *ASTScope::lookup(std::string str, bool imports)
{
    Identifier *ret = NULL;
    Identifier *id = NULL;
    if(symbols.count(str))
    {
        // XXX work around for multiple declarations, and forward declarations
        if(symbols[str]->isUndeclared() && parent)
        {
            id = parent->lookup(str);
            if(id && !id->isUndeclared()) ret = id;
        }
        ret = symbols[str];
    }

    if((!ret || ret->isUndeclared()) && parent)
    {
        id = parent->lookup(str);
        if(id && !id->isUndeclared()) ret = id;
    }


    if((!ret || ret->isUndeclared()) && imports)
    {
        for(int i = 0; i < siblings.size() && !ret; i++)
        {
            id = siblings[i]->lookup(str, false);
            if(id && !id->isUndeclared())
                ret = id;
        }
    }

    return ret;
}

Identifier *ASTScope::lookupInScope(std::string str) {
    if(symbols.count(str)){
        return symbols[str];
    }
    return NULL;
}

void ASTScope::remove(Identifier *id){
    symbols.erase(id->getName());
}

TranslationUnit *ASTScope::getUnit()
{
    return dynamic_cast<TranslationUnit*>(package);
}

std::string ASTScope::getMangledName()
{
    if(owner){
        if(parent){
            return parent->getMangledName() + "$" + owner->getName();
        }
        return owner->getName();
    }
    return "";
}

Identifier *ASTScope::resolveIdentifier(Identifier *id)
{
    if(!id->isUndeclared()){
        return id;
    }

    Identifier *res = lookup(id->getName(), true);
    if(id != res){
        remove(id);
        id = res;
    }
    return id;
}
