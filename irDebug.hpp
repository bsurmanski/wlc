#ifndef _IRDEBUG_HPP
#define _IRDEBUG_HPP

#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/DIBuilder.h>
#include <llvm/DebugInfo.h>

#include "irCodegenContext.hpp"
#include "ast.hpp"

#define CGSTR "wlc 0.1 - Jan 2014"

class IRCodegenContext;
class IRDebug
{
    public:
        IRCodegenContext *context;
        TranslationUnit *unit;
        llvm::DIBuilder di;
        llvm::DIFile diFile;

        IRDebug(IRCodegenContext *c, TranslationUnit *u) : context(c), unit(u), 
             di(*(llvm::Module*)unit->cgValue) 
        {
            di.createCompileUnit(0, u->filenm, "DIR", CGSTR, false, "", 0); 
            diFile = di.createFile(u->filenm, "DIR");
        }

        ~IRDebug()
        {
            di.finalize();
        }

        llvm::Module *getModule() { return (llvm::Module*) unit->cgValue; }

        llvm::DIFile currentFile() { return diFile; }
        llvm::DIDescriptor currentScope();
        llvm::DICompositeType createStructType(ASTType *ty);
        llvm::DIType createType(ASTType *t);
        llvm::DICompositeType createPrototype(FunctionPrototype *p);
        llvm::DISubprogram createFunction(FunctionDeclaration *f);
};

#endif
