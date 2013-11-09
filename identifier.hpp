
#ifndef _IDENTIFIER_HPP
#define _IDENTIFIER_HPP

#include <string>
struct Declaration;
struct ASTType;
struct ASTBasicType;

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

    Identifier(std::string s, IDType t = ID_UNKNOWN) : type(t), name(s), declaration(NULL) {}
    void setDeclaration(Declaration *decl, IDType t = ID_UNKNOWN);
    std::string getName() { return name; }
    ASTType *getType();
    ASTBasicType *getBasicType();
};

#endif
