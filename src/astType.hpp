#ifndef _ASTTYPE_HPP
#define _ASTTYPE_HPP

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
    virtual size_t getSize() { assert(false && "size of type is unknown"); }
    virtual size_t getAlign() { assert(false && "alignment of type is unknown"); }
    virtual size_t length() { return 1; }
};


struct DynamicTypeInfo : public TypeInfo
{
};

struct VariableDeclaration;
struct StructUnionDeclaration;

struct CompositeTypeInfo : TypeInfo
{
    virtual ASTType *getContainedType(unsigned index) = 0;
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
    virtual ASTType *getReferenceTy() { return arrayOf; }
    virtual ASTType *getContainedType(unsigned i) { return arrayOf; }
    virtual size_t getSize() = 0;
    virtual size_t getAlign() = 0;
    virtual bool isDynamic() = 0;
    virtual size_t length() = 0;
    ArrayTypeInfo(ASTType *pto) : arrayOf(pto) {}
    std::string getName();
};

struct StaticArrayTypeInfo : ArrayTypeInfo
{
    unsigned size;
    virtual size_t getSize();
    virtual size_t getAlign();
    virtual bool isDynamic() { return false; }
    virtual size_t length() { return size; }
    StaticArrayTypeInfo(ASTType *pto, int sz) : ArrayTypeInfo(pto), size(sz) {}
};

struct DynamicArrayTypeInfo : ArrayTypeInfo
{
    virtual size_t getSize();
    virtual size_t getAlign();
    virtual bool isDynamic() { return true; }
    virtual size_t length() { return 0; }
    DynamicArrayTypeInfo(ASTType *pto) : ArrayTypeInfo(pto) {}
};

struct HetrogenTypeInfo : public CompositeTypeInfo
{
    SymbolTable *scope;
    Identifier *identifier;
    std::vector<Declaration*> members; // <type, name>
    HetrogenTypeInfo(Identifier *id, SymbolTable *sc, std::vector<Declaration*> m) :
        identifier(id), scope(sc), members(m){}
    std::string getName() { return identifier->getName(); }
    virtual ASTType *getContainedType(unsigned i);
    virtual size_t getMemberOffset(std::string member) = 0;
    virtual size_t getMemberIndex(std::string member) = 0;
    Declaration *getMemberDeclaration(std::string member);
    virtual size_t getAlign();
    StructUnionDeclaration *getDeclaration() { return (StructUnionDeclaration*) identifier->getDeclaration(); }
};

struct StructTypeInfo : public HetrogenTypeInfo
{
    bool packed;
    StructTypeInfo(Identifier *id, SymbolTable *sc, std::vector<Declaration*> m) :
        HetrogenTypeInfo(id, sc, m) {}
    virtual size_t length() { return members.size(); }
    virtual size_t getSize();
    virtual size_t getMemberOffset(std::string member);
    virtual size_t getMemberIndex(std::string member);
};

struct UnionTypeInfo : public HetrogenTypeInfo
{
    UnionTypeInfo(Identifier *id, SymbolTable *sc, std::vector<Declaration*> m) :
        HetrogenTypeInfo(id, sc, m) {}
    std::string getName() { return identifier->getName(); }
    virtual size_t length() { return members.size(); }
    virtual size_t getSize();
    virtual size_t getMemberOffset(std::string member) { return 0; }
    virtual size_t getMemberIndex(std::string member) { return 0; }
};

struct NamedUnknownInfo : public TypeInfo
{
    SymbolTable *scope;
    Identifier *identifier;
    NamedUnknownInfo(Identifier *id, SymbolTable *sc) : identifier(id), scope(sc) {}
    virtual std::string getName() { return identifier->getName(); }
};

struct AliasTypeInfo : public TypeInfo
{
    Identifier *identifier;
    ASTType *alias;
    AliasTypeInfo(Identifier *id, ASTType *a) :identifier(id), alias(a) {}
    virtual size_t getSize();
    virtual size_t getAlign();
};

struct TupleTypeInfo : public CompositeTypeInfo
{
    std::vector<ASTType*> types;
    virtual ASTType *getContainedType(unsigned i) { return types[i]; }
    virtual size_t length() { return types.size(); }
    virtual size_t getSize();
    virtual size_t getAlign();
    TupleTypeInfo(std::vector<ASTType*> t) : types(t) {}
};

struct ClassTypeInfo : public HetrogenTypeInfo
{
    ClassTypeInfo *base;

    ClassTypeInfo(Identifier *id, SymbolTable *sc, ClassTypeInfo *b, std::vector<Declaration*> m) :
        HetrogenTypeInfo(id, sc, m), base(b) {}
    virtual size_t length() { return members.size(); }
    //virtual size_t getSize();
    virtual size_t getMemberOffset(std::string member) { return 0; } // TODO
    virtual size_t getMemberIndex(std::string member) { return 0; } // TODO
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

    ASTType(enum ASTTypeEnum ty, TypeInfo *i) : type(ty), pointerTy(0), cgType(0), info(i)
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
            case TYPE_POINTER: return 8;
            case TYPE_FLOAT: return 4;
            case TYPE_DOUBLE: return 8;
            case TYPE_TUPLE:
            case TYPE_ARRAY:
            case TYPE_DYNAMIC_ARRAY:
            case TYPE_STRUCT:
            default: return info->getSize();
        }
    }

    // conversion priority
    unsigned priority() const
    {
        switch(type)
        {
            case TYPE_UNION:
            case TYPE_STRUCT:
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

    bool coercesTo(ASTType *ty) const
    {
        switch(type)
        {
            case TYPE_POINTER:
                return ty->isInteger() || ty == this ||
                    getReferencedTy()->type == TYPE_VOID || ty->getReferencedTy()->type == TYPE_VOID;
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

    size_t length() const
    {
        switch(type)
        {
            case TYPE_ARRAY:
            case TYPE_DYNAMIC_ARRAY:
                return info->length();
            default: return 1;
        }
    }

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
            case TYPE_POINTER: return 8;
            case TYPE_FLOAT: return 4;
            case TYPE_DOUBLE: return 8;
            default: return info->getAlign();
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
            case TYPE_FLOAT: return "float";
            case TYPE_DOUBLE: return "double";
            case TYPE_VOID: return "void";
            default: return info->getName();
        }
    }

    ASTType *getReferencedTy() const { return info->getReferenceTy(); }
    bool isAggregate() { return type == TYPE_STRUCT || type == TYPE_UNION || type == TYPE_CLASS; }
    bool isClass() { return type == TYPE_CLASS; }
    bool isStruct() { return type == TYPE_STRUCT; }
    bool isUnion() { return type == TYPE_UNION; }
    bool isBool() { return type == TYPE_BOOL; }
    bool isInteger() { return type == TYPE_BOOL || type == TYPE_CHAR || type == TYPE_SHORT ||
        type == TYPE_INT || type == TYPE_LONG ||
        type == TYPE_UCHAR || type == TYPE_USHORT || type == TYPE_UINT || type == TYPE_ULONG; }
    bool isSigned() { return type == TYPE_CHAR || type == TYPE_SHORT ||
        type == TYPE_INT || type == TYPE_LONG; }
    bool isFloating() { return type == TYPE_FLOAT || type == TYPE_DOUBLE; }
    bool isNumeric() { return isFloating() || isInteger(); }
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
    static ASTType *getTupleTy(std::vector<ASTType *> t);
};

#endif
