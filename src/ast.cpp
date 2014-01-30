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
}

size_t StructTypeInfo::getAlign()
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
    return align;
}

std::string PointerTypeInfo::getName(){
    return ptrTo->getName() + "^";
}

std::string ArrayTypeInfo::getName(){
    return "array[" + arrayOf->getName() + "]";
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
    if(!arrayTy)
    {
        ASTType *pty = new ASTType(TYPE_ARRAY);
        pty->setTypeInfo(new ArrayTypeInfo(this));
        arrayTy = pty;
    }
    return arrayTy;
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
