
#ifndef _IDENTIFIER_HPP
#define _IDENTIFIER_HPP

#include <string>
struct Declaration;
struct Expression;
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
        ID_MODULE,
        ID_CLASS,
        ID_STRUCT,
        ID_UNION,
        ID_ALIAS,
        ID_EXPRESSION,
        ID_LABEL,
    };

    IDType type;
    std::string name;
    Declaration *declaration;
    Expression *expression;
    union
    {
        struct
        {
            ASTValue *astValue;
            ASTValue *ref; // pointer to declared variable
        };
        ASTType *astType;
    };

    Identifier(std::string s, IDType t = ID_UNKNOWN) : type(t), name(s), declaration(NULL), astValue(NULL) {}
    void setDeclaration(Declaration *decl, IDType t = ID_UNKNOWN);
    Declaration *getDeclaration() { return declaration; }
    void setExpression(Expression *e) { expression = e; type = ID_EXPRESSION; }
    Expression *getExpression() { return expression; }
    std::string getName() { return name; }
    ASTType *getType();
    ASTType *declaredType();
    ASTValue *getReference();
    ASTValue *getValue();
    void setValue(ASTValue *value);
    bool isUndeclared() { return type == ID_UNKNOWN; }
    bool isVariable() { return type == ID_VARIABLE; }
    bool isFunction() { return type == ID_FUNCTION; }
    bool isStruct() { return type == ID_STRUCT; }
    bool isUnion() { return type == ID_UNION; }
    bool isPackage() { return type == ID_PACKAGE; }
    bool isModule() { return type = ID_MODULE; }
    bool isExpression() { return type == ID_EXPRESSION; }
    bool isLabel() { return type == ID_LABEL; }

};

#endif
