
#ifndef _PARSER_HPP
#define _PARSER_HPP

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Module.h>
#include "lexer.hpp"
#include "ast.hpp"
#include "symbolTable.hpp"
#include "sourceLocation.hpp"

#include <deque>
#include <vector>
#include <map>
#include <stack>

// wait, this doesn't really do anything anymore. all the parsing in ParseContext
class Parser
{
    AST *ast;

    public:
        Parser() { ast = new AST(); }
        void parseFile(TranslationUnit *u, std::string filenm);
        void resolveImports(TranslationUnit *u);
        void parseString(const char *str);
        AST *getAST() { return ast; }

    protected:
};

class ParseContext
{
    Lexer *lexer;
    Parser *parser;
    Package *package;
    TranslationUnit *unit;
    std::stack<SymbolTable*> scope;
    //std::stack<Lexer*> lexers;

    public:
    ParseContext(Lexer *lex, Parser *parse, Package *p) : lexer(lex), parser(parse), package(p){}
    ~ParseContext() { }

    protected:
    std::deque<Token> tqueue;

    void ignoreComments() { while(lexer->peek().kind == tok::comment) lexer->ignore(); }
    //void push() { tqueue.push(getBuffer()); }
    Token get() { Token t; if(!tqueue.empty()){ t = tqueue.front(); tqueue.pop_front();} else { ignoreComments(); t = lexer->get();} return t; }
    Token linePeek() { Token t = peek(); if(!t.followsNewline()) return t; return Token(tok::semicolon); }
    Token peek() {
        if(!tqueue.empty()) return tqueue.front();
        ignoreComments();
        return lexer->peek();
    }
    void ignore() { if(!tqueue.empty()) { tqueue.pop_front(); return; } ignoreComments(); lexer->ignore(); }

    void dropLine() { // dumps entire line of input given an error (if line ends in binop, dump that too)
        bool binOp = false;
        while(!peek().followsNewline() && !binOp) {
            binOp = peek().isBinaryOp();
            ignore(); 
        }
    }

    Token lookAhead(int i = 0) { // lookAhead(0) is equivilent to peek()
        while(tqueue.size() < i+1) {
            ignoreComments(); 
            tqueue.push_back(lexer->get());
        }
        return tqueue.at(i);
    }

    //Token getBuffer() { ignoreComments(); return lexer->get(); }
    //Token peekBuffer() { ignoreComments(); return lexer->peek(); }
    //void ignoreBuffer() { ignoreComments(); lexer->ignore(); }
    //void unGet(Token tok) { lexer->unGet(tok); }
    bool eof() { return tqueue.empty() && lexer->eof(); }

    Package *currentPackage() { return package; }
    void pushScope(SymbolTable* tbl) { scope.push(tbl); }
    SymbolTable *popScope() { SymbolTable *tbl = scope.top(); scope.pop(); return tbl;}
    SymbolTable *getScope() { if(!scope.empty()) return scope.top(); return NULL; }

    //void pushLexer(Lexer *l) { lexers.push(l); }
    //Lexer *popLexer() { Lexer *lx = lexers.top(); lexers.pop(); return lx; }
    //Lexer *getLexer() { if(!lexers.empty()) return lexer.top(); return NULL; }

    public:
    void parseTranslationUnit(TranslationUnit *u, const char *unitnm);
    void parseTopLevel(TranslationUnit *unit);
    ASTType *parseType();
    void parseInclude();
    ImportExpression *parseImport();
    Statement *parseDeclarationStatement();
    Statement *parseStatement();
    Declaration *parseDeclaration();
    Expression *parseIfExpression();
    Expression *parseWhileExpression();
    Expression *parseForExpression();
    Expression *parseCastExpression(int prec = 0);
    Expression *parseExpression(int prec = 0);
    Expression *parsePrimaryExpression();
    Expression *parsePostfixExpression(int prec = 0);
    Expression *parseUnaryExpression(int prec = 0);
    Expression *parseBinaryExpression(int prec = 0);
};

#endif
