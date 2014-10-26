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
class ASTVisitor;
struct ASTScope;

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

    ScopeIterator() : scope(0) {}
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
	ScopeIterator &operator++();

    ScopeIterator operator++(int) {
        ScopeIterator it = ScopeIterator(*this);
        operator++();
        return it;
    }

	bool operator==(const ScopeIterator &it);
	bool operator!=(const ScopeIterator &it);

};

struct ASTNode;
struct ASTScope
{
    ASTScope *parent;
    Identifier *owner;
    std::vector<ASTScope*> siblings;
    std::map<std::string, Identifier *> symbols;
    std::map<std::string, bool> extensions;
    Package *package;

    enum ScopeType
    {
        Scope_Unowned, // no scope should be unowned (I think)
        Scope_Global, // contains globals. this is more like a 'module' scope...
        Scope_FunctionParameter, // only contains identifiers of current function parameters. thats it. Child scope should be 'Scope_Local' for the func body
        Scope_Struct, // within a struct or user type. struct members and methods expected here
        Scope_Local, // scope local to a function or method. runable code goes here
    };

    ScopeType type;

    bool isUnowned() { return type == Scope_Unowned; }
    bool isGlobalScope() { return type == Scope_Global; }
    bool isParameterScope() { return type == Scope_FunctionParameter; }
    bool isUserTypeScope() { return type == Scope_Struct; }
    bool isStructScope() { return type == Scope_Struct; }
    bool isLocalScope() { return type == Scope_Local; }

    typedef ScopeIterator iterator;
    ScopeIterator begin() { return ScopeIterator(this, ScopeIterator::ITER_ALL); }
    ScopeIterator end() { return ScopeIterator(this, symbols.end(), ScopeIterator::ITER_ALL); }

    void setOwner(Identifier *own) { owner = own; }
    Identifier *getOwner() { return owner; }

    ASTType* getUserTypeScope() {
        if(type == Scope_Struct && owner) {
            return owner->getDeclaredType();
        }
        return NULL;
    }

    bool extensionEnabled(std::string s)
    {
        return extensions[s] || (parent && parent->extensionEnabled(s));
    }

    bool enableExtension(std::string s)
    {
        extensions[s] = true;
		return true;
    }

    void dump();
    ASTScope(ASTScope *par = NULL, ScopeType st = Scope_Local, Package *pkg=0) :
        package(pkg), parent(par), type(st), owner(0)
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
	bool empty() { return symbols.empty(); }
    Identifier *getInScope(std::string); // retrieves and creates if non-existant (only from current scope, not parents)
    Identifier *get(std::string); // retrieves and creates if non-existant
    Identifier *lookup(std::string, bool imports=true); // same as 'get', but does not create on not-found
    Identifier *lookupInScope(std::string str);
    void remove(Identifier *id);
    Identifier *resolveIdentifier(Identifier *id);

    void accept(ASTVisitor *v);
};


#endif
