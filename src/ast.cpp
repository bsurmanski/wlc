#include "ast.hpp"

size_t StructTypeInfo::getSize()
{
    size_t sz = 0;
    VariableDeclaration *vd;
    unsigned align;
    for(int i = 0; i < members.size(); i++)
    {
        vd = members[i]->variableDeclaration();
        assert(vd && "expected variable decl, found something else");

        align = vd->getType()->align();
        if(sz % align)
            sz += (align - (sz % align));
        sz += vd->type->size();
    }
    return sz;
    //TODO: handle packed
}

ASTType *StructUnionInfo::getContainedType(unsigned i)
{
    return members[i]->getType();
}

size_t StructUnionInfo::getAlign()
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

size_t ArrayTypeInfo::getAlign(){
    return arrayOf->align();
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
        aty->setTypeInfo(new ArrayTypeInfo(this));
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
        aty->setTypeInfo(new ArrayTypeInfo(this, sz));
        arrayTy[sz] = aty;
    } else aty = arrayTy[sz];
    return aty;
}

size_t ArrayTypeInfo::getSize() {
    if(isDynamic())
        return ASTType::getCharTy()->getPointerTy()->size() + ASTType::getULongTy()->size();
    else
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
