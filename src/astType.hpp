#ifndef _ASTTYPE_HPP
#define _ASTTYPE_HPP

struct ASTVisitor;

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
    TYPE_USER,
    TYPE_FUNCTION,
    TYPE_POINTER,
    TYPE_DYNAMIC,
    TYPE_DYNAMIC_ARRAY,
    TYPE_ARRAY,
    TYPE_TUPLE,
    TYPE_VEC,
};

struct VariableDeclaration;
struct FunctionDeclaration;
struct UserTypeDeclaration;
struct ASTUserType;
struct ASTCompositeType;
struct ASTFunctionType;
#include <llvm/DebugInfo.h> //XXX

struct ASTType
{
    ASTTypeEnum kind;
    ASTType *pointerTy;
    ASTType *dynamicArrayTy;
    llvm::Type *cgType; //TODO: should not have llvm in here!
    llvm::DIType diType; //TODO: whatever, prototype

    std::map<int, ASTType*> arrayTy;

    void setTypeInfo(ASTTypeEnum en = TYPE_UNKNOWN)
    {
        if(en != TYPE_UNKNOWN) kind = en;
    }

    ASTType(enum ASTTypeEnum ty) : kind(ty), pointerTy(0), cgType(0)
    {}

    ASTType() : kind(TYPE_UNKNOWN), pointerTy(NULL), cgType(NULL)
    {}
    virtual ~ASTType(){}

    virtual bool isResolved(){ return true; }

    void accept(ASTVisitor *v);
    //ASTType(ASTTypeQual q) : qual(q), unqual(NULL), pointerTy(NULL), cgType(NULL) {}
    //virtual ~ASTType() { delete pointerTy; }
    //ASTType *getUnqual();
    ASTType *getPointerTy();
    ASTType *getArrayTy(int sz);
    ASTType *getArrayTy();
    virtual UserTypeDeclaration *getDeclaration() const { return NULL; }
    virtual size_t getSize()
    {
        switch(kind)
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
            case TYPE_POINTER: return 8;
            case TYPE_FLOAT: return 4;
            case TYPE_DOUBLE: return 8;
            case TYPE_ARRAY:
            case TYPE_DYNAMIC_ARRAY:
            default: assert(false && "unimplemented getsize");
        }
    }

    // conversion priority
    unsigned getPriority()
    {
        switch(kind)
        {
            case TYPE_USER:
                return 0;
            case TYPE_TUPLE:
                return 1;
            case TYPE_ARRAY:
                return 2;
            case TYPE_DYNAMIC_ARRAY:
                return 3;
            case TYPE_POINTER:
                return 4;
            case TYPE_BOOL:
                return 5;
            case TYPE_UCHAR:
                return 6;
            case TYPE_CHAR:
                return 7;
            case TYPE_USHORT:
                return 8;
            case TYPE_SHORT:
                return 9;
            case TYPE_UINT:
                return 10;
            case TYPE_INT:
                return 11;
            case TYPE_ULONG:
                return 12;
            case TYPE_LONG:
                return 13;
            case TYPE_FLOAT:
                return 14;
            case TYPE_DOUBLE:
                return 15;
        }
    }

    virtual bool coercesTo(ASTType *ty) const
    {
        switch(kind)
        {
            case TYPE_POINTER:
                return ty->isInteger() || ty == this ||
                    getReferencedTy()->kind == TYPE_VOID || ty->getReferencedTy()->kind == TYPE_VOID;
                //TODO: check if compatible pointer
            case TYPE_BOOL:
            case TYPE_UCHAR:
            case TYPE_CHAR:
            case TYPE_USHORT:
            case TYPE_SHORT:
            case TYPE_UINT:
            case TYPE_INT:
            case TYPE_ULONG:
            case TYPE_LONG:
            case TYPE_FLOAT:
            case TYPE_DOUBLE:
                return ty->isNumeric();
            default:
                return false;
        }
    }

    bool castsTo(ASTType *ty) const
    {
        return true;  //TODO
    }

    virtual size_t length()
    {
        return 1;
    }

    virtual size_t getAlign()
    {

        switch(kind)
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
            case TYPE_POINTER: return 8;
            case TYPE_FLOAT: return 4;
            case TYPE_DOUBLE: return 8;
            default: assert(false && "unimplemented align");
        }
    };

    virtual std::string getName()
    {
        switch(kind)
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
            case TYPE_FLOAT: return "float";
            case TYPE_DOUBLE: return "double";
            case TYPE_VOID: return "void";
            default: assert(false && "unimplemented getname");
        }
    }

    virtual ASTType *getReferencedTy() const { return NULL; }
    bool isAggregate() { return kind == TYPE_USER; } //XXX kinda...
    bool isBool() { return kind == TYPE_BOOL; }
    bool isInteger() { return kind == TYPE_BOOL || kind == TYPE_CHAR || kind == TYPE_SHORT ||
        kind == TYPE_INT || kind == TYPE_LONG ||
        kind == TYPE_UCHAR || kind == TYPE_USHORT || kind == TYPE_UINT || kind == TYPE_ULONG; }
    bool isSigned() { return kind == TYPE_CHAR || kind == TYPE_SHORT ||
        kind == TYPE_INT || kind == TYPE_LONG; }
    bool isFloating() { return kind == TYPE_FLOAT || kind == TYPE_DOUBLE; }
    bool isNumeric() { return isFloating() || isInteger(); }
    bool isVector() { return kind == TYPE_VEC; }
    bool isArray() { return kind == TYPE_ARRAY || kind == TYPE_DYNAMIC_ARRAY; }
    bool isComposite() { return compositeType(); }
    virtual bool isReference() { return false; } //TODO: name is confusing with 'getReferencedTy()'
    virtual bool isPointer() { return false; }
    virtual bool isUnknown() { return false; }
    virtual bool isClass() { return false; }
    virtual bool isInterface() { return false; }
    virtual bool isStruct() { return false; }
    virtual bool isUnion() { return false; }

    virtual ASTUserType *userType() { return NULL; }
    virtual ASTCompositeType *compositeType() { return NULL; }
    virtual ASTFunctionType *functionType() { return NULL; }

    static std::vector<ASTType *> typeCache;
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
    static ASTType *getTupleTy(std::vector<ASTType *> t);

    static ASTType *getFunctionTy(ASTType *ret, std::vector<ASTType *> param, bool vararg=false);
    static ASTType *getVoidFunctionTy();
};

struct ASTBasicType : public ASTType {
    ASTBasicType(ASTTypeEnum k) : ASTType(k) {}
};

struct ASTCompositeType : public ASTType {
    ASTCompositeType(ASTTypeEnum k) : ASTType(k) {}
    virtual ASTType *getMemberType(size_t i) = 0;
    virtual ASTCompositeType *compositeType() { return this; }
};

struct ASTFunctionType : public ASTCompositeType {
    ASTUserType *owner; // aka 'this'
    ASTType *ret;
    std::vector<ASTType*> params;
    bool vararg;

    ASTFunctionType(ASTType *r, std::vector<ASTType *> p, bool va = false) :
        ret(r), params(p), vararg(va), ASTCompositeType(TYPE_FUNCTION), owner(0) {}

    ASTFunctionType(ASTUserType *own, ASTType *r, std::vector<ASTType *> p, bool va = false) :
        ret(r), params(p), vararg(va), ASTCompositeType(TYPE_FUNCTION), owner(own) {}

    bool isVararg() { return vararg; }
    bool isMethod() { return owner; }

    ASTType *getMemberType(size_t index)
    {
        if(index == 0) return ret;
        return params[index-1];
    }

    ASTType *getReturnType() { return ret; }

    virtual ASTFunctionType *functionType() { return this; }
};

struct UserTypeDeclaration;
/**
 * Represents a user type. Since the declaration of a type may
 * appear after it's usage, the ASTUserType class is mostly
 * a shell used to proxy the Identifier, and subsequently the
 * UserTypeDeclaration class.
 *
 * The UserTypeDeclaration class should hold all of the true information
 * about a type.
 *
 * Also because of this, most of the methods of this class should not
 * be called until type resolution.
 *
 * Additionally, due to the requirement to have ASTType available before it's definition
 * (for example for a variable type, parameter type, cast), and potentially in different
 * packages, ASTUserType does not necessarily represent a unique type.
 */
struct ASTUserType : public ASTCompositeType {
    Identifier *identifier;

    ASTUserType(Identifier *id, UserTypeDeclaration *d=NULL) : ASTCompositeType(TYPE_USER),
    identifier(id) {
        //TODO: assert identifier is properly declared, declaration is present
    }

    virtual std::string getName() { return identifier->getName(); }
    virtual ASTUserType *userType() { return this; }
    virtual bool isUnknown() { return !identifier->getDeclaredType(); }
    virtual bool isOpaque();
    virtual size_t length();
    virtual size_t getSize();
    virtual size_t getAlign();
    Declaration *getMember(size_t i);
    ASTType *getMemberType(size_t i);
    long getMemberIndex(std::string member);
    long getVTableIndex(std::string method);
    long getMemberOffset(size_t i);
    virtual UserTypeDeclaration *getDeclaration() const {
        return (UserTypeDeclaration*) identifier->getDeclaration();
    }

    // only valid after AST is validated
    ASTType *getBaseType();

    virtual bool coercesTo(ASTType *ty) const {
        return getDeclaration() == ty->getDeclaration(); //TODO: might not work...
    }
    virtual bool isResolved() { return getDeclaration(); }

    ASTScope *getScope();

    virtual FunctionDeclaration *getDefaultConstructor();
    virtual bool isReference();
    virtual bool isClass();
    virtual bool isInterface();
    virtual bool isStruct();
    virtual bool isUnion();
};

struct ASTTupleType : public ASTCompositeType {
    std::vector<ASTType*> types;
    virtual ASTType *getMemberType(size_t i) { return types[i]; }
    virtual size_t length() { return types.size(); }
    virtual size_t getSize();
    virtual size_t getAlign();
    virtual std::string getName() { return ""; }
    ASTTupleType(std::vector<ASTType*> t) : ASTCompositeType(TYPE_TUPLE), types(t) {}
};

struct ASTPointerType : public ASTType {
    ASTType *ptrTo;
    virtual ASTType *getReferencedTy() const { return ptrTo; }
    ASTPointerType(ASTType *pto) : ASTType(TYPE_POINTER), ptrTo(pto) {}
    virtual bool isPointer() { return true; }
    virtual std::string getName();
};

struct ASTArrayType : public ASTCompositeType {
    ASTType *arrayOf;
    virtual ASTType *getReferencedTy() const { return arrayOf; }
    virtual ASTType *getMemberType(size_t i) { return arrayOf; }
    virtual size_t getSize()= 0;
    virtual size_t getAlign() = 0;
    virtual bool isDynamic() = 0;
    virtual size_t length() = 0;
    ASTArrayType(ASTType *pto, ASTTypeEnum kind) : arrayOf(pto), ASTCompositeType(kind) {}
    std::string getName() { return "array[" + arrayOf->getName() + "]"; }
};

struct ASTStaticArrayType : public ASTArrayType {
    unsigned size;
    virtual size_t getSize();
    virtual size_t getAlign();
    virtual bool isDynamic() { return false; }
    virtual size_t length() { return size; }
    ASTStaticArrayType(ASTType *pto, int sz) : ASTArrayType(pto, TYPE_ARRAY), size(sz){}
};

struct ASTDynamicArrayType : public ASTArrayType {
    virtual size_t getSize();
    virtual size_t getAlign();
    virtual bool isDynamic() { return true; }
    virtual size_t length() { return 0; }
    ASTDynamicArrayType(ASTType *pto) : ASTArrayType(pto, TYPE_DYNAMIC_ARRAY) {}
};

#endif
