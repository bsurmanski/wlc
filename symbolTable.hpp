#ifndef _SYMBOLTABLE_HPP
#define _SYMBOLTABLE_HPP

#include <map>
#include <string>
#include "identifier.hpp"

typedef std::map<std::string, Identifier*>::iterator SymbolIterator;

struct SymbolTable
{
    SymbolTable *parent;
    SymbolTable(SymbolTable *par = NULL) : parent(par) { if(!par) addBuiltin(); }
    std::map<std::string, Identifier *> symbols;
    SymbolIterator begin() { return symbols.begin(); }
    SymbolIterator end() { return symbols.end(); }
    void addBuiltin();
    bool contains(std::string);
    Identifier *get(std::string); // retrieves and creates if non-existant
    Identifier *lookup(std::string); // same as 'get', but does not create on not-found
};

#endif
