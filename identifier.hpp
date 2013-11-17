
#ifndef _IDENTIFIER_HPP
#define _IDENTIFIER_HPP

#include <string>
struct Declaration;
struct ASTType;
struct ASTValue;

struct Identifier
{
    enum IDType
    {
        ID_UNKNOWN,
        ID_TYPE,
        ID_VARIABLE,
        ID_FUNCTION,
        ID_PACKAGE,
        ID_CLASS,
        ID_STRUCT,
        ID_ALIAS,
    };

    IDType type;
    std::string name;
    Declaration *declaration;
    ASTValue *value;

    Identifier(std::string s, IDType t = ID_UNKNOWN) : type(t), name(s), declaration(NULL), value(NULL) {}
    void setDeclaration(Declaration *decl, IDType t = ID_UNKNOWN);
    Declaration *getDeclaration() { return declaration; }
    std::string getName() { return name; }
    ASTType *getType();
    ASTType *declaredType();
    bool isUndeclared() { return type == ID_UNKNOWN; }
    bool isVariable() { return type == ID_VARIABLE; }
    bool isFunction() { return type == ID_FUNCTION; }
    bool isStruct() { return type == ID_STRUCT; }

};

#endif
