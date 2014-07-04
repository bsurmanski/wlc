
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

    bool noMangle;
    bool isMangled;
    IDType type;
    std::string name;
    std::string mangled;
    Declaration *declaration;
    Expression *expression;
    ASTScope *table;
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
    void setDeclaration(Declaration *decl, IDType t = ID_UNKNOWN);
    Declaration *getDeclaration() { return declaration; }
    void setExpression(Expression *e) { expression = e; type = ID_EXPRESSION; }
    Expression *getExpression() { return expression; }
    void setMangle(bool val) { noMangle = !val; }
    std::string getName() { return name; }
    std::string getMangledName();
    ASTType *getType();
    ASTType *getDeclaredType();
    void setDeclaredType(ASTType *ty);
    ASTValue *getReference();
    ASTValue *getValue();
    ASTScope *getScope() { return table; }
    void setValue(ASTValue *value);
    bool isUndeclared() { return type == ID_UNKNOWN; }
    bool isVariable() { return type == ID_VARIABLE; }
    bool isFunction() { return type == ID_FUNCTION; }
    bool isUserType() { return type == ID_USER; }
    bool isStruct();
    bool isUnion();
    bool isClass();
    bool isPackage() { return type == ID_PACKAGE; }
    bool isModule() { return type = ID_MODULE; }
    bool isExpression() { return type == ID_EXPRESSION; }
    bool isLabel() { return type == ID_LABEL; }

    bool isTypeMember();
    ASTType *getMemberOwner();

    /*
    bool isTypeMember() { return getScope()->isUserTypeScope() && isVariable(); }
    bool isTypeMethod() { return getScope()->isUserTypeScope() && isFunction(); }
    */

};

#endif
