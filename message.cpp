#include "message.hpp"
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace msg;

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
    switch(level)
    {
        case OUTPUT:
            break;
        case WARNING:
            cout << "warning: "; break;
        case ERROR:
            cout << "error: "; break;
        case FAILURE:
            cout << "compiler failure: "; break;
        case UNIMPLEMENTED:
            cout << "unimplemented feature: "; break;
        case FATAL:
            cout << "fatal error: "; break;
        default:
            break;
    }

    if(!loc.isUnknown())
        cout << loc.line << ":";
    cout << msg << endl;

    if(level == FATAL) exit(-1);
    if(level == FAILURE) assert(false); //throw a fit, this compiler isnt working
}
