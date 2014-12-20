#ifndef _IRDEBUG_HPP
#define _IRDEBUG_HPP

#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 5
#include <llvm/IR/DIBuilder.h>
#endif

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 4
#include <llvm/DIBuilder.h>
#include <llvm/DebugInfo.h>
#endif

#include "irCodegenContext.hpp"
#include "ast.hpp"

#include <string>
#include <map>

#define CGSTR "wlc 0.22 - Dec 2014"
#define PROJDIR "/home/brandon/PROJECTS/C/term/wlc"

class IRCodegenContext;
struct IRTranslationUnit;
class IRDebug
{
    public:
        IRCodegenContext *context;
        IRTranslationUnit *unit;
        llvm::DIBuilder di;
        llvm::DIFile diFile;
        llvm::DICompileUnit diUnit;

        // map for complex types
        // index is mangled name
        std::map<std::string, llvm::DIType> typeMap;

        IRDebug(IRCodegenContext *c, IRTranslationUnit *u);

        ~IRDebug()
        {
            finalize();
        }

        void finalize()
        {
            di.finalize();
        }

        llvm::Module *getModule();

        llvm::DICompileUnit getCompileUnit() { return diUnit; }
        llvm::DIFile currentFile() { return diFile; }
        llvm::DIDescriptor currentScope();
        llvm::DIDescriptor createScope(llvm::DIDescriptor parent, SourceLocation loc);
        llvm::DIType createClassType(ASTType *ty);
        llvm::DICompositeType createUserType(ASTType *ty);
        llvm::DICompositeType createStructType(ASTType *ty);
        llvm::DICompositeType createUnionType(ASTType *ty);
        llvm::DICompositeType createDynamicArrayType(ASTType *ty);
        llvm::DICompositeType createArrayType(ASTType *ty);
        llvm::DICompositeType createTupleType(ASTType *ty);
        llvm::DIType createType(ASTType *t);
        llvm::DICompositeType createPrototype(ASTType *p);
        llvm::DISubprogram createFunction(FunctionDeclaration *f, llvm::Function *cgFunc);
        llvm::DIGlobalVariable createGlobal(VariableDeclaration *decl, ASTValue *val);
        llvm::Instruction *createVariable(std::string nm, ASTValue *v,
                llvm::BasicBlock *bb, SourceLocation loc, int argn = 0);
};

void createIdentMetadata(llvm::Module *m);

#endif
