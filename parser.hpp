
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
#include <stack>

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
    std::stack<SymbolTable*> scope;

    public:
    ParseContext(Lexer *lex, Parser *parse, TranslationUnit *un) : lexer(lex), parser(parse), unit(un){}
    ~ParseContext() { }

    protected:
    std::queue<Token> tqueue;

    void ignoreComments() { while(lexer->peek().kind == tok::comment) lexer->ignore(); }
    void push() { tqueue.push(getBuffer()); }
    Token get() { Token t; if(!tqueue.empty()){ t = tqueue.front(); tqueue.pop();} else { ignoreComments(); t = lexer->get();} return t; }
    Token linePeek() { Token t = peek(); if(!t.followsNewline()) return t; return Token(tok::semicolon); }
    Token peek() { 
        if(!tqueue.empty()) return tqueue.front();
        ignoreComments();
        return lexer->peek(); 
    }
    void ignore() { if(!tqueue.empty()) { tqueue.pop(); return; } ignoreComments(); lexer->ignore(); }

    Token getBuffer() { ignoreComments(); return lexer->get(); }
    Token peekBuffer() { ignoreComments(); return lexer->peek(); }
    void ignoreBuffer() { ignoreComments(); lexer->ignore(); }
    //void unGet(Token tok) { lexer->unGet(tok); }
    bool eof() { return tqueue.empty() && lexer->eof(); }

    void pushScope(SymbolTable* tbl) { scope.push(tbl); }
    SymbolTable *popScope() { SymbolTable *tbl = scope.top(); scope.pop(); return tbl;}
    SymbolTable *getScope() { if(!scope.empty()) return scope.top(); return NULL; }

    public:
    TranslationUnit *parseTranslationUnit(const char *unitnm);
    void parseTopLevel(TranslationUnit *unit);
    ASTType *parseType();
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
