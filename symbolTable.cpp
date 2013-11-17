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

Identifier *SymbolTable::lookup(std::string str)
{
    if(symbols.count(str))
    {
        // XXX work around for multiple declarations, and forward declarations
        if(symbols[str]->isUndeclared()) 
        {
            Identifier *parentLookup = parent->lookup(str);
            if(!parentLookup->isUndeclared()) return parentLookup;
        }
        return symbols[str];
    } else if (parent)
    {
        return parent->lookup(str); 
    }
    return NULL;
}
