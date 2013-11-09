#include <stdio.h>
#include <stdlib.h>
#include "token.hpp"

Token::Token(TokenKind k, std::string st) : kind(k) 
{ 
    data = NULL;
    if(kind == tok::charstring || kind == tok::identifier)
        data = new std::string(st); 
    if(kind == tok::intNum)
    {
        data = new uint64_t;
        *(uint64_t*)data = atol(st.c_str()); //XXX on 64 bit machines, we can simply store in pointer
    }
    if(kind == tok::floatNum)
    {
        data = new double; //XXX on 64 bit machines, we can simply store in pointer
        *(double*)data = 0.0;
        //*(double*)data = atod(st.c_str());
    }
}

Token::Token(const Token &other)
{
    kind = other.kind;
    characters = other.characters;
    newline = other.newline;
    data = other.data;
    /* TODO: copy data
    if(other.data && kind == tok::charstring || kind == tok::identifier)
    {
        std::string *str = ((std::string*) other.data);
        data = new std::string(*str); 
    }*/
}

Token::~Token() 
{ 
    //if(kind == tok::charstring || kind == tok::identifier) 
    //    delete ((std::string*) data); 
    //TODO: delete data
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
        return *stringData();
    }
    return getSpelling();
}
