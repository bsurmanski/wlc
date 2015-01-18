#ifndef _AST_HPP
#define _AST_HPP

#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
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
#include <sys/stat.h>
std::string getFilebase(std::string s);
#endif

unsigned getFileSize(std::string filenm);

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
struct Identifier;
struct LabelDeclaration;
struct VariableDeclaration;
struct FunctionDeclaration;
struct ImportExpression;
struct PackageExpression;
struct TypeDeclaration;
struct InterfaceDeclaration;

//TODO: dowhile

struct PackageDeclaration;
struct ModuleDeclaration;

struct AST
{
    PackageDeclaration *root;
    std::map<std::string, ModuleDeclaration*> modules;
    ModuleDeclaration *runtime;

    AST();
    ~AST();
    PackageDeclaration *getRootPackage() { return root; }
    ModuleDeclaration *getModule(std::string str)
    {
        char APATH[PATH_MAX + 1];
        realpath(str.c_str(), APATH);
        std::string astr = std::string(str);
        if(modules.count(astr)) return modules[astr];
        return NULL;
    }
    void addModule(std::string str, ModuleDeclaration *u)
    {
        char APATH[PATH_MAX + 1];
        realpath(str.c_str(), APATH);
        std::string astr = std::string(str);
        assert(!modules.count(astr) && "reimport of module!");
        modules[str] = u;
    }

    void setRuntimeModule(ModuleDeclaration *u)
    {
        //XXX HARD CODED
        modules["/usr/local/include/wl/runtime.wl"] = u;
        runtime = u;
    }

    ModuleDeclaration *getRuntimeModule() { return runtime; }

    void accept(ASTVisitor *v);
    bool validate();
};

struct ASTNode {
    private:
    unsigned refcount;
    ASTNode *parent;
    SourceLocation location;

    public:

    ASTNode() : refcount(1), parent(0) {}
    ASTNode(ASTNode *_parent, SourceLocation loc = SourceLocation()) :
        refcount(1), parent(_parent), location(loc) {}

    virtual ~ASTNode() {}

    unsigned retain() {
        refcount++;
        return refcount;
    }

    unsigned release() {
        refcount--;
        if(refcount <= 0) {
            delete this;
            return 0;
        }
        return refcount;
    }

    virtual std::string getMangledName() { return ""; }
    virtual std::string getName() { return ""; }
    virtual void accept(ASTVisitor *v) = 0;

    virtual Expression *expression() { return NULL; }
    virtual Declaration *declaration() { return NULL; }
    virtual FunctionDeclaration *functionDeclaration() { return NULL; }
    virtual VariableDeclaration *variableDeclaration() { return NULL; }
    virtual TypeDeclaration *typeDeclaration() { return NULL; }
    virtual UserTypeDeclaration *userTypeDeclaration() { return NULL; }
    virtual ClassDeclaration *classDeclaration() { return NULL; }
    virtual InterfaceDeclaration *interfaceDeclaration() { return NULL; }
    virtual ModuleDeclaration *moduleDeclaration() { return NULL; }

    // lower complex operations to simpler, more atomic operations.
    // eg 'a += b' operation should lower to 'a = a + b'
    // by default do not lower anything
    virtual ASTNode *lower() { return this; }
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
    SourceLocation loc;

    Statement(SourceLocation l) : loc(l) {}
    virtual ~Statement(){}
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

//TODO: remove while/for ASTNodes. just have metadata inside loopStatement
// about which type of loop it is
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
    bool isStatic;

    DeclarationQualifier() {
        external = false;
        decorated = true;
        implicit = false;
        weak = false;
        isConst = false;
        isStatic = false;
    }
};

//TODO: make declaration inherit expression
// this would allow statements like if(MyBaseClass bcl := cl)
struct Declaration : public Statement
{
    Identifier *identifier;
    DeclarationQualifier qualifier;
    Declaration(Identifier *id, SourceLocation l, DeclarationQualifier dqual) :
        identifier(id), Statement(l), qualifier(dqual) {}
    virtual ~Declaration(){}
    virtual std::string getName() {
        if(identifier) return identifier->getName();
        return "";
    }
    virtual std::string getMangledName() {
        if(identifier) {
            return identifier->getMangledName();
        }
        return "";
    }
    bool isExternal() { return qualifier.external; }
    bool isWeak() { return qualifier.weak; }
    bool isConstant() { return qualifier.isConst; }
    bool isStatic() { return qualifier.isStatic; }
    virtual ASTType *getType() = 0;

    Identifier *getIdentifier() { return identifier; }

    virtual Declaration *declaration() { return this; }

    virtual void accept(ASTVisitor *v);

    virtual bool isConstructor() { return false; }
    virtual bool isDestructor() { return false; }

    virtual Declaration *lower() { return this; }
};

struct ModuleDeclaration;
struct PackageDeclaration : public Declaration {
    ASTScope *scope;
    PackageDeclaration *parent;
    std::string name;

    PackageDeclaration(PackageDeclaration *p, Identifier *id, SourceLocation l, DeclarationQualifier dqual) :
        parent(p), Declaration(id, l, dqual) {
            scope = new ASTScope(NULL, ASTScope::Scope_Global, this);
            scope->setOwner(identifier);
            //TODO: subscope of super package?
        }
    ASTType *getType() { return NULL; }

    ASTScope *getScope() {
        return scope;
    }

    std::vector<PackageDeclaration*> children;

    void setSuperPackage(PackageDeclaration *p)
    {
        assert(false); // dont use this?XXX
        //superPackage = p->getIdentifier();
    }

    void addPackage(PackageDeclaration *p) {
        // XXX add identifier in scope ?
        //p->setSuperPackage(this);
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

    virtual ModuleDeclaration *moduleDeclaration() { return NULL; }

    virtual void accept(ASTVisitor *v);
};

struct ModuleDeclaration : public PackageDeclaration {
    ASTScope* importScope; // TODO: aliased imports?
    std::map<std::string, bool> extensions;

    std::string filenm;
    bool expl; // explicitly requested for compile. eg, not included


    ModuleDeclaration(PackageDeclaration *parent, Identifier *id, std::string fn = "") :
        PackageDeclaration(parent, id, SourceLocation(filenm.c_str(), 1), DeclarationQualifier()), filenm(fn), expl(false) {
            importScope = new ASTScope(NULL, ASTScope::Scope_Global, this);
        }
    ~ModuleDeclaration() { }
    virtual ModuleDeclaration *moduleDeclaration() { return this; }
    virtual void accept(ASTVisitor *v);
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

    FunctionDeclaration *nextoverload; // linked list of overloaded function declarations

    FunctionDeclaration(Identifier *id, ASTType *own, ASTType *ret, std::vector<VariableDeclaration*> params,
            bool varg,  ASTScope *sc, Statement *st, SourceLocation loc, DeclarationQualifier dqual) :
        Declaration(id, loc, dqual), owner(own), prototype(0), returnTy(ret), vararg(varg),
        parameters(params), scope(sc),
        body(st), nextoverload(0), vtableIndex(-1) {
            //if(scope)
            //    scope->setOwner(this);
        }
    virtual FunctionDeclaration *functionDeclaration() { return this; }
    ASTScope *getScope() { return scope; }
    ASTType *getReturnType() { return returnTy; }
    bool isVararg() { return vararg; }

    Expression *getDefaultParameter(unsigned parami);

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
    bool isOverloaded() { return nextoverload; } //XXX will not work for last overload in list; but it should only be used on first overload anyways
    bool isVirtual() { return owner && (owner->isClass() || owner->isInterface()); }
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
    std::vector<Declaration*> staticMembers; // these are seperate so that it is easier to work with.
    std::vector<Declaration*> members;

    UserTypeDeclaration(Identifier *id, ASTScope *sc, SourceLocation loc, DeclarationQualifier dqual) :
            TypeDeclaration(id, loc, dqual), scope(sc), constructor(0), destructor(0),
            type(new ASTUserType(id, this))
    {
        if(scope) scope->setOwner(id);
        identifier->setDeclaredType(type);
        identifier->addDeclaration(this, Identifier::ID_USER);
    }

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
    virtual void accept(ASTVisitor *v);
    virtual UserTypeDeclaration *userTypeDeclaration() { return this; }
    void addMethod(FunctionDeclaration *fdecl) {
        methods.push_back(fdecl);
    }

    virtual FunctionDeclaration *getMethod(std::string name, ASTFunctionType *opt_ty=NULL);

    void addMember(Declaration *decl) {
        members.push_back(decl);
    }
    void addStaticMember(Declaration *decl) {
        staticMembers.push_back(decl);
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

    virtual FunctionDeclaration *getMethod(std::string name, ASTFunctionType *opt_ty=NULL);

    void populateVTable();
    virtual size_t getSize() const;
    virtual size_t getAlign() const { return 8; } //XXX align of pointer
    long getMemberIndex(std::string member);
    virtual ClassDeclaration *classDeclaration() { return this; }
};

// used to represent a vtable for a given class that conforms to an interface
struct InterfaceVTable {
    InterfaceDeclaration *interface;
    UserTypeDeclaration *sourceType;
    std::vector<FunctionDeclaration*> vtable;
    ASTValue *cgvalue;

    InterfaceVTable(InterfaceDeclaration *id, UserTypeDeclaration *cd) : interface(id), sourceType(cd), cgvalue(NULL) {}

    void addFunction(FunctionDeclaration* fd) { vtable.push_back(fd); }
    void setValue(ASTValue *v) { cgvalue = v; }
    ASTValue *getValue() { return cgvalue; }
};

struct InterfaceDeclaration : public UserTypeDeclaration {
    std::map<std::string, InterfaceVTable*> vtables; //vtables of conforming structs and classes; used in CG
    InterfaceDeclaration(Identifier *id, ASTScope *sc, SourceLocation loc, DeclarationQualifier dqual) :
        UserTypeDeclaration(id, sc, loc, dqual) {}
    virtual size_t getSize() const { return 0; } //TODO?
    virtual long getMemberIndex(std::string member) {
        emit_message(msg::FAILURE, "member index is meaningless for interface");
        return 0;
    }

    void setVTable(std::string srcMangledName, InterfaceVTable *vt) {
        vtables[srcMangledName] = vt;
    }

    InterfaceVTable *getVTable(std::string srcMangledName) {
        if(vtables.count(srcMangledName)) return vtables[srcMangledName];
        return NULL;
    }

    virtual InterfaceDeclaration *interfaceDeclaration() { return this; }

    void populateVTable();
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
struct AllocExpression;
struct HeapAllocExpression;
struct StackAllocExpression;
struct FunctionExpression;

class CodegenContext;

struct Expression : public Statement
{
    ASTValue *value;

    virtual ASTValue *getValue(CodegenContext *ctx); // this just calls 'codegenExpression', but caches value

    virtual bool isLValue() { return false; }
    virtual bool isConstant() { return false; }
    virtual ASTType *getType() { return NULL; }
    virtual ASTType *getDeclaredType() { return NULL; }

    virtual bool isType() { return getDeclaredType(); }
    virtual bool isValue() { return getType(); }
    virtual bool isScope() { return false; } // do something with this

    Expression(SourceLocation l = SourceLocation()) : Statement(l), value(NULL) {}
    virtual Expression *expression() { return this; }

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
    virtual AllocExpression *allocExpression() { return NULL; }
    virtual HeapAllocExpression *heapAllocExpression() { return NULL; }
    virtual StackAllocExpression *stackAllocExpression() { return NULL; }
    FunctionExpression *functionExpression() { return NULL; }

    virtual Expression *lower() { return this; }

    //TODO: overrides

    virtual void accept(ASTVisitor *v);

    virtual Expression *coerceTo(ASTType *ty);

    virtual bool coercesTo(ASTType *ty) { return getType()->coercesTo(ty); }

    virtual int asInteger() {
        emit_message(msg::ERROR, "this value cannot be converted to integer", loc);
        return 0;
    }

    virtual std::string asString() { return ""; }
};

// "value.ptr"
// created during lowering, because this is created only for arrays and interfaces
struct DotPtrExpression : public Expression {
    Expression *lhs; // must be array or interface expression
    DotPtrExpression(Expression *val, SourceLocation l = SourceLocation()) : Expression(l), lhs(val) {}
    virtual ASTType *getType() {
        //return ASTType::getVoidTy()->getPointerTy();
        return lhs->getType()->getPointerElementTy()->getPointerTy();
    }
};

struct TypeExpression : public Expression
{
    ASTType *type;
    TypeExpression(ASTType *t, SourceLocation l = SourceLocation()) : Expression(l), type(t) {}
    virtual TypeExpression *typeExpression() { return this; }
    virtual ASTType *getDeclaredType() { return type; }
    virtual void accept(ASTVisitor *v);

    virtual std::string asString() {
        return type->getName();
    }
};

// used for semantic analysis of New Expression arguments
// this expression is a placeholder for the new allocation
struct DummyExpression : public Expression {
    ASTType *type;
    DummyExpression(ASTType *t) : type(t) {}
    virtual ASTType *getType() { return type; }
};

//XXX should NewExpression extend CallExpression?
struct FunctionExpression;
struct NewExpression : public Expression
{
    enum Alloc {
        STACK,
        HEAP
    };

    Alloc alloc;
    bool call;
    ASTType *type;

    //Expression *alloc; // XXX validate this field
    FunctionExpression *function; // found in validation
    std::list<Expression*> args;
    virtual ASTType *getType() {
        if(type->isArray()) return type; //XXX bit silly?
        if(alloc == STACK) return type;

        //else heap alloc
        return type->getReferenceTy();
    }
    NewExpression(ASTType *t, Alloc all, std::list<Expression*> a, bool c, SourceLocation l = SourceLocation()) :
        Expression(l), type(t), alloc(all), args(a), call(c), function(NULL) {}
    virtual NewExpression *newExpression() { return this; }
    virtual void accept(ASTVisitor *v);

    virtual Expression *lower();

    virtual std::string asString() {
        std::stringstream str;
        if(alloc == STACK) {
            str << type->getName();
        } else { //HEAP
            str << "new " << type->getName();
        }

        if(call) {
            str << "(";

            std::list<Expression*>::iterator it = args.begin();
            while(it != args.end()) {
                str << (*it)->asString();
                it++;
                if(it != args.end()) str << ", ";
            }

            str << ")";
        }

        return str.str();
    }
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

    virtual bool coercesTo(ASTType *ty);

    virtual ASTTupleType *getType();
    TupleExpression(std::vector<Expression*> e, SourceLocation l = SourceLocation()) :
        Expression(l), members(e) {}
    virtual TupleExpression *tupleExpression() { return this; }
    virtual void accept(ASTVisitor *v);

    virtual std::string asString() {
        std::stringstream str;
        str << "[";
        for(int i = 0; i < members.size(); i++) {
            str << members[i]->asString();
        }
        str << "]";
        return str.str();
    }
};

struct CastExpression : public Expression
{
    ASTType *type;
    Expression *expression;
    virtual ASTType *getType() { return type; }
    virtual CastExpression *castExpression() { return this; }
    CastExpression(ASTType *ty, Expression *exp, SourceLocation l = SourceLocation()) :
        Expression(l), type(ty), expression(exp){}

    virtual bool isConstant() {
        return expression->isConstant();
    }

    virtual void accept(ASTVisitor *v);

    virtual int asInteger() {
        return expression->asInteger();
    }


    virtual std::string asString() {
        return type->getName() + ": " + expression->asString();
    }
};

// should be pure virtual
struct PostfixExpression : public Expression
{
    PostfixExpression(SourceLocation l = SourceLocation()) : Expression(l) {}
    //virtual ~PostfixExpression();
    virtual PostfixExpression *postfixExpression() { return this; }
    virtual void accept(ASTVisitor *v);
};

//XXX hacky
// represents a function used in a function call.
// either a function pointer, or a declaration from an overload
struct FunctionExpression : public Expression {
    FunctionDeclaration *overload;
    Expression *fpointer;

    ASTType *getType() {
        if(overload) return overload->getType();
        if(fpointer) return fpointer->getType();
        return NULL;
    }
    FunctionExpression(SourceLocation loc = SourceLocation()) : Expression(loc), overload(NULL), fpointer(NULL) {}
    FunctionExpression(FunctionDeclaration *ol, SourceLocation loc = SourceLocation()) : Expression(loc), overload(ol), fpointer(NULL) {}
    FunctionExpression(Expression *fp, SourceLocation loc = SourceLocation()) : Expression(loc), overload(NULL), fpointer(fp) {}
    FunctionExpression *functionExpression() { return this; }
    bool isFunctionPointer() { return fpointer; }

    virtual std::string asString() {
        //XXX
        return "";
    }
};

struct CallExpression : public PostfixExpression
{
    bool isConstructor; //XXX a bit hacky
    virtual ~CallExpression() {}
    virtual CallExpression *callExpression() { return this; }

    Expression *function;
    FunctionExpression *resolvedFunction; //resolved in 'validate' pass
    std::list<Expression *> args; //TODO: make special argument expression to allow for name arguments?

    CallExpression(Expression *f, std::list<Expression*> a, SourceLocation l = SourceLocation()) :
        PostfixExpression(l), function(f), resolvedFunction(NULL), args(a), isConstructor(false) {}

    virtual ASTType *getType() {
        // this is a constructor call
        if(isConstructor) {
            // XXX this could be straight up wrong; double check when I'm not sleepy.
            // would a constructor on pointer always be a struct?
            if(args.front()->getType()->isPointer())
                return args.front()->getType()->getPointerElementTy();
            else
                return args.front()->getType();
        }

        if(!function->getType()) {
            return function->getDeclaredType();
        }

        ASTFunctionType *fty = function->getType()->asFunctionType();
        if(!fty) return NULL;
        return fty->getReturnType();
    }
    virtual void accept(ASTVisitor *v);

    virtual Expression *lower();

    virtual std::string asString() {
        std::stringstream str;
        str << function->asString() << "(";

        std::list<Expression*>::iterator it = args.begin();
        while(it != args.end()) {
            str << (*it)->asString();
            it++;
            if(it != args.end()) str << ", ";
        }

        str << ")";
        return str.str();
    }
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
    virtual ASTType *getType() {
        ASTType *lhsty = lhs->getType();
        if(lhsty->getPointerElementTy()) {
            return lhsty->getPointerElementTy();
        }

        if(lhsty->isTuple()) {
            int ind = index->asInteger();
            return lhsty->asTuple()->getMemberType(ind);
        }

        //XXX provide type if expression is const and type is struct?

        return NULL;
    }
    virtual void accept(ASTVisitor *v);

    virtual std::string asString() {
        return lhs->asString() + "[" + index->asString() + "]";
    }
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

//XXX lower dot expression to index expression?
struct DotExpression : public PostfixExpression
{
    virtual ~DotExpression() {}
    Expression *lhs;
    std::string rhs;
    virtual bool isLValue() { return lhs->isLValue(); } //TODO: only LValue if RHS is member
    DotExpression(Expression *l, std::string r, SourceLocation lo = SourceLocation()) :
        PostfixExpression(lo), lhs(l), rhs(r) {}
    virtual DotExpression *dotExpression() { return this; }
    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType();

    virtual Expression *lower();

    virtual std::string asString() {
        return lhs->asString() + "." + rhs;
    }
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

    virtual std::string asString() {
        Operator o((tok::TokenKind) op);
        if(o.isPostfixOp()) {
            return lhs->asString() + o.asString();
        } else {
            return o.asString() + lhs->asString();
        }
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

    virtual ASTType *getType();
    virtual Expression *lower();

    virtual std::string asString() { return lhs->asString() + " " + op.asString() +  " " + rhs->asString(); }
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

    Identifier *id;
    bool isUserType() { return id->isUserType(); }
    bool isVariable() { return id->isVariable(); }
    bool isFunction() { return id->isFunction(); }
    virtual bool isLValue() { return id->isVariable(); }
    IdentifierExpression(Identifier *i, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), id(i) {}
    Identifier *identifier() { return id; }
    Declaration *getDeclaration() { return id->getDeclaration(); }
    std::string getName() { return id->getName(); }
    virtual IdentifierExpression *identifierExpression() { return this; }
    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() { return id->getType(); }
    virtual ASTType *getDeclaredType() {
        if(id->isUserType()) return id->getDeclaredType();
        return NULL;
    }

    virtual bool isType() { return id->isType(); }
    virtual bool isValue() { return id->isValue(); }
    virtual bool isScope() { return false; } // do something with this

    virtual bool isConstant() {
        return id->getDeclaration() &&
            id->getDeclaration()->isConstant();
    }

    virtual int asInteger() {
        if(isConstant()) {
            VariableDeclaration *vdecl = id->getDeclaration()->variableDeclaration();
            if(vdecl && vdecl->value) {
                return vdecl->value->asInteger();
            }
        } else {
            emit_message(msg::ERROR, "attempt to convert non-const identifier to int", loc);
        }
        return 0;
    }

    virtual std::string asString() { return id->getName(); }
};

struct IdOpExpression : public Expression {
    Expression *expression;
    enum Type {
        Delete,
        Retain,
        Release
    };

    Type type;

    IdOpExpression(Expression *e, Type t, SourceLocation l = SourceLocation()) :
        Expression(l), expression(e), type(t) {}
    virtual void accept(ASTVisitor *v);
    bool isDelete() { return type == Delete; }
    bool isRetain() { return type == Retain; }
    bool isRelease() { return type == Release; }

    virtual std::string asString() {
        std::stringstream str;
        switch(type) {
            case Delete:
                str << "delete";
                break;
            case Retain:
                str << "retain";
                break;
            case Release:
                str << "release";
                break;
        }
        str << expression->asString();
        return str.str();
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

    //XXX need to escape string?
    virtual std::string asString() { return "\"" + string + "\""; }
};

struct PackExpression : public PrimaryExpression {
    virtual ~PackExpression() {}
    unsigned filesize;
    std::string filename;

    virtual ASTType *getType();

    virtual bool isConstant() { return true; }
    PackExpression(std::string filenm, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), filename(filenm) {
            filesize = getFileSize(filenm);
        }
    virtual PackExpression *packExpression() { return this; }
    virtual void accept(ASTVisitor *v);
    virtual Expression *lower();
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

    virtual std::string asString() {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
};

struct IntExpression : public NumericExpression {
    uint64_t value;
    IntExpression(ASTType *ty, uint64_t val, SourceLocation l = SourceLocation()) : NumericExpression(ty, l), value(val) {}
    virtual IntExpression *intExpression() { return this; }

    virtual int asInteger() {
            return value;
    }

    virtual std::string asString() {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
};

/*
 * used to represent memory allocated on stack or heap
 *
 * This expression is a bit weird because there isn't any explicit
 * code equivilent. Essentially, this is generated so validation works
 * nicer. For example: to validate the constructor call of a NewExpression,
 * the code expects a value to represent the allocated value.
 * Also, this provides a simple signifier for the codeGenerator.
 */
struct AllocExpression : public PrimaryExpression {
    ASTType *type;
    AllocExpression(ASTType *ty, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), type(ty) {}

    virtual ASTType *getType() { return type->getReferenceTy(); }
    virtual bool isLValue() { return true; }
    virtual bool isConstant() { return false; }
    virtual AllocExpression *allocExpression() { return this; }

    void accept(ASTVisitor *v);
};

struct HeapAllocExpression : public AllocExpression {
    HeapAllocExpression(ASTType *ty, SourceLocation l = SourceLocation()) :
        AllocExpression(ty, l) {}
    HeapAllocExpression *heapAllocExpression() { return this; }
};

struct StackAllocExpression : public AllocExpression {
    virtual ASTType *getType() { return type; }
    StackAllocExpression(ASTType *ty, SourceLocation l = SourceLocation()) :
        AllocExpression(ty, l) {}
    StackAllocExpression *stackAllocExpression() { return this; }
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
    ModuleDeclaration *module;
    virtual ImportExpression *importExpression() { return this; }
    ImportExpression(Expression *im, ModuleDeclaration *mod, SourceLocation l = SourceLocation()) :
        TopLevelExpression(l), expression(im), module(mod) { }
    virtual void accept(ASTVisitor *v);
    virtual ASTType *getType() { return NULL; } //TODO XXX

    virtual std::string asString() { return "import " + expression->asString(); }
};

struct IncludeExpression : public TopLevelExpression
{
    virtual ASTType *getType() { return NULL; } //TODO XXX
};

#endif
