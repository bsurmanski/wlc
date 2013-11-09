
#include <cstdio>

#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include <fstream>
#include <vector>
#include <iostream>

#include "parser.hpp"
#include "streamLexer.hpp"
#include "ast.hpp"

using namespace llvm;
using namespace std;

ASTQualType ParseContext::parseType()
{
    bool ptr = false;
    while(peek().is(tok::caret)) { ptr = true; ignore(); }
    Token t = get();
    assert(t.is(tok::identifier) || t.isKeywordType());
    Identifier *id = scope->lookup(t.toString());
    /*
    static ASTBasicType intTy(4);
    static ASTBasicType charTy(1);
    static ASTPointerType charPTy(&charTy);

    bool ptr = false;


    if(ptr && peek().is(tok::kw_char))
    {
        ignore();
        return &charPTy;
    } else if(peek().is(tok::kw_char))
    {
        ignore();
        return &charTy;
    } else if(peek().is(tok::kw_int))
    {
        ignore();
        return &intTy;
    }
    */

    //TODO: pointers again
    return ASTQualType(id);

    //assert(false && "unhandled type");
}

TranslationUnit *ParseContext::parseTranslationUnit(const char *unitnm)
{
    TranslationUnit *unit = new TranslationUnit(NULL); //TODO: identifier
    while(peek().isNot(tok::eof))
    {
        parseTopLevel(unit); // modifies t-unit
    }
    return unit;
}

// parse top level expression and apply to translation unit
void ParseContext::parseTopLevel(TranslationUnit *unit)
{
    Statement *stmt;
    switch(peek().kind)
    {
        case tok::kw_import:
            //TODO: add to symbol table
            parseImport(); //TODO: find and import new translation unit
            break;

        case tok::semicolon:
            ignore();
            break;

        default:
            stmt = parseDeclarationStatement();
            unit->statements.push_back(stmt);
    }
}

ImportExpression *ParseContext::parseImport()
{
    cout << "import not impl; ignoring" << endl;
    ignore(); // ignore import 
    if(linePeek().is(tok::dot))
    {
        // relative path
        ignore(); // dot
    }
    ignore(); // ignore id
    while(linePeek().is(tok::dot))
    {
        ignore(); // ignore dot
        ignore(); // ignore id
    }
    return new ImportExpression;
}

Statement *ParseContext::parseDeclarationStatement()
{
    Declaration *decl;

    decl = parseDeclaration();
    printf("New declaration: %s\n", decl->getName().c_str());
    //TODO: register decl
    return new DeclarationStatement(decl);
}

Statement *ParseContext::parseStatement()
{
    switch(peek().kind)
    {
        case tok::lbrace: // TODO: lbrace as statement instead of expression?
        case tok::kw_if:
        case tok::kw_while:
        case tok::kw_for:
        case tok::kw_switch:
            return new ExpressionStatement(parseExpression());

        case tok::kw_var:
        case tok::kw_int:
        case tok::kw_char:
        case tok::kw_short:
        case tok::kw_float:
        case tok::kw_double:
            return parseDeclarationStatement();

        case tok::kw_return:
            ignore();
            if(!peek().followsNewline())
                return new ReturnStatement(parseExpression()); //TODO: newline thing
            return new ReturnStatement(NULL);

        case tok::semicolon:
            ignore();
        default:
            return NULL;
    }
}

Declaration *ParseContext::parseDeclaration()
{
    //TODO: parse function decl specs

    ASTQualType rtype = parseType();

    Token t_id = get();
    assert(t_id.is(tok::identifier));
    assert(!scope->contains(t_id.toString()) && "redeclaration!");
    Identifier *id = scope->get(t_id.toString());

    //TODO parse decl specs

    vector<pair<ASTQualType, std::string> > args;

    if(peek().is(tok::lparen)) // function decl
    {
        ignore(); // lparen 
        while(!peek().is(tok::rparen))
        {
            //Token at_type = get();
            ASTQualType aty = parseType();
            Token t_name = get();
            args.push_back(pair<ASTQualType, std::string>(aty, t_name.toString()));
            if(peek().is(tok::comma))
            {
                ignore();
                continue;
            }
        }
        ignore(); //rparen
        Statement *stmt = parseStatement();

        FunctionPrototype *proto = new FunctionPrototype(rtype, args);
        Declaration *decl = new FunctionDeclaration(id, proto, stmt); //TODO: prototype, name
        id->setDeclaration(decl, Identifier::ID_FUNCTION);
        return decl;
    }

    if(peek().is(tok::equal))
    {
        //TODO: default value
    }
}

Expression *ParseContext::parseExpression()
{
    return parseBinaryExpression();
}

Expression *ParseContext::parsePostfixExpression()
{
    Expression *exp = parsePrimaryExpression();
    if(peek().getPostfixPrecidence())
    {
        if(peek().is(tok::dot)) // member
        {
             
        } else if(peek().is(tok::lparen)) // call
        {
            //TODO: eat call 
        } else if(peek().is(tok::lbracket)) //index/slice
        {
            //TODO: eat index 
        } else {assert(false && "somethings up");} //TODO: ++ and --
    }

    //TODO: parse postfix of postfix. eg somecall()++;
    return exp;
}

Expression *ParseContext::parseUnaryExpression(int prec)
{
    if(peek().getUnaryPrecidence())
    {
        int op = get().kind;
        return new UnaryExpression(op, parseExpression());
    }

    return parsePostfixExpression();
}

Expression *ParseContext::parsePrimaryExpression()
{
    if(peek().is(tok::charstring))
    {
        return new StringExpression(get().stringData());
    }

    if(peek().is(tok::intNum))
    {
        return new NumericExpression(NumericExpression::INT, get().intData());
    }

    if(peek().is(tok::floatNum))
    {
        return new NumericExpression(NumericExpression::DOUBLE, get().floatData());
    }

    //TODO: eat char constant

    if(peek().is(tok::lparen)) // bracket expression
    {
        ignore();
        Expression *exp = parseExpression();
        assert(peek().kind == tok::rparen && "Expected right parenthsis!");
        ignore();
        return exp;
    }

    if(peek().is(tok::lbrace))
    {
        ignore(); // eat lbrace

        vector<Statement*> stmts;
        while(peek().isNot(tok::rbrace))
        {
            stmts.push_back(parseStatement());
        }
        ignore(); // eat rbrace
        return new BlockExpression(stmts);
    }
}

Expression *ParseContext::parseBinaryExpression(int prec)
{
    Expression *lhs = parseUnaryExpression();

    if(peek().getBinaryPrecidence())
    {
        int op = peek().kind;
        return new BinaryExpression(op, lhs, parseBinaryExpression(get().getBinaryPrecidence())); 
    }

    return lhs;
}

void Parser::parseFile(const char *filenm)
{
    ifstream stream(filenm);
    Lexer *lexer = new StreamLexer(stream);
    ParseContext context(lexer, this, NULL);

    TranslationUnit *unit = context.parseTranslationUnit(NULL); //TODO: identifier
    ast->units[filenm] = unit; //TODO: symbol table

    delete lexer;
}
