#ifndef _AST_HPP
#define _AST_HPP

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include "symbolTable.hpp"
#include "identifier.hpp"

struct Statement;
struct Declaration;
struct Expression;
struct DeclarationExpression;
struct TranslationUnit;
struct ASTType;
struct Identifier;

//TODO: use
struct Package
{
    SymbolTable *scope;
    Identifier *identifier;
    std::vector<TranslationUnit*> units;
    Identifier *getIdentifier() { return identifier; }
};

// also Module
struct TranslationUnit
{
    SymbolTable *scope;
    Identifier *identifier;
    std::vector<Statement*> statements;
    TranslationUnit(Identifier *id) : identifier(id), scope(new SymbolTable) {}
    ~TranslationUnit() { delete scope; }
    std::string getName() { return ""; }
};

struct AST
{
    //TODO: packages
    std::map<std::string, TranslationUnit*> units;
};

//XXX ?
struct ASTValue
{
    ASTType *type;
    ASTValue(ASTType *ty) : type(ty) {}
    virtual ~ASTValue(){}
};

struct FunctionDeclaration;

struct ASTString : public ASTValue
{
    std::string value;
    ASTString(std::string str) : ASTValue(NULL), value(str) {} //TODO astvalue (string)
};

struct ASTTypeQual
{
    bool isConst;
    ASTTypeQual() : isConst(false) {}
};

struct ASTQualType
{
    bool isConst;
    bool isExtern;
    bool isStatic;
    Identifier *identifier;
    ASTQualType(Identifier *id) : identifier(id){}
    ASTQualType(const ASTQualType &qt) :   isConst(qt.isConst), 
                                    isExtern(qt.isExtern),
                                    isStatic(qt.isStatic),
                                    identifier(qt.identifier) {}
};

struct ASTPointerType;
struct ASTType
{
    Identifier *identifier;
    ASTPointerType *pointerTy;

    ASTType() {}
    virtual size_t size() const = 0;
};

struct ASTPointerType : public ASTType
{
    ASTType *ptrTo;
    ASTPointerType(ASTType *ty) : ptrTo(ty) { if(!ty->pointerTy) ty->pointerTy = this; }
    virtual size_t size() const { return sizeof(void*); }
};

struct ASTTypeSpec
{
    bool isSigned;
    ASTTypeSpec() : isSigned(false) {}
    ASTTypeSpec(bool defaultSigned) : isSigned(defaultSigned) {}
};

struct ASTBasicType : public ASTType
{
    size_t sz;
    ASTTypeSpec typeSpec;

    virtual size_t size() const { return sz; }
    ASTBasicType(size_t s, bool defaultSigned = false) : sz(s), typeSpec(defaultSigned) {}
};

struct ASTStructType : public ASTType
{
    std::vector<std::pair<ASTType*, std::string> > members;
    virtual size_t size() const
    {
        size_t sz = 0;
        for(int i = 0; i < members.size(); i++)
        {
            sz += members[i].first->size();
        }
    }
};

struct Declaration
{
    Identifier *identifier;
    Declaration(Identifier *id) : identifier(id) {}
    virtual ~Declaration(){}
    virtual std::string getName() { if(identifier) return identifier->getName(); return ""; }
};

struct FunctionPrototype 
{
    ASTQualType returnType;
    std::vector<std::pair<ASTQualType, std::string> > parameters;
    FunctionPrototype(ASTQualType rty, std::vector<std::pair<ASTQualType, std::string> > param) : returnType(rty), parameters(param) {}
};

struct FunctionDeclaration : public Declaration
{
    FunctionPrototype *prototype;
    Statement *body;
    FunctionDeclaration(Identifier *id, FunctionPrototype *p, Statement *st) : Declaration(id), prototype(p), body(st) {}
};

struct VariableDeclaration : public Declaration
{
    //Identifier *type;
    ASTQualType qualType;
    VariableDeclaration(ASTQualType ty, Identifier *nm) : Declaration(nm), qualType(ty) {}
};

struct TypeDeclaration : public Declaration
{
    ASTType *type;
    TypeDeclaration(Identifier *id, ASTType *ty) : Declaration(id), type(ty) {}
};

struct Expression
{
    Expression() {}
    virtual ~Expression(){}
};

struct PostfixExpression : public Expression
{
};

struct CallExpression : public PostfixExpression
{
    FunctionDeclaration *func;
    std::vector<Expression *> args;
};

struct IndexExpression : public PostfixExpression
{

};

struct MemberExpression : public PostfixExpression
{

};

struct UnaryExpression : public Expression
{
    Expression *lhs;
    unsigned op;
    UnaryExpression(unsigned o, Expression *l) : lhs(l), op(o) {}
};

struct BinaryExpression : public Expression
{
    Expression *lhs;
    Expression *rhs;
    unsigned op;
    BinaryExpression(unsigned o, Expression *l, Expression *r) : lhs(l), rhs(r), op(o) {}
};

struct PrimaryExpression : public Expression
{};

struct IdentifierExpression : public PrimaryExpression
{
    Identifier *id;
    IdentifierExpression(Identifier *i) : id(i) {}
    Identifier *identfier() { return id; }
};

struct StringExpression : public PrimaryExpression
{
    std::string *string;
    StringExpression(std::string *str) : string(new std::string(*str)) {}
};

struct NumericExpression : public PrimaryExpression
{
    enum NumericType
    {
        FLOAT,
        DOUBLE,
        CHAR,
        INT,
    };

    NumericType type;

    union
    {
        double floatValue;
        uint64_t intValue;
        char charValue;
    };

    NumericExpression(NumericType t, double val) : type(t), floatValue(val) {}
    NumericExpression(NumericType t, uint64_t val) : type(t), intValue(val) {}
};

struct DeclarationExpression : public Expression
{
    Declaration *decl; 
};

struct Statement;
// value of (bool) 0. if 'pass' is encountered, value of pass
struct BlockExpression : public Expression
{
    //TODO: sym table
    std::vector<Statement*> statements;
    BlockExpression(std::vector<Statement*> s) : statements(s) {}
};

// value of following expression, or (bool) 1 if not found

// value of following expression. usually block, with (bool) 0 / (bool) 1 pass
struct IfExpression : public Expression
{
    Expression *condition;
    Expression *body;
    Expression *elsebranch;
};

// value same as if
struct WhileExpression : public Expression
{
    Expression *condition;
    Expression *body;
};

struct ForExpression : public Expression
{
    Expression *decl;
    Expression *cond;
    Expression *update;
    Expression *body;
};

struct SwitchExpression : public Expression
{
    //TODO
};

struct ImportExpression : public Expression
{
    TranslationUnit *import;
};

struct Statement
{
    virtual ~Statement(){}
};

struct DeclarationStatement : public Statement
{
    Declaration *declaration;
    DeclarationStatement(Declaration *decl) : declaration(decl) {}
};

struct ExpressionStatement : public Statement
{
    Expression *expression;
    ExpressionStatement(Expression *exp) : expression(exp){}
};

// also works like pass. will assign to $ ?
struct ReturnStatement : public Statement
{
    Expression *expression;
    ReturnStatement(Expression *exp) : expression(exp) {}
};

struct PassStatement : public Statement 
{
    // XXX will assign to special variable '$', which is created on blockExpression entry. essentially, assigning on pass
    // can be transformed into if(true) $ = val; else $ = val2; x = $;
};

/*
 * once 
 * { 
 *  DOSTUFF 
 * }; : DOSTUFF will only happen once, on first encounter of the once statement
 */
struct OnceStatement : public Statement
{
    Statement *stmt;
};

#endif
