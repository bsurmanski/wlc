#ifndef _SYMBOLTABLE_HPP
#define _SYMBOLTABLE_HPP

#include <map>
#include <string>
#include "identifier.hpp"

struct SymbolTable
{
    SymbolTable *parent;
    SymbolTable() : parent(NULL) { addBuiltin(); }
    std::map<std::string, Identifier *> symbols;
    void addBuiltin();
    bool contains(std::string);
    Identifier *get(std::string); // retrieves and creates if non-existant
    Identifier *lookup(std::string); // same as 'get', but does not create on not-found
};

#endif
