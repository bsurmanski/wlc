#ifndef _SYMBOLTABLE_HPP
#define _SYMBOLTABLE_HPP

#include <map>
#include <vector>
#include <string>
#include <iterator>
#include "identifier.hpp"

//typedef std::map<std::string, Identifier*>::iterator SymbolIterator;
struct Package;
struct TranslationUnit;
struct ASTVisitor;

// TODO: create specialized iterator, that can traverse dependent on variable type
// iterates different declaration kinds
struct ScopeIterator {
    enum Type {
        ITER_ALL,
        ITER_PACKAGE,
        ITER_TYPE,
        ITER_VARIABLES,
        ITER_FUNCTIONS,
        ITER_UNKNOWN,
    };
    ASTScope *scope;
    Type type;
    std::map<std::string, Identifier*>::iterator base;
    bool recurse;

    ScopeIterator() : scope(0){}
    ScopeIterator(ASTScope *sc, Type t, bool rec=false);
    ScopeIterator(ASTScope *sc, std::map<std::string, Identifier*>::iterator b,
            Type t, bool rec=false);
    ScopeIterator(const ScopeIterator& it){
        scope = it.scope;
        type = it.type;
        base = it.base;
        recurse = it.recurse;
    }

    Identifier *operator*(){ return base->second; }
    Identifier *operator->(){ return base->second; }
    ScopeIterator &operator++() {
        base++;
        return *this;
    }

    ScopeIterator operator++(int) {
        ScopeIterator it = ScopeIterator(*this);
        operator++();
        return it;
    }

    bool operator==(const ScopeIterator &it){ return it.base == base; }
    bool operator!=(const ScopeIterator &it){ return !(*this == it); }

};

struct ASTNode;
struct ASTScope
{
    ASTScope *parent;
    ASTNode *owner;
    std::vector<ASTScope*> siblings;
    std::map<std::string, Identifier *> symbols;
    std::map<std::string, bool> extensions;
    Package *package;

    enum ScopeType
    {
        Scope_Unowned,
        Scope_Global,
        Scope_FunctionParameter,
        Scope_Struct,
        Scope_Local,
    };

    ScopeType type;

    typedef ScopeIterator iterator;
    ScopeIterator begin() { return ScopeIterator(this, ScopeIterator::ITER_ALL); }
    ScopeIterator end() { return ScopeIterator(this, symbols.end(), ScopeIterator::ITER_ALL); }

    void setOwner(ASTNode *own) { owner = own; }
    /*
    Package *isPackageScope() { return dynamic_cast<Package*>(owner); }
    FunctionDeclaration *isFunctionScope() { return dynamic_cast<FunctionDeclaration*>(owner); }
    UserTypeDeclaration *isUserTypeScope() { return dynamic_cast<UserTypeDeclaration*>(owner); }
    */

    bool extensionEnabled(std::string s)
    {
        return extensions[s] || (parent && parent->extensionEnabled(s));
    }

    bool enableExtension(std::string s)
    {
        extensions[s] = true;
    }

    void dump();
    ASTScope(ASTScope *par = NULL, ScopeType st = Scope_Local, Package *pkg=0) :
        package(pkg), parent(par), type(st)
    {
        if(!par) addBuiltin();
        if(par) package = par->package;
    }

    std::string getMangledName();

    Package *getPackage() { return package; }
    TranslationUnit *getUnit();

    ScopeType getScopeType() { return type; }
    void addSibling(ASTScope *t);
    void addBuiltin();
    bool contains(std::string);
    Identifier *getInScope(std::string); // retrieves and creates if non-existant (only from current scope, not parents)
    Identifier *get(std::string); // retrieves and creates if non-existant
    Identifier *lookup(std::string, bool imports=true); // same as 'get', but does not create on not-found
    Identifier *lookupInScope(std::string str);
    void remove(Identifier *id);
    Identifier *resolveIdentifier(Identifier *id);

    void accept(ASTVisitor *v);
};


#endif
