#include "message.hpp"
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace msg;

static int errLvl = 0;

int currentErrorLevel()
{
    return errLvl;
}

void cond_message(int cond, int level, std::string msg, SourceLocation loc)
{
    if(cond) emit_message(level, msg, loc);
}

void assert_message(int assert, int lvl, std::string msg, SourceLocation loc)
{
    if(!assert) emit_message(lvl, msg, loc);
}

void emit_message(int level, std::string msg, SourceLocation loc)
{
    if(level > errLvl) errLvl = level;

    if(loc.filenm)
        cerr << loc.filenm << ":";

    if(loc.line)
        cerr << loc.line << ": ";

    switch(level)
    {
        case OUTPUT:
            break;
        case DEBUGGING:
            cerr << "debug: "; break;
        case WARNING:
            cerr << "warning: "; break;
        case ERROR:
            cerr << "error: "; break;
        case FAILURE:
            cerr << "compiler failure: "; break;
        case UNIMPLEMENTED:
            cerr << "unimplemented feature: "; break;
        case FATAL:
            cerr << "fatal error: "; break;
        default:
            break;
    }

    cerr << msg << endl;

    if(level == FATAL) assert(false);
    if(level == FAILURE) assert(false); //throw a fit, this compiler isnt working
}
