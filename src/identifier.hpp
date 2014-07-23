
#ifndef _IDENTIFIER_HPP
#define _IDENTIFIER_HPP

#include <string>
struct Declaration;
struct Expression;
struct ASTType;
struct ASTValue;
struct ASTScope;
struct StructDeclaration;
struct UnionDeclaration;
struct ClassDeclaration;

struct Identifier
{
    enum IDType
    {
        ID_UNKNOWN,
        ID_TYPE,
        ID_VARIABLE,
        ID_FUNCTION,
        ID_PACKAGE,
        ID_MODULE,
        ID_USER, //user type
        ID_ALIAS,
        ID_EXPRESSION,
        ID_LABEL,
        ID_ANY,
    };

    bool isMangled;
    IDType kind;
    std::string name;
    std::string mangled; // mangled base name (without parameter/return qualifier)
    Declaration *declaration;
    Expression *expression;

    ASTScope *table; // scope in which identifier is defined
    union
    {
        struct
        {
            ASTValue *astValue;
            ASTValue *ref; // pointer to declared variable
        };
        ASTType *astType;
    };

    Identifier(ASTScope *ta, std::string s, IDType t = ID_UNKNOWN);
    void addDeclaration(Declaration *decl, IDType t = ID_UNKNOWN);
    Declaration *getDeclaration() { return declaration; }
    void setExpression(Expression *e) { expression = e; kind = ID_EXPRESSION; }
    Expression *getExpression() { return expression; }
    std::string getName() { return name; }
    std::string getMangledName();
    ASTType *getType();
    ASTType *getDeclaredType();
    void setDeclaredType(ASTType *ty);
    ASTValue *getReference();
    ASTValue *getValue();
    ASTScope *getScope() { return table; }
    void setValue(ASTValue *value);
    bool isUndeclared() { return kind == ID_UNKNOWN; }
    bool isVariable() { return kind == ID_VARIABLE; }
    bool isFunction() { return kind == ID_FUNCTION; }
    bool isUserType() { return kind == ID_USER; }
    bool isStruct();
    bool isUnion();
    bool isClass();
    bool isInterface();
    bool isPackage() { return kind == ID_PACKAGE; }
    bool isModule() { return kind == ID_MODULE; }
    bool isExpression() { return kind == ID_EXPRESSION; }
    bool isLabel() { return kind == ID_LABEL; }

    bool isTypeMember();
    ASTType *getMemberOwner();

    /*
    bool isTypeMember() { return getScope()->isUserTypeScope() && isVariable(); }
    bool isTypeMethod() { return getScope()->isUserTypeScope() && isFunction(); }
    */

};

#endif
