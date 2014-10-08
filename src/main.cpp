
#include "ast.hpp"
#include "parser.hpp"
#include "streamLexer.hpp"
#include "irCodegenContext.hpp"
#include "message.hpp"
#include "config.hpp"

#ifdef WIN32
#include "win_getopt.h"
#include <Windows.h>
#undef ERROR // conflicts with msg::ERROR
#pragma import clang.lib

#else
#include <unistd.h>
#include <dirent.h>
#include <ftw.h>
#endif

#include <sys/stat.h>

#include <vector>
#include <string.h>

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 5
#include <llvm/Linker/Linker.h>
#endif

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 4
#include <llvm/Linker.h>
#endif

#ifdef WIN32
static void deleteDir(std::string dirnm) {
	//TODO
}

static std::string createTempDir() {
	TCHAR pathnm[64];
	GetTempPath(64, pathnm);
	CreateDirectory(pathnm, NULL);
	return std::string(pathnm);
}
#else
static int remove_ftw(const char *path, const struct stat *st, int, struct FTW*) {
	int rv = remove(path);

	if (rv)
		perror(path);

	return rv;
}

static void deleteDir(std::string dirnm) {
	nftw(dirnm.c_str(), remove_ftw, 8, FTW_PHYS | FTW_DEPTH);
}

static std::string createTempDir() {
	char ftemplate[32] = "/tmp/wlcXXXXXX\0";
	return std::string(mkdtemp(ftemplate));
}
#endif

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

        if(!ast->validate()){
            emit_message(msg::ERROR, "invalid AST");
        }

        IRCodegenContext cg;
        std::string outputo = cg.codegenAST(ast, params);

        // TODO: check if codegen succeeded before attempting to link
        if(params.link && currentErrorLevel() < msg::ERROR)
            link(params, outputo);
    } else
    {
        emit_message(msg::FATAL, "no input files");
    }
}

WLConfig parseCmd(int argc, char **argv)
{
    WLConfig params;
    params.cmd = std::string(argv[0]);
    params.files = std::vector<std::string>();
	params.tempName = createTempDir();
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
                params.output = std::string(optarg);
                break;
            case '?':
                if(optopt == 'l' || optopt == 'L' || optopt == 'I')
                {
                    emit_message(msg::FATAL, std::string("missing argument to '-") +
                            std::string((char*) &optopt, 1) + std::string("'"));
                    break;
                }
            default:
                emit_message(msg::FATAL, std::string("unrecognized command line option '-") +
                        std::string(optarg) + std::string("'"));
                break;
        }
    }
    return params;
}

void deinit(WLConfig &config)
{
	deleteDir(config.tempName);
}

int main(int argc, char **argv)
{
    WLConfig param = parseCmd(argc, argv);
    if(currentErrorLevel()) return -1; // failure to parse args
    compile(param);
    deinit(param);
    return 0;
}
