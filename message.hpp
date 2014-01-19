#ifndef _MESSAGE_HPP
#define _MESSAGE_HPP

#include <string>
#include "sourceLocation.hpp"

namespace msg
{
    enum msg_level
    {
        OUTPUT = 0, // simple output. not an issue
        WARNING = 1, // warning message, but not a problem
        ERROR = 2, // problem compiling. cannot finish
        FAILURE = 3, // internal failure; unexpected behaviour on compiler's part
        UNIMPLEMENTED = 4, // unimplemented feature. this shouldnt be happening...
        FATAL = 5, // can not even continue. something really wrong happened
    };
};

#define assert_action(cond, do) if(cond) do;

void cond_message(int cond, int level, std::string msg, SourceLocation loc = SourceLocation());
void assert_message(int assert, int lvl, std::string msg, SourceLocation loc = SourceLocation());
void emit_message(int level, std::string msg, SourceLocation loc = SourceLocation());

#endif
