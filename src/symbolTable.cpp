#include "symbolTable.hpp"
#include "ast.hpp"

void SymbolTable::addBuiltin()
{
    //XXX dont think this is needed
//#define BTYPE(X,SZ,SIGN) Identifier *id_##X = get(#X); id_##X->setDeclaration(new TypeDeclaration(id_##X, new ASTBasicType(id_##X,SZ,SIGN)), Identifier::ID_TYPE);
//#include "tokenkinds.def"
}


bool SymbolTable::contains(std::string str)
{
    return symbols.count(str) || (parent && parent->contains(str));
}

void SymbolTable::addSibling(SymbolTable *t)
{
    siblings.push_back(t);
}

Identifier *SymbolTable::getInScope(std::string str)
{
    if(symbols.count(str))
    {
        return symbols[str];
    } else
    {
        Identifier *id = new Identifier(str);
        symbols[str] = id;
        return id;
    }
}

Identifier *SymbolTable::get(std::string str)
{
    Identifier *id = lookup(str);

    if(!id)
    {
        id = new Identifier(str);
        symbols[str] = id;
    }

    return id;
}

Identifier *SymbolTable::lookup(std::string str, bool imports)
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
