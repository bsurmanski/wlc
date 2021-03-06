
#ifndef _TOKEN_HPP
#define _TOKEN_HPP

#include <string>
#include <stdint.h>

#include "sourceLocation.hpp"

namespace tok
{
    enum TokenKind
    {
#define TOK(X) X,
#include "tokenkinds.def"
        NUM_TOKENS
    };
}

struct Operator {
    enum {
        // unary
        preinc,
        predec,
        positive,
        negative,
        negate,
        bitflip,
        derefrence,
        addressOf,

        // postfix
        call,
        index,
        postinc,
        postdec,
        member,

        // binary
        add,
        sub,
        mul,
        pow,
        div,
        bor,
        band,
        bxor,
        lor,
        land,
        mod,

        // comparison
        less,
        lessequal,
        greater,
        greaterequal,
        equal,
        notequal,


    };

    //TODO: use above enum
    tok::TokenKind kind;
    Operator(tok::TokenKind k) : kind(k) {}

    int getBinaryPrecidence();
    int getPostfixPrecidence();
    int getUnaryPrecidence();
    bool isBinaryOp();
    bool isPostfixOp();
    bool isUnaryOp();
    bool isAssignOp();
    bool isLogicalOp();
    bool isCompoundAssignOp();
    std::string asString();
};

using namespace tok;

// higher precidence means stronger binding, 0 means no binding
int getBinaryPrecidence(tok::TokenKind tkind);
int getPostfixPrecidence(tok::TokenKind tkind);
int getUnaryPrecidence(tok::TokenKind tkind);
bool isCompoundAssignOp(tok::TokenKind tkind);
bool isAssignOp(tok::TokenKind tkind);
bool isLogicalOp(tok::TokenKind tkind);
tok::TokenKind getCompoundAssignBase(tok::TokenKind tkind);

struct Token
{
    TokenKind kind;
    unsigned short characters;
    bool newline;
    SourceLocation loc;

    SourceLocation getLocation() { return loc; }

    union
    {
        std::string *strData; // no union with classes
        int64_t iData;
        uint64_t uiData;
        double fData;
    };

    Token() : kind(tok::none), iData(0) {}
	Token(TokenKind k) : kind(k), iData(0) { }
    Token(TokenKind k, std::string st);
    Token(TokenKind k, double data);
    Token(const Token &other);
    ~Token();

    bool isKeyword();
    bool is(tok::TokenKind k) const { return kind == k; }
    bool isNot(tok::TokenKind k) const { return kind != k; }

    bool isBinaryOp() { return getBinaryPrecidence(); }
    bool isUnaryOp() { return getUnaryPrecidence(); }
    bool followsNewline() { return newline; }

    std::string stringData() { if(kind == tok::charstring || kind == tok::identifier) return *strData;
        return ""; }
    //TODO: parse other datatypes as int?
    int64_t intData() { if(kind == tok::intNum) { return iData; } return 0; }
    //TODO: parse other datatypes as float?
    double floatData() { if(kind == tok::floatNum) { return fData; } return 0.0; }

    void setLength(short toklen) { characters = toklen; }
    std::string getSpelling();
    void dump();

    bool isKeywordType()
    {
        switch(kind)
        {
#define BTYPE(X,SZ,SN) case tok::kw_##X: return true;
#define FTYPE(X,SZ) case tok::kw_##X: return true;
#include"tokenkinds.def"

            case tok::kw_var: return true;

            default:
                return false;
        }
    }


    int getBinaryPrecidence() { return ::getBinaryPrecidence((TokenKind) kind); }
    int getPostfixPrecidence() { return ::getPostfixPrecidence((TokenKind) kind); }
    int getUnaryPrecidence() { return ::getUnaryPrecidence((TokenKind) kind); }

    std::string toString();
};

#endif
