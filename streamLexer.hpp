#ifndef _STREAMLEXER_HPP
#define _STREAMLEXER_HPP

#include <string>
#include <fstream>

#include "token.hpp"
#include "lexer.hpp"

class StreamLexer : public Lexer
{
    std::ifstream *stream;

    public:

    StreamLexer() : stream(NULL) {}

    StreamLexer(std::ifstream &st) : stream(&st)
    {
        setStream(st);
    }

    void setStream(std::ifstream &st)
    {
        stream = &st;
        advance();
    }

    virtual int peekChar()
    {
        return stream->peek();
    }

    virtual void ignoreChar()
    {
        stream->ignore();
    }

    virtual int getChar()
    {
        return stream->get();
    }

    virtual bool eofChar()
    {
        return stream->eof();
    }

    /*
    void unGet(Token tok)
    {
        stream->seekg(-current.characters, std::ios_base::cur);
        current = tok;
    }
    */

    virtual bool advance()
    {
        std::streampos before = stream->tellg();
        current = lex();
        std::streampos after = stream->tellg();
        current.setLength(after - before);
        if(current.kind == tok::none || current.kind == tok::eof) return false;
        return true;
    }
};

#endif
