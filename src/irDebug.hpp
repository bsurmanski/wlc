#ifndef _IRDEBUG_HPP
#define _IRDEBUG_HPP

#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/DIBuilder.h>
#include <llvm/DebugInfo.h>

#include "irCodegenContext.hpp"
#include "ast.hpp"

#define CGSTR "wlc 0.11 - Jan 2014 (GGJ)"
#define PROJDIR "/home/brandon/PROJECTS/C/term/wlc"

class IRCodegenContext;
class IRTranslationUnit;
class IRDebug
{
    public:
        IRCodegenContext *context;
        IRTranslationUnit *unit;
        llvm::DIBuilder di;
        llvm::DIFile diFile;
        llvm::DICompileUnit diUnit;

        IRDebug(IRCodegenContext *c, IRTranslationUnit *u);

        ~IRDebug()
        {
            di.finalize();
        }

        llvm::Module *getModule();

        llvm::DICompileUnit getCompileUnit() { return diUnit; }
        llvm::DIFile currentFile() { return diFile; }
        llvm::DIDescriptor currentScope();
        llvm::DIDescriptor createScope(llvm::DIDescriptor parent, SourceLocation loc);
        llvm::DICompositeType createStructType(ASTType *ty);
        llvm::DICompositeType createUnionType(ASTType *ty);
        llvm::DICompositeType createDynamicArrayType(ASTType *ty);
        llvm::DICompositeType createArrayType(ASTType *ty);
        llvm::DICompositeType createTupleType(ASTType *ty);
        llvm::DIType createType(ASTType *t);
        llvm::DICompositeType createPrototype(FunctionPrototype *p);
        llvm::DISubprogram createFunction(FunctionDeclaration *f);
        llvm::DIGlobalVariable createGlobal(VariableDeclaration *decl, ASTValue *val);
        llvm::Instruction *createVariable(std::string nm, ASTValue *v,
                llvm::BasicBlock *bb, SourceLocation loc, int argn = 0);
};

void createIdentMetadata(llvm::Module *m);

#endif
