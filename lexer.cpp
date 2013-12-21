
#include "lexer.hpp"

#include <llvm/Support/ErrorHandling.h>
#include <string>

using namespace std;

/*
std::string Token::toString()
{
    if(isIdentifier())
    {
        return *((string*) data);
    } else if(isLiteral())
    {
        return *((string*) data);
    } else if(isKeyword())
    {
        return keywords[kind - KW_BEGIN];
    } else if(isOp())
    {
        return operators[kind - OP_BEGIN];
    }

    return "";
}*/

Token Lexer::lexWord()
{
    char curChar;
    string tokstr;
    do {
        curChar = getChar();
        tokstr += curChar;
    } while(isalnum(peekChar()) || peekChar() == '_');

#define KEYWORD(X) if(#X == tokstr) return Token(tok::kw_##X);
#include "tokenkinds.def"

    // is identifier
    return Token(tok::identifier, tokstr);
}

// includes comments
Token Lexer::lexPunct()
{
#define IFCONSUMEBREAK(X,Y) if(peekChar() == X){ ignoreChar(); Y; break; }
    tok::TokenKind kind = tok::unknown;
    switch(peekChar())
    {
        case '+':
            ignoreChar();
            IFCONSUMEBREAK('+', kind = tok::plusplus);
            kind = tok::plus;
            break;
        case '-':
            ignoreChar();
            IFCONSUMEBREAK('-', kind = tok::minusminus);
            kind = tok::minus;
            break;
        case '*':
            ignoreChar();
            IFCONSUMEBREAK('*', kind = tok::starstar);
            kind = tok::star;
            break;
        case '/':
            ignoreChar();
            IFCONSUMEBREAK('/', kind = tok::comment; while(peekChar() != '\n' && !eofChar()) ignoreChar(); );
            if(peekChar() == '*')
            {
                ignoreChar();
                while(peekChar() != '*' && !eofChar())
                {
                    ignore();
                    if(peekChar() == '/') 
                    {
                        ignore();
                        break;
                    }
                }
                kind = tok::comment;
                break;
            }
            kind = tok::slash;
            break;
        case '=':
            ignoreChar();
            IFCONSUMEBREAK('=', kind = tok::equalequal);
            kind = tok::equal;
            break;
        case '<':
            ignoreChar();
            IFCONSUMEBREAK('<', kind = tok::lessless);
            IFCONSUMEBREAK('=', kind = tok::lessequal);
            kind = tok::less;
            break;
        case '>':
            ignoreChar();
            IFCONSUMEBREAK('>', kind = tok::greatergreater);
            IFCONSUMEBREAK('=', kind = tok::greaterequal);
            kind = tok::greater;
            break;
        case '(':
            ignoreChar();
            kind = tok::lparen;
            break;
        case ')':
            ignoreChar();
            kind = tok::rparen;
            break;
        case '{':
            ignoreChar();
            kind = tok::lbrace;
            break;
        case '}':
            ignoreChar();
            kind = tok::rbrace;
            break;
        case '[':
            ignoreChar();
            kind = tok::lbracket;
            break;
        case ']':
            ignoreChar();
            kind = tok::rbracket;
            break;
        case ',':
            ignoreChar();
            kind = tok::comma;
            break;
        case ';':
            ignoreChar();
            kind = tok::semicolon;
            break;
        case '&':
            ignoreChar();
            IFCONSUMEBREAK('&', kind = tok::ampamp);
            kind = tok::amp;
            break;
        case '|':
            ignoreChar();
            IFCONSUMEBREAK('|', kind = tok::barbar);
            kind = tok::bar;
            break;
        case '^':
            ignoreChar();
            kind = tok::caret;
            break;
        case '%':
            ignoreChar();
            kind = tok::percent;
            break;
        case '!':
            ignoreChar();
            kind = tok::bang;
            break;
        case ':':
            ignoreChar();
            kind = tok::colon;
            break;
        case '~':
            ignoreChar();
            kind = tok::tilde;
            break;
        case '.':
            ignoreChar();
            IFCONSUMEBREAK('.', IFCONSUMEBREAK('.', kind = tok::dotdotdot); kind = tok::dotdot; break;);
            kind = tok::dot;
            break;
        default:
            kind = tok::unknown;
            break;
    }
    return Token(kind);
}

static int getEscapeChar(char c)
{
    switch(c)
    {
        case '\'': return '\'';
        case '"': return '\"';
        case '\\': return '\\';
        case '?': return '\?';
        case 'a': return '\a';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'n': return'\n';
        case 'r': return '\r';
        case 't': return'\t';
        case 'v': return '\v';
    }
    //TODO octal, hex escape sequence
    return '~'; // XXX UNKNOWN ESCAPE CHAR
}

// string literal
Token Lexer::lexString()
{
    string tokstr;
    char curChar;
    bool escape = false;
    ignoreChar(); // ignore opening "
    while((curChar = getChar()) != '"')
    {
        if(escape)
        {
            tokstr += getEscapeChar(curChar);
            escape = false;
            continue;
        }

        if(curChar == '\\')
        {
            escape = true;
            continue;
        }

        escape = false;
        tokstr += curChar;
    }

    return Token(tok::charstring, tokstr);
}

Token Lexer::lexNumber()
{
    //TODO hexstrings fp, etc
    bool fp = false;
    string numstr;
    while(isdigit(peekChar()) || peekChar() == '.')
    {
        if(peekChar() == '.')
        {
            assert(!fp && "invalid numeric constant");
            fp = true;
        }
        numstr += getChar();
    }
    if(fp) return Token(tok::floatNum, numstr);
    return Token(tok::intNum, numstr);
}

Token Lexer::getTok()
{

    if(eofChar())
    {
        return Token(tok::eof);
    }

    char c = peekChar();

    if(isalpha(c) || c == '_')
    {
        return lexWord();
    }

    if(c == '"')
    {
        return lexString();
    }

    if(isdigit(c) || c == '\'')
    {
        return lexNumber();
    }

    if(ispunct(c))
    {
        return lexPunct();
    }
    return Token(tok::none);
}

Token Lexer::lex()
{
    bool ws = false;
    char c;
    while(isspace(c = peekChar()))
    {
        if(c == '\n')
        {
            ws = true;
            line++;
        }
        ignoreChar();
    }

    Token tok = getTok();
    tok.newline = ws;
    return tok;
}
