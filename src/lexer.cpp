
#include "lexer.hpp"

#include <assert.h>
#include <string>

using namespace std;

Token Lexer::lexWord()
{
    char curChar;
    string tokstr;
    do {
        curChar = getChar();
        tokstr += curChar;
    } while(isalnum(peekChar()) || peekChar() == '_');

    if(tokstr == "and") return Token(tok::ampamp);
    if(tokstr == "or") return Token(tok::barbar);
    if(tokstr == "not") return Token(tok::bang);

#define KEYWORD(X) if(#X == tokstr) return Token(tok::kw_##X);
#define RESERVED(X) if(#X == tokstr) emit_message(msg::ERROR, "keyword '" + tokstr + "' is reserved");
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
            IFCONSUMEBREAK('=', kind = tok::plusequal);
            kind = tok::plus;
            break;
        case '-':
            ignoreChar();
            IFCONSUMEBREAK('-', kind = tok::minusminus);
            IFCONSUMEBREAK('=', kind = tok::minusequal);
            kind = tok::minus;
            break;
        case '*':
            ignoreChar();
            IFCONSUMEBREAK('*', kind = tok::starstar);
            IFCONSUMEBREAK('=', kind = tok::starequal);
            kind = tok::star;
            break;
        case '/':
            ignoreChar();
            IFCONSUMEBREAK('=', kind = tok::slashequal);
            IFCONSUMEBREAK('/', kind = tok::comment; while(peekChar() != '\n' && !eofChar()) ignoreChar(); );
            if(peekChar() == '*')
            {
                ignoreChar();
                while(true) {
                    while(peekChar() != '*' && !eofChar())
                        ignoreChar();

                    assert(peekChar() == '*');
                    ignoreChar();

                    if(peekChar() == '/')
                    {
                        ignoreChar();
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
            IFCONSUMEBREAK('=', kind = tok::ampequal);
            kind = tok::amp;
            break;
        case '|':
            ignoreChar();
            IFCONSUMEBREAK('|', kind = tok::barbar);
            IFCONSUMEBREAK('=', kind = tok::barequal);
            kind = tok::bar;
            break;
        case '^':
            ignoreChar();
            IFCONSUMEBREAK('=', kind = tok::caretequal);
            kind = tok::caret;
            break;
        case '%':
            ignoreChar();
            IFCONSUMEBREAK('=', kind = tok::percentequal);
            kind = tok::percent;
            break;
        case '!':
            ignoreChar();
            IFCONSUMEBREAK('=', kind = tok::bangequal);
            kind = tok::bang;
            break;
        case ':':
            ignoreChar();
            IFCONSUMEBREAK('=', kind = tok::colonequal);
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

//digit is in ASCII
static unsigned xdigittoi(unsigned dig)
{
    if(dig >= '0' && dig <= '9')
    {
        return dig - '0';
    } else if(dig >= 'a' && dig <= 'f')
    {
        return dig - 'a' + 10;
    } else if(dig >= 'A' && dig <= 'F')
    {
        return dig - 'a' + 10;
    }
    assert(false && "attempt to parse invalid HEX string");
}

static unsigned odigittoi(unsigned dig)
{
    if(dig >= '0' && dig < '8')
    {
        return dig - '0';
    }
    assert(false && "attempt to parse invalid OCT string");
}

static bool isodigit(unsigned dig)
{
    return dig >= '0' && dig < '8';
}

Token Lexer::lexNumber()
{
    //TODO hexstrings fp, etc
    bool fp = false;
    string numstr;
    numstr += getChar();
    if(numstr[0] == '0' && tolower(peekChar()) == 'x') //hex
    {
        ignoreChar(); // ignore 'x'
        //numstr = ""; // empty the string
        unsigned num = 0;
        while(isxdigit(peekChar()) || peekChar() == '.')
        {
            if(peekChar() == '.') {
                assert(!fp && "invalid numeric constant, hexadecimal floating point not supported");
                fp = true;
            } else
            {
                num = num << 4;
                num += xdigittoi(getChar());
            }
        }
        return Token(tok::intNum, num);

    } else if(numstr[0] == '0' && tolower(peekChar()) == 'o') //octal
    {
        ignoreChar(); // ignore 'b'
        unsigned num = 0;
        numstr = ""; // empty the string
        while(isodigit(peekChar()) || peekChar() == '.')
        {
            if(peekChar() == '.') {
                assert(!fp && "invalid numeric constant, binary floating point not supported");
                fp = true;
            } else
            {
                num = num << 3;
                num += odigittoi(getChar());
            }
        }
        return Token(tok::intNum, num);
    } else if(numstr[0] == '0' && tolower(peekChar()) == 'b') // binary
    {
        ignoreChar(); // ignore 'b'
        unsigned num = 0;
        while(peekChar()  == '0' || peekChar() == '1' || peekChar() == '.')
        {
            if(peekChar() == '.') {
                assert(!fp && "invalid numeric constant, binary floating point not supported");
                fp = true;
            } else
            {
                num = num << 1;
                if(getChar() == '1') num++;
            }
        }

        return Token(tok::intNum, num);
    } else //normal decinal number
    {
        while(isdigit(peekChar()) || peekChar() == '.')
        {
            if(peekChar() == '.')
            {
                assert(!fp && "invalid numeric constant");
                fp = true;
            }
            numstr += getChar();
            while(peekChar() == '_') ignore();
        }

        if(peekChar() == 'f')
        {
            fp = true;
            ignoreChar();
        }
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

    SourceLocation loc(filenm, this->line);

    Token tok = getTok();
    tok.newline = ws;
    tok.loc = loc;
    return tok;
}
