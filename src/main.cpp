
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
#include <string.h>

void link(WLConfig params, std::string outputo)
{
        std::string dynamiclinker = "/lib64/ld-linux-x86-64.so.2";

        std::string libdirstr = "";
        std::string incdirstr = "";
        std::string libstr = "";
        std::string incstr = "";


        for(int i = 0; i < params.libdirs.size(); i++)
        {
            libdirstr += " -L" + params.libdirs[i];
        }

        for(int i = 0; i < params.incdirs.size(); i++)
        {
            incdirstr += " -l" + params.incdirs[i];
        }

        for(int i = 0; i < params.lib.size(); i++)
        {
            libstr += " -l" + params.lib[i];
        }

        for(int i = 0; i < params.inc.size(); i++)
        {
            libstr += " -i" + params.inc[i];
        }

        //std::string linkcmd = "ld " + outputo +
        //    " /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc -lm -dynamic-linker " +
        //    dynamiclinker + libdirstr + incdirstr + libstr + incstr + " -o " + params.output;
        std::string linkcmd = "clang " + outputo + " -lc -lm " + libdirstr + incdirstr + libstr + incstr + " -o " + params.output;
        int err = system(linkcmd.c_str());
        if(err)
        {
            printf("err: %d\n", err);
            emit_message(msg::FATAL, "error linking: " + linkcmd);
        }
}

void compile(WLConfig params)
{
    if(params.files.size())
    {
        Parser parser;
        AST *ast = parser.getAST();

        TranslationUnit *runtime = new TranslationUnit(ast->getRootPackage(),
                "/usr/local/include/wl/runtime.wl");
        ast->setRuntimeUnit(runtime);
        parser.parseFile(runtime, "/usr/local/include/wl/runtime.wl");

        for(int i = 0; i < params.files.size(); i++)
        {
            if(!ast->getUnit(params.files[i])) // check if file already parsed
            {
                TranslationUnit *unit = new TranslationUnit(ast->getRootPackage(), params.files[i]);
                std::string filenm = params.files[i];

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
        std::string outputo = cg.codegenAST(ast, params);

        if(params.link)
            link(params, outputo);
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
    char ftemplate[32] = "/tmp/wlcXXXXXX\0";
    params.tempName = mkdtemp(ftemplate);
    params.output = "a.out"; // default output string

    int c;
    while((c = getopt(argc, argv, "-gcSl:L:I:o:")) != -1)
    {
        switch(c)
        {
            case 1: // other options
                params.files.push_back(optarg);
                break;
            case 'g':
                params.debug = true;
                break;
            case 'c':
                assert(!params.emitllvm && "-c and -S are currently exclusive");
                params.link = false;
                break;
            case 'S':
                //assert(params.link && "-c and -S are currently exclusive");
                params.emitllvm = true;
                params.link = false;
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
                break;
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

void deinit(WLConfig &config)
{
    rmdir(config.tempName.c_str());
}

int main(int argc, char **argv)
{
    WLConfig param = parseCmd(argc, argv);
    if(currentErrorLevel()) return -1; // failure to parse args
    compile(param);
    deinit(param);
    return 0;
}
