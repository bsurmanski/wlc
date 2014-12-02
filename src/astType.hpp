#ifndef _ASTTYPE_HPP
#define _ASTTYPE_HPP

#include <sstream>

class ASTVisitor;

/*
struct ASTTypeQual
{
    bool isConst;
    ASTTypeQual() : isConst(false) {}
    ASTTypeQual(bool c) : isConst(c) {}
}; */

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
struct ASTPointerType;
struct ASTTupleType;
struct ASTArrayType;
struct ASTStaticArrayType;
struct ASTDynamicArrayType;

//XXX probably should not have llvm type/debug info in ASTType
#include <llvm/IR/Type.h>
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 5
#include <llvm/IR/DebugInfo.h>
#endif

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 4
#include <llvm/DebugInfo.h>
#endif

struct ASTType
{
    ASTTypeEnum kind;
    ASTType *pointerTy;
    ASTType *dynamicArrayTy;
    ASTType *constTy;

    struct Mod {
        bool isConst;
        bool isRef;
        Mod() : isConst(false), isRef(false) {}
    } mod;

    llvm::Type *cgType; //TODO: should not have llvm in here!
    llvm::DIType diType; //TODO: whatever, prototype

    std::map<int, ASTType*> arrayTy;

    ASTType(enum ASTTypeEnum ty) : kind(ty), pointerTy(0),
        dynamicArrayTy(0), constTy(0), cgType(0), mod()
    {}

    ASTType() : kind(TYPE_UNKNOWN), pointerTy(NULL),
        dynamicArrayTy(0), constTy(0), cgType(NULL), mod()
    {}
    virtual ~ASTType(){}

    virtual bool isResolved(){ return true; }

    void accept(ASTVisitor *v);
    //virtual ~ASTType() { delete pointerTy; }
    virtual ASTType *getUnqual() { return this; }
    ASTType *getPointerTy();
    ASTType *getReferenceTy();
    ASTType *getArrayTy(Expression *sz);
    ASTType *getArrayTy(unsigned sz);
    ASTType *getArrayTy();
    virtual UserTypeDeclaration *getDeclaration() const;
    virtual size_t getSize();

    // conversion priority
    unsigned getPriority();

    virtual bool coercesTo(ASTType *ty);

    //TODO
    bool castsTo(ASTType *ty) const { return true; }
    virtual size_t length() { return 1; }
    virtual size_t getAlign();
    virtual std::string getName();
    virtual std::string getMangledName();

    virtual ASTType *getPointerElementTy() const { return NULL; }
    bool isAggregate() { return kind == TYPE_USER; } //XXX kinda...
    bool isBool() { return kind == TYPE_BOOL; }
    bool isVoid() { return kind == TYPE_VOID; }
    bool isInteger() { return kind == TYPE_BOOL || kind == TYPE_CHAR || kind == TYPE_SHORT ||
        kind == TYPE_INT || kind == TYPE_LONG ||
        kind == TYPE_UCHAR || kind == TYPE_USHORT || kind == TYPE_UINT || kind == TYPE_ULONG; }
    bool isSigned() { return kind == TYPE_CHAR || kind == TYPE_SHORT ||
        kind == TYPE_INT || kind == TYPE_LONG; }
    bool isFloating() { return kind == TYPE_FLOAT || kind == TYPE_DOUBLE; }
    bool isNumeric() { return isFloating() || isInteger(); }
    bool isVector() { return kind == TYPE_VEC; }
    //bool isArray() { return kind == TYPE_ARRAY || kind == TYPE_DYNAMIC_ARRAY; }
    bool isComposite() { return asCompositeType(); }
    virtual bool isReference() { return false; }
    virtual bool isUnknown() { return false; }
    bool isPointer() { return asPointer(); }
    bool isFunctionPointer() { return isPointer() && getPointerElementTy()->isFunctionType(); }
    bool isUserTypePointer() { return isPointer() && getPointerElementTy()->isUserType(); }
    bool isVoidPointer() { return isPointer() && getPointerElementTy()->isVoid(); }
    bool isUserType() { return asUserType(); }
    bool isClass() { return asClass(); }
    bool isInterface() { return asInterface(); }
    bool isStruct() { return asStruct(); }
    bool isUnion() { return asUnion(); }
    bool isFunctionType() { return asFunctionType(); }
    bool isTuple() { return asTuple(); }
    bool isArray() { return asArray(); }
    bool isDArray() { return asDArray(); }
    bool isSArray() { return asSArray(); }

    virtual ASTFunctionType *asFunctionType() { return NULL; }
    virtual ASTCompositeType *asCompositeType() { return NULL; }
    virtual ASTUserType *asUserType() { return NULL; }
    virtual ASTUserType *asClass() { return NULL; }
    virtual ASTUserType *asInterface() { return NULL; }
    virtual ASTUserType *asUnion() { return NULL; }
    virtual ASTUserType *asStruct() { return NULL; }
    virtual ASTPointerType *asPointer() { return NULL; }
    virtual ASTTupleType *asTuple() { return NULL; }
    virtual ASTArrayType *asArray() { return NULL; }
    virtual ASTDynamicArrayType *asDArray() { return NULL; }
    virtual ASTStaticArrayType *asSArray() { return NULL; }
    virtual bool is(ASTType *t) { return this == t; } // NOTE: only valid for basic type, where type is statically defined
    virtual bool extends(ASTType *t) { return false; }


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

    static ASTFunctionType *getFunctionTy(ASTType *ret, std::vector<ASTType *> param, bool vararg=false);
    static ASTType *getVoidFunctionTy();
};

// type decorator; used for const/volatile
struct ASTTypeDecorator : public ASTType {
    ASTType *base;
};

// used for primative types
struct ASTBasicType : public ASTType {
    ASTBasicType(ASTTypeEnum k) : ASTType(k) {}
};

struct ASTCompositeType : public ASTType {
    ASTCompositeType(ASTTypeEnum k) : ASTType(k) {}
    virtual ASTType *getMemberType(size_t i) = 0;
    virtual ASTCompositeType *asCompositeType() { return this; }
    virtual size_t length() = 0;
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

    virtual size_t length() { return params.size(); }
    virtual ASTFunctionType *asFunctionType() { return this; }
    bool isVararg() { return vararg; }
    bool isMethod() { return owner; }

    virtual std::string getName() {
        std::string name = ret->getName() + " function(";
        for(int i = 0; i < params.size(); i++){
            if((i+1) < params.size()){
                name = name + params[i]->getName() + ", ";
            } else {
                name = name + params[i]->getName();
            }
        }
        name = name + ")";
        return name;
    }

    virtual std::string getMangledName() {
        char buf[32];
        sprintf(buf, "%02lu", params.size());
        std::string name = std::string("f") + std::string(buf) + ret->getMangledName();
        for(int i = 0; i < params.size(); i++){
            name = name + "$$" + params[i]->getMangledName();
        }
        return name;
    }

    ASTType *getMemberType(size_t index)
    {
        if(index == 0) return ret;
        return params[index-1];
    }

    ASTType *getReturnType() { return ret; }

    virtual ASTFunctionType *functionType() { return this; }

    virtual bool is(ASTType *t) {
        if(ASTFunctionType *oth = dynamic_cast<ASTFunctionType*>(t)) {
            if(!getReturnType()->is(oth->getReturnType()) ||
                    params.size() != oth->params.size()) return false;
            for(int i = 0; i < params.size(); i++) {
                if(!params[i]->is(oth->params[i])) return false;
            }
            return true;
        }
        return false;
    }
};

struct UserTypeDeclaration;
/**
 * Represents a user type. Since the declaration of a type may
 * appear after it's usage, the ASTUserType class is mostly
 * a shell used to proxy the Identifier, and subsequently the
 * UserTypeDeclaration class. This is also why class/struct/union types
 * can not be represented as separate classes inheriting ASTUserType.
 *
 * The UserTypeDeclaration class should hold all of the true information
 * about a type.
 *
 * Also because of the late binding, most of the methods of this class should not
 * be called until type resolution.
 *
 * Additionally, due to the requirement to have ASTType available before it's definition
 * (for example for a variable type, parameter type, cast), and potentially in different
 * packages, ASTUserType does not necessarily represent a unique type. This is quite inconvenient,
 * but due to the late binding, I believe it is necessary
 */
struct ASTUserType : public ASTCompositeType {
    Identifier *identifier;

    ASTUserType(Identifier *id, UserTypeDeclaration *d=NULL) : ASTCompositeType(TYPE_USER),
    identifier(id) {
        //TODO: assert identifier is properly declared, declaration is present
    }

    virtual std::string getName() { return identifier->getName(); }
    virtual std::string getMangledName() {
        if(isClass())
            return "UC" + identifier->getMangledName();
        if(isUnion())
            return "UU" + identifier->getMangledName();
        if(isInterface())
            return "UI" + identifier->getMangledName();
        if(isStruct())
            return "US" + identifier->getMangledName();
	assert(false && "unknown user type");
	return "";
    }
    virtual ASTUserType *asUserType() { return this; }
    virtual ASTUserType *asClass();
    virtual ASTUserType *asInterface();
    virtual ASTUserType *asUnion();
    virtual ASTUserType *asStruct();

    virtual bool isUnknown() { return !identifier->getDeclaredType(); }
    virtual bool isOpaque();
    virtual size_t length();
    virtual size_t getSize();
    virtual size_t getAlign();
    Declaration *getMember(size_t i);
    Declaration *getMember(std::string member);
    ASTType *getMemberType(size_t i);
    long getMemberIndex(std::string member);
    long getVTableIndex(std::string method);
    long getMemberOffset(size_t i);
    virtual UserTypeDeclaration *getDeclaration() const {
        return (UserTypeDeclaration*) identifier->getDeclaration();
    }

    // only valid after AST is validated
    ASTType *getBaseType();
    Identifier *getBaseIdentifier();

    virtual bool coercesTo(ASTType *ty) {
        return getDeclaration() == ty->getDeclaration() || //TODO: might not work...
                    (isClass() && ty->isClass() && extends(ty)) ||
                    (isClass() && ty->isBool()) ||
                    (isClass() && ty->isVoidPointer());
    }
    virtual bool isResolved() { return getDeclaration(); }

    ASTScope *getScope();

    virtual FunctionDeclaration *getDefaultConstructor();
    virtual FunctionDeclaration *getConstructor();
    virtual FunctionDeclaration *getDestructor();
    virtual bool isReference();
    virtual bool is(ASTType *t);
    virtual bool extends(ASTType *t);
};

struct ASTTupleType : public ASTCompositeType {
    std::vector<ASTType*> types;
    virtual ASTTupleType *asTuple() { return this; }
    virtual ASTType *getMemberType(size_t i) { return types[i]; }
    virtual size_t length() { return types.size(); }
    virtual size_t getSize();
    virtual size_t getAlign();
    virtual std::string getName() {
        std::string name = "tuple[";
        for(int i = 0; i < types.size(); i++) {
            if((i+1) < types.size()) {
                name = name + types[i]->getName() + ", ";
            } else {
                name = name + types[i]->getName() + "]";
            }
        }
        return name;
    }

    virtual std::string getMangledName() {
	std::stringstream sstream;
        sstream << "t" << types.size();
        for(int i = 0; i < types.size(); i++){
            if((i+1) < types.size()) {
                sstream << types[i]->getMangledName() << "$$";
            } else {
                sstream << types[i]->getMangledName();
            }
        }
        return sstream.str();
    }

    virtual bool is(ASTType *t) {
        if(ASTTupleType *oth = dynamic_cast<ASTTupleType*>(t)) {
            if(types.size() == oth->types.size()) {
                for(int i = 0; i < types.size(); i++) {
                    if(!types[i]->is(oth->types[i])) {
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
    }

    virtual bool coercesTo(ASTType *ty) {
        ASTTupleType *otup = ty->asTuple();
        if(!otup || length() != otup->length()) return false;

        for(int i = 0; i < length(); i++) {
            // TODO: allow tuple coercion if one tuple contains
            // base pointer to other class?
            // eg [int, SomeClass] -> [int, BaseClass]
            if(!getMemberType(i)->is(otup->getMemberType(i))) {
                return false;
            }
        }

        return true;
    }

    ASTTupleType(std::vector<ASTType*> t) : ASTCompositeType(TYPE_TUPLE), types(t) {}
};

struct ASTPointerType : public ASTType {
    ASTType *ptrTo;
    virtual ASTType *getPointerElementTy() const { return ptrTo; }
    ASTPointerType(ASTType *pto) : ASTType(TYPE_POINTER), ptrTo(pto) {}
    virtual ASTPointerType *asPointer() { return this; }
    virtual std::string getName();
    virtual std::string getMangledName();
    virtual bool is(ASTType *t) {
        if(ASTPointerType *oth = dynamic_cast<ASTPointerType*>(t)) {
            return ptrTo->is(oth->ptrTo);
        }
        return false;
    }

    virtual bool coercesTo(ASTType *ty) {
        if(ty->isInteger()) return true;
        ASTPointerType *pty = ty->asPointer();
        if(!pty) return false;

        return ptrTo->is(pty->ptrTo) ||
            // allow implicit cast from void pointer
            getPointerElementTy()->kind == TYPE_VOID ||
            // allow implicit cast to void pointer
            (pty->getPointerElementTy() &&
             pty->getPointerElementTy()->kind == TYPE_VOID);
        //TODO: check if compatible pointer
    }
};

struct ASTArrayType : public ASTCompositeType {
    ASTType *arrayOf;
    virtual ASTType *getPointerElementTy() const { return arrayOf; }
    virtual ASTType *getMemberType(size_t i) { return arrayOf; }
    virtual size_t getSize()= 0;
    virtual size_t getAlign() = 0;
    virtual bool isDynamic() = 0;
    virtual size_t length() = 0;
    ASTArrayType(ASTType *pto, ASTTypeEnum kind) : arrayOf(pto), ASTCompositeType(kind) {}
    std::string getName() { return arrayOf->getName() + "[]"; }
    std::string getMangledName() { return "a" + arrayOf->getMangledName(); }

    virtual ASTArrayType *asArray() { return this; }
};

struct ASTStaticArrayType : public ASTArrayType {
    Expression *size;
    virtual size_t getSize();
    virtual size_t getAlign();
    virtual bool isDynamic() { return false; }
    virtual size_t length();
    ASTStaticArrayType(ASTType *pto, Expression *sz) : ASTArrayType(pto, TYPE_ARRAY), size(sz){}
    virtual bool coercesTo(ASTType *ty) {
        if(ASTStaticArrayType *saty = dynamic_cast<ASTStaticArrayType*>(ty)) {
            return arrayOf->is(saty->arrayOf);
        }

        if(arrayOf->getPointerTy()->is(ty)) {
            return true;
        }

        return false;
    }

    virtual ASTStaticArrayType *asSArray() { return this; }
    std::string getName() {
        std::stringstream ss;
        ss << arrayOf->getName() << "[" << length() << "]";
        return ss.str();
    }
    //std::string getName() { return "Array[" + arrayOf->getName() + "]"; }
    //std::string getMangledName() { return "A" + arrayOf->getMangledName(); }
};

struct ASTDynamicArrayType : public ASTArrayType {
    virtual size_t getSize();
    virtual size_t getAlign();
    virtual bool isDynamic() { return true; }
    virtual size_t length() { return 0; }
    ASTDynamicArrayType(ASTType *pto) : ASTArrayType(pto, TYPE_DYNAMIC_ARRAY) {}
    virtual bool coercesTo(ASTType *ty) {
        if(ASTDynamicArrayType *daty = dynamic_cast<ASTDynamicArrayType*>(ty)) {
            return arrayOf->is(daty->arrayOf);
        }

        if(arrayOf->getPointerTy()->is(ty)) {
            return true;
        }

        return false;
    }

    virtual ASTDynamicArrayType *asDArray() { return this; }
};

#endif
