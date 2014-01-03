
using namespace std;

#include "ast.hpp"
#include "parser.hpp"
#include "streamLexer.hpp"
#include "irCodegenContext.hpp"
#include <llvm/Linker.h>

int main(int argc, char **argv)
{
    Parser parser;
    AST *ast = parser.getAST();
    TranslationUnit *unit = new TranslationUnit(NULL, "test.wl"); //TODO
    ast->addUnit("test.wl", unit);
    parser.parseFile(unit, "test.wl");
    IRCodegenContext cg;
    cg.codegenAST(ast);
    //llvm::Linker linker(cg.module);
    return 0;
}
