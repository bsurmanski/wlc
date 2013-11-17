#include "ast.hpp"

ASTPointerType *ASTType::getPointerTy() 
{ 
    if(!pointerTy) pointerTy = new ASTPointerType(this); 
    return pointerTy; 
} 

//static
ASTType *ASTType::voidTy = 0;
ASTType *ASTType::intTy = 0;
ASTType *ASTType::charTy = 0;
ASTType *ASTType::getVoidTy() { if(!voidTy) voidTy = new ASTType(TYPE_VOID); return voidTy; }
ASTType *ASTType::getIntTy() { if(!intTy) intTy = new ASTType(TYPE_INT); return intTy; }
ASTType *ASTType::getCharTy() { if(!charTy) charTy = new ASTType(TYPE_CHAR); return charTy; }
