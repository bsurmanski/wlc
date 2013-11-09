
#ifndef _TOKEN_HPP
#define _TOKEN_HPP

#include <string>
#include <stdint.h>

namespace tok
{
    enum TokenKind
    {
#define TOK(X) X,
#include "tokenkinds.def"
        NUM_TOKENS
    };
}

using namespace tok;

struct Token
{
    TokenKind kind; 
    unsigned short characters;
    bool newline;
    void *data;

    Token() : kind(tok::none), data(NULL) {}
    Token(TokenKind k) : kind(k), data(NULL) {}
    Token(TokenKind k, std::string st);
    Token(const Token &other);
    ~Token();

    bool isKeyword();
    bool is(tok::TokenKind k) const { return kind == k; }
    bool isNot(tok::TokenKind k) const { return kind != k; }

    bool isBinaryOp() { return getBinaryPrecidence(); }
    bool isUnaryOp() { return getUnaryPrecidence(); }
    bool followsNewline() { return newline; }

    std::string *stringData() { if(kind == tok::charstring || kind == tok::identifier) return (std::string *) data; return NULL; }
    //TODO: parse other datatypes as int
    uint64_t intData() { if(kind == tok::intNum) { return *(uint64_t*) data; } return 0; } 
    //TODO: parse other datatypes as float
    double floatData() { if(kind == tok::floatNum) { return *(double*) data; } return 0.0; } 

    void setLength(short toklen) { characters = toklen; }
    std::string getSpelling();
    void dump();

    bool isKeywordType()
    {
        switch(kind)
        {
            case tok::kw_void:
            case tok::kw_bool:
            case tok::kw_char:
            case tok::kw_short:
            case tok::kw_int:
            case tok::kw_long:
            case tok::kw_float:
            case tok::kw_double:
                return true;

            default:
                return false;
        }
    }

    // higher precidence means stronger binding, 0 means no binding
    int getBinaryPrecidence()
    {
        switch(kind)
        {
            case tok::comma:
                return 1;
            case tok::barbar:
                return 2;
            case tok::ampamp:
                    return 3;
            case tok::bar:
                    return 4;
            case tok::caret:
                    return 5;
            case tok::amp:
                    return 6;
            case tok::equalequal:
                    return 7;
            case tok::less:
            case tok::lessequal:
            case tok::greater:
            case tok::greaterequal:
                    return 8;
            case tok::lessless:
            case tok::greatergreater:
                    return 9;
            case tok::plus:
            case tok::minus:
                    return 10;
            case tok::star:
            case tok::slash:
            case tok::percent:
                    return 11;
            default:
                    return 0;
        }
    }

    int getPostfixPrecidence()
    {
        switch(kind)
        {
            case tok::dot:
            case tok::lparen:
            case tok::lbracket:
            case tok::plusplus:
            case tok::minusminus:
                return 13;
            default:
                return 0;
        }
    }

    int getUnaryPrecidence()
    {
        switch(kind)
        {
            case tok::plusplus:
            case tok::minusminus:
            case tok::plus:
            case tok::minus:
            case tok::bang:
            case tok::tilde:
            case tok::caret:
            case tok::amp:
                return 12;
            default:
                return 0;
        }
    }

    std::string toString();
};

#endif
