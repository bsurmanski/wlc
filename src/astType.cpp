#include "ast.hpp"
#include "message.hpp"

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
    return getMember(i)->getType();
}

long ASTUserType::getMemberIndex(std::string member){
    UserTypeDeclaration *utd = getDeclaration();
    return getDeclaration()->getMemberIndex(member);
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
size_t ASTStaticArrayType::getAlign() {
    return arrayOf->getAlign();
}

size_t ASTStaticArrayType::getSize() {
    return size * arrayOf->getSize();
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
size_t ASTTupleType::getSize()
{
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

size_t ASTTupleType::getAlign()
{
    size_t max = 0;
    for(int i = 0; i < types.size(); i++)
    {
        if(types[i]->getAlign() > max)
            max = types[i]->getAlign();
    }
    return max;
}

//
// ASTType
//
ASTType *ASTType::getPointerTy()
{
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

ASTType *ASTType::getArrayTy()
{
    if(!dynamicArrayTy)
    {
        dynamicArrayTy = new ASTDynamicArrayType(this);
    }
    return dynamicArrayTy;
}

ASTType *ASTType::getArrayTy(int sz)
{
    ASTType *aty = 0;
    if(!arrayTy.count(sz))
    {
        aty = arrayTy[sz] = new ASTStaticArrayType(this, sz);
    } else aty = arrayTy[sz];
    return aty;
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

void ASTType::accept(ASTVisitor *v) {
    //v->visitType(this);
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

ASTUserType *ASTUserType::asClass() { if(dynamic_cast<ClassDeclaration*>(identifier->getDeclaration())) return this; }
ASTUserType *ASTUserType::asInterface() { if(dynamic_cast<InterfaceDeclaration*>(identifier->getDeclaration())) return this; }
ASTUserType *ASTUserType::asStruct() { if(dynamic_cast<StructDeclaration*>(identifier->getDeclaration())) return this; }
ASTUserType *ASTUserType::asUnion() { if(dynamic_cast<UnionDeclaration*>(identifier->getDeclaration())) return this; }

bool ASTUserType::is(ASTType *t) {
    if(ASTUserType *uty = t->asUserType()) {
        return getDeclaration() == t->getDeclaration();
    }
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
