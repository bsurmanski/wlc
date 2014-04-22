#ifndef _AST_HPP
#define _AST_HPP

#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "symbolTable.hpp"
#include "identifier.hpp"
#include "sourceLocation.hpp"
#include "token.hpp"
#include "astType.hpp"

struct Statement;
struct Declaration;
struct Expression;
struct DeclarationExpression;
struct TranslationUnit;
struct Identifier;
struct LabelDeclaration;
struct VariableDeclaration;
struct FunctionDeclaration;
struct ImportExpression;
struct PackageExpression;
struct TypeDeclaration;

//TODO: switch, dowhile

//TODO: use
struct Package
{
    SymbolTable *scope;
    Package *parent;
    Identifier *identifier;
    void *cgValue; // opaque pointer to specific codegen information
    std::string name;

    Package(Package *par=0, std::string nm="wl") : parent(par), scope(NULL), cgValue(NULL),
        identifier(NULL)
    {
        name = nm;
        if(parent)
            identifier = parent->getScope()->getInScope(name);
    }
    virtual ~Package() { if(scope) delete scope; }

    Identifier *getIdentifier() { return identifier; }
    SymbolTable *getScope() {
        if(!scope)
            scope = new SymbolTable(NULL, SymbolTable::Scope_Global, this);
        return scope;
    } //TODO: subscope of parent ?

    std::vector<Package*> children;

    void setParent(Package *p)
    {
        assert(false); // dont use this?XXX
        //parent = p->getIdentifier();
    }

    void addPackage(Package *p) {
        // XXX add identifier in scope ?
        //p->setParent(this);
        children.push_back(p);
    }

    std::string getMangledName()
    {
        if(parent)
            return parent->getMangledName() + identifier->getName();
        return identifier->getName();
    }

    Identifier *lookup(std::string str) { return getScope()->lookup(str); }

    virtual bool isTranslationUnit() { return false; }
};

// also Module
struct TranslationUnit : public Package
{
    std::vector<ImportExpression*> imports; //TODO: aliased imports?
    std::vector<TranslationUnit*> importUnits; //TODO: aliased imports?
    std::vector<TypeDeclaration*> types;
    std::vector<VariableDeclaration*> globals;
    std::vector<FunctionDeclaration*> functions;

    std::map<std::string, bool> extensions;

    std::string filenm;
    bool expl; // explicitly requested for compile. eg, not included
    static std::string getFilebase(std::string s)
    {
        size_t lastDot = s.find_last_of(".");
        if(lastDot != std::string::npos)
            s = s.substr(0, lastDot);
        return basename(s.c_str());
    }

    TranslationUnit(Package *parent, std::string fn = "") :
        Package(parent, getFilebase(fn)), filenm(fn), expl(false) {}
    ~TranslationUnit() { }
    virtual bool isTranslationUnit() { return true; }
    std::string getName() { return identifier->getName(); }
};

struct AST
{
    Package *root;
    std::map<std::string, TranslationUnit*> units;
    TranslationUnit *runtime;

    AST() { root = new Package; }
    ~AST() { delete root; }
    Package *getRootPackage() { return root; }
    TranslationUnit *getUnit(std::string str)
    {
        char APATH[PATH_MAX + 1];
        realpath(str.c_str(), APATH);
        std::string astr = std::string(str);
        if(units.count(astr)) return units[astr];
        return NULL;
    }
    void addUnit(std::string str, TranslationUnit *u)
    {
        char APATH[PATH_MAX + 1];
        realpath(str.c_str(), APATH);
        std::string astr = std::string(str);
        assert(!units.count(astr) && "reimport of translation unit!");
        units[str] = u;
    }

    void setRuntimeUnit(TranslationUnit *u)
    {
        units["/usr/local/include/wl/runtime.wl"] = u;
        runtime = u;
    }

    TranslationUnit *getRuntimeUnit() { return runtime; }
};

struct FunctionDeclaration;

/***
 *
 * DECLARATIONS
 *
 ***/

struct Declaration
{
    Identifier *identifier;
    SourceLocation loc;
    bool external;
    Declaration(Identifier *id, SourceLocation l, bool ext = false) : identifier(id), loc(l), external(ext) {}
    virtual ~Declaration(){}
    virtual std::string getName(bool mangle=false)
    {
        if(identifier) return identifier->getName();
        return "";
    }
    bool isExternal() { return external; }
    virtual ASTType *getType() = 0;

    virtual FunctionDeclaration *functionDeclaration() { return NULL; }
    virtual VariableDeclaration *variableDeclaration() { return NULL; }
};

struct FunctionDeclaration : public Declaration
{
    llvm::DISubprogram diSubprogram;
    ASTType *prototype;
    std::vector<std::string> paramNames;
    std::vector<Expression*> paramValues;
    SymbolTable *scope;
    Statement *body;
    void *cgValue;
    FunctionDeclaration(Identifier *id, ASTType *p, std::vector<std::string> pname,
            std::vector<Expression*> pvals, SymbolTable *sc,
            Statement *st, SourceLocation loc) :
        Declaration(id, loc), prototype(p), paramNames(pname), paramValues(pvals), scope(sc),
        body(st), cgValue(NULL) {}
    virtual FunctionDeclaration *functionDeclaration() { return this; }
    virtual std::string getName(bool mangle=false)
    {
        if(mangle){}
        else {
            return Declaration::getName();
        }
    }
    SymbolTable *getScope() { return scope; }
    ASTType *getReturnType() { return dynamic_cast<FunctionTypeInfo*>(prototype->info)->ret; }

    virtual ASTType *getType() { return prototype;  } //TODO: 'prototype' should be 'type'?
};

struct LabelDeclaration : public Declaration
{
    LabelDeclaration(Identifier *id, SourceLocation loc) : Declaration(id, loc) {}
    virtual ASTType *getType() { return 0; }
};

struct VariableDeclaration : public Declaration
{
    //Identifier *type;
    ASTType *type;
    Expression *value; // initial value
    VariableDeclaration(ASTType *ty, Identifier *nm, Expression *val, SourceLocation loc, bool ext = false) : Declaration(nm, loc, ext), type(ty), value(val) {}
    virtual VariableDeclaration *variableDeclaration() { return this; }
    virtual ASTType *getType() { return type; }
};

struct ArrayDeclaration : public VariableDeclaration
{
    ArrayDeclaration(ASTType *ty, Identifier *nm, Expression *val, SourceLocation loc, bool ext = false) :
        VariableDeclaration(ty, nm, val, loc, ext) {}
    virtual ArrayDeclaration *arrayDeclaration() { return this; }
};

struct TypeDeclaration : public Declaration
{
    TypeDeclaration(Identifier *id, SourceLocation loc) : Declaration(id, loc) {}

    virtual ASTType *getType() { return NULL; } // XXX return a 'type' type?
    virtual ASTType *getDeclaredType() = 0;
    virtual ASTType *setDeclaredType(ASTType *ty) = 0;
};

struct BasicTypeDeclaration : public TypeDeclaration
{
    ASTType *type;
    BasicTypeDeclaration(Identifier *id, ASTType *ty, SourceLocation loc) :
        TypeDeclaration(id, loc), type(ty)
    {
    }

    virtual ASTType *getDeclaredType() { return type; }
    virtual ASTType *setDeclaredType(ASTType *ty) { type = ty; }
};

struct StructUnionDeclaration : public TypeDeclaration
{
    std::vector<Declaration*> members;
    StructUnionDeclaration(Identifier *id, ASTType *ty, std::vector<Declaration*> m, SourceLocation loc) : TypeDeclaration(id, loc), members(m) {}

    virtual ASTType *getDeclaredType() { return identifier->getDeclaredType(); }
    virtual ASTType *setDeclaredType(ASTType *ty) { identifier->setDeclaredType(ty); }
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
struct CompoundExpression;
struct BlockExpression;
struct ElseExpression;
struct IfExpression;
struct LoopExpression;
struct WhileExpression;
struct ForExpression;
struct PassExpression;
struct SwitchExpression;
struct TopLevelExpression;
struct ImportExpression;
struct PackageExpression;
struct CastExpression;
struct TypeExpression;
struct UseExpression;
struct TupleExpression;

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
    void setLocation(SourceLocation l) { loc = l; }
    SourceLocation loc;

    virtual bool isLValue() { return false; }
    virtual bool isConstant() { return false; }
    virtual ASTType *getType() { return NULL; }

    Expression(SourceLocation l = SourceLocation()) : loc(l) {}
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
    virtual CompoundExpression *compoundExpression() { return NULL; }
    virtual BlockExpression *blockExpression() { return NULL; }
    virtual ElseExpression *elseExpression() { return NULL; }
    virtual IfExpression *ifExpression() { return NULL; }
    virtual LoopExpression *loopExpression() { return NULL; }
    virtual WhileExpression *whileExpression() { return NULL; }
    virtual ForExpression *forExpression() { return NULL; }
    virtual PassExpression *passExpression() { return NULL; }
    virtual SwitchExpression *switchExpression() { return NULL; }
    virtual TopLevelExpression *topLevelExpression() { return NULL; }
    virtual ImportExpression *importExpression() { return NULL; }
    virtual PackageExpression *packageExpression() { return NULL; }
    virtual CastExpression *castExpression() { return NULL; }
    virtual UseExpression *useExpression() { return NULL; }
    virtual TypeExpression *typeExpression() { return NULL; }
    virtual TupleExpression *tupleExpression() { return NULL; }

    //TODO: overrides
};

struct TypeExpression : public Expression
{
    ASTType *type;
    TypeExpression(ASTType *t, SourceLocation l = SourceLocation()) : Expression(l), type(t) {}
    virtual TypeExpression *typeExpression() { return this; }
};

struct NewExpression : public Expression
{
    ASTType *type;
    virtual ASTType *getType() { return type; }
    NewExpression(ASTType *t, SourceLocation l = SourceLocation()) : Expression(l), type(t) {}
};

struct DeleteExpression : public Expression
{
    IdentifierExpression *expression;
    DeleteExpression(IdentifierExpression *e, SourceLocation l = SourceLocation()) :
        Expression(l), expression(e){}
};

struct TupleExpression : public Expression
{
    std::vector<Expression*> members;
    virtual bool isLValue() {
        bool ret = members.size() > 0;
        for(int i = 0; i < members.size() && ret; i++)
            if(!members[i]->isLValue()) ret = false;
        return ret;
    }
    TupleExpression(std::vector<Expression*> e, SourceLocation l = SourceLocation()) :
        Expression(l), members(e) {}
    virtual TupleExpression *tupleExpression() { return this; }
};

struct CastExpression : public Expression
{
    ASTType *type;
    Expression *expression;
    virtual ASTType *getType() { return type; }
    virtual CastExpression *castExpression() { return this; }
    CastExpression(ASTType *ty, Expression *exp, SourceLocation l = SourceLocation()) :
        Expression(l), type(ty), expression(exp){}
};

struct PostfixExpression : public Expression
{
    PostfixExpression(SourceLocation l = SourceLocation()) : Expression(l) {}
    virtual PostfixExpression *postfixExpression() { return this; }
};

struct CallExpression : public PostfixExpression
{
    virtual CallExpression *callExpression() { return this; }
    Expression *function;
    virtual ASTType *getType() { return function->getType(); }
    std::vector<Expression *> args;
    CallExpression(Expression *f, std::vector<Expression*> a, SourceLocation l = SourceLocation()) :
        PostfixExpression(l), function(f), args(a) {}
};

struct IndexExpression : public PostfixExpression
{
    virtual IndexExpression *indexExpression() { return this; }
    virtual bool isLValue() { return lhs->isLValue(); }
    Expression *lhs;
    Expression *index;
    IndexExpression(Expression *l, Expression *i, SourceLocation lo = SourceLocation()) :
        PostfixExpression(lo), lhs(l), index(i) {}
};

struct PostfixOpExpression : public PostfixExpression
{
    int op;
    Expression *lhs;
    PostfixOpExpression(Expression *l, int o, SourceLocation lo = SourceLocation()) :
        PostfixExpression(lo), lhs(l), op(o) {}
};

struct DotExpression : public PostfixExpression
{
    Expression *lhs;
    std::string rhs;
    virtual bool isLValue() { return lhs->isLValue(); }
    DotExpression(Expression *l, std::string r, SourceLocation lo = SourceLocation()) :
        PostfixExpression(lo), lhs(l), rhs(r) {}
};

struct UnaryExpression : public Expression
{
    virtual UnaryExpression *unaryExpression() { return this; }
    Expression *lhs;
    unsigned op;

    virtual bool isLValue() { return op == tok::caret; }
    virtual bool isConstant() { return lhs->isConstant(); }
    UnaryExpression(unsigned o, Expression *l, SourceLocation lo = SourceLocation()) :
        Expression(lo), lhs(l), op(o) {}
};

struct BinaryExpression : public Expression
{
    virtual BinaryExpression *binaryExpression() { return this; }
    Expression *lhs;
    Expression *rhs;
    unsigned op;
    virtual bool isConstant() { return lhs->isConstant() && rhs->isConstant(); }
    BinaryExpression(unsigned o, Expression *l, Expression *r, SourceLocation lo = SourceLocation()) :
        Expression(lo), lhs(l), rhs(r), op(o) {}
};

struct PrimaryExpression : public Expression
{
    PrimaryExpression(SourceLocation l = SourceLocation()) : Expression(l){}
    virtual PrimaryExpression *primaryExpression() { return this; }
};

struct IdentifierExpression : public PrimaryExpression
{
    Identifier *id;
    virtual bool isLValue() { return id->isVariable(); }
    IdentifierExpression(Identifier *i, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), id(i) {}
    Identifier *identifier() { return id; }
    std::string getName() { return id->getName(); }
    virtual IdentifierExpression *identifierExpression() { return this; }
};

//struct TypeExpression : public Expression

struct StringExpression : public PrimaryExpression
{
    std::string string;
    virtual ASTType *getType() { return ASTType::getCharTy()->getArrayTy(string.length()); }
    virtual bool isConstant() { return true; }
    StringExpression(std::string str, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), string(str) {}
    virtual StringExpression *stringExpression() { return this; }
};

// XXX super ugly. Maybe just hold an ASTValue* or something instead of this mess?
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
    ASTType *astType;

    union
    {
        double floatValue;
        uint64_t intValue;
        char charValue;
    };

    virtual ASTType *getType() { return astType; }
    virtual bool isConstant() { return true; }
    NumericExpression(NumericType t, ASTType* ty, double val, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), type(t),  astType(ty), floatValue(val) {}
    NumericExpression(NumericType t, ASTType *ty, uint64_t val, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), type(t), astType(ty), intValue(val) {}
    virtual NumericExpression *numericExpression() { return this; }
};

struct DeclarationExpression : public Expression
{
    Declaration *decl;
    virtual ASTType *getType() { return decl->getType(); }
    DeclarationExpression(SourceLocation l = SourceLocation()) : Expression(l) {}
    virtual DeclarationExpression *declarationExpression() { return this; }
};

struct Statement;


// value of (bool) 0. if 'pass' is encountered, value of pass
//TODO: probably shouldn't be an expression
struct CompoundExpression : public Expression
{
    //TODO: sym table
    std::vector<Statement*> statements;
    CompoundExpression(std::vector<Statement*> s, SourceLocation l = SourceLocation()) :
        Expression(l), statements(s) {}
    virtual CompoundExpression *compoundExpression() { return this; }
};

// value of following expression, or (bool) 1 if not found

struct BlockExpression : public Expression
{
    SymbolTable *scope;
    Statement *body;
    virtual BlockExpression *blockExpression() { return this; }
    BlockExpression(SymbolTable *sc, Statement *b, SourceLocation l = SourceLocation()) :
        scope(sc), body(b), Expression(l) {}
};

struct ElseExpression : public BlockExpression
{
    ElseExpression(SymbolTable *sc, Statement *b, SourceLocation l = SourceLocation()) :
        BlockExpression(sc, b, l) {}
    virtual ElseExpression *elseExpression() { return this; }
};

// value of following expression. usually block, with (bool) 0 / (bool) 1 pass
struct IfExpression : public BlockExpression
{
    Expression *condition;
    ElseExpression *elsebr;
    virtual IfExpression *ifExpression() { return this; }
    IfExpression(SymbolTable *sc, Expression *c, Statement *b, ElseExpression *e,
            SourceLocation l = SourceLocation()) :
        BlockExpression(sc, b, l), condition(c), elsebr(e) {}
};

struct LoopExpression : public BlockExpression
{
    Expression *condition;
    Statement *update;
    ElseExpression *elsebr;
    LoopExpression *loopExpression() { return this; }
    LoopExpression(SymbolTable *sc, Expression *c, Statement *u, Statement *b, ElseExpression *el,
            SourceLocation l = SourceLocation()) : BlockExpression(sc, b, l), condition(c),
                                                update(u), elsebr(el) {}
};

// value same as if
struct WhileExpression : public LoopExpression
{
    virtual WhileExpression *whileExpression() { return this; }
    WhileExpression(SymbolTable *sc, Expression *c, Statement *b, ElseExpression *e,
            SourceLocation l = SourceLocation()) :
        LoopExpression(sc, c, NULL, b, e, l) {}
};

struct ForExpression : public LoopExpression
{
    Statement *decl;
    virtual ForExpression *forExpression() { return this; }
    ForExpression(SymbolTable *sc, Statement *d, Expression *c, Statement *u,
            Statement *b, ElseExpression *e,
            SourceLocation l = SourceLocation()) : LoopExpression(sc, c, u, b, e, l),
        decl(d) {}
};

struct SwitchExpression : public BlockExpression
{
    Expression *condition;
    virtual SwitchExpression *switchExpression() { return this; }
    SwitchExpression(SymbolTable *sc, Expression *cond, Statement *b,
            SourceLocation l = SourceLocation())
        : BlockExpression(sc, b, l), condition(cond) {}
};

struct PackageExpression : public Expression
{
    Expression *package;
    virtual PackageExpression *packageExpression() { return this; }
    PackageExpression(Expression *p, SourceLocation l = SourceLocation())
        : Expression(l), package(p) {}
};


struct TopLevelExpression : public Expression
{
    TopLevelExpression(SourceLocation l = SourceLocation()) : Expression(l) {}
    virtual TopLevelExpression *topLevelExpression() { return this; }
};

struct UseExpression : public TopLevelExpression
{
    virtual UseExpression *useExpression() { return this; }
};

// import <STRING> |
// import <IDENTIFIER>
struct ImportExpression : public TopLevelExpression
{
    Expression *expression;
    TranslationUnit *unit;
    virtual ImportExpression *importExpression() { return this; }
    ImportExpression(Expression *im, TranslationUnit *u, SourceLocation l = SourceLocation()) :
        TopLevelExpression(l), expression(im), unit(u) { }
};

struct IncludeExpression : public TopLevelExpression
{

};


/***
 *
 * STATEMENTS
 *
 ***/

struct Statement
{
    Statement(SourceLocation l) : loc(l) {}
    virtual ~Statement(){}
    SourceLocation loc;
    SourceLocation getLocation() { return loc; }
};

struct BreakStatement : Statement
{
    BreakStatement(SourceLocation l) : Statement(l) {}
};

struct ContinueStatement : Statement
{
    ContinueStatement(SourceLocation l) : Statement(l) {}
};


struct LabelStatement : Statement
{
    Identifier *identifier;
    LabelStatement(Identifier *id, SourceLocation l) : Statement(l), identifier(id) {}
};

struct CaseStatement : public Statement
{
    std::vector<Expression *> values;
    CaseStatement(std::vector<Expression*> vals, SourceLocation l = SourceLocation()) :
        Statement(l), values(vals) {}
};

struct GotoStatement : Statement
{
    Identifier *identifier;
    GotoStatement(Identifier *id, SourceLocation l) : Statement(l), identifier(id) {}
};

struct DeclarationStatement : public Statement
{
    Declaration *declaration;
    DeclarationStatement(Declaration *decl, SourceLocation l) : Statement(l), declaration(decl) {}
};

struct ExpressionStatement : public Statement
{
    Expression *expression;
    ExpressionStatement(Expression *exp, SourceLocation l) : Statement(l), expression(exp){}
};

// also works like pass. will assign to $ ?
struct ReturnStatement : public Statement
{
    Expression *expression;
    ReturnStatement(Expression *exp, SourceLocation l) : Statement(l), expression(exp) {}
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
    OnceStatement(SourceLocation l) : Statement(l) {}
};

#endif
