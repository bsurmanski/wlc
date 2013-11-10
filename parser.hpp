
#ifndef _PARSER_HPP
#define _PARSER_HPP

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Module.h>
#include "lexer.hpp"
#include "ast.hpp"
#include "symbolTable.hpp"

#include <queue>
#include <vector>
#include <map>

// wait, this doesn't really do anything anymore. all the parsing in ParseContext
class Parser
{
    AST *ast;

    public:
        Parser() { ast = new AST(); }
        void parseFile(const char *filenm);
        void parseString(const char *str);
        AST *getAST() { return ast; }

    protected:
};

class ParseContext
{
    Lexer *lexer;
    Parser *parser;
    TranslationUnit *unit;
    SymbolTable *scope;

    public:
    ParseContext(Lexer *lex, Parser *parse, TranslationUnit *un) : lexer(lex), parser(parse), unit(un), scope(new SymbolTable) {}
    ~ParseContext() { delete scope; }

    protected:
    std::queue<Token> tqueue;

    void push() { tqueue.push(get()); }
    Token pop() { Token t; if(!tqueue.empty()){ t = tqueue.front(); tqueue.pop();} else { t = get();} return t; }

    void ignoreComments() { while(lexer->peek().kind == tok::comment) lexer->ignore(); }
    Token get() { ignoreComments(); return lexer->get(); }
    Token linePeek() { Token t = peek(); if(!t.followsNewline()) return t; return Token(tok::semicolon); }
    Token peek() { ignoreComments(); return lexer->peek(); }
    void ignore() { ignoreComments(); lexer->ignore(); }
    //void unGet(Token tok) { lexer->unGet(tok); }
    bool eof() { return lexer->eof(); }

    public:
    TranslationUnit *parseTranslationUnit(const char *unitnm);
    void parseTopLevel(TranslationUnit *unit);
    ASTQualType parseType();
    ImportExpression *parseImport();
    Statement *parseDeclarationStatement();
    Statement *parseStatement();
    Declaration *parseDeclaration();
    Expression *parseExpression(int prec = 0);
    Expression *parsePrimaryExpression();
    Expression *parsePostfixExpression(int prec = 0);
    Expression *parseUnaryExpression(int prec = 0);
    Expression *parseBinaryExpression(int prec = 0);
};

#endif
