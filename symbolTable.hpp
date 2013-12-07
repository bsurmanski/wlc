#ifndef _SYMBOLTABLE_HPP
#define _SYMBOLTABLE_HPP

#include <map>
#include <vector>
#include <string>
#include "identifier.hpp"

typedef std::map<std::string, Identifier*>::iterator SymbolIterator;

struct SymbolTable
{
    SymbolTable *parent;
    std::vector<SymbolTable*> siblings;
    std::map<std::string, Identifier *> symbols;

    SymbolTable(SymbolTable *par = NULL) : parent(par) { if(!par) addBuiltin(); }
    SymbolIterator begin() { return symbols.begin(); }
    SymbolIterator end() { return symbols.end(); }
    void addSibling(SymbolTable *t);
    void addBuiltin();
    bool contains(std::string);
    Identifier *getInScope(std::string); // retrieves and creates if non-existant (only from current scope, not parents)
    Identifier *get(std::string); // retrieves and creates if non-existant
    Identifier *lookup(std::string, bool imports=true); // same as 'get', but does not create on not-found
};

#endif
