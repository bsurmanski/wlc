#include "ast.hpp"

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

/*
ASTType *ASTType::getArrayTy(int len)
{
    return NULL;
}
*/

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
