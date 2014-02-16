#ifndef _AST_HPP
#define _AST_HPP

#include <assert.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <limits.h>

#include "symbolTable.hpp"
#include "identifier.hpp"
#include "sourceLocation.hpp"

struct Statement;
struct Declaration;
struct Expression;
struct DeclarationExpression;
struct TranslationUnit;
struct ASTType;
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
    Identifier *parent;
    Identifier *identifier;
    void *cgValue; // opaque pointer to specific codegen information

    Package() : identifier(NULL), scope(NULL), cgValue(NULL) {}
    Package(Identifier *id) : identifier(id), scope(NULL), cgValue(NULL) {
        if(id) id->type = Identifier::ID_PACKAGE;
        //TODO: scope should have parent?
    }
    virtual ~Package() { if(scope) delete scope; }

    Identifier *getIdentifier() { return identifier; }
    SymbolTable *getScope() { if(!scope) scope = new SymbolTable; return scope; } //TODO: subscope of parent

    std::vector<Package*> children;

    void setParent(Package *p)
    {
        parent = p->getIdentifier();
    }

    void addPackage(Package *p) {
        //scope->add(str, p->getIdentifier());
        p->setParent(this);
        children.push_back(p);
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
    std::string filenm;
    bool expl; // explicitly requested for compile. eg, not included
    TranslationUnit(Identifier *id, std::string fn = "") : Package(id), filenm(fn), expl(false) {}
    ~TranslationUnit() { }
    virtual bool isTranslationUnit() { return true; }
    std::string getName() { return ""; }
};

struct AST
{
    Package *root;
    std::map<std::string, TranslationUnit*> units;

    AST() { root = new Package; }
    ~AST() { delete root; }
    Package *getRootPackage() { return root; }
    TranslationUnit *getUnit(std::string str)
    {
        char APATH[PATH_MAX + 1];
        realpath(str.c_str(), APATH);
        std::string astr = std::string(str);
        if(units.count(astr)) return units[astr];
        //TranslationUnit *nunit = new TranslationUnit(NULL); //TODO: identifier ?
        //return nunit;
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

enum ASTTypeEnum
{
    TYPE_UNKNOWN,
    TYPE_UNKNOWN_USER, //unknown user declared type
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG,
    TYPE_UCHAR,
    TYPE_USHORT,
    TYPE_UINT,
    TYPE_ULONG,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_CLASS,
    TYPE_FUNCTION,
    TYPE_POINTER,
    TYPE_DYNAMIC,
    TYPE_DYNAMIC_ARRAY,
    TYPE_ARRAY,
    TYPE_TUPLE,
    TYPE_VEC,
};

struct TypeInfo
{
    virtual ~TypeInfo() {}
    virtual ASTType *getReferenceTy() { return NULL; }
    virtual std::string getName() { return ""; }
    virtual size_t getSize() { return 0; }
    virtual size_t getAlign() { return 0; }
};


struct DynamicTypeInfo : public TypeInfo
{
};

struct VariableDeclaration;
struct StructDeclaration;

struct CompositeTypeInfo : TypeInfo
{
    virtual ASTType *getContainedType(unsigned index) = 0;
    virtual unsigned length() = 0;
};

struct PointerTypeInfo : public TypeInfo
{
    ASTType *ptrTo;
    virtual ASTType *getReferenceTy() { return ptrTo; }
    PointerTypeInfo(ASTType *pto) : ptrTo(pto) {}
    std::string getName();
};

struct ArrayTypeInfo : public CompositeTypeInfo
{
    ASTType *arrayOf;
    unsigned size;
    virtual ASTType *getReferenceTy() { return arrayOf; }
    virtual ASTType *getContainedType(unsigned i) { return arrayOf; }
    virtual unsigned length() { return size; }
    ArrayTypeInfo(ASTType *pto, int sz = 0) : arrayOf(pto), size(sz) {}
    bool isDynamic() { return size == 0; }
    std::string getName();
};

struct StructTypeInfo : public CompositeTypeInfo
{
    bool packed;
    SymbolTable *scope;
    Identifier *identifier;
    std::vector<Declaration*> members; // <type, name>
    StructTypeInfo(Identifier *id, SymbolTable *sc, std::vector<Declaration*> m) :
        identifier(id), scope(sc), members(m), packed(false){}
    std::string getName() { return identifier->getName(); }
    virtual ASTType *getContainedType(unsigned i);
    virtual unsigned length() { return members.size(); }
    virtual size_t getSize();
    virtual size_t getAlign();
    StructDeclaration *getDeclaration() { return (StructDeclaration*) identifier->getDeclaration(); }
};

struct NamedUnknownInfo : public TypeInfo
{
    SymbolTable *scope;
    Identifier *identifier;
    NamedUnknownInfo(Identifier *id, SymbolTable *sc) : identifier(id), scope(sc) {}
};

struct AliasTypeInfo : public TypeInfo
{
    Identifier *identifier;
    ASTType *alias;
    AliasTypeInfo(Identifier *id, ASTType *a) :identifier(id), alias(a) {}
};

struct TupleTypeInfo : public CompositeTypeInfo
{
    std::vector<ASTType*> types;
    virtual ASTType *getContainedType(unsigned i) { return types[i]; }
    virtual unsigned length() { return types.size(); }
    //virtual size_t getSize();
    //virtual size_t getAlign();
    TupleTypeInfo(std::vector<ASTType*> t) : types(t) {}
};


#include <llvm/DebugInfo.h> //XXX
struct ASTType
{
    ASTTypeEnum type;
    ASTType *pointerTy;
    ASTType *dynamicArrayTy;
    llvm::Type *cgType; //TODO: should not have llvm in here!
    llvm::DIType diType; //TODO: whatever, prototype

    std::map<int, ASTType*> arrayTy;

    TypeInfo *info;
    void setTypeInfo(TypeInfo *i, ASTTypeEnum en = TYPE_UNKNOWN)
    {
        info = i;
        if(en != TYPE_UNKNOWN) type = en;
    }
    TypeInfo *getTypeInfo() { return info; }

    ASTType(enum ASTTypeEnum ty) : type(ty), pointerTy(0), cgType(0), info(0)
    {}

    ASTType() : type(TYPE_UNKNOWN), pointerTy(NULL), cgType(NULL), info(0) {}
    //ASTType(ASTTypeQual q) : qual(q), unqual(NULL), pointerTy(NULL), cgType(NULL) {}
    //virtual ~ASTType() { delete pointerTy; }
    //ASTType *getUnqual();
    ASTType *getPointerTy();
    ASTType *getArrayTy(int sz);
    ASTType *getArrayTy();
    size_t size() const
    {
        switch(type)
        {
            case TYPE_CHAR:
            case TYPE_UCHAR:
            case TYPE_BOOL: return 1;
            case TYPE_USHORT:
            case TYPE_SHORT: return 2;
            case TYPE_UINT:
            case TYPE_INT: return 4;
            case TYPE_ULONG:
            case TYPE_LONG: return 8;
            case TYPE_TUPLE:
            case TYPE_STRUCT: return info->getSize();
            case TYPE_POINTER: return 8;
            case TYPE_FLOAT: return 4;
            case TYPE_DOUBLE: return 8;
            default: assert(false && "havent sized this type yet"); return 0; //TODO
        }
    };

    size_t align() const
    {

        switch(type)
        {
            case TYPE_CHAR:
            case TYPE_UCHAR:
            case TYPE_BOOL: return 1;
            case TYPE_USHORT:
            case TYPE_SHORT: return 2;
            case TYPE_UINT:
            case TYPE_INT: return 4;
            case TYPE_ULONG:
            case TYPE_LONG: return 8;
            case TYPE_TUPLE:
            case TYPE_STRUCT: return info->getAlign();
            case TYPE_POINTER: return 8;
            case TYPE_FLOAT: return 4;
            case TYPE_DOUBLE: return 8;
            default: assert(false && "havent sized this type yet"); return 0; //TODO
        }
    };

    std::string getName()
    {
        switch(type)
        {
            case TYPE_CHAR: return "char";
            case TYPE_UCHAR: return "uchar";
            case TYPE_BOOL: return "bool";
            case TYPE_SHORT: return "short";
            case TYPE_USHORT: return "ushort";
            case TYPE_INT: return "int";
            case TYPE_UINT: return "uint";
            case TYPE_LONG: return "long";
            case TYPE_ULONG: return "ulong";
            default: return info->getName();
        }
    }

    ASTType *getReferencedTy() { return info->getReferenceTy(); }
    bool isAggregate() { return type == TYPE_STRUCT || type == TYPE_UNION || type == TYPE_CLASS; }
    bool isClass() { return type == TYPE_CLASS; }
    bool isStruct() { return type == TYPE_STRUCT; }
    bool isBool() { return type == TYPE_BOOL; }
    bool isInteger() { return type == TYPE_BOOL || type == TYPE_CHAR || type == TYPE_SHORT ||
        type == TYPE_INT || type == TYPE_LONG ||
        type == TYPE_UCHAR || type == TYPE_USHORT || type == TYPE_UINT || type == TYPE_ULONG; }
    bool isSigned() { return type == TYPE_CHAR || type == TYPE_SHORT ||
        type == TYPE_INT || type == TYPE_LONG; }
    bool isFloating() { return type == TYPE_FLOAT || type == TYPE_DOUBLE; }
    bool isVector() { return type == TYPE_VEC; }
    bool isArray() { return type == TYPE_ARRAY || type == TYPE_DYNAMIC_ARRAY; }
    bool isPointer() { return this && type == TYPE_POINTER; } //TODO: shouldnt need to test for this
    bool isComposite() { return dynamic_cast<CompositeTypeInfo*>(info); }

#define DECLTY(NM) static ASTType *NM; static ASTType *get##NM();
    DECLTY(VoidTy)
    DECLTY(BoolTy)

    DECLTY(CharTy)
    DECLTY(ShortTy)
    DECLTY(IntTy)
    DECLTY(LongTy)

    DECLTY(UCharTy)
    DECLTY(UShortTy)
    DECLTY(UIntTy)
    DECLTY(ULongTy)

    DECLTY(FloatTy)
    DECLTY(DoubleTy)
#undef DECLTY

    static ASTType *DynamicTy; static ASTType *getDynamicTy();
    //static ASTType *getTupleTy(std::vector<ASTType *ty> t);
};

struct ASTValueInfo
{
    virtual ~ASTValueInfo(){}
};

struct ASTValue
{
    bool lValue;
    ASTType *type;
    llvm::Value *cgValue; //XXX
    llvm::DIVariable debug; //XXX
    ASTType *getType() { return type; }
    ASTValue(ASTType *ty, void *cgv = NULL, bool lv = false) : type(ty),
        cgValue((llvm::Value*) cgv), lValue(lv) {}
    bool isLValue() { return lValue; }
};

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
    virtual std::string getName() { if(identifier) return identifier->getName(); return ""; }
    virtual ASTType *getType() = 0;

    virtual FunctionDeclaration *functionDeclaration() { return NULL; }
    virtual VariableDeclaration *variableDeclaration() { return NULL; }
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
    llvm::DISubprogram diSubprogram;
    FunctionPrototype *prototype;
    SymbolTable *scope;
    Statement *body;
    void *cgValue;
    FunctionDeclaration(Identifier *id, FunctionPrototype *p, SymbolTable *sc, Statement *st, SourceLocation loc) : Declaration(id, loc), prototype(p), scope(sc), body(st), cgValue(NULL) {}
    virtual FunctionDeclaration *functionDeclaration() { return this; }
    SymbolTable *getScope() { return scope; }
    ASTType *getReturnType() { return prototype->returnType; }

    virtual ASTType *getType() { return prototype->returnType; } //TODO: prototype should be type?
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
    Expression *sz;
    ArrayDeclaration(ASTType *ty, Identifier *nm, Expression *val, Expression *s, SourceLocation loc, bool ext = false) :
        VariableDeclaration(ty, nm, val, loc, ext), sz(s) {}
    virtual ArrayDeclaration *arrayDeclaration() { return this; }
};

struct TypeDeclaration : public Declaration
{
    ASTType *type;
    TypeDeclaration(Identifier *id, ASTType *ty, SourceLocation loc) : Declaration(id, loc), type(ty) {}

    virtual ASTType *getType() { return type; }
};

struct StructDeclaration : public TypeDeclaration
{
    std::vector<Declaration*> members;
    StructDeclaration(Identifier *id, ASTType *ty, std::vector<Declaration*> m, SourceLocation loc) : TypeDeclaration(id, ty, loc), members(m) {}
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
struct PackageExpression;
struct CastExpression;

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
    virtual BlockExpression *blockExpression() { return NULL; }
    virtual IfExpression *ifExpression() { return NULL; }
    virtual WhileExpression *whileExpression() { return NULL; }
    virtual ForExpression *forExpression() { return NULL; }
    virtual PassExpression *passExpression() { return NULL; }
    virtual SwitchExpression *switchExpression() { return NULL; }
    virtual ImportExpression *importExpression() { return NULL; }
    virtual PackageExpression *packageExpression() { return NULL; }
    virtual CastExpression *castExpression() { return NULL; }

    //TODO: overrides
};

struct NewExpression : public Expression
{
    ASTType *type;
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
    TupleExpression(std::vector<Expression*> e, SourceLocation l = SourceLocation()) :
        Expression(l), members(e) {}
};

struct CastExpression : public Expression
{
    ASTType *type;
    Expression *expression;
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
    std::vector<Expression *> args;
    CallExpression(Expression *f, std::vector<Expression*> a, SourceLocation l = SourceLocation()) :
        PostfixExpression(l), function(f), args(a) {}
};

struct IndexExpression : public PostfixExpression
{
    virtual IndexExpression *indexExpression() { return this; }
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
    DotExpression(Expression *l, std::string r, SourceLocation lo = SourceLocation()) :
        PostfixExpression(lo), lhs(l), rhs(r) {}
};

struct UnaryExpression : public Expression
{
    virtual UnaryExpression *unaryExpression() { return this; }
    Expression *lhs;
    unsigned op;
    UnaryExpression(unsigned o, Expression *l, SourceLocation lo = SourceLocation()) :
        Expression(lo), lhs(l), op(o) {}
};

struct BinaryExpression : public Expression
{
    virtual BinaryExpression *binaryExpression() { return this; }
    Expression *lhs;
    Expression *rhs;
    unsigned op;
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

    NumericExpression(NumericType t, ASTType* ty, double val, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), type(t),  astType(ty), floatValue(val) {}
    NumericExpression(NumericType t, ASTType *ty, uint64_t val, SourceLocation l = SourceLocation()) :
        PrimaryExpression(l), type(t), astType(ty), intValue(val) {}
    virtual NumericExpression *numericExpression() { return this; }
};

struct DeclarationExpression : public Expression
{
    Declaration *decl;
    DeclarationExpression(SourceLocation l = SourceLocation()) : Expression(l) {}
    virtual DeclarationExpression *declarationExpression() { return this; }
};

struct Statement;


// value of (bool) 0. if 'pass' is encountered, value of pass
//TODO: probably shouldn't be an expression
struct BlockExpression : public Expression
{
    //TODO: sym table
    std::vector<Statement*> statements;
    BlockExpression(std::vector<Statement*> s, SourceLocation l = SourceLocation()) :
        Expression(l), statements(s) {}
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
    IfExpression(Expression *c, Statement *b, Statement *e, SourceLocation l = SourceLocation()) :
        Expression(l), condition(c), body(b), elsebranch(e) {}
};

// value same as if
struct WhileExpression : public Expression
{
    Expression *condition;
    Statement *body;
    Statement *elsebranch;
    virtual WhileExpression *whileExpression() { return this; }
    WhileExpression(Expression *c, Statement *b, Statement *e, SourceLocation l = SourceLocation()) :
        Expression(l), condition(c), body(b), elsebranch(e) {}
};

struct ForExpression : public Expression
{
    Statement *decl;
    Expression *condition;
    Statement *update;
    Statement *body;
    Statement *elsebranch;
    virtual ForExpression *forExpression() { return this; }
    ForExpression(Statement *d, Expression *c, Statement *u, Statement *b, Statement *e,
            SourceLocation l = SourceLocation()) :
        Expression(l), decl(d), condition(c), update(u), body(b), elsebranch(e) {}
};

struct SwitchExpression : public Expression
{
    //TODO
    Expression *_switch;
    std::vector<Expression*> cases;
    std::vector<Statement*> statements;
    virtual SwitchExpression *switchExpression() { return this; }
    SwitchExpression(SourceLocation l = SourceLocation()) : Expression(l) {}
};

struct PackageExpression : public Expression
{
    Expression *package;
    virtual PackageExpression *packageExpression() { return this; }
    PackageExpression(Expression *p, SourceLocation l = SourceLocation())
        : Expression(l), package(p) {}
};

// import <STRING> |
// import <IDENTIFIER>
struct ImportExpression : public Expression
{
    Expression *expression;
    TranslationUnit *unit;
    virtual ImportExpression *importExpression() { return this; }
    ImportExpression(Expression *im, TranslationUnit *u, SourceLocation l = SourceLocation()) :
        Expression(l), expression(im), unit(u) { }
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

struct PassStatement : public Statement
{
    PassStatement(SourceLocation l) : Statement(l){}
    // XXX will assign to special variable '$', which is created on blockExpression entry. essentially, assigning on pass
    // can be transformed into if(true) $ = val; else $ = val2; x = $;
    // ... maybe
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
