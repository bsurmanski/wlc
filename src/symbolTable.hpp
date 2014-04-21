#ifndef _SYMBOLTABLE_HPP
#define _SYMBOLTABLE_HPP

#include <map>
#include <vector>
#include <string>
#include "identifier.hpp"

typedef std::map<std::string, Identifier*>::iterator SymbolIterator;
struct Package;
struct TranslationUnit;

struct SymbolTable
{
    SymbolTable *parent;
    std::vector<SymbolTable*> siblings;
    std::map<std::string, Identifier *> symbols;
    std::map<std::string, bool> extensions;
    Package *package;

    enum ScopeType
    {
        Scope_Global,
        Scope_FunctionParameter,
        Scope_Struct,
        Scope_Local,
    };

    ScopeType type;

    bool extensionEnabled(std::string s)
    {
        return extensions[s] || (parent && parent->extensionEnabled(s));
    }

    bool enableExtension(std::string s)
    {
        extensions[s] = true;
    }

    void dump();
    SymbolTable(SymbolTable *par = NULL, ScopeType st = Scope_Local, Package *pkg=0) :
        package(pkg), parent(par), type(st)
    {
        if(!par) addBuiltin();
        if(par) package = par->package;
    }

    std::string getMangledName();

    Package *getPackage() { return package; }
    TranslationUnit *getUnit();

    ScopeType getScopeType() { return type; }
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
