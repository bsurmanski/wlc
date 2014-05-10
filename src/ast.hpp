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

#include "symbolTable.hpp"
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
    virtual void accept(ASTVisitor *v) = 0;
};

struct Package : public ASTNode
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

    virtual void accept(ASTVisitor *v);
};

// also Module
//TODO: should TU and Package be seperate? or maybe merged?
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
    virtual std::string getName(bool mangle=false)
    {
        if(identifier) return identifier->getName();
        return "";
    }
    bool isExternal() { return external; }
    virtual ASTType *getType() = 0;

    virtual FunctionDeclaration *functionDeclaration() { return NULL; }
    virtual VariableDeclaration *variableDeclaration() { return NULL; }

    virtual void accept(ASTVisitor *v);
};

struct FunctionDeclaration : public Declaration
{
    llvm::DISubprogram diSubprogram;
    ASTType *prototype;
    std::vector<Identifier*> paramNames;
    std::vector<Expression*> paramValues;
    SymbolTable *scope;
    Statement *body;
    void *cgValue;
    FunctionDeclaration(Identifier *id, ASTType *p, std::vector<Identifier*> pname,
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
    VariableDeclaration(ASTType *ty, Identifier *nm, Expression *val, SourceLocation loc, bool ext = false) : Declaration(nm, loc, ext), type(ty), value(val) {}
    virtual VariableDeclaration *variableDeclaration() { return this; }
    virtual ASTType *getType() { return type; }
    virtual void accept(ASTVisitor *v);
};

// XXX seems pretty useless, bounce down to VariableDeclaraation ?
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
    virtual void accept(ASTVisitor *v);
};

//TODO: rename to 'Hetrogen'? Used in classes as well as struct, union
struct StructUnionDeclaration : public TypeDeclaration
{
    ASTType *type;
    StructUnionDeclaration(Identifier *id, ASTType *ty, SourceLocation loc) :
        TypeDeclaration(id, loc), type(ty) {}

    virtual ASTType *getDeclaredType() { return type; }
    virtual ASTType *setDeclaredType(ASTType *ty) {
        identifier->setDeclaredType(ty);
        type = ty;
    }
    virtual void accept(ASTVisitor *v);
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
    SymbolTable *scope;
    SymbolTable *getScope() { return scope; }
    std::vector<Statement*> statements;
    CompoundStatement(SymbolTable *sc, std::vector<Statement*> s, SourceLocation l = SourceLocation()) :
        scope(sc), Statement(l), statements(s) {}
    virtual CompoundStatement *compoundStatement() { return this; }
    virtual void accept(ASTVisitor *v);
};

// value of following expression, or (bool) 1 if not found

struct BlockStatement : public Statement
{
    SymbolTable *scope;
    SymbolTable *getScope() { return scope; }
    Statement *body;
    virtual BlockStatement *blockStatement() { return this; }
    BlockStatement(SymbolTable *sc, Statement *b, SourceLocation l = SourceLocation()) :
        scope(sc), body(b), Statement(l) {}
    virtual void accept(ASTVisitor *v);
};

struct ElseStatement : public BlockStatement
{
    ElseStatement(SymbolTable *sc, Statement *b, SourceLocation l = SourceLocation()) :
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
    IfStatement(SymbolTable *sc, Expression *c, Statement *b, ElseStatement *e,
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
    LoopStatement(SymbolTable *sc, Expression *c, Statement *u, Statement *b, ElseStatement *el,
            SourceLocation l = SourceLocation()) : BlockStatement(sc, b, l), condition(c),
                                                update(u), elsebr(el) {}
    virtual void accept(ASTVisitor *v);
};

// value same as if
struct WhileStatement : public LoopStatement
{
    virtual WhileStatement *whileStatement() { return this; }
    WhileStatement(SymbolTable *sc, Expression *c, Statement *b, ElseStatement *e,
            SourceLocation l = SourceLocation()) :
        LoopStatement(sc, c, NULL, b, e, l) {}
    virtual void accept(ASTVisitor *v);
};

struct ForStatement : public LoopStatement
{
    Statement *decl;
    virtual ForStatement *forStatement() { return this; }
    ForStatement(SymbolTable *sc, Statement *d, Expression *c, Statement *u,
            Statement *b, ElseStatement *e,
            SourceLocation l = SourceLocation()) : LoopStatement(sc, c, u, b, e, l),
        decl(d) {}
    virtual void accept(ASTVisitor *v);
};

struct SwitchStatement : public BlockStatement
{
    Expression *condition;
    virtual SwitchStatement *switchStatement() { return this; }
    SwitchStatement(SymbolTable *sc, Expression *cond, Statement *b,
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
    virtual bool isLValue() { return id->isVariable(); }
    IdentifierExpression(Identifier *i, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), id(i) {}
    Identifier *identifier() { return id; }
    std::string getName() { return id->getName(); }
    virtual IdentifierExpression *identifierExpression() { return this; }
    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() { return id->getType(); }
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



class ASTVisitor {
    std::stack<SymbolTable*> scope;
    std::stack<FunctionDeclaration*> function;

    public:
    SymbolTable *getCurrentScope() { return scope.top(); }
    void pushScope(SymbolTable *sc) { scope.push(sc); }
    SymbolTable *popScope() { SymbolTable *sc = scope.top(); scope.pop(); return sc; }

    FunctionDeclaration *getCurrentFunction() { return function.top(); }
    void pushFunction(FunctionDeclaration *fdecl) { function.push(fdecl); }
    FunctionDeclaration *popFunction() {
        FunctionDeclaration *fd = function.top();
        function.pop();
        return fd;
    }

    virtual void visitPackage(Package *pak) {}
    virtual void visitTranslationUnit(TranslationUnit *tu) {}

    virtual void visitDeclaration(Declaration *decl){}
    virtual void visitExpression(Expression *exp){}
    virtual void visitStatement(Statement *stmt){}
    //virtual void visitType(ASTType *ty){}

    virtual void visitFunctionDeclaration(FunctionDeclaration *decl){}
    virtual void visitLabelDeclaration(LabelDeclaration *decl){}
    virtual void visitVariableDeclaration(VariableDeclaration *decl){}
    virtual void visitTypeDeclaration(TypeDeclaration *decl){}
    virtual void visitStructUnionDeclaration(StructUnionDeclaration *decl){}

    virtual void visitUnaryExpression(UnaryExpression *exp){}
    virtual void visitBinaryExpression(BinaryExpression *exp){}
    virtual void visitPostfixExpression(PostfixExpression *exp){}
    virtual void visitPostfixOpExpression(PostfixOpExpression *exp){}
    virtual void visitPrimaryExpression(PrimaryExpression *exp){}
    virtual void visitCallExpression(CallExpression *exp){}
    virtual void visitIndexExpression(IndexExpression *exp){}
    virtual void visitIdentifierExpression(IdentifierExpression *exp){}
    virtual void visitNumericExpression(NumericExpression *exp){}
    virtual void visitStringExpression(StringExpression *exp){}
    virtual void visitImportExpression(ImportExpression *exp){}
    virtual void visitPackageExpression(PackageExpression *exp){}
    virtual void visitCastExpression(CastExpression *exp){}
    virtual void visitUseExpression(UseExpression *exp){}
    virtual void visitTypeExpression(TypeExpression *exp){}
    virtual void visitTupleExpression(TupleExpression *exp){}
    virtual void visitDotExpression(DotExpression *exp){}
    virtual void visitNewExpression(NewExpression *exp){}
    virtual void visitDeleteExpression(DeleteExpression *exp){}

    virtual void visitBreakStatement(BreakStatement *stmt){}
    virtual void visitContinueStatement(ContinueStatement *stmt){}
    virtual void visitLabelStatement(LabelStatement *stmt){}
    virtual void visitCaseStatement(CaseStatement *stmt){}
    virtual void visitGotoStatement(GotoStatement *stmt){}
    virtual void visitDeclarationStatement(DeclarationStatement *stmt){}
    virtual void visitReturnStatement(ReturnStatement *stmt){}

    virtual void visitCompoundStatement(CompoundStatement *exp){}
    virtual void visitBlockStatement(BlockStatement *exp){}
    virtual void visitElseStatement(ElseStatement *exp){}
    virtual void visitIfStatement(IfStatement *exp){}
    virtual void visitLoopStatement(LoopStatement *exp){}
    virtual void visitWhileStatement(WhileStatement *exp){}
    virtual void visitForStatement(ForStatement *exp){}
    virtual void visitSwitchStatement(SwitchStatement *exp){}
};

#endif
