#ifndef _SOURCELOCATION_HPP
#define _SOURCELOCATION_HPP

struct TranslationUnit;

struct SourceLocation
{
    TranslationUnit *unit;
    int line;
    SourceLocation() : unit(NULL), line(0) {}
    SourceLocation(TranslationUnit *u, int l) : unit(u), line(l) {}
};

#endif
