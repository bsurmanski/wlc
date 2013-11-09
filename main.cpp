
using namespace std;

#include "ast.hpp"
#include "parser.hpp"
#include "streamLexer.hpp"
#include "irCodegenContext.hpp"

int main(int argc, char **argv)
{
    Parser parser;
    parser.parseFile("test.wl");
    AST *ast = parser.getAST();
    IRCodegen(ast);
    return 0;
}
