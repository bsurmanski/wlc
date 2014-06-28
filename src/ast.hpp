#ifndef _AST_HPP
#define _AST_HPP

#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "astScope.hpp"
#include "identifier.hpp"
#include "sourceLocation.hpp"
#include "token.hpp"
#include "astType.hpp"

struct ASTVisitor;

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

//TODO: dowhile

struct Package;
struct TranslationUnit;

struct AST
{
    Package *root;
    std::map<std::string, TranslationUnit*> units;
    TranslationUnit *runtime;

    AST();
    ~AST();
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

    void accept(ASTVisitor *v);
    bool validate();
};

struct ASTNode {
    ASTNode() {}
    virtual std::string getMangledName() { return ""; }
    virtual std::string getName() { return ""; }
    virtual void accept(ASTVisitor *v) = 0;

    virtual Declaration *declaration() { return NULL; }
    virtual FunctionDeclaration *functionDeclaration() { return NULL; }
    virtual VariableDeclaration *variableDeclaration() { return NULL; }
    virtual TypeDeclaration *typeDeclaration() { return NULL; }
    virtual UserTypeDeclaration *userTypeDeclaration() { return NULL; }
    virtual ClassDeclaration *classDeclaration() { return NULL; }
};

struct Package : public ASTNode
{
    ASTScope *scope;
    Package *parent;
    Identifier *identifier;
    void *cgValue; // opaque pointer to specific codegen information
    std::string name;

    Package(Package *par=0, std::string nm="wl");

    virtual ~Package() { if(scope) delete scope; }

    Identifier *getIdentifier() { return identifier; }
    ASTScope *getScope() {
        if(!scope){
            scope = new ASTScope(NULL, ASTScope::Scope_Global, this);
            scope->setOwner(identifier);
        }
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
        //Identifier *id = scope->get(p->getName());
    }

    virtual std::string getMangledName()
    {
        if(parent)
            return parent->getMangledName() + identifier->getName();
        return identifier->getName();
    }

    virtual std::string getName() {
        return identifier->getName();
    }

    Identifier *lookup(std::string str) { return getScope()->lookup(str); }

    virtual bool isTranslationUnit() { return false; }

    virtual void accept(ASTVisitor *v);
};

// also Module
//TODO: should TU and Package be seperate? or maybe merged?
struct TranslationUnit : public Package
{
    std::vector<ImportExpression*> imports; //TODO: aliased imports?
    std::vector<TranslationUnit*> importUnits; //TODO: aliased imports?
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
    virtual void accept(ASTVisitor *v);
};


/***
 *
 * DECLARATIONS
 *
 ***/

struct Declaration : public ASTNode
{
    Identifier *identifier;
    SourceLocation loc;
    bool external;
    Declaration(Identifier *id, SourceLocation l, bool ext = false) :
        identifier(id), loc(l), external(ext) {}
    virtual ~Declaration(){}
    virtual std::string getName()
    {
        if(identifier) return identifier->getName();
        return "";
    }
    virtual std::string getMangledName(){
        if(identifier){
            return identifier->getMangledName();
        }
        return "";
    }
    bool isExternal() { return external; }
    virtual ASTType *getType() = 0;

    Identifier *getIdentifier() { return identifier; }

    virtual Declaration *declaration() { return this; }

    virtual void accept(ASTVisitor *v);
};

struct PackageDeclaration : public Declaration {
    Package *package;
    PackageDeclaration(Package *p, Identifier *id, SourceLocation l, bool ext=false) :
        package(p), Declaration(id, l, ext) {}
    ASTType *getType() { return NULL; }
};

struct FunctionDeclaration : public Declaration
{
    llvm::DISubprogram diSubprogram;
    ASTType *prototype;
    std::vector<VariableDeclaration*> parameters;
    ASTScope *scope;
    Statement *body;
    void *cgValue;
    FunctionDeclaration(Identifier *id, ASTType *p, std::vector<VariableDeclaration*> params,
            ASTScope *sc, Statement *st, SourceLocation loc) :
        Declaration(id, loc), prototype(p), parameters(params), scope(sc),
        body(st), cgValue(NULL) {
            //if(scope)
            //    scope->setOwner(this);
        }
    virtual FunctionDeclaration *functionDeclaration() { return this; }
    ASTScope *getScope() { return scope; }
    ASTType *getReturnType() { return prototype->functionType()->ret; }

    virtual ASTType *getType() { return prototype;  } //TODO: 'prototype' should be 'type'?
    virtual void accept(ASTVisitor *v);
};

struct LabelDeclaration : public Declaration
{
    LabelDeclaration(Identifier *id, SourceLocation loc) : Declaration(id, loc) {}
    virtual ASTType *getType() { return 0; }
    virtual void accept(ASTVisitor *v);
};

struct VariableDeclaration : public Declaration
{
    //Identifier *type;
    ASTType *type;
    Expression *value; // initial value
    VariableDeclaration(ASTType *ty, Identifier *nm, Expression *val, SourceLocation loc, bool ext = false) : Declaration(nm, loc, ext), type(ty), value(val){}
    virtual VariableDeclaration *variableDeclaration() { return this; }
    virtual ASTType *getType() { return type; }
    virtual void accept(ASTVisitor *v);
};

struct TypeDeclaration : public Declaration
{
    TypeDeclaration(Identifier *id, SourceLocation loc) : Declaration(id, loc) {}

    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() { return NULL; } // XXX return a 'type' type?
    virtual ASTType *getDeclaredType() = 0;
    //virtual std::string getName() { return "" };
    virtual size_t getSize() const { assert(false && "unknown size"); }
    virtual size_t getAlign() const { assert(false && "unknown align"); }
    virtual TypeDeclaration *typeDeclaration() { return this; }
};

struct CompositeTypeDeclaration : public TypeDeclaration
{
    virtual ASTType *getContainedType(unsigned index) = 0;
};

struct UserTypeDeclaration : public TypeDeclaration
{
    ASTType *type;
    ASTScope *scope;
    std::vector<FunctionDeclaration*> methods;
    std::vector<Declaration*> members;

    UserTypeDeclaration(Identifier *id, ASTScope *sc,
                std::vector<Declaration*> m, std::vector<FunctionDeclaration*> met, SourceLocation loc) :
            TypeDeclaration(id, loc), scope(sc), members(m), methods(met),
            type(new ASTUserType(id, this))
    {
        if(scope) scope->setOwner(id);
        identifier->setDeclaredType(type);
        identifier->setDeclaration(this, Identifier::ID_USER);
    }

    UserTypeDeclaration(Identifier *id, ASTType *ty, SourceLocation loc) :
        TypeDeclaration(id, loc), type(ty) {}

    ASTScope *getScope() { return scope; }
    virtual Identifier *lookup(std::string member)
    {
        return getScope()->lookupInScope(member);
    }
    virtual ASTType *getDeclaredType() { return type; }
    virtual size_t length() const { return members.size(); }
    virtual size_t getAlign() const;
    virtual size_t getSize() const = 0;
    virtual long getMemberIndex(std::string member) = 0;
    virtual long getVTableIndex(std::string method) { return -1; }
    virtual void accept(ASTVisitor *v);
    virtual UserTypeDeclaration *userTypeDeclaration() { return this; }
};

struct ClassDeclaration : public UserTypeDeclaration {
    Identifier *base;
    ASTValue *typeinfo; //XXX this might be a bad place for this
    ClassDeclaration(Identifier *id, ASTScope *sc, Identifier *bs,
            std::vector<Declaration*> m, std::vector<FunctionDeclaration*> met, SourceLocation loc) :
        UserTypeDeclaration(id, sc, m, met, loc), base(bs), typeinfo(0) {
    }
    virtual Identifier *lookup(std::string member){
        Identifier *id = getScope()->lookupInScope(member);
        if(base){
            UserTypeDeclaration *bdecl = base->getDeclaration()->userTypeDeclaration();
            if(!id && bdecl) id = bdecl->lookup(member);
        }
        return id;
    }
    virtual size_t getSize() const;
    long getMemberIndex(std::string member);
    virtual ClassDeclaration *classDeclaration() { return this; }
};

struct StructDeclaration : public UserTypeDeclaration {
    bool packed;
    StructDeclaration(Identifier *id, ASTScope *sc, std::vector<Declaration*> m,
            std::vector<FunctionDeclaration*> met, SourceLocation loc) :
        UserTypeDeclaration(id, sc, m, met, loc), packed(false) {
        }
    virtual size_t getSize() const;
    long getMemberIndex(std::string member);
};

struct UnionDeclaration : public UserTypeDeclaration {
    UnionDeclaration(Identifier *id, ASTScope *sc, std::vector<Declaration*> m,
            std::vector<FunctionDeclaration*> met, SourceLocation loc) :
        UserTypeDeclaration(id, sc, m, met, loc) {}
    virtual size_t getSize() const;
    long getMemberIndex(std::string member) { return 0; }
};

/***
 *
 * STATEMENTS
 *
 ***/
struct CompoundStatement;
struct BlockStatement;
struct ElseStatement;
struct IfStatement;
struct LoopStatement;
struct WhileStatement;
struct ForStatement;
struct SwitchStatement;

struct Statement : public ASTNode
{
    Statement(SourceLocation l) : loc(l) {}
    virtual ~Statement(){}
    SourceLocation loc;
    void setLocation(SourceLocation l) { loc = l; }
    SourceLocation getLocation() { return loc; }
    virtual void accept(ASTVisitor *v);

    virtual CompoundStatement *compoundStatement() { return NULL; }
    virtual BlockStatement *blockStatement() { return NULL; }
    virtual ElseStatement *elseStatement() { return NULL; }
    virtual IfStatement *ifStatement() { return NULL; }
    virtual LoopStatement *loopStatement() { return NULL; }
    virtual WhileStatement *whileStatement() { return NULL; }
    virtual ForStatement *forStatement() { return NULL; }
    virtual SwitchStatement *switchStatement() { return NULL; }
};

struct BreakStatement : public Statement
{
    BreakStatement(SourceLocation l) : Statement(l) {}
    virtual void accept(ASTVisitor *v);
};

struct ContinueStatement : public Statement
{
    ContinueStatement(SourceLocation l) : Statement(l) {}
    virtual void accept(ASTVisitor *v);
};


struct LabelStatement : public Statement
{
    Identifier *identifier;
    LabelStatement(Identifier *id, SourceLocation l) : Statement(l), identifier(id) {}
    virtual void accept(ASTVisitor *v);
};

struct CaseStatement : public Statement
{
    std::vector<Expression *> values;
    CaseStatement(std::vector<Expression*> vals, SourceLocation l = SourceLocation()) :
        Statement(l), values(vals) {}
    virtual void accept(ASTVisitor *v);
};

struct GotoStatement : Statement
{
    Identifier *identifier;
    GotoStatement(Identifier *id, SourceLocation l) : Statement(l), identifier(id) {}
    virtual void accept(ASTVisitor *v);
};

struct DeclarationStatement : public Statement
{
    Declaration *declaration;
    DeclarationStatement(Declaration *decl, SourceLocation l) : Statement(l), declaration(decl) {}
    virtual void accept(ASTVisitor *v);
};

// also works like pass. will assign to $ ?
struct ReturnStatement : public Statement
{
    Expression *expression;
    ReturnStatement(Expression *exp, SourceLocation l) : Statement(l), expression(exp) {}
    virtual void accept(ASTVisitor *v);
};

// value of (bool) 0. if 'pass' is encountered, value of pass
//TODO: probably shouldn't be an expression
struct CompoundStatement : public Statement
{
    //TODO: sym table
    ASTScope *scope;
    ASTScope *getScope() { return scope; }
    std::vector<Statement*> statements;
    CompoundStatement(ASTScope *sc, std::vector<Statement*> s, SourceLocation l = SourceLocation()) :
        scope(sc), Statement(l), statements(s) {
            //if(scope)
            //    scope->setOwner(this);
        }
    virtual CompoundStatement *compoundStatement() { return this; }
    virtual void accept(ASTVisitor *v);
};

// value of following expression, or (bool) 1 if not found

struct BlockStatement : public Statement
{
    ASTScope *scope;
    ASTScope *getScope() { return scope; }
    Statement *body;
    virtual BlockStatement *blockStatement() { return this; }
    BlockStatement(ASTScope *sc, Statement *b, SourceLocation l = SourceLocation()) :
        scope(sc), body(b), Statement(l) {
            //if(scope)
            //    scope->setOwner(this);
        }
    virtual void accept(ASTVisitor *v);
};

struct ElseStatement : public BlockStatement
{
    ElseStatement(ASTScope *sc, Statement *b, SourceLocation l = SourceLocation()) :
        BlockStatement(sc, b, l) {}
    virtual ElseStatement *elseStatement() { return this; }
    virtual void accept(ASTVisitor *v);
};

// value of following expression. usually block, with (bool) 0 / (bool) 1 pass
struct IfStatement : public BlockStatement
{
    Expression *condition;
    ElseStatement *elsebr;
    virtual IfStatement *ifStatement() { return this; }
    IfStatement(ASTScope *sc, Expression *c, Statement *b, ElseStatement *e,
            SourceLocation l = SourceLocation()) :
        BlockStatement(sc, b, l), condition(c), elsebr(e) {}
    virtual void accept(ASTVisitor *v);
};

struct LoopStatement : public BlockStatement
{
    Expression *condition;
    Statement *update;
    ElseStatement *elsebr;
    LoopStatement *loopStatement() { return this; }
    LoopStatement(ASTScope *sc, Expression *c, Statement *u, Statement *b, ElseStatement *el,
            SourceLocation l = SourceLocation()) : BlockStatement(sc, b, l), condition(c),
                                                update(u), elsebr(el) {}
    virtual void accept(ASTVisitor *v);
};

// value same as if
struct WhileStatement : public LoopStatement
{
    virtual WhileStatement *whileStatement() { return this; }
    WhileStatement(ASTScope *sc, Expression *c, Statement *b, ElseStatement *e,
            SourceLocation l = SourceLocation()) :
        LoopStatement(sc, c, NULL, b, e, l) {}
    virtual void accept(ASTVisitor *v);
};

struct ForStatement : public LoopStatement
{
    Statement *decl;
    virtual ForStatement *forStatement() { return this; }
    ForStatement(ASTScope *sc, Statement *d, Expression *c, Statement *u,
            Statement *b, ElseStatement *e,
            SourceLocation l = SourceLocation()) : LoopStatement(sc, c, u, b, e, l),
        decl(d) {}
    virtual void accept(ASTVisitor *v);
};

struct SwitchStatement : public BlockStatement
{
    Expression *condition;
    virtual SwitchStatement *switchStatement() { return this; }
    SwitchStatement(ASTScope *sc, Expression *cond, Statement *b,
            SourceLocation l = SourceLocation())
        : BlockStatement(sc, b, l), condition(cond) {}
    virtual void accept(ASTVisitor *v);
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
struct PassExpression;
struct TopLevelExpression;
struct ImportExpression;
struct PackageExpression;
struct CastExpression;
struct TypeExpression;
struct UseExpression;
struct TupleExpression;
struct DotExpression;
struct NewExpression;
struct DeleteExpression;

struct Expression : public Statement
{

    virtual bool isLValue() { return false; }
    virtual bool isConstant() { return false; }
    virtual ASTType *getType() { return NULL; }
    virtual ASTType *getDeclaredType() { return NULL; }

    Expression(SourceLocation l = SourceLocation()) : Statement(l) {}
    virtual UnaryExpression *unaryExpression() { return NULL; }
    virtual BinaryExpression *binaryExpression() { return NULL; }
    virtual PrimaryExpression *primaryExpression() { return NULL; }
    virtual CallExpression *callExpression() { return NULL; }
    virtual PostfixExpression *postfixExpression() { return NULL; }
    virtual IndexExpression *indexExpression() { return NULL; }
    virtual IdentifierExpression *identifierExpression() { return NULL; }
    virtual NumericExpression *numericExpression() { return NULL; }
    virtual StringExpression *stringExpression() { return NULL; }
    virtual PassExpression *passExpression() { return NULL; }
    virtual TopLevelExpression *topLevelExpression() { return NULL; }
    virtual ImportExpression *importExpression() { return NULL; }
    virtual PackageExpression *packageExpression() { return NULL; }
    virtual CastExpression *castExpression() { return NULL; }
    virtual UseExpression *useExpression() { return NULL; }
    virtual TypeExpression *typeExpression() { return NULL; }
    virtual TupleExpression *tupleExpression() { return NULL; }
    virtual DotExpression *dotExpression() { return NULL; }
    virtual NewExpression *newExpression() { return NULL; }
    virtual DeleteExpression *deleteExpression() { return NULL; }

    //TODO: overrides

    virtual void accept(ASTVisitor *v);
};

struct TypeExpression : public Expression
{
    ASTType *type;
    TypeExpression(ASTType *t, SourceLocation l = SourceLocation()) : Expression(l), type(t) {}
    virtual TypeExpression *typeExpression() { return this; }
    virtual ASTType *getDeclaredType() { return type; }
    virtual void accept(ASTVisitor *v);
};

struct NewExpression : public Expression
{
    ASTType *type;
    virtual ASTType *getType() { return type; }
    NewExpression(ASTType *t, SourceLocation l = SourceLocation()) : Expression(l), type(t) {}
    virtual NewExpression *newExpression() { return this; }
    virtual void accept(ASTVisitor *v);
};

struct DeleteExpression : public Expression
{
    IdentifierExpression *expression;
    DeleteExpression(IdentifierExpression *e, SourceLocation l = SourceLocation()) :
        Expression(l), expression(e){}
    virtual DeleteExpression *deleteExpression() { return this; }
    virtual void accept(ASTVisitor *v);
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
    virtual void accept(ASTVisitor *v);
};

struct CastExpression : public Expression
{
    ASTType *type;
    Expression *expression;
    virtual ASTType *getType() { return type; }
    virtual CastExpression *castExpression() { return this; }
    CastExpression(ASTType *ty, Expression *exp, SourceLocation l = SourceLocation()) :
        Expression(l), type(ty), expression(exp){}
    virtual void accept(ASTVisitor *v);
};

struct PostfixExpression : public Expression
{
    PostfixExpression(SourceLocation l = SourceLocation()) : Expression(l) {}
    virtual PostfixExpression *postfixExpression() { return this; }
    virtual void accept(ASTVisitor *v);
};

struct CallExpression : public PostfixExpression
{
    virtual CallExpression *callExpression() { return this; }
    Expression *function;
    virtual ASTType *getType() { return function->getType(); }
    std::vector<Expression *> args;
    CallExpression(Expression *f, std::vector<Expression*> a, SourceLocation l = SourceLocation()) :
        PostfixExpression(l), function(f), args(a) {}
    virtual void accept(ASTVisitor *v);
};

struct IndexExpression : public PostfixExpression
{
    virtual IndexExpression *indexExpression() { return this; }
    virtual bool isLValue() { return lhs->isLValue(); }
    Expression *lhs;
    Expression *index;
    IndexExpression(Expression *l, Expression *i, SourceLocation lo = SourceLocation()) :
        PostfixExpression(lo), lhs(l), index(i) {}
    virtual void accept(ASTVisitor *v);
};

struct PostfixOpExpression : public PostfixExpression
{
    int op;
    Expression *lhs;
    PostfixOpExpression(Expression *l, int o, SourceLocation lo = SourceLocation()) :
        PostfixExpression(lo), lhs(l), op(o) {}
    virtual void accept(ASTVisitor *v);
};

struct DotExpression : public PostfixExpression
{
    Expression *lhs;
    std::string rhs;
    virtual bool isLValue() { return lhs->isLValue(); }
    DotExpression(Expression *l, std::string r, SourceLocation lo = SourceLocation()) :
        PostfixExpression(lo), lhs(l), rhs(r) {}
    virtual DotExpression *dotExpression() { return this; }
    virtual void accept(ASTVisitor *v);
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
    virtual void accept(ASTVisitor *v);

    virtual ASTType *getType() {
        switch(op){
            case tok::plusplus:
            case tok::minusminus:
            case tok::plus:
            case tok::minus:
            case tok::tilde:
                     return lhs->getType();
            case tok::bang:
                     return ASTType::getBoolTy();
            case tok::caret:
                     return lhs->getType()->getReferencedTy();
            case tok::amp:
                     return lhs->getType()->getPointerTy();
        }
    }
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
    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() {
        switch(op){
            case tok::equal:
            case tok::colonequal:
            case tok::plusequal:
            case tok::minusequal:
            case tok::starequal:
            case tok::slashequal:
            case tok::ampequal:
            case tok::barequal:
            case tok::caretequal:
            case tok::percentequal:
                return lhs->getType();
            case tok::barbar:
            case tok::kw_or:
            case tok::ampamp:
            case tok::kw_and:
            case tok::bangequal:
            case tok::equalequal:
            case tok::less:
            case tok::lessequal:
            case tok::greater:
            case tok::greaterequal:
                return ASTType::getBoolTy();
            case tok::plus:
            case tok::minus:
            case tok::lessless:
            case tok::greatergreater:
            case tok::star:
            case tok::slash:
            case tok::percent:
            case tok::bar:
            case tok::amp:
                //TODO: resolve type
                return lhs->getType();
        }
    }
};

struct PrimaryExpression : public Expression
{
    PrimaryExpression(SourceLocation l = SourceLocation()) : Expression(l){}
    virtual PrimaryExpression *primaryExpression() { return this; }
    virtual void accept(ASTVisitor *v);
};

struct IdentifierExpression : public PrimaryExpression
{
    Identifier *id;
    bool isUserType() { return id->isUserType(); }
    bool isVariable() { return id->isVariable(); }
    bool isFunction() { return id->isFunction(); }
    virtual bool isLValue() { return id->isVariable(); }
    IdentifierExpression(Identifier *i, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), id(i) {}
    Identifier *identifier() { return id; }
    std::string getName() { return id->getName(); }
    virtual IdentifierExpression *identifierExpression() { return this; }
    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() { return id->getType(); }
    virtual ASTType *getDeclaredType() {
        if(id->isUserType()) return id->getDeclaredType();
        return NULL;
    }
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
    virtual void accept(ASTVisitor *v);
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
    virtual void accept(ASTVisitor *v);
};

struct Statement;



struct PackageExpression : public Expression
{
    Expression *package;
    virtual PackageExpression *packageExpression() { return this; }
    PackageExpression(Expression *p, SourceLocation l = SourceLocation())
        : Expression(l), package(p) {}
    virtual void accept(ASTVisitor *v);
};


struct TopLevelExpression : public Expression
{
    TopLevelExpression(SourceLocation l = SourceLocation()) : Expression(l) {}
    virtual TopLevelExpression *topLevelExpression() { return this; }
};

struct UseExpression : public TopLevelExpression
{
    virtual UseExpression *useExpression() { return this; }
    virtual void accept(ASTVisitor *v);
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
    virtual void accept(ASTVisitor *v);
};

struct IncludeExpression : public TopLevelExpression
{

};

#endif
