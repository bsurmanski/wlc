#include "ast.hpp"
#include "message.hpp"
#include "astVisitor.hpp"

//
// UserType
//

ASTType *ASTUserType::getBaseType(){
    Identifier *base = getBaseIdentifier();
    if(base) {
        return base->getDeclaredType();
    }
    return NULL;
}

Identifier *ASTUserType::getBaseIdentifier() {
    ClassDeclaration *cldecl = getDeclaration()->classDeclaration();
    if(cldecl) {
        return cldecl->base;
    }
    return NULL;
}

ASTScope *ASTUserType::getScope() {
    return getDeclaration()->getScope();
}

bool ASTUserType::isOpaque() {
    return !getDeclaration()->length() && !getBaseType(); // XXX should be if type does not have body
}

Declaration *ASTUserType::getMember(size_t i) {
    return getDeclaration()->members[i];
}

Declaration *ASTUserType::getMember(std::string member) {
    long mi = getMemberIndex(member);
    if(mi < 0) return NULL;
    return getMember(mi);
}

size_t ASTUserType::length() {
    return getDeclaration()->length();
}

size_t ASTUserType::getSize() {
    return getDeclaration()->getSize();
}
size_t ASTUserType::getAlign() {
    return getDeclaration()->getAlign();
}

ASTType *ASTUserType::getMemberType(size_t i) {
    Declaration *memb = getMember(i);
    if(memb) return memb->getType();
    return NULL;
}

long ASTUserType::getMemberIndex(std::string member){
    UserTypeDeclaration *utd = getDeclaration();
    if(!utd) return -1;
    return utd->getMemberIndex(member);
}

long ASTUserType::getVTableIndex(std::string method){
    ClassDeclaration *cldecl = getDeclaration()->classDeclaration();
    for(int i = 0; i < cldecl->vtable.size(); i++) {
        if(cldecl->vtable[i]->getName() == method) {
            return i;
        }
    }
    return 0; //TODO
}


long ASTUserType::getMemberOffset(size_t i) {
    return 0; //TODO
}

//
// PointerASTType
//
std::string ASTPointerType::getName(){
    return ptrTo->getName() + "^";
}

std::string ASTPointerType::getMangledName()
{
    return "p" + ptrTo->getMangledName();
}

//
// StaticArrayType
//
size_t ASTStaticArrayType::length() {
    return size->asInteger();
}

size_t ASTStaticArrayType::getAlign() {
    return arrayOf->getAlign();
}

size_t ASTStaticArrayType::getSize() {
    return size->asInteger() * arrayOf->getSize();
}

//
// DynamicArrayType
//
size_t ASTDynamicArrayType::getAlign() {
    return ASTType::getCharTy()->getPointerTy()->getAlign();
}

size_t ASTDynamicArrayType::getSize() {
    return ASTType::getCharTy()->getPointerTy()->getSize() + ASTType::getULongTy()->getSize();
}

//
// ASTTupleType
//
size_t ASTTupleType::getSize() {
    size_t sz = 0;
    unsigned align;
    for(int i = 0; i < types.size(); i++)
    {
        align = types[i]->getAlign();
        if(sz % align)
            sz += (align - (sz % align));
        sz += types[i]->getSize();
    }
    return sz;
}

size_t ASTTupleType::getAlign() {
    size_t max = 0;
    for(int i = 0; i < types.size(); i++)
    {
        if(types[i]->getAlign() > max)
            max = types[i]->getAlign();
    }
    return max;
}

bool ASTTupleType::coercesTo(ASTType *ty) {
    if(ASTTupleType *otup = ty->asTuple()) {
        if(length() != otup->length()) return false;
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
    if(ASTStaticArrayType *sarr = ty->asSArray()) {
        if(length() != sarr->length()) return false;
        for(int i = 0; i < length(); i++) {
            // TODO: allow tuple coercion if one tuple contains
            // base pointer to other class?
            // eg [int, SomeClass] -> [int, BaseClass]
            if(!getMemberType(i)->is(sarr->getMemberType(i))) {
                return false;
            }
        }
        return true;
    }
    return false;
}

//
// ASTType
//

ASTTypeEnum ASTType::getKind() const {
    return kind;
}

bool ASTType::isResolved() {
    return true;
}

ASTType *ASTType::getUnqual() {
    return this;
}

UserTypeDeclaration *ASTType::getDeclaration() const {
    return NULL;
}

size_t ASTType::getSize() {
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

unsigned ASTType::getPriority() {
    switch(kind) {
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
        default:
            assert(false && "unknown type");
    }
    return 0;
}

bool ASTType::coercesTo(ASTType *ty) {
    switch(kind) {
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
        default: break;
    }
    return false;
}

//TODO
bool ASTType::castsTo(ASTType *ty) const {
    return true;
}

size_t ASTType::length() {
    return 1;
}

size_t ASTType::getAlign() {

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

std::string ASTType::getName()
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

    std::string ASTType::getMangledName() {
        switch(kind){
            case TYPE_CHAR: return "s08";
            case TYPE_UCHAR: return "u08";
            case TYPE_BOOL: return "b08";
            case TYPE_SHORT: return "s16";
            case TYPE_USHORT: return "u16";
            case TYPE_INT: return "s32";
            case TYPE_UINT: return "u32";
            case TYPE_LONG: return "s64";
            case TYPE_ULONG: return "u64";
            case TYPE_FLOAT: return "f32";
            case TYPE_DOUBLE: return "f64";
            case TYPE_VOID: return "v00";
            default: assert(false && "unimplemented getname");
        }
    }

ASTType *ASTType::getPointerTy() {
    if(!pointerTy)
    {
        pointerTy = new ASTPointerType(this);
    }
    return pointerTy;
}

ASTType *ASTType::getReferenceTy() {
    if(isReference()) {
        return this;
    } else {
        return getPointerTy();
    }
}

ASTType *ASTType::getArrayTy() {
    if(!dynamicArrayTy)
    {
        dynamicArrayTy = new ASTDynamicArrayType(this);
    }
    return dynamicArrayTy;
}

ASTType *ASTType::getArrayTy(unsigned sz) {
    return getArrayTy(new IntExpression(ASTType::getIntTy(), sz));
}

ASTType *ASTType::getArrayTy(Expression *sz) {
    ASTType *aty = 0;

    if(sz->intExpression()) {
        int isz = sz->intExpression()->asInteger();
        if(arrayTy.count(isz)) {
            aty = arrayTy[isz];
        } else {
            aty = arrayTy[isz] = new ASTStaticArrayType(this, sz);
        }
    }

    if(!aty) {
        aty = new ASTStaticArrayType(this, sz);
    }

    return aty;
}

ASTType *ASTType::getConstTy() {
    if(isConst()) return this;

    if(!constTy) {
        constTy = new ASTConstDecorator(this);
    }

    return constTy;
}

// declare all of the static ASTType instances, and their getter methods
#if 0
#define DECLTY(TY,NM) ASTType *ASTType::NM = 0; ASTType *ASTType::get##NM() { \
    if(!NM) NM = new ASTType(TY); return NM; \
}
#endif

std::vector<ASTType *> ASTType::typeCache;

#define DECLTY(TYENUM, NM) ASTType *ASTType::get##NM() { \
    static int id = -1; \
    ASTType *ty; \
    if(id < 0) { \
        ty = new ASTBasicType(TYENUM); \
        id = typeCache.size(); \
        typeCache.push_back(ty); \
    } \
    return ASTType::typeCache[id]; \
}

    DECLTY(TYPE_VOID, VoidTy)
    DECLTY(TYPE_BOOL, BoolTy)

    DECLTY(TYPE_CHAR, CharTy)
    DECLTY(TYPE_SHORT, ShortTy)
    DECLTY(TYPE_INT, IntTy)
    DECLTY(TYPE_LONG, LongTy)

    DECLTY(TYPE_UCHAR, UCharTy)
    DECLTY(TYPE_USHORT, UShortTy)
    DECLTY(TYPE_UINT, UIntTy)
    DECLTY(TYPE_ULONG, ULongTy)

    DECLTY(TYPE_FLOAT, FloatTy)
    DECLTY(TYPE_DOUBLE, DoubleTy)
    DECLTY(TYPE_DYNAMIC, DynamicTy)
#undef DECLTY

ASTType *ASTType::DynamicTy = 0;

ASTType *ASTType::getTupleTy(std::vector<ASTType *> t)
{
    // TODO: type cache
    return new ASTTupleType(t);
}

ASTFunctionType *ASTType::getFunctionTy(ASTType *ret, std::vector<ASTType *> param, bool vararg)
{
    //TODO: type cache?
    return new ASTFunctionType(ret, param, vararg);
}

ASTType *ASTType::getVoidFunctionTy() {
    return getFunctionTy(getVoidTy(), std::vector<ASTType*>());
}

//TODO: use const char?
ASTType *ASTType::getStringTy(Expression *sz) {
    //ASTType *cchar = ASTType::getCharTy()->getConstTy();
    ASTType *cchar = ASTType::getCharTy();
    return cchar->getArrayTy(sz);
}

//TODO: use const char?
ASTType *ASTType::getStringTy(unsigned sz) {
    //ASTType *cchar = ASTType::getCharTy()->getConstTy();
    ASTType *cchar = ASTType::getCharTy();
    return cchar->getArrayTy(sz);
}

void ASTType::accept(ASTVisitor *v) {
    v->visitType(this);
    //TODO subtypes, etc
}

//TODO: default constructor?
FunctionDeclaration *ASTUserType::getDefaultConstructor() {
    return NULL; //getDeclaration()->getDefaultConstructor();
}

FunctionDeclaration *ASTUserType::getConstructor() {
    return getDeclaration()->constructor;
}

FunctionDeclaration *ASTUserType::getDestructor() {
    return getDeclaration()->destructor;
}

ASTUserType *ASTUserType::asClass() {
       	if(dynamic_cast<ClassDeclaration*>(identifier->getDeclaration())) return this;
	return NULL;
}
ASTUserType *ASTUserType::asInterface() {
       	if(dynamic_cast<InterfaceDeclaration*>(identifier->getDeclaration())) return this;
	return NULL;
}
ASTUserType *ASTUserType::asStruct() {
       	if(dynamic_cast<StructDeclaration*>(identifier->getDeclaration())) return this;
	return NULL;
}
ASTUserType *ASTUserType::asUnion() {
       	if(dynamic_cast<UnionDeclaration*>(identifier->getDeclaration())) return this;
	return NULL;
}

bool ASTUserType::is(ASTType *t) {
    if(t->isUserType()) {
        return getDeclaration() == t->getDeclaration();
    }
    return false;
}

bool ASTUserType::extends(ASTType *t) {
    ClassDeclaration *cldecl;
    if((cldecl = getDeclaration()->classDeclaration()) && cldecl->base) {
        return cldecl->base->getDeclaration() == t->getDeclaration() ||
            cldecl->base->getDeclaredType()->extends(t);
    }
    return false;
}

bool ASTUserType::isReference() {
    return dynamic_cast<ClassDeclaration*>(identifier->getDeclaration());
}

bool ASTStaticArrayType::coercesTo(ASTType *ty) {
    if(ASTStaticArrayType *saty = dynamic_cast<ASTStaticArrayType*>(ty)) {
        return arrayOf->is(saty->arrayOf);
    }

    if(ASTDynamicArrayType *daty = dynamic_cast<ASTDynamicArrayType*>(ty)) {
        return arrayOf->is(daty->arrayOf);
    }

    if(arrayOf->getPointerTy()->is(ty)) {
        return true;
    }

    return false;
}
