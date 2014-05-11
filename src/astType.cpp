#include "ast.hpp"

//
// StructTypeInfo
//
size_t StructTypeInfo::getSize()
{
    size_t sz = 0;
    VariableDeclaration *vd;
    unsigned align;
    for(int i = 0; i < members.size(); i++)
    {
        vd = members[i]->variableDeclaration();
        if(!vd) continue;

        align = vd->getType()->getAlign();
        if(!packed && sz % align)
            sz += (align - (sz % align));
        sz += vd->type->getSize();
    }
    return sz;
}

size_t StructTypeInfo::getMemberOffset(std::string member)
{
    size_t offset = 0;
    unsigned align = 0;
    VariableDeclaration *vd;
    for(int i = 0; i < members.size(); i++)
    {
        if(members[i]->identifier->getName() == member)
        {
            return offset;
        }
        vd = members[i]->variableDeclaration();
        if(!vd) continue;
        align = vd->getType()->getAlign();
        if(!packed && offset % align)
            offset += (align - (offset % align));
        offset += vd->getType()->getSize();
    }
}

size_t StructTypeInfo::getMemberOffset(size_t index)
{
    size_t offset = 0;
    unsigned align = 0;
    VariableDeclaration *vd;
    for(int i = 0; i < index; i++)
    {
        vd = members[i]->variableDeclaration();
        if(!vd) continue;
        align = vd->getType()->getAlign();
        if(!packed && offset % align)
            offset += (align - (offset % align));
        offset += vd->getType()->getSize();
    }
    return offset;
}

long StructTypeInfo::getMemberIndex(std::string member)
{
    for(int i = 0; i < members.size(); i++)
    {
        if(members[i]->identifier->getName() == member)
        {
            return i; //TODO: mangling
        }
    }
}

//
// HetrogenTypeInfo
//
Declaration *HetrogenTypeInfo::getMember(size_t i){
    return members[i];
}

Declaration *HetrogenTypeInfo::getMemberByName(std::string member)
{
    for(int i = 0; i < members.size(); i++)
        if(members[i]->identifier->getName() == member)
            return members[i];
}

ASTType *HetrogenTypeInfo::getContainedType(unsigned i)
{
    return members[i]->getType();
}

size_t HetrogenTypeInfo::getAlign()
{
    size_t align = 0;
    VariableDeclaration *vd;
    for(int i = 0; i < members.size(); i++)
    {
        vd = members[i]->variableDeclaration();
        assert(vd && "expected variable decl, found something else");
        if(vd->getType()->getAlign() > align)
            align = vd->getType()->getAlign();
    }
    if(align == 0) return 1; //TODO: empty struct, bad!
    return align;
    //TODO: handle packed
}

//
// UnionTypeInfo
//
size_t UnionTypeInfo::getSize()
{
    size_t sz = 0;
    VariableDeclaration *vd;
    for(int i = 0; i < members.size(); i++)
    {
        vd = members[i]->variableDeclaration();
        if(vd->getType()->getSize() > sz)
            sz = vd->getType()->getSize();
    }
}

//
// ClassTypeInfo
//

// sort members in descending order for most efficient packing
bool classSort(Declaration *decla, Declaration *declb){
    return decla->getType()->getSize() > declb->getType()->getSize();
}

void ClassTypeInfo::sortMembers() {
    std::stable_sort(members.begin(), members.end(), classSort);
}

HetrogenTypeInfo *ClassTypeInfo::baseHetrogenTypeInfo() {
    if(base){
        return dynamic_cast<HetrogenTypeInfo*>(base->getDeclaredType()->info);
    }
}

size_t ClassTypeInfo::getSize() {
    sortMembers();

    size_t sz = base ? base->getDeclaredType()->getSize() : 0;
    VariableDeclaration *vd;
    unsigned align; //TODO: padding past base?
    for(int i = 0; i < members.size(); i++)
    {
        vd = members[i]->variableDeclaration();
        if(!vd) continue;

        align = vd->getType()->getAlign();
        if(sz % align)
            sz += (align - (sz % align));
        sz += vd->type->getSize();
    }
    return sz;
}

size_t ClassTypeInfo::length(){
    size_t count = members.size();
    if(HetrogenTypeInfo *hti = baseHetrogenTypeInfo())
    {
        count += hti->length();
    }
    return count;
}

Declaration *ClassTypeInfo::getMember(size_t index) {
    size_t baselen = 0;
    if(HetrogenTypeInfo *hti = baseHetrogenTypeInfo()){
        baselen = hti->length();
        if(index < baselen) return hti->getMember(index);
    }
    index -= baselen;
    if(index > members.size()) return NULL;
    return members[index];
}

size_t ClassTypeInfo::getMemberOffset(size_t index)
{
    size_t offset = 0;
    unsigned align = 0;
    VariableDeclaration *vd;
    for(int i = 0; i < index; i++)
    {
        vd = members[i]->variableDeclaration();
        if(!vd) continue;
        align = vd->getType()->getAlign();
        if(offset % align)
            offset += (align - (offset % align));
        offset += vd->getType()->getSize();
    }
    return offset;
}

size_t ClassTypeInfo::getMemberOffset(std::string member){
    if(member == "vtable") return 0;
    if(member == "refs") return 8;

    sortMembers();

    size_t offset = 16;
    unsigned align = 0;

    VariableDeclaration *vd;
    for(int i = 0; i < members.size(); i++)
    {
        if(members[i]->identifier->getName() == member)
        {
            return offset;
        }
        vd = members[i]->variableDeclaration();
        if(!vd) continue;
        align = vd->getType()->getAlign();
        if(offset % align)
            offset += (align - (offset % align));
        offset += vd->getType()->getSize();
    }
}

long ClassTypeInfo::getMemberIndex(std::string member) {
    //if(member == "vtable") return 0;
    //if(member == "refs") return 1;

    sortMembers();

    long index;
    if(base && (index = baseHetrogenTypeInfo()->getMemberIndex(member)) >= 0 ){
        return index;
    }

    for(int i = 0; i < members.size(); i++)
    {
        if(members[i]->identifier->getName() == member)
        {
            return i + (base ? baseHetrogenTypeInfo()->length() : 0);
        }
    }

    return -1;
}

Declaration *ClassTypeInfo::getMemberByName(std::string member){
    Declaration *decl = NULL;
    if(base && (decl = baseHetrogenTypeInfo()->getMemberByName(member))){
        return decl;
    }
    for(int i = 0; i < members.size(); i++)
    {
        if(members[i]->identifier->getName() == member) {
            return members[i];
        }
    }

    return NULL;
}

//
// PointerASTType
//
std::string ASTPointerType::getName(){
    return ptrTo->getName() + "^";
}

//
// StaticArrayTypeInfo
//
size_t ASTStaticArrayType::getAlign() const {
    return arrayOf->getAlign();
}

size_t ASTStaticArrayType::getSize() const {
    return size * arrayOf->getSize();
}

//
// DynamicArrayTypeInfo
//
size_t ASTDynamicArrayType::getAlign() const {
    return ASTType::getCharTy()->getPointerTy()->getAlign();
}

size_t ASTDynamicArrayType::getSize() const {
    return ASTType::getCharTy()->getPointerTy()->getSize() + ASTType::getULongTy()->getSize();
}

//
// ASTTupleType
//
size_t ASTTupleType::getSize() const
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

size_t ASTTupleType::getAlign() const
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

#define DECLTY(TY, NM) ASTType *ASTType::get##NM() { \
    static int id = -1; \
    ASTType *ty; \
    if(id < 0) { \
        ty = new ASTType(TY); \
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
    return new ASTType(TYPE_FUNCTION, new FunctionTypeInfo(ret, param, vararg));
}

void ASTType::accept(ASTVisitor *v) {
    //v->visitType(this);
    //TODO subtypes, etc
}
