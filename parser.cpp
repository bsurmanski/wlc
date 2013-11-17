
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

ASTType *ParseContext::parseType()
{
    ASTType *type = NULL;
    bool ptr = false;
    Token t = get();
    assert(t.is(tok::identifier) || t.isKeywordType());
    if(t.isKeywordType())
    {
       switch(t.kind)
       {
        case tok::kw_int:
            type = ASTType::getIntTy(); break;
        case tok::kw_char:
            type = ASTType::getCharTy(); break;
        case tok::kw_void:
            type = ASTType::getVoidTy(); break;
        default:
            assert(false && "I haven't done this one yet...");
       }
    } else {
        Identifier *id = getScope()->lookup(t.toString());
        //TODO: do something with id...
    }
    while(peek().is(tok::caret)) 
    {
        ignore();
        type = type->getPointerTy();
    }

    return type;
}

TranslationUnit *ParseContext::parseTranslationUnit(const char *unitnm)
{
    TranslationUnit *unit = new TranslationUnit(NULL); //TODO: identifier
    pushScope(unit->scope); //TODO: scope stack
    while(peek().isNot(tok::eof))
    {
        parseTopLevel(unit); // modifies t-unit
    }
    popScope();
    assert(!getScope() && "somethings up with scope!");
    return unit;
}

// parse top level expression and apply to translation unit
void ParseContext::parseTopLevel(TranslationUnit *unit)
{
    Statement *stmt;
    Declaration *decl;
    switch(peek().kind)
    {
        case tok::kw_import:
            //TODO: add to symbol table
            //TODO: find and import new translation unit
            unit->imports.push_back(parseImport());
            break;

        case tok::semicolon:
            ignore();
            break;

        default:
            //stmt = parseDeclarationStatement();
            decl = parseDeclaration();
            if(dynamic_cast<TypeDeclaration*>(decl))
            {
                unit->types.push_back((TypeDeclaration*) decl);
            } else if(dynamic_cast<VariableDeclaration*>(decl))
            {
                unit->globals.push_back((VariableDeclaration*) decl);
            } else if(dynamic_cast<FunctionDeclaration*>(decl))
            {
                unit->functions.push_back((FunctionDeclaration*) decl);
            } else
            {
                assert(false && "what sort of global statement did i just parse?");
            }
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
        case tok::kw_int:
        case tok::kw_char:
        case tok::kw_short:
        case tok::kw_float:
        case tok::kw_double:
                return parseDeclarationStatement();

        case tok::identifier:
            push();
            if(peekBuffer().is(tok::identifier) || peekBuffer().is(tok::caret))
                return parseDeclarationStatement();
            goto PARSEEXP;
        case tok::lbrace: // TODO: lbrace as statement instead of expression?
        case tok::kw_if:
        case tok::kw_while:
        case tok::kw_for:
        case tok::kw_switch:
        case tok::intNum:
        case tok::floatNum:
        case tok::charstring:
PARSEEXP:
            return new ExpressionStatement(parseExpression());

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

    ASTType *type = parseType();

    Token t_id = get();
    assert(t_id.is(tok::identifier));
    assert(!getScope()->contains(t_id.toString()) && "redeclaration!");
    Identifier *id = getScope()->get(t_id.toString());

    //TODO parse decl specs


    if(peek().is(tok::lparen)) // function decl
    {
        vector<pair<ASTType*, std::string> > args;
        ignore(); // lparen 
        bool vararg = false;
        while(!peek().is(tok::rparen))
        {
            //Token at_type = get();
            if(peek().is(tok::dotdotdot))
            {
                vararg = true;
                ignore();
                assert(peek().is(tok::rparen) && "expected )");
                break;
            }

            ASTType *aty = parseType();
            Token t_name = get();
            args.push_back(pair<ASTType*, std::string>(aty, t_name.toString()));
            if(peek().is(tok::comma))
            {
                ignore();
                continue;
            } else
            {
                assert(peek().is(tok::rparen) && "expected , or )");
            }
        }
        ignore(); //rparen

        SymbolTable *funcScope = new SymbolTable(getScope());
        pushScope(funcScope);
        Statement *stmt = parseStatement();
        popScope();

        FunctionPrototype *proto = new FunctionPrototype(type, args, vararg);
        Declaration *decl = new FunctionDeclaration(id, proto, funcScope, stmt);
        id->setDeclaration(decl, Identifier::ID_FUNCTION);
        return decl;
    }


    Expression *defaultValue = NULL;
    if(peek().is(tok::equal))
    {
        ignore(); // ignore =
        defaultValue = parseExpression();
    }

    Declaration *decl = new VariableDeclaration(type, id, defaultValue);
    id->setDeclaration(decl, Identifier::ID_VARIABLE);

    //TODO: comma, multiple decl
    return decl;
}

Expression *ParseContext::parseExpression(int prec)
{
    return parseBinaryExpression(prec);
}

Expression *ParseContext::parsePostfixExpression(int prec)
{
    Expression *exp = parsePrimaryExpression();
    if((peek().getPostfixPrecidence()) > prec)
    {
        if(peek().is(tok::dot)) // member
        {
             
        } else if(peek().is(tok::lparen)) // call
        {
            ignore(); // ignore lparen
            vector<Expression*> args;
            while(peek().isNot(tok::rparen))
            {
                args.push_back(parseExpression(getBinaryPrecidence(tok::comma))); 
                assert(peek().is(tok::comma) || peek().is(tok::rparen) && "expected , or )");
                if(peek().is(tok::comma)) ignore(); // ignore comma or rparen
            }
            ignore(); // ignore rparen
            return new CallExpression(exp, args);
            //TODO: eat call 
        } else if(peek().is(tok::lbracket)) //index/slice
        {
            //TODO: eat index 
        } else {assert(false && "this doesn't look like a postfix expresion to me!");} //TODO: ++ and --
    }

    //TODO: parse postfix of postfix. eg somecall()++;
    return exp;
}

Expression *ParseContext::parseUnaryExpression(int prec)
{
    int op;
    int opPrec;
    if((opPrec = peek().getUnaryPrecidence()) > prec)
    {

        op = get().kind;
        return new UnaryExpression(op, parsePostfixExpression(opPrec));
    }

    return parsePostfixExpression(prec);
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

    if(peek().is(tok::identifier))
    {
        return new IdentifierExpression(getScope()->get(get().toString()));
    }
}

Expression *ParseContext::parseBinaryExpression(int prec)
{
    Expression *lhs = parseUnaryExpression(prec);

    int op;
    int opPrec;
    while((opPrec = peek().getBinaryPrecidence()) > prec)
    {
        op = peek().kind;
        ignore();
        Expression *rhs = parseBinaryExpression(opPrec);
        assert(rhs && "invalid binary op, expected RHS");
        lhs = new BinaryExpression(op, lhs, rhs); 
    }
    assert(lhs && "somethings up");

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
