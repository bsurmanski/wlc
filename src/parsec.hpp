#ifndef _PARSEC_HPP
#define _PARSEC_HPP

#include <clang-c/Index.h>
#include <vector>
#include <iostream>
#include "ast.hpp"
#include "message.hpp"
#include "sourceLocation.hpp"

void parseCImport(ModuleDeclaration *mod,
        std::string filenm,
        SourceLocation loc);

#endif
