#ifndef _LEXER_HPP
#define _LEXER_HPP

#include <string>
#include <fstream>

#include "token.hpp"

class Lexer
{
    protected:
    Token current;

    Lexer(){}
    virtual int peekChar() = 0;
    virtual void ignoreChar() = 0;
    virtual int getChar() = 0;
    virtual bool eofChar() = 0;

    public:

    virtual Token peek()
    {
        return current;
    }

    virtual Token get()
    {
        Token ret = current;
        advance();
        return ret;
    }

    //virtual void unGet(Token tok);

    virtual void ignore()
    {
        advance();
    }

    virtual bool eof()
    {
        return current.is(tok::eof);
    }

    virtual bool advance() = 0;

    Token lex();
    Token getTok();
    Token lexWord();
    Token lexPunct();
    Token lexString();
    Token lexNumber();
};

#endif
