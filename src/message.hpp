#ifndef _MESSAGE_HPP
#define _MESSAGE_HPP

#include <string>
#include "sourceLocation.hpp"

namespace msg
{
    enum msg_level
    {
        OUTPUT = 0, // simple output. not an issue
        DEBUGGING = 1,
        WARNING = 2, // warning message, but not a problem
        ERROR = 3, // problem compiling. cannot finish
        FAILURE = 4, // internal failure; unexpected behaviour on compiler's part
        UNIMPLEMENTED = 5, // unimplemented feature. this shouldnt be happening...
        FATAL = 6, // can not even continue. something really wrong happened
    };
};

#define assert_action(cond, do) if(cond) do;

int currentErrorLevel();
void cond_message(int cond, int level, std::string msg, SourceLocation loc = SourceLocation());
void assert_message(int assert, int lvl, std::string msg, SourceLocation loc = SourceLocation());
void emit_message(int level, std::string msg, SourceLocation loc = SourceLocation());

#endif
