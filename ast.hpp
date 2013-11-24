#ifndef _AST_HPP
#define _AST_HPP

#include <assert.h>
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
struct VariableDeclaration;
struct FunctionDeclaration;
struct ImportExpression;
struct TypeDeclaration;

//TODO: use
struct Package
{
    SymbolTable *scope;
    Identifier *parent;
    Identifier *identifier;
    //std::vector<TranslationUnit*> units;
    Identifier *getIdentifier() { return identifier; }
};

// also Module
struct TranslationUnit
{
    SymbolTable *scope;
    Identifier *identifier;
    std::vector<ImportExpression*> imports; //TODO: aliased imports?
    std::vector<TypeDeclaration*> types;
    std::vector<VariableDeclaration*> globals;
    std::vector<FunctionDeclaration*> functions;
    TranslationUnit(Identifier *id) : identifier(id), scope(new SymbolTable) {}
    ~TranslationUnit() { delete scope; }
    std::string getName() { return ""; }
};

struct AST
{
    //TODO: packages
    std::map<std::string, TranslationUnit*> units;
};

struct FunctionDeclaration;

struct ASTTypeQual
{
    bool isConst;
    bool isExtern;
    bool isStatic;
    ASTTypeQual() : isConst(false), isExtern(false), isStatic(false) {}
    ASTTypeQual(bool c, bool e, bool s) : isConst(c), isExtern(e), isStatic(s) {}
};

struct ASTConcreteType;
struct ASTPointerType;
struct ASTArrayType;

enum ASTTypeEnum
{
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_CLASS,
    TYPE_FUNCTION,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_VEC,
};
struct ASTType
{
    // XXX pointerTy leaks when used as pass by value TODO: pass by reference only
    ASTTypeEnum type;
    ASTPointerType *pointerTy;
    //ASTArrayType *arrayTy;
    void *cgType;

    union DOSTUFF
    {
        void *pointerInfo;
        void *arrayInfo;
        void *functionInfo;
    };

    ASTType(enum ASTTypeEnum ty) : type(ty), pointerTy(0), cgType(0)
    {}

    ASTType() : pointerTy(NULL), cgType(NULL) {}
    //ASTType(ASTTypeQual q) : qual(q), unqual(NULL), pointerTy(NULL), cgType(NULL) {}
    //virtual ~ASTType() { delete pointerTy; }
    ASTType *getUnqual();
    ASTPointerType *getPointerTy();
    ASTArrayType *getArrayTy(int len); //TODO: different lengths are different types??
    virtual ASTPointerType *asPointerTy() { return NULL;}
    virtual ASTConcreteType *asConcreteTy() { return NULL;}
    virtual ASTType *getReferencedTy() { return NULL; }
    virtual size_t size() const 
    {
        switch(type)
        {
            case TYPE_CHAR: 
            case TYPE_BOOL: return 1;
            case TYPE_SHORT: return 2;
            case TYPE_INT: return 3;
            case TYPE_LONG: return 4;
            default: assert(false && "havent sized this type yet"); return 0; //TODO
        }
    };

    bool isAggregate() { return type == TYPE_STRUCT || type == TYPE_UNION || type == TYPE_CLASS; }
    bool isClass() { return type == TYPE_CLASS; }
    bool isStruct() { return type == TYPE_STRUCT; }
    bool isInteger() { return type == TYPE_BOOL || type == TYPE_CHAR || type == TYPE_SHORT || type == TYPE_INT || type == TYPE_LONG; }
    bool isFloating() { return type == TYPE_FLOAT || type == TYPE_DOUBLE; }
    bool isVector() { return type == TYPE_VEC; }
    bool isPointer() { return this && type == TYPE_POINTER; } //TODO: shouldnt test for this

    static ASTType *voidTy;
    static ASTType *boolTy;
    static ASTType *charTy;
    static ASTType *intTy;
    static ASTType *longTy;

    static ASTType *getVoidTy();
    static ASTType *getBoolTy();
    static ASTType *getIntTy();
    static ASTType *getCharTy();
    static ASTType *getLongTy();
};

/*
 * named types, as apposed to reference/pointer types
 */
struct ASTConcreteType : public ASTType
{
    Identifier *identifier;
    ASTConcreteType(Identifier *id) : identifier(id) {}
    virtual ASTConcreteType *asConcreteTy() { return this; }
};

struct ASTPointerType : public ASTType
{
    ASTType *ptrTo;
    ASTPointerType(ASTType *ty) : ASTType(TYPE_POINTER), ptrTo(ty) { if(!ty->pointerTy) ty->pointerTy = this; }
    virtual ASTType *getReferencedTy() { return ptrTo; }
    virtual size_t size() const { return sizeof(void*); }
    virtual ASTPointerType *asPointerTy() { return this; }
};

struct ASTArrayType : public ASTType
{
    ASTType *ptrTo;
    uint64_t length;
    virtual ASTType *getReferencedTy() { return ptrTo; }
    //ASTArrayType(ASTType *ty, int len) : ASTType(TYPE_ARRAY), ptrTo(ty) { if(!ty->arrayTy) ty->arrayTy = this; }
};

struct ASTStructType : public ASTConcreteType
{
    std::vector<std::pair<Identifier *, Identifier *> > members; // <type, name>
    ASTStructType(Identifier *id, std::vector<std::pair<Identifier*,Identifier*> > m) : 
        ASTConcreteType(id), members(m) {}
};

struct ASTValue
{
    ASTType *type;
    void *cgValue;
    ASTValue(ASTType *ty, void *cgv = NULL) : type(ty), cgValue(cgv) {}
};

/***
 *
 * DECLARATIONS
 *
 ***/

struct Declaration
{
    Identifier *identifier;
    Declaration(Identifier *id) : identifier(id) {}
    virtual ~Declaration(){}
    virtual std::string getName() { if(identifier) return identifier->getName(); return ""; }
};

struct FunctionPrototype 
{
    ASTType *returnType;
    std::vector<std::pair<ASTType*, std::string> > parameters;
    bool vararg;
    FunctionPrototype(ASTType *rty, std::vector<std::pair<ASTType*, std::string> > param, bool varg = false) : returnType(rty), parameters(param), vararg(varg) {}
};

struct FunctionDeclaration : public Declaration
{
    FunctionPrototype *prototype;
    SymbolTable *scope; 
    Statement *body;
    FunctionDeclaration(Identifier *id, FunctionPrototype *p, SymbolTable *sc, Statement *st) : Declaration(id), prototype(p), scope(sc), body(st) {}
};

struct VariableDeclaration : public Declaration
{
    //Identifier *type;
    ASTType *type;
    Expression *value; // initial value
    VariableDeclaration(ASTType *ty, Identifier *nm, Expression *val) : Declaration(nm), type(ty), value(val) {}
};

struct TypeDeclaration : public Declaration
{
    ASTType *type;
    TypeDeclaration(Identifier *id, ASTType *ty) : Declaration(id), type(ty) {}
};

struct StructDeclaration : public TypeDeclaration
{
    std::vector<Declaration*> members;
    StructDeclaration(Identifier *id, ASTType *ty, std::vector<Declaration*> m) : TypeDeclaration(id, ty), members(m) {}
};

/***
 *
 * EXPRESSIONS
 *
 ***/

struct UnaryExpression; 
struct BinaryExpression; 
struct PrimaryExpression; 
struct CallExpression; 
struct PostfixExpression; 
struct IndexExpression; 
struct IdentifierExpression; 
struct NumericExpression; 
struct StringExpression; 
struct DeclarationExpression; 
struct BlockExpression; 
struct IfExpression; 
struct WhileExpression; 
struct ForExpression; 
struct PassExpression; 
struct SwitchExpression; 
struct ImportExpression; 

struct Expression
{
    /*
    enum ExpressionType
    {
        EXP_UNARY,
        EXP_BINARY,
        EXP_PRIMARY,
        EXP_CALL,
        EXP_POSTFIX,
        EXP_INDEX,
        EXP_MEMBER,
        EXP_IDENTIFIER,
        EXP_NUMERIC,
        EXP_DECLARATION,
        EXP_BLOCK,
        EXP_IF,
        EXP_WHILE,
        EXP_FOR,
        EXP_PASS,
        EXP_SWITCH,
        EXP_IMPORT,
    };
    */
    Expression() {}
    virtual UnaryExpression *unaryExpression() { return NULL; }
    virtual BinaryExpression *binaryExpression() { return NULL; }
    virtual PrimaryExpression *primaryExpression() { return NULL; }
    virtual CallExpression *callExpression() { return NULL; }
    virtual PostfixExpression *postfixExpression() { return NULL; }
    virtual IndexExpression *indexExpression() { return NULL; }
    virtual IdentifierExpression *identifierExpression() { return NULL; }
    virtual NumericExpression *numericExpression() { return NULL; }
    virtual StringExpression *stringExpression() { return NULL; }
    virtual DeclarationExpression *declarationExpression() { return NULL; }
    virtual BlockExpression *blockExpression() { return NULL; }
    virtual IfExpression *ifExpression() { return NULL; }
    virtual WhileExpression *whileExpression() { return NULL; }
    virtual ForExpression *forExpression() { return NULL; }
    virtual PassExpression *passExpression() { return NULL; }
    virtual SwitchExpression *switchExpression() { return NULL; }
    virtual ImportExpression *importExpression() { return NULL; }

    //TODO: overrides
};

struct PostfixExpression : public Expression
{
    virtual PostfixExpression *postfixExpression() { return this; }
};

struct CallExpression : public PostfixExpression
{
    virtual CallExpression *callExpression() { return this; }
    Expression *function;
    std::vector<Expression *> args;
    CallExpression(Expression *f, std::vector<Expression*> a) : function(f), args(a) {}
};

struct IndexExpression : public PostfixExpression
{
    virtual IndexExpression *indexExpression() { return this; }
    Expression *lhs;
    Expression *index;
    IndexExpression(Expression *l, Expression *i) : lhs(l), index(i) {}
};

struct UnaryExpression : public Expression
{
    virtual UnaryExpression *unaryExpression() { return this; }
    Expression *lhs;
    unsigned op;
    UnaryExpression(unsigned o, Expression *l) : lhs(l), op(o) {}
};

struct BinaryExpression : public Expression
{
    virtual BinaryExpression *binaryExpression() { return this; }
    Expression *lhs;
    Expression *rhs;
    unsigned op;
    BinaryExpression(unsigned o, Expression *l, Expression *r) : lhs(l), rhs(r), op(o) {}
};

struct PrimaryExpression : public Expression
{
    virtual PrimaryExpression *primaryExpression() { return this; }
};

struct IdentifierExpression : public PrimaryExpression
{
    Identifier *id;
    IdentifierExpression(Identifier *i) : id(i) {}
    Identifier *identifier() { return id; }
    std::string getName() { return id->getName(); }
    virtual IdentifierExpression *identifierExpression() { return this; }
};

//struct TypeExpression : public Expression

struct StringExpression : public PrimaryExpression
{
    std::string string;
    StringExpression(std::string str) : string(str) {}
    virtual StringExpression *stringExpression() { return this; }
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
    virtual NumericExpression *numericExpression() { return this; }
};

struct DeclarationExpression : public Expression
{
    Declaration *decl; 
    virtual DeclarationExpression *declarationExpression() { return this; }
};

struct Statement;
// value of (bool) 0. if 'pass' is encountered, value of pass
//TODO: probably shouldn't be an expression
struct BlockExpression : public Expression
{
    //TODO: sym table
    std::vector<Statement*> statements;
    BlockExpression(std::vector<Statement*> s) : statements(s) {}
    virtual BlockExpression *blockExpression() { return this; }
};

// value of following expression, or (bool) 1 if not found

// value of following expression. usually block, with (bool) 0 / (bool) 1 pass
struct IfExpression : public Expression
{
    Expression *condition;
    Statement *body;
    Statement *elsebranch;
    virtual IfExpression *ifExpression() { return this; }
    IfExpression(Expression *c, Statement *b, Statement *e) : condition(c), body(b), elsebranch(e) {}
};

// value same as if
struct WhileExpression : public Expression
{
    Expression *condition;
    Statement *body;
    Statement *elsebranch;
    virtual WhileExpression *whileExpression() { return this; }
    WhileExpression(Expression *c, Statement *b, Statement *e) : condition(c), body(b), elsebranch(e) {}
};

//TODO: for
struct ForExpression : public Expression
{
    Expression *decl;
    Expression *cond;
    Expression *update;
    Expression *body;
    virtual ForExpression *forExpression() { return this; }
};

struct SwitchExpression : public Expression
{
    //TODO
    virtual SwitchExpression *switchExpression() { return this; }
};

struct ImportExpression : public Expression
{
    TranslationUnit *import;
    virtual ImportExpression *importExpression() { return this; }
};


/***
 *
 * STATEMENTS
 *
 ***/

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
