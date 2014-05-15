#include "ast.hpp"
#include "message.hpp"

//
// UserType
//

ASTType *ASTUserType::getTypeResolution() {
    if(getDeclaration())
        return getDeclaration()->getDeclaredType();
    return NULL;
}

bool ASTUserType::isOpaque() {
    return !getDeclaration()->members.size();
}

Declaration *ASTUserType::getMember(size_t i) {
    return getDeclaration()->members[i];
}

size_t ASTUserType::length() {
    UserTypeDeclaration *utd = getDeclaration();
    return utd->members.size();
}

size_t ASTUserType::getSize() {
    UserTypeDeclaration *utd = getDeclaration();
    return utd->getSize();
}
size_t ASTUserType::getAlign() {
    UserTypeDeclaration *utd = getDeclaration();
    return utd->getAlign();
}

ASTType *ASTUserType::getMemberType(size_t i) {
    return getMember(i)->getType();
}

Declaration *ASTUserType::getMemberByName(std::string name) {
    UserTypeDeclaration *utd = getDeclaration();
    for(int i = 0; i < utd->members.size(); i++){
        if(utd->members[i]->getName() == name)
            return utd->members[i];
    }
    return NULL;
}

long ASTUserType::getMemberIndex(std::string member){
    UserTypeDeclaration *utd = getDeclaration();

    if(dynamic_cast<StructDeclaration*>(utd)) {
        for(int i = 0; i < utd->members.size(); i++){
            if(utd->members[i]->identifier->getName() == member){
                return i;
            }
        }
    } else if(dynamic_cast<UnionDeclaration*>(utd)) {
        return 0;
    } else if(dynamic_cast<ClassDeclaration*>(utd)) {
        long index;
        emit_message(msg::ERROR, "feature currently broke: class index");
    }
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

ASTType *ASTType::getFunctionTy(ASTType *ret, std::vector<ASTType *> param, bool vararg)
{
    //TODO: type cache?
    return new ASTFunctionType(ret, param, vararg);
}

void ASTType::accept(ASTVisitor *v) {
    //v->visitType(this);
    //TODO subtypes, etc
}

bool ASTUserType::isClass() { return dynamic_cast<ClassDeclaration*>(identifier->getDeclaration()); }
bool ASTUserType::isStruct() { return dynamic_cast<StructDeclaration*>(identifier->getDeclaration()); }
bool ASTUserType::isUnion() { return dynamic_cast<UnionDeclaration*>(identifier->getDeclaration()); }
