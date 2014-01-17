#ifndef _SOURCELOCATION_HPP
#define _SOURCELOCATION_HPP

struct TranslationUnit;

struct SourceLocation
{
    TranslationUnit *unit;
    int line;
    int ch;
    SourceLocation() : unit(NULL), line(1), ch(1) {}
    SourceLocation(TranslationUnit *u, int l, int cha = 1) : unit(u), line(l), ch(cha) {}
};

#endif
