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

#ifdef WIN32
#include<filesystem>
#define PATH_MAX 255
char *realpath(const char *path, char *resolve);
std::string getFilebase(std::string s);
#else
#include <unistd.h>
std::string getFilebase(std::string s);
#endif

#include "astScope.hpp"
#include "identifier.hpp"
#include "sourceLocation.hpp"
#include "token.hpp"
#include "astType.hpp"
#include "message.hpp"

class ASTVisitor;

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

    // lower complex operations to simpler, more atomic operations.
    // eg 'a += b' operation should lower to 'a = a + b'
    // by default do not lower anything
    virtual ASTNode *lower() { return this; }
};

struct Package : public ASTNode
{
    ASTScope *scope;
    Package *parent;
    Identifier *identifier;
    void *cgValue; // XXX opaque pointer to specific codegen information
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


    TranslationUnit(Package *parent, std::string fn = "") :
        Package(parent, getFilebase(fn)), filenm(fn), expl(false) {}
    ~TranslationUnit() { }
    virtual bool isTranslationUnit() { return true; }
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

    virtual Statement *lower() { return this; }
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
 * DECLARATIONS
 *
 ***/

struct DeclarationQualifier
{
    bool external;
    bool decorated;
    bool implicit;
    bool weak;
    bool isConst;

    DeclarationQualifier() {
        external = false;
        decorated = true;
        implicit = false;
        weak = false;
        isConst = false;
    }
};

struct Declaration : public Statement
{
    Identifier *identifier;
    DeclarationQualifier qualifier;
    Declaration(Identifier *id, SourceLocation l, DeclarationQualifier dqual) :
        identifier(id), Statement(l), qualifier(dqual) {}
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
    bool isExternal() { return qualifier.external; }
    bool isWeak() { return qualifier.weak; }
    bool isConstant() { return qualifier.isConst; }
    virtual ASTType *getType() = 0;

    Identifier *getIdentifier() { return identifier; }

    virtual Declaration *declaration() { return this; }

    virtual void accept(ASTVisitor *v);

    virtual bool isConstructor() { return false; }
    virtual bool isDestructor() { return false; }

    virtual Declaration *lower() { return this; }
};

struct PackageDeclaration : public Declaration {
    Package *package;
    PackageDeclaration(Package *p, Identifier *id, SourceLocation l, DeclarationQualifier dqual) :
        package(p), Declaration(id, l, dqual) {}
    ASTType *getType() { return NULL; }
};

struct FunctionDeclaration : public Declaration
{
    llvm::DISubprogram diSubprogram;
    ASTType *owner; // aka 'this'
    ASTFunctionType *prototype; // generated on call to 'getType'. should be called after resolution only
    ASTType *returnTy;
    std::vector<VariableDeclaration*> parameters;
    bool vararg;
    int vtableIndex;
    ASTScope *scope;
    Statement *body;
    void *cgValue;

    FunctionDeclaration *nextoverload; // linked list of overloaded function declarations

    FunctionDeclaration(Identifier *id, ASTType *own, ASTType *ret, std::vector<VariableDeclaration*> params,
            bool varg,  ASTScope *sc, Statement *st, SourceLocation loc, DeclarationQualifier dqual) :
        Declaration(id, loc, dqual), owner(own), prototype(0), returnTy(ret), vararg(varg),
        parameters(params), scope(sc),
        body(st), cgValue(NULL), nextoverload(0), vtableIndex(-1) {
            //if(scope)
            //    scope->setOwner(this);
        }
    virtual FunctionDeclaration *functionDeclaration() { return this; }
    ASTScope *getScope() { return scope; }
    ASTType *getReturnType() { return returnTy; }
    bool isVararg() { return vararg; }


    int getVTableIndex() { return vtableIndex; }
    void setVTableIndex(int vti) { vtableIndex = vti; }

    virtual ASTFunctionType *getType();
    int minExpectedParameters(); // *minimum* parameters allowed; accounts for default values
    int maxExpectedParameters(); // maximum params expected

    virtual std::string getMangledName(){
        if(identifier && qualifier.decorated){
            return identifier->getMangledName() + "$$" + getType()->getMangledName();
        } else if(identifier){
            return identifier->getName();
        }
        return "";
    }

    virtual void accept(ASTVisitor *v);
    bool isOverloaded() { return nextoverload; }
    FunctionDeclaration *getNextOverload() { return nextoverload; }

    // perhaps a bit of a silly way to do it
    virtual bool isConstructor() { return identifier->getName() == "this"; }
    virtual bool isDestructor() { return identifier->getName() == "~this"; }
};

struct LabelDeclaration : public Declaration
{
    // declaration qualifiers on labels are meaningless
    LabelDeclaration(Identifier *id, SourceLocation loc) : Declaration(id, loc, DeclarationQualifier()) {}
    virtual ASTType *getType() { return 0; }
    virtual void accept(ASTVisitor *v);
};

struct VariableDeclaration : public Declaration
{
    //Identifier *type;
    ASTType *type;
    Expression *value; // initial value
    VariableDeclaration(ASTType *ty, Identifier *nm, Expression *val, SourceLocation loc, DeclarationQualifier dqual)
        : Declaration(nm, loc, dqual), type(ty), value(val){}
    virtual VariableDeclaration *variableDeclaration() { return this; }
    virtual ASTType *getType() { return type; }
    virtual void accept(ASTVisitor *v);
};

struct TypeDeclaration : public Declaration
{
    TypeDeclaration(Identifier *id, SourceLocation loc, DeclarationQualifier dqual) : Declaration(id, loc, dqual) {}

    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() { return NULL; } // XXX return a 'type' type?
    virtual ASTType *getDeclaredType() = 0;
    //virtual std::string getName() { return "" };
	virtual size_t getSize() const { assert(false && "unknown size"); return 0; }
	virtual size_t getAlign() const { assert(false && "unknown align"); return 0;  }
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
    FunctionDeclaration *constructor; // linked list of overloadeded constructors; also in 'methods'
    FunctionDeclaration *destructor; // destructor (should only be one); also in 'methods'
    std::vector<FunctionDeclaration*> methods; //locally declared methods; does not include methods defined in parent
    std::vector<Declaration*> members;

    UserTypeDeclaration(Identifier *id, ASTScope *sc, SourceLocation loc, DeclarationQualifier dqual) :
            TypeDeclaration(id, loc, dqual), scope(sc), constructor(0), destructor(0),
            type(new ASTUserType(id, this))
    {
        if(scope) scope->setOwner(id);
        identifier->setDeclaredType(type);
        identifier->addDeclaration(this, Identifier::ID_USER);
    }

    /*
    UserTypeDeclaration(Identifier *id, ASTType *ty, SourceLocation loc, DeclarationQualifier dqual) :
        TypeDeclaration(id, loc, dqual), type(ty), {}
        */

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
    void addMethod(FunctionDeclaration *fdecl) {
        methods.push_back(fdecl);
    }
    void addMember(Declaration *decl) {
        members.push_back(decl);
    }
    void addConstructor(FunctionDeclaration *fdecl) {
        if(!constructor) {
            constructor = fdecl;
        } else {
            fdecl->nextoverload = constructor;
            constructor = fdecl;
        }
    }
    void setDestructor(FunctionDeclaration *fdecl) {
        // should only be one
        destructor = fdecl;
    }
    //virtual FunctionDeclaration *getDefaultConstructor(); // XXX default, or no arg?
    //virtual void setDefaultConstructor(FunctionDeclaration *f) { defaultConstructor = f; }
};

struct ClassDeclaration : public UserTypeDeclaration {
    Identifier *base;
    ASTValue *typeinfo; //XXX this might be a bad place for this
    std::vector<FunctionDeclaration*> vtable; // populated during validation
    ClassDeclaration(Identifier *id, ASTScope *sc, Identifier *bs,
            SourceLocation loc, DeclarationQualifier dqual) :
        UserTypeDeclaration(id, sc, loc, dqual), base(bs), typeinfo(0) {
    }
    virtual Identifier *lookup(std::string member){
        Identifier *id = getScope()->lookupInScope(member);
        if(base){
            UserTypeDeclaration *bdecl = base->getDeclaration()->userTypeDeclaration();
            if(!id && bdecl) id = bdecl->lookup(member);
        }
        return id;
    }

    void populateVTable();
    virtual size_t getSize() const;
    virtual size_t getAlign() const { return 8; } //XXX align of pointer
    long getMemberIndex(std::string member);
    virtual ClassDeclaration *classDeclaration() { return this; }
};

struct InterfaceDeclaration : public UserTypeDeclaration {
    InterfaceDeclaration(Identifier *id, ASTScope *sc, SourceLocation loc, DeclarationQualifier dqual) :
        UserTypeDeclaration(id, sc, loc, dqual) {}
    virtual size_t getSize() const { return 0; } //TODO?
    virtual long getMemberIndex(std::string member) {
        emit_message(msg::FAILURE, "member index is meaningless for interface");
        return 0;
    }
};

struct StructDeclaration : public UserTypeDeclaration {
    bool packed;
    StructDeclaration(Identifier *id, ASTScope *sc,
           SourceLocation loc, DeclarationQualifier dqual) :
        UserTypeDeclaration(id, sc, loc, dqual), packed(false) {
        }
    virtual size_t getSize() const;
    long getMemberIndex(std::string member);
};

struct UnionDeclaration : public UserTypeDeclaration {
    UnionDeclaration(Identifier *id, ASTScope *sc,
            SourceLocation loc, DeclarationQualifier dqual) :
        UserTypeDeclaration(id, sc, loc, dqual) {}
    virtual size_t getSize() const;
    long getMemberIndex(std::string member) { return 0; }
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
struct IntExpression;
struct FloatExpression;
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
    virtual IntExpression *intExpression() { return NULL; }
    virtual FloatExpression *floatExpression() { return NULL; }
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

    virtual Expression *lower() { return this; }

    //TODO: overrides

    virtual void accept(ASTVisitor *v);

    virtual int asInteger() {
        emit_message(msg::ERROR, "this value cannot be converted to integer", loc);
        return 0;
    }
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
    bool call;
    ASTType *type;
    std::vector<Expression*> args;
    virtual ASTType *getType() { return type; }
    NewExpression(ASTType *t, std::vector<Expression*> a, bool c, SourceLocation l = SourceLocation()) :
        Expression(l), type(t), args(a), call(c) {}
    virtual NewExpression *newExpression() { return this; }
    virtual void accept(ASTVisitor *v);
};

struct IdOpExpression : public Expression {
    IdentifierExpression *expression;
    enum Type {
        Delete,
        Retain,
        Release
    };

    Type type;

    IdOpExpression(IdentifierExpression *e, Type t, SourceLocation l = SourceLocation()) :
        Expression(l), expression(e), type(t) {}
    virtual void accept(ASTVisitor *v);
    bool isDelete() { return type == Delete; }
    bool isRetain() { return type == Retain; }
    bool isRelease() { return type == Release; }
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

    virtual bool isConstant() {
        for(int i = 0; i < members.size(); i++) {
            if(!members[i]->isConstant()) return false;
        }
        return true;
    }

    virtual ASTType *getType();
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

// should be pure virtual
struct PostfixExpression : public Expression
{
    PostfixExpression(SourceLocation l = SourceLocation()) : Expression(l) {}
    //virtual ~PostfixExpression();
    virtual PostfixExpression *postfixExpression() { return this; }
    virtual void accept(ASTVisitor *v);
};

struct CallExpression : public PostfixExpression
{
    virtual ~CallExpression() {}
    virtual CallExpression *callExpression() { return this; }
    Expression *function;
    virtual ASTType *getType() { return function->getType(); }
    std::vector<Expression *> args; //TODO: make special argument expression to allow for name arguments?
    CallExpression(Expression *f, std::vector<Expression*> a, SourceLocation l = SourceLocation()) :
        PostfixExpression(l), function(f), args(a) {}
    virtual void accept(ASTVisitor *v);
};

struct IndexExpression : public PostfixExpression
{
    virtual ~IndexExpression() {}
    virtual IndexExpression *indexExpression() { return this; }
    virtual bool isLValue() { return lhs->isLValue(); }
    Expression *lhs;
    Expression *index;
    IndexExpression(Expression *l, Expression *i, SourceLocation lo = SourceLocation()) :
        PostfixExpression(lo), lhs(l), index(i) {}
    virtual ASTType *getType() { return NULL; } //TODO XXX
    virtual void accept(ASTVisitor *v);
};

struct PostfixOpExpression : public PostfixExpression
{
    virtual ~PostfixOpExpression() {}
    int op;
    Expression *lhs;
    PostfixOpExpression(Expression *l, int o, SourceLocation lo = SourceLocation()) :
        PostfixExpression(lo), lhs(l), op(o) {}
    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() { return NULL; } //TODO XXX
};

struct DotExpression : public PostfixExpression
{
    virtual ~DotExpression() {}
    Expression *lhs;
    std::string rhs;
    virtual bool isLValue() { return lhs->isLValue(); }
    DotExpression(Expression *l, std::string r, SourceLocation lo = SourceLocation()) :
        PostfixExpression(lo), lhs(l), rhs(r) {}
    virtual DotExpression *dotExpression() { return this; }
    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() { return NULL; } //TODO XXX
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
                     return lhs->getType()->getPointerElementTy();
            case tok::amp:
                     return lhs->getType()->getPointerTy();
        }
	return NULL;
    }
};

struct BinaryExpression : public Expression
{
    virtual BinaryExpression *binaryExpression() { return this; }
    Expression *lhs;
    Expression *rhs;
    Operator op;
    virtual bool isConstant() { return lhs->isConstant() && rhs->isConstant(); }
    BinaryExpression(tok::TokenKind o, Expression *l, Expression *r, SourceLocation lo = SourceLocation()) :
        Expression(lo), lhs(l), rhs(r), op(o) {}
    virtual void accept(ASTVisitor *v);

    virtual ASTType *getType() {
        switch(op.kind){
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
	return NULL;
    }

    virtual Expression *lower();
};

struct PrimaryExpression : public Expression
{
    virtual ~PrimaryExpression() {}
    PrimaryExpression(SourceLocation l = SourceLocation()) : Expression(l){}
    virtual PrimaryExpression *primaryExpression() { return this; }
    virtual void accept(ASTVisitor *v);
};

struct IdentifierExpression : public PrimaryExpression
{
    virtual ~IdentifierExpression() {}

    bool local; // whether this identifer is prefixed with a unary dot
    Identifier *id;
    bool isUserType() { return id->isUserType(); }
    bool isVariable() { return id->isVariable(); }
    bool isFunction() { return id->isFunction(); }
    virtual bool isLValue() { return id->isVariable(); }
    IdentifierExpression(Identifier *i, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), id(i), local(false) {}
    Identifier *identifier() { return id; }
    std::string getName() { return id->getName(); }
    virtual IdentifierExpression *identifierExpression() { return this; }
    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() { return id->getType(); }
    virtual ASTType *getDeclaredType() {
        if(id->isUserType()) return id->getDeclaredType();
        return NULL;
    }
    void setLocal(bool b) { local = b; }
    bool isLocal() { return local; }
    virtual bool isConstant() {
        return id->getDeclaration() &&
            id->getDeclaration()->isConstant();
    }

    virtual int asInteger() {
        if(isConstant()) {
            VariableDeclaration *vdecl = id->getDeclaration()->variableDeclaration();
            if(vdecl && vdecl->value && vdecl->value->intExpression()) {
                return vdecl->value->asInteger();
            } else {
                std::string err = "cannot convert constant of type " + vdecl->value->getType()->getName() + " to static int";
                emit_message(msg::ERROR, err, loc);
            }
        } else {
            emit_message(msg::ERROR, "attempt to convert non-const identifier to int", loc);
        }
        return 0;
    }
};

//struct TypeExpression : public Expression

struct StringExpression : public PrimaryExpression
{
    virtual ~StringExpression() {}
    std::string string;

    virtual ASTType *getType();

    virtual bool isConstant() { return true; }
    StringExpression(std::string str, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), string(str) {}
    virtual StringExpression *stringExpression() { return this; }
    virtual void accept(ASTVisitor *v);
};

struct NumericExpression : public PrimaryExpression
{
    virtual ~NumericExpression() {}

    ASTType *astType;

    virtual ASTType *getType() { return astType; }
    virtual bool isConstant() { return true; }
    NumericExpression(ASTType* ty, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), astType(ty) {}

    virtual NumericExpression *numericExpression() { return this; }
    virtual void accept(ASTVisitor *v);
}; //XXX subclass as bool/char type also?

struct FloatExpression : public NumericExpression {
    double value;
    FloatExpression(ASTType *ty, double val, SourceLocation l = SourceLocation()) : NumericExpression(ty, l), value(val) {}
    virtual FloatExpression *floatExpression() { return this; }
};

struct IntExpression : public NumericExpression {
    uint64_t value;
    IntExpression(ASTType *ty, uint64_t val, SourceLocation l = SourceLocation()) : NumericExpression(ty, l), value(val) {}
    virtual IntExpression *intExpression() { return this; }

    virtual int asInteger() {
            return value;
    }
};


struct Statement;



struct PackageExpression : public Expression
{
    Expression *package;
    virtual PackageExpression *packageExpression() { return this; }
    PackageExpression(Expression *p, SourceLocation l = SourceLocation())
        : Expression(l), package(p) {}
    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() { return NULL; } //TODO XXX
};


struct TopLevelExpression : public Expression
{
    TopLevelExpression(SourceLocation l = SourceLocation()) : Expression(l) {}
    virtual TopLevelExpression *topLevelExpression() { return this; }
    virtual ASTType *getType() { return NULL; } //TODO XXX
};

struct UseExpression : public TopLevelExpression
{
    virtual UseExpression *useExpression() { return this; }
    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() { return NULL; } //TODO XXX
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
    virtual ASTType *getType() { return NULL; } //TODO XXX
};

struct IncludeExpression : public TopLevelExpression
{
    virtual ASTType *getType() { return NULL; } //TODO XXX
};

#endif
