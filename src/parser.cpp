
#include <cstdio>

#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include <clang-c/Index.h>

#include <fstream>
#include <vector>
#include <iostream>

#include "parser.hpp"
#include "streamLexer.hpp"
#include "ast.hpp"
#include "message.hpp"

using namespace llvm;
using namespace std;

// prepare for recovery if failed parsing
void ParseContext::pushRecover()
{
    recoveryState.push_back(rqueue.size());
}

// drop recovery info once not needed
void ParseContext::popRecover()
{
    recoveryState.pop_back();
    if(!recoveryState.size())
    {
        while(rqueue.size())
        {
            rqueue.pop_front();
        }
    }
}

// recover from failed parsing
void ParseContext::recover()
{
    int endpop = recoveryState.back();
    recoveryState.pop_back();
    while(rqueue.size() > endpop) 
    {
        tqueue.push_front(rqueue.back());
        rqueue.pop_back();
    }
}

ASTType *ParseContext::parseType(Expression **arrayInit)
{
    ASTType *type = NULL;
    bool ptr = false;
    Token t = get();
    //cond_message(t.isNot(tok::identifier) && !t.isKeywordType(), msg::FAILURE, "unrecognized type");
    if(t.isNot(tok::identifier) && !t.isKeywordType()) return NULL;
    if(t.isKeywordType())
    {
       switch(t.kind)
       {
       case tok::kw_var:
           type = ASTType::getDynamicTy(); break;
       case tok::kw_bool:
           type = ASTType::getBoolTy(); break;

        case tok::kw_int8:
        case tok::kw_char:
            type = ASTType::getCharTy(); break;
        case tok::kw_uint8:
        case tok::kw_uchar:
            type = ASTType::getUCharTy(); break;

        case tok::kw_int16:
        case tok::kw_short:
            type = ASTType::getShortTy(); break;
        case tok::kw_uint16:
        case tok::kw_ushort:
            type = ASTType::getUShortTy(); break;

        case tok::kw_int32:
        case tok::kw_int:
            type = ASTType::getIntTy(); break;
        case tok::kw_uint32:
        case tok::kw_uint:
            type = ASTType::getUIntTy(); break;

        case tok::kw_int64:
        case tok::kw_long:
            type = ASTType::getLongTy(); break;
        case tok::kw_uint64:
        case tok::kw_ulong:
            type = ASTType::getULongTy(); break;

        case tok::kw_float:
        case tok::kw_float32:
            type = ASTType::getFloatTy(); break;
        case tok::kw_float64:
        case tok::kw_double:
            type = ASTType::getDoubleTy(); break;
        case tok::kw_void:
            type = ASTType::getVoidTy(); break;
        default:
            emit_message(msg::UNIMPLEMENTED, "unparsed type");
       }
    } else {
        //TODO: should be get so structs dont need fwd decl?
        Identifier *id = getScope()->get(t.toString()); 
        if(!id) {
            emit_message(msg::ERROR, "unknown type or variable", t.loc);
            return NULL;
        }
        type = id->declaredType();
        //TODO: do something with id...
    }

    while(true)
    {
        if(peek().is(tok::caret))
        {
            ignore(); // eat ^
            if(type == ASTType::getVoidTy()) type = ASTType::getCharTy(); // cannot have void*, use char*
            type = type->getPointerTy();
            continue;
        }

        if(peek().is(tok::lbracket)) // XXX should this be parsed as part of the type, or as a postfix expression?
        {
            ignore(); // eat [
            type = type->getArrayTy();
            if(peek().isNot(tok::rbracket))
            {
                Expression *rsz = parseExpression(); //TODO: array size
                if(arrayInit) *arrayInit = rsz;
            }

            if(peek().isNot(tok::rbracket)) {
                emit_message(msg::ERROR, "expected ']'", peek().loc); 
                dropLine();
                return NULL;
            }
            ignore(); // eat ]
            continue;
        }

        break; // if no post-modifiers are found, exit loop
    }

    return type;
}


// parse top level expression and apply to translation unit
void ParseContext::parseTopLevel(TranslationUnit *unit)
{
    Statement *stmt;
    Declaration *decl;
    SourceLocation l = peek().getLocation();
    switch(peek().kind)
    {
        case tok::kw_import:
            //TODO: add to symbol table
            //TODO: find and import new translation unit
            unit->imports.push_back(parseImport());
            break;

        case tok::kw_include:
            parseInclude();

        case tok::kw_package:
            ignore();
            new PackageExpression(parseExpression(), l); //TODO: do something with expr
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
                emit_message(msg::FAILURE, "what sort of global statement did i just parse?");
            }
    }
}

void ParseContext::parseInclude()
{
    ignore();
    Expression *includeExpression = parseExpression();
    if(StringExpression *sexp = dynamic_cast<StringExpression*>(includeExpression))
    {
        ifstream stream(sexp->string.c_str());
        Lexer *incLexer = new StreamLexer(stream);
        Lexer *currentLexer = lexer;
        lexer = incLexer;
        //while(peek().isNot(tok::eof))
        //{
        //    parseTopLevel(unit); //TODO: support inlined includes (inside functions and stuff)
        //}
        //TODO: include not yet supported
        emit_message(msg::UNIMPLEMENTED, "include not yet impl");
        lexer = currentLexer;
        delete incLexer;
    } else 
    {
        emit_message(msg::ERROR, "unknown include directive");
    }
}

ImportExpression *ParseContext::parseImport()
{
    SourceLocation loc = peek().loc;

    ignore(); // ignore import
    
    bool special = false;
    std::string parserType;
    if(peek().is(tok::lparen)) // special import
    {
        ignore(); // ignore (
        special = true;
        Expression *specialImportType = parseExpression();
        if(!dynamic_cast<IdentifierExpression*>(specialImportType))
        {
            emit_message(msg::ERROR, "invalid special import expression", loc);
            dropLine();
            return NULL;
        }
        parserType = ((IdentifierExpression*)specialImportType)->getName();

        ignore(); // ignore )
    }

    Expression *importExpression = parseExpression();

    TranslationUnit *importedUnit = NULL;
    //XXX deffer importing to end of TU parse? Would allow 'package' expression and subsequent imports to work as expected, I imagine
    if(StringExpression *sexp = dynamic_cast<StringExpression*>(importExpression))
    { 
        importedUnit = parser->getAST()->getUnit(sexp->string);
        if(!importedUnit) // This TU hasnt been loaded from file yet, DOIT
        {
            importedUnit = new TranslationUnit(NULL, sexp->string);  //TODO: identifier
            parser->getAST()->addUnit(sexp->string, importedUnit);

            if(special)
            {
                if(parserType == "C")
                {
                    emit_message(msg::OUTPUT, "parsing C import", loc); 
                    parseCImport(importedUnit, sexp->string, loc);
                    emit_message(msg::OUTPUT, "finished C import", loc); 
                } else
                {
                    emit_message(msg::ERROR, "unknown parser type '" + parserType + "'", loc);
                }
            } else
            {
                parser->parseFile(importedUnit, sexp->string, loc);
            }
        }
    }

    cond_message(!importedUnit, msg::FAILURE, "failed to import translationUnit");

    unit->importUnits.push_back(importedUnit); //TODO: check if in import list first
    getScope()->addSibling(importedUnit->getScope());
    return new ImportExpression(importExpression, importedUnit, loc);
}

Statement *ParseContext::parseDeclarationStatement()
{
    Declaration *decl;

    SourceLocation loc = peek().getLocation();
    decl = parseDeclaration();

    //TODO: register decl
    return new DeclarationStatement(decl, loc);
}

Statement *ParseContext::parseStatement()
{
    int n = 1; //lookahead
    int ad = 0; //array depth
    SourceLocation loc = peek().getLocation();
    ASTType *declType = NULL;
    Identifier *id = NULL;
    switch(peek().kind)
    {
        case tok::lbracket:
        case tok::identifier:
            id = getScope()->lookup(peek().toString()); 
            if(!id || id->isUndeclared())
            {
                pushRecover();
                declType = parseType(NULL);
                if(declType && peek().is(tok::identifier)) //look ahead to see if decl
                {
                    recover();
                    // FALLTHROUGH TO DECL (yes, we reparse the type. oh well...)
                } else
                {
                    recover();
                    goto PARSEEXP; // this seems to be an expression?
                }
            } else if(id->isStruct()) //XXX Class
            {
                //FALLTHROUGH TO DECL
            } else goto PARSEEXP; //IF NOT UNDECLARED OR STRUCT, THIS IS PROBABLY AN EXPRESSION

        case tok::kw_extern:
        case tok::kw_struct:
        case tok::kw_var:
#define BTYPE(X,SZ,SN) case tok::kw_##X:
#define FTYPE(X,SZ) case tok::kw_##X:
#include "tokenkinds.def"
                return parseDeclarationStatement();

            /*
            if(lookAhead(n).is(tok::colon)) // cast
            {
                goto PARSEEXP;
            }
            if(lookAhead(n).is(tok::identifier)) // declaration
            {
                return parseDeclarationStatement();
            }*/
            goto PARSEEXP;
        case tok::lbrace: // TODO: lbrace as statement instead of expression?
        case tok::kw_import:
        case tok::kw_if:
        case tok::kw_while:
        case tok::kw_for:
        case tok::kw_switch:
        case tok::intNum:
        case tok::floatNum:
        case tok::kw_true:
        case tok::kw_false:
        case tok::kw_null:
        case tok::charstring:
        case tok::amp:
        case tok::caret:
        case tok::plusplus:
        case tok::minusminus:
        case tok::lparen:
PARSEEXP:
            return new ExpressionStatement(parseExpression(), loc);

        case tok::kw_return:
            ignore();
            if(!peek().followsNewline()) //TODO: newline thing
                return new ReturnStatement(parseExpression(), loc);
            return new ReturnStatement(NULL, loc); // does not parse past newline incase of return in if statement

        case tok::kw_label:
            ignore();
            if(peek().isNot(tok::identifier))
            {
                emit_message(msg::ERROR, 
                    "expected identifier following label", loc);
                dropLine();
                return NULL;
            }
            return new LabelStatement(getScope()->get(get().toString()), loc);

        case tok::kw_goto:
            ignore();
            if(peek().isNot(tok::identifier))
            {
                emit_message(msg::ERROR,
                    "expected identifier following goto", loc);
                dropLine();
                return NULL;
            }
            return new GotoStatement(getScope()->get(get().toString()), loc);

        case tok::kw_continue:
            ignore();
            return new ContinueStatement(loc);
            
        case tok::kw_break:
            ignore();
            return new BreakStatement(loc);

        case tok::semicolon:
            ignore();
        default:
            return NULL;
    }
}

Declaration *ParseContext::parseDeclaration()
{
    bool external = false;
    if(peek().is(tok::kw_extern)) { external = true; ignore(); }

    //TODO: parse function decl specs
    if(peek().is(tok::kw_struct)) //parse struct
    {
        ignore(); // eat "struct"

        Token t_id = get(); // eat ID
        if(t_id.isNot(tok::identifier)){
            emit_message(msg::ERROR, 
                "expected struct name following 'struct' keyword", t_id.loc);
            dropLine();
            return NULL;
        }

        if(getScope()->contains(t_id.toString()) && 
                !(getScope()->lookup(t_id.toString()))->isUndeclared()){
            emit_message(msg::ERROR, 
                std::string("redeclaration of struct ") + 
                string("'") + t_id.toString() + string("'"), t_id.loc);
            dropLine();
            return NULL;
        }

        Identifier *id = getScope()->get(t_id.toString());
        if(peek().isNot(tok::lbrace) && peek().isNot(tok::semicolon)){
            emit_message(msg::ERROR,
                "expected '{' following struct declarator", peek().loc);
            dropLine();
            return NULL;
        }

        SymbolTable *tbl = NULL;
        vector<Declaration*> members;
        if(peek().is(tok::lbrace))
        {
            ignore(); // eat lbrace
            tbl = new SymbolTable(getScope()); 
            pushScope(tbl);
            while(peek().isNot(tok::rbrace))
            {
                Declaration *d = parseDeclaration();
                members.push_back(d);
                while(peek().is(tok::semicolon)) ignore();
            }
            ignore(); //eat rbrace
            popScope();
        } else ignore(); // eat semicolon

        StructTypeInfo *sti = new StructTypeInfo(id, tbl, members); //TODO: use info
        StructDeclaration *sdecl = new StructDeclaration(id, NULL, members, t_id.loc);
        id->declaredType()->setTypeInfo(sti, TYPE_STRUCT);
        id->setDeclaration(sdecl, Identifier::ID_STRUCT);
        return sdecl;
        //return new TypeDeclaration();
    }

    Expression *arrayInit = 0;
    ASTType *type = parseType(&arrayInit);

    Token t_id = get();
    if(t_id.isNot(tok::identifier)){
        emit_message(msg::ERROR, 
            "expected identifier following type (declaration)", t_id.loc);
        dropLine();
        return NULL;
    }
    if(getScope()->contains(t_id.toString())){
        emit_message(msg::ERROR, 
            string("redeclaration of variable ") + string("'") + t_id.toString() + string("'"), 
            t_id.loc);
        dropLine();
        return NULL;
    }

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
                if(peek().isNot(tok::rparen)){
                    emit_message(msg::ERROR, 
                        "expected ) following vararg declaration ('...') ", peek().loc);
                    dropLine();
                    return NULL;
                }

                break;
            }

            ASTType *aty = parseType(NULL);
            Token t_name = get();
            args.push_back(pair<ASTType*, std::string>(aty, t_name.toString()));
            if(peek().is(tok::comma))
            {
                ignore();
                continue;
            } else
            {
                if(peek().isNot(tok::rparen)){
                    emit_message(msg::ERROR, 
                        "expected ',' or ')' in function declaration", peek().loc);
                    dropLine();
                    return NULL;
                }
            }
        }
        ignore(); //rparen

        SymbolTable *funcScope = new SymbolTable(getScope());
        pushScope(funcScope);
        Statement *stmt = parseStatement();
        popScope();

        FunctionPrototype *proto = new FunctionPrototype(type, args, vararg);
        Declaration *decl = new FunctionDeclaration(id, proto, funcScope, stmt, t_id.loc);
        id->setDeclaration(decl, Identifier::ID_FUNCTION);
        return decl;
    }

    Expression *defaultValue = NULL;
    if(peek().is(tok::equal))
    {
        ignore(); // ignore =
        defaultValue = parseExpression();
    } else if(type->type == TYPE_DYNAMIC)
    {
        emit_message(msg::ERROR, "dynamic type 'var' without initializer", t_id.loc);
    }

    if(ArrayTypeInfo *ati = dynamic_cast<ArrayTypeInfo*>(type->info))
    {
        Declaration *adecl = new ArrayDeclaration(type, id, defaultValue, arrayInit, t_id.loc, external);
        id->setDeclaration(adecl, Identifier::ID_VARIABLE);
        return adecl;
    }

    Declaration *decl = new VariableDeclaration(type, id, defaultValue, t_id.loc, external);
    id->setDeclaration(decl, Identifier::ID_VARIABLE);

    //TODO: comma, multiple decl
    return decl;
}

Expression *ParseContext::parseIfExpression()
{
    SourceLocation loc = peek().loc;
    Expression *cond = NULL;
    Statement *body = NULL;
    Statement *els = NULL;
    assert_message(peek().is(tok::kw_if), msg::ERROR, "expected 'if' keyword", peek().loc);
    ignore(); // ignore if

    if(peek().isNot(tok::lparen)){
        emit_message(msg::ERROR, 
            "expected '(' following 'if' keyword", peek().loc);
        dropLine();
        return NULL;
    }
    ignore();
    cond = parseExpression();
    if(peek().isNot(tok::rparen)){
        emit_message(msg::ERROR, "expected ')' following 'if' condition", 
            peek().loc);
        dropLine();
        return NULL;
    }
    ignore();

    body = parseStatement();
    if(peek().is(tok::kw_else))
    {
        ignore(); // else kw
        els = parseStatement();
    }

    return new IfExpression(cond, body, els,loc);
}

Expression *ParseContext::parseWhileExpression()
{
    SourceLocation loc = peek().loc;
    Expression *cond = NULL;
    Statement *body = NULL;
    Statement *els = NULL;
    assert_message(peek().is(tok::kw_while), msg::ERROR, "expected 'while' keyword", peek().loc);
    ignore(); // eat 'while'

    assert_message(peek().is(tok::lparen), msg::ERROR,
            "expected '(' following 'while' keyword", peek().loc);
    ignore(); // eat '('
    cond = parseExpression();
    assert_message(peek().is(tok::rparen), msg::ERROR, 
            "expected ')' following 'while' condition", peek().loc);
    ignore(); // eat ')'

    body = parseStatement();

    if(peek().is(tok::kw_else))
    {
        ignore(); // else kw
        els = parseStatement();
    }

    return new WhileExpression(cond, body, els, loc);
}

Expression *ParseContext::parseForExpression()
{
    SourceLocation loc = peek().loc;
    Statement *decl = NULL;
    Expression *cond = NULL;
    Statement *upd = NULL;
    Statement *body = NULL;
    Statement *els = NULL;

    if(!peek().is(tok::kw_for)) {
        emit_message(msg::ERROR, "expected 'for' keyword", peek().loc);
        return NULL;
    }
    ignore(); // ignore for

    if(!peek().is(tok::lparen)) {
    emit_message(msg::ERROR, 
            "expected '(' following 'for' keyword", peek().loc);
        return NULL;
    }
    ignore(); // eat '('

    decl = parseStatement();
    if(!peek().is(tok::semicolon)) {
        emit_message(msg::ERROR, "expected ';' following 'for' declaration", peek().loc);
        return NULL;
    }
    ignore(); // eat ';'

    cond = parseExpression();
    if(!peek().is(tok::semicolon)) {
        emit_message(msg::ERROR, "expected ';' following for condition", peek().loc);
        return NULL;
    }
    ignore(); // eat ';'

    upd = parseStatement();
    if(!peek().is(tok::rparen)) {
        emit_message(msg::ERROR, "expected ')', following 'for' update statement", peek().loc);
        return NULL;
    }
    ignore(); // eat ')'

    body = parseStatement();

    if(peek().is(tok::kw_else))
    {
        ignore();
        els = parseStatement();
    }

    return new ForExpression(decl, cond, upd, body, els, loc);
}

Expression *ParseContext::parseSwitchExpression()
{
    if(peek().isNot(tok::kw_switch)) {
        emit_message(msg::FAILURE, "expected 'switch' keyword", peek().loc);
        return NULL;
    }
    ignore(); // eat 'switch'

    if(peek().isNot(tok::lparen)){
        emit_message(msg::FAILURE, 
            "expected '(' following 'switch' keyword", peek().loc);
        return NULL;
    }
    ignore(); // eat '('

    Expression *exp = parseExpression();

    if(peek().isNot(tok::rparen)){
        emit_message(msg::FAILURE, 
            "expected ')' following 'switch' condition", peek().loc);
        return NULL;
    }
    ignore(); // eat ')'

    emit_message(msg::UNIMPLEMENTED, "switch keyword unimplemented", peek().loc);
    return NULL; //TODO: parse body
}

Expression *ParseContext::parseCastExpression(int prec)
{
    SourceLocation loc = peek().loc;
    ASTType *type = parseType(NULL);
    if(!peek().is(tok::colon)) {
        emit_message(msg::ERROR, "expected colon for cast operator!", peek().loc);
        dropLine();
        return NULL;
    }
    ignore(); // eat ':'
    return new CastExpression(type, parseExpression(12), loc); // 12 == cast priority
}

Expression *ParseContext::parseExpression(int prec)
{
    int n = 1; //look ahead
    while(lookAhead(n).is(tok::caret))
        n++;
    if(lookAhead(n).is(tok::colon))
    {
        return parseCastExpression(prec);
    }

    switch(peek().kind)
    {
        case tok::kw_if:
            return parseIfExpression();
        case tok::kw_while:
            return parseWhileExpression();
        case tok::kw_for:
            return parseForExpression();
        case tok::kw_switch:
            return parseSwitchExpression();
        case tok::kw_import:
            return parseImport();
        case tok::lbracket:
            return parseTupleExpression();
        default:
        return parseBinaryExpression(prec);
    }
}

Expression *ParseContext::parseTupleExpression()
{
    if(peek().isNot(tok::lbracket))
    {
        emit_message(msg::ERROR, "expected tuple expression", peek().loc);
        dropLine();
        return NULL;
    }
    
    ignore();

    std::vector<Expression*> members;
    while(peek().isNot(tok::rbracket))
    {
        members.push_back(parseExpression()); 

        if(peek().isNot(tok::comma) && peek().isNot(tok::rbracket))
        {
            emit_message(msg::ERROR, "expected ',' or ']' delimitting tuple expression", peek().loc);
            dropLine();
            return NULL;
        }

        if(peek().is(tok::comma)) ignore(); //ignore comma
    }

    ignore(); // ignore ]

    return new TupleExpression(members);
}

Expression *ParseContext::parsePostfixExpression(int prec)
{
    SourceLocation loc = peek().loc;
    Expression *exp = parsePrimaryExpression();
    while((peek().getPostfixPrecidence()) > prec)
    {
        if(peek().is(tok::lparen)) // call
        {
            ignore(); // ignore lparen
            vector<Expression*> args;
            while(peek().isNot(tok::rparen))
            {
                args.push_back(parseExpression(getBinaryPrecidence(tok::comma)));
                if(!peek().is(tok::comma) && !peek().is(tok::rparen)) {
                    emit_message(msg::ERROR, 
                            "expected ',' or ')' following call parameter", peek().loc);
                    return NULL;
                }
                if(peek().is(tok::comma)) ignore(); // ignore comma or rparen
            }
            ignore(); // ignore rparen
            exp = new CallExpression(exp, args, loc);
        } else if(peek().is(tok::lbracket)) //index/slice
        {
            ignore();
            Expression *index = parseExpression();
            if(!peek().is(tok::rbracket)) { 
                emit_message(msg::ERROR, "expected ']' following index expression", peek().loc);
                return NULL;
            }
            ignore();
            exp = new IndexExpression(exp, index, loc);
        } else if(peek().is(tok::dot))
        {
            ignore(); //ignore .
            exp = new DotExpression(exp, get().toString(), loc);
        }
        else if(peek().is(tok::plusplus))
        {
            ignore(); // ignore ++
            exp = new PostfixOpExpression(exp, tok::plusplus, loc);
        } else if(peek().is(tok::minusminus))
        {
            ignore(); // ignore --
            exp = new PostfixOpExpression(exp, tok::minusminus, loc);
        } else 
            emit_message(msg::FAILURE, 
                    "this doesn't look like a postfix expresion to me!", 
                    peek().loc);
    }

    //TODO: parse postfix of postfix. eg somecall()++;
    return exp;
}

Expression *ParseContext::parseUnaryExpression(int prec)
{
    SourceLocation loc = peek().loc;
    int op;
    int opPrec;
    if((opPrec = peek().getUnaryPrecidence()) > prec)
    {
        op = get().kind;
        return new UnaryExpression(op, parseUnaryExpression(opPrec), loc);
    }

    return parsePostfixExpression(prec);
}

Expression *ParseContext::parsePrimaryExpression()
{
    SourceLocation loc = peek().loc;
    if(peek().is(tok::charstring))
    {
        return new StringExpression(get().stringData(), loc);
    }

    if(peek().is(tok::intNum))
    {
        return new NumericExpression(NumericExpression::INT, ASTType::getIntTy(), 
                get().intData(), loc);
    }

    if(peek().is(tok::floatNum))
    {
        return new NumericExpression(NumericExpression::DOUBLE, ASTType::getDoubleTy(),
                get().floatData(), loc);
    }

    if(peek().is(tok::kw_true))
    {
        ignore();
        return new NumericExpression(NumericExpression::INT, ASTType::getBoolTy(), 
                (uint64_t) 1L, loc); //TODO: bool type
    }
    if(peek().is(tok::kw_false))
    {
        ignore();
        return new NumericExpression(NumericExpression::INT, ASTType::getBoolTy(), 
                (uint64_t) 0L, loc); //TODO: bool
    }
    if(peek().is(tok::kw_null))
    {
        ignore();
        return new NumericExpression(NumericExpression::INT, ASTType::getVoidTy()->getPointerTy(), 
                (uint64_t) 0L, loc); //TODO: void^
    }

    //TODO: eat char constant

    if(peek().is(tok::lparen)) // paren expression
    {
        ignore();
        Expression *exp = parseExpression();
        if(peek().isNot(tok::rparen)) {
            emit_message(msg::ERROR, "Expected ')' following parenthesised expression", peek().loc);
            return NULL;
        }
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
        return new BlockExpression(stmts, loc);
    }

    if(peek().is(tok::identifier))
    {
        Identifier *id = getScope()->get(get().toString());
        return new IdentifierExpression(id, loc);
    }
}

Expression *ParseContext::parseBinaryExpression(int prec)
{
    SourceLocation lhsLoc = peek().loc;
    Expression *lhs = parseUnaryExpression(prec);

    int op;
    int opPrec;
    while((opPrec = linePeek().getBinaryPrecidence()) > prec)
    {
        op = linePeek().kind;
        ignore();
        SourceLocation rhsLoc = peek().loc;
        Expression *rhs = parseExpression(opPrec);
        if(!rhs) {
            emit_message(msg::FAILURE, "invalid binary op, expected RHS", rhsLoc);
            return NULL;
        }
        lhs = new BinaryExpression(op, lhs, rhs, lhsLoc);
    }
    //assert(lhs && "somethings up");
    if(!lhs) {
        emit_message(msg::FAILURE, "LHS of binary expression may not be valid expression", lhsLoc);
        return NULL;
    }

    return lhs;
}

void ParseContext::parseTranslationUnit(TranslationUnit *unit, const char *unitnm)
{
    //TranslationUnit *unit = parser->getAST()->getUnit(unitnm);
    //TranslationUnit *unit = new TranslationUnit(currentPackage()->getScope()->lookup(unitnm)); //TODO: identifier
    this->unit = unit;
    currentPackage()->addPackage(unit);

    pushScope(unit->getScope());
    while(peek().isNot(tok::eof))
    {
        parseTopLevel(unit); // modifies t-unit
    }
    popScope();
    assert_message(!getScope(), msg::FAILURE, "invalid scope stack!", peek().loc);
}

void Parser::parseFile(TranslationUnit *unit, std::string filenm, SourceLocation l)
{
    ifstream stream(filenm.c_str());
    
    if(stream.fail())
    {
        emit_message(msg::ERROR, "unknown file '" + filenm + "'", l);
        return;
    }

    Lexer *lexer = new StreamLexer(stream);
    lexer->setFilename(filenm.c_str());
    ParseContext context(lexer, this, ast->getRootPackage());
    context.parseTranslationUnit(unit, filenm.c_str()); //TODO: identifier
    delete lexer;

    //ast->getRootPackage()->addPackage("", unit);
    //ast->units[filenm] = unit; //TODO: symbol table

}


/// C Parsing fun stuff

ASTType *ASTStructTypeFromCType(TranslationUnit *unit, CXType ctype)
{
    vector<Declaration*> members;
    SymbolTable *tbl = new SymbolTable(unit->getScope());
    CXString cxname = clang_getTypeSpelling(ctype);
    string name = clang_getCString(cxname);
    string sstruct = "struct ";
    if(name.compare(0, sstruct.length(), sstruct) == 0) //remove 'struct' keyword
    {
        name = name.substr(sstruct.length());
    }

    Identifier *id = unit->getScope()->lookup(name);
    
    if(!id)
    {
        id = unit->getScope()->get(name);
        StructTypeInfo *sti = new StructTypeInfo(id, tbl, members);
        StructDeclaration *sdecl = new StructDeclaration(id, NULL, members, SourceLocation());
        id->declaredType()->setTypeInfo(sti, TYPE_STRUCT);
        id->setDeclaration(sdecl, Identifier::ID_STRUCT);
    }

    return id->declaredType();
}


ASTType *ASTTypeFromCType(TranslationUnit *unit, CXType ctype)
{
    ASTType *ty = 0;
    switch(ctype.kind)
    {
        case CXType_Void: 
        case CXType_Unexposed: //TODO: create a specific type for unexposed?
            return ASTType::getVoidTy();
        case CXType_Bool: return ASTType::getBoolTy();
        case CXType_Char_U:
        case CXType_UChar: return ASTType::getUCharTy();
        case CXType_Char_S:
        case CXType_SChar: return ASTType::getCharTy();
        case CXType_UShort: return ASTType::getUShortTy();
        case CXType_Short: return ASTType::getShortTy();
        case CXType_UInt: return ASTType::getUIntTy();
        case CXType_Int: return ASTType::getIntTy();
        case CXType_Long: 
        case CXType_LongLong: 
            return ASTType::getLongTy();
        case CXType_ULong: 
        case CXType_ULongLong: 
            return ASTType::getULongTy();
        case CXType_Float:
            return ASTType::getFloatTy();
        case CXType_Double:
            return ASTType::getDoubleTy();
        case CXType_Pointer:
            ty = ASTTypeFromCType(unit, clang_getPointeeType(ctype));
            if(ty) return ty->getPointerTy();
            return NULL;
        case CXType_Record:
            return ASTStructTypeFromCType(unit, ctype);
        case CXType_Typedef:
            return ASTTypeFromCType(unit, clang_getCanonicalType(ctype));
        default:
            emit_message(msg::WARNING, "failed conversion of CType to WLType: " + 
                    string(clang_getCString(clang_getTypeSpelling(ctype))));
            return NULL;
    }
}

CXChildVisitResult CVisitor(CXCursor cursor, CXCursor parent, void *tUnit)
{
    TranslationUnit* unit = (TranslationUnit *) tUnit;

    if(cursor.kind == CXCursor_FunctionDecl) 
    {
        CXType fType = clang_getCursorType(cursor);
        int nargs = clang_getNumArgTypes(fType);
        vector<pair<ASTType*, std::string> > argType;
        for(int i = 0; i < nargs; i++)
        {
            ASTType *astArgTy = 
                    ASTTypeFromCType(unit, clang_getArgType(fType, i));
            if(!astArgTy) goto ERR;

            argType.push_back( pair<ASTType*, std::string>(
                        astArgTy, "")
                    );
        }

        CXString cxname = clang_getCursorSpelling(cursor);
        std::string name = clang_getCString(cxname);
        ASTType *rType = ASTTypeFromCType(unit, clang_getResultType(fType));

        if(!rType) goto ERR;

        Identifier *id = unit->getScope()->get(name);
        FunctionPrototype *proto = new FunctionPrototype(rType, argType,
                clang_isFunctionTypeVariadic(fType));
        FunctionDeclaration *fdecl = new FunctionDeclaration(id, proto, 0, 0, SourceLocation());
        id->setDeclaration(fdecl, Identifier::ID_FUNCTION);

        unit->functions.push_back(fdecl);

        printf("c function decl! '%s'\n", clang_getCString(cxname));
    }

    return CXChildVisit_Continue;
ERR:
    emit_message(msg::WARNING, "failed to convert function to WL typing: " + 
            string(clang_getCString(clang_getCursorSpelling(cursor))));
    return CXChildVisit_Continue;
}

void ParseContext::parseCImport(TranslationUnit *unit, 
        std::string filenm, 
        SourceLocation loc)
{
    const char *commandArgs[] = {
        "-internal-isystem /usr/include",
        "-c-isystem /usr/include",
        "-v",
        0,
    };

    CXIndex Idx = clang_createIndex(1,1);
    CXTranslationUnit Unit = clang_createTranslationUnitFromSourceFile(
            Idx,
            filenm.c_str(),
            3,
            commandArgs,
            0, 
            0);

    emit_message(msg::OUTPUT, "finished clang unit-create. begin visit");

    clang_visitChildren(clang_getTranslationUnitCursor(Unit), CVisitor, unit);
}
