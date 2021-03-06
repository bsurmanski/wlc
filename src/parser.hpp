
#ifndef _PARSER_HPP
#define _PARSER_HPP

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Module.h>
#include "lexer.hpp"
#include "ast.hpp"
#include "astScope.hpp"
#include "sourceLocation.hpp"
#include "file.hpp"

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
        void parseFile(ModuleDeclaration *mod, File *file, SourceLocation l = SourceLocation());
        void parseString(const char *str);
        AST *getAST() { return ast; }

    protected:
};

/**
 * parsing is pretty strange in WLC. The strangest parts revolve around identifiers.
 * identifiers are user defined names, used as variable names, function names, and
 * struct names. In WL, global variables need not be defined before use. So, for example
 * a function can be declared below it's usage. This has some strange side effects for parsing.
 * When encountering an identifier, we often don't know whether it is a variable, function,
 * struct, or simply undefined. This complicates determining expressions from declarations.
 * The way I have solved it, is to treat unknown identifiers as declarations, and traceback
 * if it turns out I am wrong
 */
class ParseContext
{
    Lexer *lexer;
    Parser *parser;
    PackageDeclaration *package;
    ModuleDeclaration *module;
    std::stack<ASTScope*> scope;
    //std::stack<Lexer*> lexers;

    public:
    ParseContext(Lexer *lex, Parser *parse, PackageDeclaration *p) : lexer(lex), parser(parse),
        package(p) {}
    ~ParseContext() { }

    AST *getAST() { return parser->getAST(); }

    protected:
    std::deque<int> recoveryState; // tokens left over in rqueue after current recover
    std::deque<Token> rqueue; // recovery queue
    std::deque<Token> tqueue; // token lookahead queue

    void ignoreComments() {
        while(lexer->peek().is(tok::comment))
            lexer->ignore();
    }
    //void push() { tqueue.push(getBuffer()); }
    Token get() {
        Token t;
        if(!tqueue.empty()){
            t = tqueue.front();
            tqueue.pop_front();
        } else {
            ignoreComments();
            t = lexer->get();
        }
        if(recoveryState.size()) rqueue.push_back(t);
        return t;
    }

    Token linePeek() { Token t = peek(); if(!t.followsNewline()) return t; return Token(tok::semicolon); }
    Token peek() {
        if(!tqueue.empty()) return tqueue.front();
        ignoreComments();
        return lexer->peek();
    }
    void ignore() { get(); }

    void dropLine() { // dumps entire line of input given an error (if line ends in binop, dump that too)
        //bool binOp = false;
        //while(!peek().followsNewline() && !binOp) {
        //    binOp = peek().isBinaryOp();
        //    ignore();
        //}
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

    PackageDeclaration *currentPackage() { return package; }
    ModuleDeclaration *getModule() { return module; }
    void pushScope(ASTScope* tbl) { scope.push(tbl); }
    ASTScope *popScope() { ASTScope *tbl = scope.top(); scope.pop(); return tbl;}
    ASTScope *getScope() { if(!scope.empty()) return scope.top(); return NULL; }

    //void pushLexer(Lexer *l) { lexers.push(l); }
    //Lexer *popLexer() { Lexer *lx = lexers.top(); lexers.pop(); return lx; }
    //Lexer *getLexer() { if(!lexers.empty()) return lexer.top(); return NULL; }

    public:
    void parseModule(ModuleDeclaration *u);
    void parseTopLevel(ModuleDeclaration *unit);
    void pushRecover();
    void popRecover();
    void recover();

    ASTType *parseBasicType();
    ASTType *parseScalarType(); //scalar type is anything but pointer/array. this includes struct and tuples
    ASTType *parsePointerModifiedType(ASTType *base);
    ASTType *parseArrayModifiedType(ASTType *base);
    ASTType *parseFunctionModifiedType(ASTType *base); //where base is the return-type
    ASTType *parseModifiedType(ASTType *base);
    ASTType *parseType();

    void parseInclude();
    ImportExpression *parseImport();
    Statement *parseStatement();
    CaseStatement *parseCaseStatement();
    Statement *parseIfStatement();
    Statement *parseSwitchStatement();
    Statement *parseWhileStatement();
    Statement *parseForStatement();
    Statement *parseCompoundStatement();
    DeclarationQualifier parseDeclarationQualifier();
    Declaration *parseDeclaration();
    TopLevelExpression *parseTopLevelExpression();
    UseExpression *parseUseExpression();
    Expression *parseCastExpression(int prec = 0);
    Expression *parseExpression(int prec = 0);
    Expression *parseNewExpression();
    Expression *parseIdOpExpression();
    Expression *parseIdentifierExpression();
    Expression *parsePrimaryExpression();
    void parseArgumentList(std::list<Expression*> &args);
    Expression *parsePostfixExpression(int prec = 0);
    Expression *parseUnaryExpression(int prec = 0);
    Expression *parseBinaryExpression(int prec = 0);
    Expression *parseTupleExpression();
};

#endif
