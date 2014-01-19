#ifndef _SOURCELOCATION_HPP
#define _SOURCELOCATION_HPP

struct TranslationUnit;

struct SourceLocation
{
    TranslationUnit *unit;
    int line;
    int ch;
    bool isUnknown() { return !line && !ch; }
    SourceLocation() : unit(NULL), line(1), ch(1) {}
    SourceLocation(TranslationUnit *u, int l, int cha = 1) : unit(u), line(l), ch(cha) {}
};

struct SourceSlice
{
    SourceLocation loc;
    int length;
    SourceSlice(SourceLocation lo, int len) : loc(lo), length(len) {}

    SourceLocation getBegin() { return loc; }
    SourceLocation getEnd() { 
        SourceLocation end = loc;
        end.ch += length; //TODO: what if crosses lines?
        return end; 
    }
};

#endif
