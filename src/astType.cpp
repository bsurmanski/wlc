#include "ast.hpp"

size_t StructTypeInfo::getSize()
{
    size_t sz = 0;
    VariableDeclaration *vd;
    unsigned align;
    for(int i = 0; i < members.size(); i++)
    {
        vd = members[i]->variableDeclaration();
        if(!vd) continue;

        align = vd->getType()->align();
        if(!packed && sz % align)
            sz += (align - (sz % align));
        sz += vd->type->size();
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
        align = vd->getType()->align();
        if(!packed && offset % align)
            offset += (align - (offset % align));
        offset += vd->getType()->size();
    }
}

size_t StructTypeInfo::getMemberIndex(std::string member)
{
    for(int i = 0; i < members.size(); i++)
    {
        if(members[i]->identifier->getName() == member)
        {
            return i; //TODO: mangling
        }
    }
}

Declaration *HetrogenTypeInfo::getMemberDeclaration(std::string member)
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
        if(vd->getType()->align() > align)
            align = vd->getType()->align();
    }
    if(align == 0) return 1; //TODO: empty struct, bad!
    return align;
    //TODO: handle packed
}

size_t UnionTypeInfo::getSize()
{
    size_t sz = 0;
    VariableDeclaration *vd;
    for(int i = 0; i < members.size(); i++)
    {
        vd = members[i]->variableDeclaration();
        if(vd->getType()->size() > sz)
            sz = vd->getType()->size();
    }
}

std::string PointerTypeInfo::getName(){
    return ptrTo->getName() + "^";
}

std::string ArrayTypeInfo::getName(){
    return "array[" + arrayOf->getName() + "]";
}

size_t StaticArrayTypeInfo::getAlign(){
    return arrayOf->align();
}

size_t DynamicArrayTypeInfo::getAlign(){
    return ASTType::getCharTy()->getPointerTy()->align();
}

ASTType *ASTType::getPointerTy()
{
    if(!pointerTy)
    {
        ASTType *pty = new ASTType(TYPE_POINTER);
        pty->setTypeInfo(new PointerTypeInfo(this));
        pointerTy = pty;
    }
    return pointerTy;
}

ASTType *ASTType::getArrayTy()
{
    if(!dynamicArrayTy)
    {
        ASTType *aty = new ASTType(TYPE_DYNAMIC_ARRAY);
        aty->setTypeInfo(new DynamicArrayTypeInfo(this));
        dynamicArrayTy = aty;
    }
    return dynamicArrayTy;
}

ASTType *ASTType::getArrayTy(int sz)
{
    ASTType *aty = 0;
    if(!arrayTy.count(sz))
    {
        aty = new ASTType(TYPE_ARRAY);
        aty->setTypeInfo(new StaticArrayTypeInfo(this, sz));
        arrayTy[sz] = aty;
    } else aty = arrayTy[sz];
    return aty;
}

size_t DynamicArrayTypeInfo::getSize() {
    return ASTType::getCharTy()->getPointerTy()->size() + ASTType::getULongTy()->size();
}

size_t StaticArrayTypeInfo::getSize() {
    return size * arrayOf->size();
}

size_t AliasTypeInfo::getSize()
{
    return alias->size();
}

size_t AliasTypeInfo::getAlign()
{
    return alias->align();
}

size_t TupleTypeInfo::getSize()
{
    size_t sz = 0;
    unsigned align;
    for(int i = 0; i < types.size(); i++)
    {
        align = types[i]->align();
        if(sz % align)
            sz += (align - (sz % align));
        sz += types[i]->size();
    }
    return sz;
}

size_t TupleTypeInfo::getAlign()
{
    size_t max = 0;
    for(int i = 0; i < types.size(); i++)
    {
        if(types[i]->align() > max)
            max = types[i]->align();
    }
    return max;
}

// declare all of the static ASTType instances, and their getter methods
#define DECLTY(TY,NM) ASTType *ASTType::NM = 0; ASTType *ASTType::get##NM() { if(!NM) NM = new ASTType(TY); return NM; }
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
#undef DECLTY

ASTType *ASTType::DynamicTy = 0;

ASTType *ASTType::getDynamicTy()
{
    if(!DynamicTy) DynamicTy = new ASTType(TYPE_DYNAMIC);
    return DynamicTy;
}


ASTType *ASTType::getTupleTy(std::vector<ASTType *> t)
{
    // TODO: type cache
    ASTType *ty = new ASTType();
    ty->setTypeInfo(new TupleTypeInfo(t), TYPE_TUPLE);
    return ty;
}

ASTType *ASTType::getFunctionTy(ASTType *ret, std::vector<ASTType *> param, bool vararg)
{
    //TODO: type cache?
    return new ASTType(TYPE_FUNCTION, new FunctionTypeInfo(ret, param, vararg));
}
