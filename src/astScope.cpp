#include "astScope.hpp"
#include "ast.hpp"

#include <iostream>

void ASTScope::addBuiltin()
{
    //XXX dont think this is needed
//#define BTYPE(X,SZ,SIGN) Identifier *id_##X = get(#X); id_##X->setDeclaration(new TypeDeclaration(id_##X, new ASTBasicType(id_##X,SZ,SIGN)), Identifier::ID_TYPE);
//#include "tokenkinds.def"
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
            if(id && !id->isUndeclared()) ret = id;
        }
    }

    return ret;
}

TranslationUnit *ASTScope::getUnit()
{
    return dynamic_cast<TranslationUnit*>(package);
}

std::string ASTScope::getMangledName()
{
    return package->getMangledName();
}
