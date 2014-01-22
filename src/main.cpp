
using namespace std;

#include "ast.hpp"
#include "parser.hpp"
#include "streamLexer.hpp"
#include "irCodegenContext.hpp"
#include "message.hpp"
#include "config.hpp"
#include <unistd.h>
#include <vector>
#include <llvm/Linker.h>


void compile(WLConfig params)
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
        cg.codegenAST(ast, params);
    } else
    {
        emit_message(msg::FATAL, "no input files");
    }
}

WLConfig parseCmd(int argc, char **argv)
{
    WLConfig params;
    params.cmd = string(argv[0]);
    params.files = vector<string>();

    int c;
    while((c = getopt(argc, argv, "-gl:L:I:o:")) != -1)
    {
        switch(c)
        {
            case 1: // other options
                params.files.push_back(optarg);
                break;
            case 'g':
                params.debug = true;
                break;
            case 'l':
                params.lib.push_back(optarg);
                break;
            case 'L':
                params.libdirs.push_back(optarg);
                break;
            case 'I':
                params.incdirs.push_back(optarg);
                break;
            case 'o':
                params.output = string(optarg);
            case '?':
                if(optopt == 'l' || optopt == 'L' || optopt == 'I')
                {
                    emit_message(msg::FATAL, string("missing argument to '-") + 
                            string((char*) &optopt, 1) + string("'"));
                    break;
                }
            default:
                emit_message(msg::FATAL, string("unrecognized command line option '-") + 
                        string(optarg) + string("'"));
                break;
        }
    }
    return params;
}

int main(int argc, char **argv)
{
    WLConfig param = parseCmd(argc, argv);
    if(currentErrorLevel()) return -1; // failure to parse args
    compile(param);
    return 0;
}
