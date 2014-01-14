
using namespace std;

#include "ast.hpp"
#include "parser.hpp"
#include "streamLexer.hpp"
#include "irCodegenContext.hpp"
#include <vector>
#include <llvm/Linker.h>

struct WLParams
{
    string cmd;
    vector<string> files;
};

void compile(WLParams params)
{
    if(params.files.size())
    {
        Parser parser;
        AST *ast = parser.getAST();
        for(int i = 0; i < params.files.size(); i++)
        {
            if(!ast->getUnit(params.files[i])) // check if file already parsed
            {
                TranslationUnit *unit = new TranslationUnit(NULL, params.files[i]); //TODO
                unit->expl = true;
                ast->addUnit(params.files[i], unit);
                parser.parseFile(unit, params.files[i]);
            } else
            {
                TranslationUnit *u = ast->getUnit(params.files[i]);
                u->expl = true;
            }
        }
        IRCodegenContext cg;
        cg.codegenAST(ast);
    } else
    {
        printf("fatal error: no input files\n");
    }
}

WLParams parseCmd(int argc, char **argv)
{
    WLParams params;
    params.cmd = string(argv[0]);
    params.files = vector<string>();

    for(int i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            printf("option ignored: %s\n", argv[i]);
            continue; //TODO: parse option
        }
        params.files.push_back(argv[i]);
    }
    return params;
}

int main(int argc, char **argv)
{
    WLParams param = parseCmd(argc, argv);
    compile(param);
    return 0;
}
