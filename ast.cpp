#include "ast.hpp"

ASTPointerType *ASTType::getPointerTy() 
{ 
    if(!pointerTy) pointerTy = new ASTPointerType(this); 
    return pointerTy; 
} 

ASTArrayType *getArrayTy(int len)
{
    return NULL;
}

//static
ASTType *ASTType::voidTy = 0;
ASTType *ASTType::intTy = 0;
ASTType *ASTType::boolTy = 0;
ASTType *ASTType::charTy = 0;
ASTType *ASTType::longTy = 0;
ASTType *ASTType::getVoidTy() { if(!voidTy) voidTy = new ASTType(TYPE_VOID); return voidTy; }
ASTType *ASTType::getIntTy() { if(!intTy) intTy = new ASTType(TYPE_INT); return intTy; }
ASTType *ASTType::getCharTy() { if(!charTy) charTy = new ASTType(TYPE_CHAR); return charTy; }
ASTType *ASTType::getLongTy() { if(!longTy) longTy = new ASTType(TYPE_LONG); return longTy; }
ASTType *ASTType::getBoolTy() { if(!boolTy) boolTy = new ASTType(TYPE_BOOL); return boolTy; }
