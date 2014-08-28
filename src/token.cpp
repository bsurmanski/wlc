#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "token.hpp"
#include "message.hpp"

int Operator::getBinaryPrecidence() {
    return ::getBinaryPrecidence(kind);
}

int Operator::getPostfixPrecidence() {
    return ::getPostfixPrecidence(kind);
}

int Operator::getUnaryPrecidence() {
    return ::getUnaryPrecidence(kind);
}

bool Operator::isBinaryOp() {
    return getBinaryPrecidence();
}

//TODO: should distinguish between postfix ++ and prefix ++
bool Operator::isPostfixOp() {
    return getPostfixPrecidence();
}

bool Operator::isUnaryOp() {
    return getUnaryPrecidence();
}

bool Operator::isAssignOp() {
    return ::isAssignOp(kind);
}

bool Operator::isCompoundAssignOp() {
    ::isCompoundAssignOp(kind);
}


bool isCompoundAssignOp(tok::TokenKind tkind) {
    switch(tkind)
    {
        case tok::colonequal:
        case tok::plusequal:
        case tok::minusequal:
        case tok::starequal:
        case tok::slashequal:
        case tok::ampequal:
        case tok::barequal:
        case tok::caretequal:
        case tok::percentequal:
            return true;
        default:
            return false;
    }
}

tok::TokenKind getCompoundAssignBase(tok::TokenKind tkind) {
    switch(tkind)
    {
        case tok::colonequal: emit_message(msg::UNIMPLEMENTED, "operator lowering for cast equal unimpl");
        case tok::plusequal: return tok::plus;
        case tok::minusequal: return tok::minus;
        case tok::starequal: return tok::star;
        case tok::slashequal: return tok::slash;
        case tok::ampequal: return tok::amp;
        case tok::barequal: return tok::bar;
        case tok::caretequal: return tok::caret;
        case tok::percentequal: return tok::percent;
        default:
            return (tok::TokenKind) 0;
    }
}

bool isAssignOp(tok::TokenKind tkind)
{
    return isCompoundAssignOp(tkind) || tkind == tok::equal;
}

// higher precidence means stronger binding, 0 means no binding
int getBinaryPrecidence(tok::TokenKind tkind)
{
    switch(tkind)
    {
        case tok::equal:
        case tok::colonequal:
        case tok::plusequal:
        case tok::minusequal:
        case tok::starequal:
        case tok::slashequal:
        case tok::ampequal:
        case tok::barequal:
        case tok::caretequal:
        case tok::percentequal:
                return 2;
        case tok::barbar:
        case tok::kw_or:
            return 3;
        case tok::ampamp:
        case tok::kw_and:
                return 4;
        case tok::colon: // cast
        case tok::bar:
                return 5;
        case tok::caret:
                return 6;
        case tok::amp:
                return 7;
        case tok::bangequal:
        case tok::equalequal:
                return 8;
        case tok::less:
        case tok::lessequal:
        case tok::greater:
        case tok::greaterequal:
                return 9;
        case tok::plus:
        case tok::minus:
                return 10;
        case tok::lessless:
        case tok::greatergreater:
        case tok::star:
        case tok::slash:
        case tok::percent:
                return 11;
        default:
                return 0;
    }
}


int getPostfixPrecidence(tok::TokenKind tkind)
{
    switch(tkind)
    {
        case tok::lparen:
        case tok::lbracket:
        case tok::plusplus:
        case tok::minusminus:
        case tok::dot:
            return 13;
        default:
            return 0;
    }
}

int getUnaryPrecidence(tok::TokenKind tkind)
{
    switch(tkind)
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

Token::Token(TokenKind k, std::string st) : kind(k)
{
    if(kind == tok::charstring || kind == tok::identifier)
        strData = new std::string(st);
    else if(kind == tok::intNum)
    {
        iData = atoll(st.c_str());
    }
    else if(kind == tok::floatNum)
    {
        fData = strtod(st.c_str(), NULL);
    } else { assert(false && "havent bothered with conversion of this constant type yet..."); }
}

Token::Token(TokenKind k, double data) : kind(k)
{
    if(kind == tok::intNum)
    {
        iData = data;
    } else if(kind == tok::floatNum)
    {
        fData = data;
    } else assert(false && "invalid token kind initialization");
}

Token::Token(const Token &other)
{
    kind = other.kind;
    characters = other.characters;
    newline = other.newline;
    loc = other.loc;
    if(other.is(tok::charstring) || other.is(tok::identifier)) strData = new std::string(*other.strData);
    else iData = other.iData;
}

Token::~Token()
{
}

void Token::dump()
{
    switch(kind)
    {
        #define TOK(X) case tok::X: printf(#X); printf("\n"); break;
        #define PUNCTUATOR(X,Y) case tok::X: printf(#X); printf("\n"); break;
        #define KEYWORD(X) case tok::kw_##X: printf("kw_"); printf(#X); printf("\n"); break;
#include "tokenkinds.def"
    }
}

bool Token::isKeyword()
{
    switch(kind)
    {
#define KEYWORD(X) case kw_ ## X:
#include "tokenkinds.def"
        return true;
        default:
        return false;
    }
}

std::string Token::getSpelling()
{
    switch (kind)
    {
#define TOK(X) case X: return #X;
#define PUNCTUATOR(X,Y) case tok::X: return Y;
#define KEYWORD(X) case kw_ ## X: return #X;
#include "tokenkinds.def"

    }
}

std::string Token::toString()
{
    if(is(tok::identifier))
    {
        return stringData();
    }
    return getSpelling();
}
