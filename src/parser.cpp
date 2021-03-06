
#include <cstdio>
#include <fcntl.h>

#ifdef WIN32
#else
#include <unistd.h>
#endif

#include <fstream>
#include <vector>
#include <iostream>

#include "parser.hpp"
#include "streamLexer.hpp"
#include "ast.hpp"
#include "message.hpp"
#include "parsec.hpp"


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

ASTType *ParseContext::parseBasicType() {
    Token t = get();
   switch(t.kind) {
   case tok::kw_var:
       return ASTType::getDynamicTy();
   case tok::kw_bool:
       return ASTType::getBoolTy();

    case tok::kw_int8:
    case tok::kw_char:
        return ASTType::getCharTy();
    case tok::kw_uint8:
    case tok::kw_uchar:
        return ASTType::getUCharTy();

    case tok::kw_int16:
    case tok::kw_short:
        return ASTType::getShortTy();
    case tok::kw_uint16:
    case tok::kw_ushort:
        return ASTType::getUShortTy();

    case tok::kw_int32:
    case tok::kw_int:
        return ASTType::getIntTy();
    case tok::kw_uint32:
    case tok::kw_uint:
        return ASTType::getUIntTy();

    case tok::kw_int64:
    case tok::kw_long:
        return ASTType::getLongTy();
    case tok::kw_uint64:
    case tok::kw_ulong:
        return ASTType::getULongTy();

    case tok::kw_float:
    case tok::kw_float32:
        return ASTType::getFloatTy();
    case tok::kw_float64:
    case tok::kw_double:
        return ASTType::getDoubleTy();
    case tok::kw_void:
        return ASTType::getVoidTy();
    default:
        emit_message(msg::UNIMPLEMENTED, "unparsed type");
   }
   return NULL;
}

/**
 * A scalar type is any type excluding pointers and arrays.
 * This may be a little misleading because 'scalar' in this context
 * includes structs and tuples. This is to allow handling of complex
 * new expressions like 'new int[i + 5]'; the NewExpression needs to be
 * aware of the variable size 'i+5' and arrayTypes have no concept of
 * exact variable length
 */
ASTType *ParseContext::parseScalarType() {
    Token t = peek();

    // basic keyword type
    if(t.isKeywordType()) {
        return parseBasicType();
    }

    // tuple type
    if(t.is(tok::lbracket)) {
        std::vector<ASTType*> tupleTypes;
        ignore(); //ignore [
        while(true) //break below
        {
            ASTType *tupleMember = parseType();
            if(!tupleMember) {
                return NULL; //this is (probably) not a type, it is an expression
                //emit_message(msg::ERROR, "expected subtype while parsing tuple type", peek().loc);
            }
            tupleTypes.push_back(tupleMember);
            if(peek().is(tok::rbracket)) break;
            if(peek().is(tok::comma)) ignore();
            else {
                emit_message(msg::ERROR, "expected comma in tuple type/expression", peek().loc);
                return NULL;
            }
        }
        ignore(); // eat ]

        return ASTType::getTupleTy(tupleTypes);
    }

    if(t.is(tok::identifier)) {
        ignore();
        //TODO: should be get so structs dont need fwd decl?
        Identifier *id = getScope()->get(t.toString());
        if(!id) {
            emit_message(msg::ERROR, "unknown type or variable", t.loc);
            return NULL;
        }
        return id->getDeclaredType();
        //TODO: do something with id...
    }
    return NULL;
}

ASTType *ParseContext::parsePointerModifiedType(ASTType *base) {
    if(peek().is(tok::caret)) {
        ignore(); // eat ^
        return base->getPointerTy();
    }
    return base;
}

ASTType *ParseContext::parseArrayModifiedType(ASTType *base) {
    if(peek().is(tok::lbracket)) {
        ASTType *type = base;
        ignore(); // eat [

        // contains index; statically sized
        if(peek().isNot(tok::rbracket)) {
            Expression *arrsz = parseExpression();

            /*
            if(!arrsz->isConstant()) {
                emit_message(msg::ERROR, "array size must be constant", peek().loc);
                return NULL;
            } */

            if(!arrsz->getType()->coercesTo(ASTType::getIntTy())) {
                emit_message(msg::ERROR, "array size must be an integer value", peek().loc);
                return NULL;
            }

            type = type->getArrayTy(arrsz);
        } else { // no index, dynamically sized
            type = type->getArrayTy();
        }

        if(peek().isNot(tok::rbracket)) {
            emit_message(msg::ERROR, "expected ']'", peek().loc);
            dropLine();
            return NULL;
        }
        ignore(); // eat ]
        return type;
    }

    return base;
}

ASTType *ParseContext::parseFunctionModifiedType(ASTType *base) {
    if(peek().is(tok::kw_function)) {
        ASTType *ret = base;
        ignore(); // eat 'function'
        if(peek().isNot(tok::lparen)) {
            emit_message(msg::ERROR, "expected '(' in function pointer type specification");
            return NULL;
        }
        ignore(); // eat '('

        std::vector<ASTType*> params;
        while(peek().isNot(tok::rparen)){
            ASTType *param = parseType();
            params.push_back(param);

            if(peek().is(tok::comma)) {
                ignore();
            } else if(peek().is(tok::rparen)) {
                break;
            } else {
                emit_message(msg::ERROR, "expected ',' or ')' in function type specification");
                return NULL;
            } // TODO: vararg
        }

        ignore(); //eat ')' of function type

        return ASTType::getFunctionTy(ret, params)->getPointerTy(); // 'function' is a pointer
    }

    return base;
}

ASTType *ParseContext::parseModifiedType(ASTType *base) {
    ASTType *type = base;
    while(true)
    {
        if(peek().is(tok::caret)) {
            type = parsePointerModifiedType(type);
            continue;
        }

        if(peek().is(tok::lbracket)) {
            type = parseArrayModifiedType(type);
            continue;
        }

        if(peek().is(tok::kw_function)) {
            type = parseFunctionModifiedType(type);
            continue;
        }

        break; // if no post-modifiers are found, exit loop
    }

    return type;

}

ASTType *ParseContext::parseType() {
    ASTType *type = parseScalarType();
    return parseModifiedType(type);
}

// parse top level expression and apply to module
void ParseContext::parseTopLevel(ModuleDeclaration *module)
{
    Declaration *decl;
    SourceLocation l = peek().getLocation();
    switch(peek().kind)
    {
        //XXX also found in 'parseExpression'
        case tok::kw_import:
            //TODO: find and import new translation unit
            //module->imports.push_back(parseImport());
            parseImport();
            //TODO: use the 'importExpression' returned from 'parseImport'
            break;

        case tok::kw_use:
            parseUseExpression();
            break;

        case tok::kw_include:
            parseInclude();
            break;

        case tok::kw_package:
            ignore();
            new PackageExpression(parseExpression(), l); //TODO: do something with expr
            break;

        case tok::semicolon:
            ignore();
            break;

        default:
            decl = parseDeclaration();
            if(!decl)
            {
                emit_message(msg::FAILURE, "invalid global statement, expected global, function declaration");
            }
    }
}

TopLevelExpression *ParseContext::parseTopLevelExpression()
{
    switch(peek().kind)
    {
        default: break;
        //XXX
    }
	return NULL;
}

UseExpression *ParseContext::parseUseExpression()
{
    if(peek().isNot(tok::kw_use))
    {
        emit_message(msg::FAILURE, "expected 'use' keyword", peek().loc);
        return NULL;
    }
    ignore();

    if(peek().isNot(tok::charstring))
    {
        emit_message(msg::ERROR, "expected string following 'use' keyword", peek().loc);
        return NULL;
    }

    std::string extname = get().stringData();
    getScope()->enableExtension(extname);
    return new UseExpression();
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

    if(peek().isNot(tok::kw_import)){
        emit_message(msg::FAILURE, "expected 'import' keyword", peek().loc);
        return NULL;
    }

    ignore(); // ignore import

    bool special = false;
    std::string parserType;
    if(peek().is(tok::lparen)) { // special import
        ignore(); // ignore (
        special = true;
        if(peek().isNot(tok::identifier)){
            emit_message(msg::ERROR, "expected identifier as special import specifier", loc);
        }

        parserType = get().toString();

        ignore(); // ignore )
    }

    Expression *importExpression = parseExpression();

    ModuleDeclaration *importedModule = NULL;
    AST *ast = parser->getAST();
    //XXX deffer importing to end of TU parse? Would allow 'package' expression and subsequent imports to work as expected, I imagine
    if(StringExpression *sexp = dynamic_cast<StringExpression*>(importExpression)) {
        std::string filenm = sexp->string;
        std::string modnm = getFilebase(filenm);
        importedModule = ast->getModule(filenm);
        if(!importedModule) { // This TU hasnt been loaded from file yet, DOIT
            Identifier *m_id = getModule()->importScope->get(modnm);

            if(special) {
                if(getScope()->extensionEnabled("importc") && parserType == "C") {
                    static ModuleDeclaration *CModule = NULL;
                    if(!CModule) {
                        Identifier *c_id = getModule()->importScope->get("__C");
                        CModule = new ModuleDeclaration(ast->getRootPackage(), c_id, "__C");
                        c_id->addDeclaration(CModule, Identifier::ID_MODULE);
                    }
                    importedModule = CModule;
                    m_id->addDeclaration(importedModule, Identifier::ID_MODULE);
                    ast->addModule(sexp->string, importedModule);
                    parseCImport(importedModule, sexp->string, loc);
                } else {
                    emit_message(msg::ERROR, "unknown import type '" + parserType + "'", loc);
                }
            } else {
                importedModule = new ModuleDeclaration(ast->getRootPackage(), m_id, sexp->string);
                m_id->addDeclaration(importedModule, Identifier::ID_MODULE);
                ast->addModule(sexp->string, importedModule);
                parser->parseFile(importedModule, new File(sexp->string), loc);
            }
        }

        //XXX add a local identifier to refer to module
        //Identifier *local_mid = getScope()->getInScope(modnm);
        //local_mid->addDeclaration(importedModule, Identifier::ID_MODULE);
    }

    cond_message(!importedModule, msg::FAILURE, "failed to import module");

    getScope()->addSibling(importedModule->getScope());
    return new ImportExpression(importExpression, importedModule, loc);
}

Statement *ParseContext::parseStatement()
{
    SourceLocation loc = peek().getLocation();
    ASTType *declType = NULL;
    Identifier *id = NULL;
    switch(peek().kind)
    {
        case tok::identifier:
            id = getScope()->lookup(peek().toString());
            if(id && id->isVariable()) goto PARSEEXP;
        case tok::lbracket:
                pushRecover();
                declType = parseType();
                if(declType && peek().is(tok::identifier)) //look ahead to see if decl
                {
                    recover();
                    // FALLTHROUGH TO DECL (yes, we reparse the type. oh well...)
                } else
                {
                    recover();
                    goto PARSEEXP; // this seems to be an expression?
                }

        case tok::kw_extern:
        case tok::kw_implicit: //implicit constructor/conversion
        case tok::kw_undecorated:
        case tok::kw_const:
        case tok::kw_weak:
        case tok::kw_static:
        case tok::kw_union:
        case tok::kw_class:
        case tok::kw_struct:
        case tok::kw_interface:
        case tok::kw_var:
#define BTYPE(X,SZ,SN) case tok::kw_##X:
#define FTYPE(X,SZ) case tok::kw_##X:
#include "tokenkinds.def"
                return parseDeclaration();

            /*
            if(lookAhead(n).is(tok::colon)) // cast
            {
                goto PARSEEXP;
            }
            if(lookAhead(n).is(tok::identifier)) // declaration
            {
                return parseDeclaration();
            }*/
            goto PARSEEXP;

        case tok::kw_this:
        case tok::kw_import:
        case tok::kw_pack:
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
            return parseExpression();

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
            id = getScope()->get(get().toString());
            id->addDeclaration(NULL, Identifier::ID_LABEL); // set as label; XXX do we need decl?
            return new LabelStatement(id, loc);

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

        case tok::kw_case:
            return parseCaseStatement();

        case tok::semicolon:
            ignore();
            return NULL;
        case tok::kw_if:
            return parseIfStatement();
        case tok::kw_while:
            return parseWhileStatement();
        case tok::kw_for:
            return parseForStatement();
        case tok::kw_switch:
            return parseSwitchStatement();
        case tok::lbrace: // TODO: lbrace as statement instead of expression?
            return parseCompoundStatement();
        default:
            return parseExpression();
    }
}

DeclarationQualifier ParseContext::parseDeclarationQualifier()
{
    DeclarationQualifier dq = DeclarationQualifier();

    while(true) {
        if(peek().is(tok::kw_extern)){
            dq.external = true;
            ignore();
            continue;
        }
        if(peek().is(tok::kw_undecorated)){
            dq.decorated = false;
            ignore();
            continue;
        }
        if(peek().is(tok::kw_weak)) {
            dq.weak = true;
            ignore();
            continue;
        }
        if(peek().is(tok::kw_implicit)){
            dq.implicit = true;
            ignore();
            continue;
        }
        if(peek().is(tok::kw_const)) {
            dq.isConst = true;
            ignore();
            continue;
        }

        if(peek().is(tok::kw_static)) {
            dq.isStatic = true;
            ignore();
            continue;
        }

        break; //if no more qualifiers, exit loop
    }

    return dq;
}

//TODO: split up into seperate parts? parse class, func, type, etc
Declaration *ParseContext::parseDeclaration()
{
    SourceLocation loc = peek().loc;

    //TODO: check for decl quals on labels. they are meaningless; should be syntax error
    DeclarationQualifier dqual = parseDeclarationQualifier();

    //TODO: parse function decl specs
    if(peek().is(tok::kw_struct) || peek().is(tok::kw_union) ||
            peek().is(tok::kw_class) || peek().is(tok::kw_interface)) //parse struct
    {
        TokenKind kind = peek().kind;
        ignore(); // eat "struct" etc

        Token t_id = get(); // eat ID
        if(t_id.isNot(tok::identifier)) {
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
        Identifier *baseId = NULL;

        if(peek().is(tok::colon)) {
            ignore(); // eat ':'
            if(kind != kw_class) {
                emit_message(msg::ERROR, "only classes can inherit from a base", peek().loc);
            }

            baseId = getScope()->get(get().toString());
        } else if(kind == kw_class && id->getName() != "Object") { //TODO: inherits void
            static Identifier *objectId = NULL;
            if(!objectId) objectId = getAST()->getRuntimeModule()->lookup("Object");
            if(!objectId) {
                emit_message(msg::FAILURE, "runtime package not found; could not find an 'Object' declaration");
            }

            baseId = objectId;
        }

        if(peek().isNot(tok::lbrace) && peek().isNot(tok::semicolon)) {
            emit_message(msg::ERROR,
                "expected '{' following struct declarator", peek().loc);
            dropLine();
            return NULL;
        }

        ASTScope *tbl = new ASTScope(getScope(), ASTScope::Scope_Struct);
        tbl->setOwner(id);
        pushScope(tbl);

        UserTypeDeclaration *sdecl = NULL;

        switch(kind) {
            case kw_union:
                sdecl = new UnionDeclaration(id, tbl, loc, dqual);
                break;
            case kw_struct:
                sdecl = new StructDeclaration(id, tbl, loc, dqual);
                break;
            case kw_class:
                sdecl = new ClassDeclaration(id, tbl, baseId, loc, dqual);
                break;
            case kw_interface:
                sdecl = new InterfaceDeclaration(id, tbl, loc, dqual);
                break;
            default:
                emit_message(msg::FAILURE, "unknown declaration kind", loc);
        }

        if(peek().is(tok::lbrace)) {
            ignore(); // eat lbrace
            while(peek().isNot(tok::rbrace)) {
                Declaration *d = parseDeclaration();

                if(FunctionDeclaration *fdecl = d->functionDeclaration()) {
                    if(kind == kw_interface && d->functionDeclaration()->body) {
                        emit_message(msg::ERROR, "interface methods should not have associated bodies", loc);
                    }

                    if(d->isConstructor()) {
                        sdecl->addConstructor(fdecl);
                        goto NEXT;
                    }

                    if(d->isDestructor()) {
                        if(sdecl->destructor) {
                            emit_message(msg::ERROR, "destructor can not be overloadeded", d->loc);
                        }
                        if(d->functionDeclaration()->maxExpectedParameters() > 0) {
                            emit_message(msg::ERROR, "destructor should not have any arguments", d->loc);
                        }
                        sdecl->setDestructor(fdecl);
                        goto NEXT;
                    }

                    sdecl->addMethod(fdecl);
                } else { // is variable declaration (probably... *TODO)
                    if(kind == kw_interface) {
                        emit_message(msg::ERROR, "interfaces cannot contain variable members, methods only", loc);
                    } else {
                        if(d->isStatic()) {
                            sdecl->addStaticMember(d);
                        } else {
                            sdecl->addMember(d);
                        }
                    }
                }

NEXT:
                while(peek().is(tok::semicolon)) ignore();
            }

            if(peek().isNot(tok::rbrace)) {
                emit_message(msg::ERROR, "expected right brace following user type declaration", loc);
                popScope();
                return NULL;
            }
            ignore(); //eat rbrace
        } else {
            if(peek().isNot(tok::semicolon)) {
                emit_message(msg::ERROR, "expected '{' or ';' following user type declaration identifier", loc);
                popScope();
                return NULL;
            }
            ignore(); // eat semicolon
        }

        popScope();

        return sdecl;
    }

    ASTType *type; // return type
    Token t_id;
    Identifier *id;
    SourceLocation idLoc;

    // this is a bit messy
    // if we see the 'this' keyword, should be a constructor. jump to parsing a function
    if(peek().is(tok::kw_this)) {
        if(!getScope()->isUserTypeScope()) {
            emit_message(msg::ERROR, "'this' keyword found outside of user type scope", peek().loc);
        }
        idLoc = peek().loc;
        ignore(); // ignore 'this'
        type = ASTType::getVoidTy();
        id = getScope()->getInScope("this");
        goto PARSEFUNC;
    }

    // same as above; destructor
    if(peek().is(tok::tilde)) {
        ignore();
        if(peek().isNot(tok::kw_this)) {
            emit_message(msg::ERROR, "expected 'this' keyword in declaration following '~'", peek().loc);
        }
        idLoc = peek().loc;
        ignore(); // ignore 'this'
        type = ASTType::getVoidTy();
        id = getScope()->getInScope("~this");
        goto PARSEFUNC;
    }

    type = parseType();

    t_id = get();
    idLoc = t_id.loc;
    if(t_id.isNot(tok::identifier)){
        emit_message(msg::ERROR,
            "expected identifier following type (declaration)", t_id.loc);

        dropLine();
        return NULL;
    }

    // this identifier has already been defined! if lparen is upcoming, this is a function, give it a free pass to allow for overloading
    if(getScope()->lookupInScope(t_id.toString()) && peek().isNot(tok::lparen)){
        emit_message(msg::ERROR,
            string("redeclaration of variable ") + string("'") + t_id.toString() + string("'"),
            t_id.loc);
        dropLine();
        return NULL;
    }

    id = getScope()->getInScope(t_id.toString());
    if(id->getName() == "main") dqual.decorated = false; // dont mangle main

    //TODO parse decl specs

PARSEFUNC:
    if(peek().is(tok::lparen)) // function decl
    {
        Identifier *owner = NULL;
        std::vector<VariableDeclaration*> parameters;
        ignore(); // lparen

        if(getScope()->owner && getScope()->owner->getDeclaredType()) {
            owner = getScope()->owner;
        }

        ASTScope *funcScope = new ASTScope(getScope());
        pushScope(funcScope);

        ASTType *thisTy = NULL;
        // is a method
        if(owner){
            // referenceTy, so that structs pass around pointers
            thisTy = owner->getDeclaredType()->getReferenceTy();
            Identifier *this_id = getScope()->getInScope("this");
            VariableDeclaration *this_decl = new VariableDeclaration(thisTy, this_id, NULL, loc, DeclarationQualifier());
            this_id->addDeclaration(this_decl, Identifier::ID_VARIABLE);
            //parameters.push_back(this_decl);
        }

        bool vararg = false;
        while(!peek().is(tok::rparen))
        {
            DeclarationQualifier qualifier = parseDeclarationQualifier();
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

            ASTType *aty = parseType();
            Token t_name = get();
            Identifier *paramId = getScope()->getInScope(t_name.toString());

            Expression *defaultValue = NULL;

            if(peek().is(tok::equal))
            {
                ignore(); // eat '='
                defaultValue = parseExpression();
            }

            VariableDeclaration *paramDecl = new VariableDeclaration(aty, paramId, defaultValue, loc, qualifier);
            paramId->addDeclaration(paramDecl, Identifier::ID_VARIABLE);
            parameters.push_back(paramDecl);

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

        // body
        Statement *stmt = parseStatement();
        popScope();

        ASTType *ownty = owner ? owner->getDeclaredType() : NULL;
        Declaration *decl = new FunctionDeclaration(id, ownty, type, parameters, vararg,
                funcScope, stmt, idLoc, dqual);
        id->addDeclaration(decl, Identifier::ID_FUNCTION);
        return decl;
    }

    Expression *defaultValue = NULL;
    if(peek().is(tok::equal) || peek().is(tok::colonequal))
    {
        ignore(); // ignore =
        defaultValue = parseExpression();
    } else if(type->kind == TYPE_DYNAMIC) //XXX make isDynamic() function
    {
        emit_message(msg::ERROR, "dynamic type 'var' without initializer", idLoc);
    }

    Declaration *decl = new VariableDeclaration(type, id, defaultValue, idLoc, dqual);
    id->addDeclaration(decl, Identifier::ID_VARIABLE);

    //TODO: comma, multiple decl
    return decl;
}

Statement *ParseContext::parseCompoundStatement(){
    SourceLocation loc = peek().getLocation();
    ignore(); // eat lbrace
    ASTScope *tbl = new ASTScope(getScope());
    pushScope(tbl);
    vector<Statement*> stmts;
    while(peek().isNot(tok::rbrace)) {
        if(Statement *stmt = parseStatement())
            stmts.push_back(stmt);
    }
    popScope();
    ignore(); // eat rbrace
    return new CompoundStatement(tbl, stmts, loc);
}

Statement *ParseContext::parseIfStatement()
{
    SourceLocation loc = peek().loc;
    Expression *cond = NULL;
    Statement *body = NULL;
    ElseStatement *els = NULL;
    ASTScope *parentScope = getScope();
    ASTScope *scope = new ASTScope(parentScope);
    pushScope(scope);
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
    popScope();

    if(peek().is(tok::kw_else))
    {
        SourceLocation elseLoc = peek().loc;
        ignore(); // else kw
        ASTScope *elseScope = new ASTScope(parentScope);
        pushScope(elseScope);
        els = new ElseStatement(elseScope, parseStatement(), elseLoc);
        popScope();
    }

    return new IfStatement(scope, cond, body, els, loc);
}

Statement *ParseContext::parseWhileStatement() {
    SourceLocation loc = peek().loc;
    Expression *cond = NULL;
    Statement *body = NULL;
    ElseStatement *els = NULL;
    ASTScope *parentScope = getScope();
    ASTScope *scope = new ASTScope(parentScope);
    pushScope(scope);
    assert_message(peek().is(tok::kw_while), msg::ERROR, "expected 'while' keyword", peek().loc);
    ignore(); // eat 'while'

    assert_message(peek().is(tok::lparen), msg::ERROR,
            "expected '(' following 'while' keyword", peek().loc);
    ignore(); // eat '('
    if(peek().isNot(tok::rparen)) cond = parseExpression();
    assert_message(peek().is(tok::rparen), msg::ERROR,
            "expected ')' following 'while' condition", peek().loc);
    ignore(); // eat ')'

    body = parseStatement();
    popScope();

    if(peek().is(tok::kw_else))
    {
        ignore(); // else kw
        SourceLocation eloc = peek().loc;
        ASTScope *elseScope = new ASTScope(parentScope);
        pushScope(elseScope);
        els = new ElseStatement(elseScope, parseStatement(), eloc);
        popScope();
    }

    return new WhileStatement(scope, cond, body, els, loc);
}

Statement *ParseContext::parseForStatement()
{
    SourceLocation loc = peek().loc;
    Statement *decl = NULL;
    Expression *cond = NULL;
    Statement *upd = NULL;
    Statement *body = NULL;
    ElseStatement *els = NULL;
    ASTScope *parentScope = getScope();
    ASTScope *scope = new ASTScope(parentScope);
    pushScope(scope);

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

    if(peek().isNot(tok::semicolon)) decl = parseStatement();
    if(!peek().is(tok::semicolon)) {
        emit_message(msg::ERROR, "expected ';' following 'for' declaration", peek().loc);
        return NULL;
    }
    ignore(); // eat ';'

    if(peek().isNot(tok::semicolon)) cond = parseExpression();
    if(!peek().is(tok::semicolon)) {
        emit_message(msg::ERROR, "expected ';' following for condition", peek().loc);
        return NULL;
    }
    ignore(); // eat ';'

    if(peek().isNot(tok::rparen)) upd = parseStatement();
    if(!peek().is(tok::rparen)) {
        emit_message(msg::ERROR, "expected ')', following 'for' update statement", peek().loc);
        return NULL;
    }
    ignore(); // eat ')'

    body = parseStatement();
    popScope();

    if(peek().is(tok::kw_else))
    {
        SourceLocation eloc = peek().loc;
        ignore();
        ASTScope *elseScope = new ASTScope(parentScope);
        pushScope(elseScope);
        els = new ElseStatement(elseScope, parseStatement(), eloc);
        popScope();
    }

    return new ForStatement(scope, decl, cond, upd, body, els, loc);
}

CaseStatement *ParseContext::parseCaseStatement()
{
    SourceLocation loc = peek().loc;
    if(peek().isNot(tok::kw_case))
    {
        emit_message(msg::FAILURE, "expected 'case' keyword", peek().loc);
        return NULL;
    }
    ignore(); // eat 'case'

    std::vector<Expression*> cases;

    cases.push_back(parseExpression());
    while(peek().is(tok::comma))
    {
        ignore(); // ignore comma
        cases.push_back(parseExpression());
    }

    return new CaseStatement(cases, loc);
}

Statement *ParseContext::parseSwitchStatement()
{
    SourceLocation loc = peek().loc;
    ASTScope *scope = new ASTScope(getScope());
    pushScope(scope);

    if(peek().isNot(tok::kw_switch)) {
        emit_message(msg::FAILURE, "expected 'switch' keyword", peek().loc);
        popScope();
        return NULL;
    }
    ignore(); // eat 'switch'

    if(peek().isNot(tok::lparen)){
        emit_message(msg::FAILURE,
            "expected '(' following 'switch' keyword", peek().loc);
        popScope();
        return NULL;
    }
    ignore(); // eat '('

    Expression *exp = parseExpression();

    if(peek().isNot(tok::rparen)){
        emit_message(msg::FAILURE,
            "expected ')' following 'switch' condition", peek().loc);
        popScope();
        return NULL;
    }
    ignore(); // eat ')'

    Statement *body = parseStatement();
    popScope();

    return new SwitchStatement(scope, exp, body, loc);
}

Expression *ParseContext::parseCastExpression(int prec)
{
    SourceLocation loc = peek().loc;
    ASTType *type = parseType();
    if(!peek().is(tok::colon)) {
        emit_message(msg::ERROR, "expected colon for cast operator!", peek().loc);
        dropLine();
        return NULL;
    }
    ignore(); // eat ':'
    return new CastExpression(type, parseExpression(12), loc); // 12 == cast priority
}


Expression *ParseContext::parseIdOpExpression() {
    SourceLocation loc = peek().loc;
    IdOpExpression::Type type;
    if(peek().is(tok::kw_delete)) {
        type = IdOpExpression::Delete;
    } else if(peek().is(tok::kw_retain)) {
        type = IdOpExpression::Retain;
    } else if(peek().is(tok::kw_release)) {
        type = IdOpExpression::Release;
    } else {
        emit_message(msg::ERROR, "expected id operator (delete, retain, release)", loc);
        return NULL;
    }
    ignore(); // ignore op
    Expression *exp = parseExpression();
    return new IdOpExpression(exp, type, loc);
}

Expression *ParseContext::parseNewExpression()
{
    bool call = false;
    SourceLocation loc = peek().loc;
    assert(peek().is(tok::kw_new));
    ignore(); //ignore 'new'
    ASTType *t = parseType();
    std::list<Expression*> args;

    if(peek().is(tok::lparen)) {
        call = true;
        parseArgumentList(args);
    }

    //Expression *alloc = new HeapAllocExpression(t, loc);

    return new NewExpression(t, NewExpression::HEAP, args, call, loc);
}

Expression *ParseContext::parseExpression(int prec)
{
    switch(peek().kind) {
        case tok::kw_import:
            return parseImport();
        case tok::kw_delete:
        case tok::kw_retain:
        case tok::kw_release:
            return parseIdOpExpression();
        default:
        return parseBinaryExpression(prec);
    }
}

Expression *ParseContext::parseTupleExpression()
{
    SourceLocation loc = peek().loc;
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

    return new TupleExpression(members, loc);
}

void ParseContext::parseArgumentList(std::list<Expression*> &args) {
    if(peek().isNot(tok::lparen)) {
        //error
    }
    ignore(); // ignore lparen

    while(peek().isNot(tok::rparen))
    {
        args.push_back(parseExpression(getBinaryPrecidence(tok::comma)));
        if(peek().isNot(tok::comma) && peek().isNot(tok::rparen)) {
            emit_message(msg::ERROR,
                    "expected ',' or ')' following call parameter", peek().loc);
            return;
        }
        if(peek().is(tok::comma)) ignore(); // ignore comma or rparen
    }
    ignore(); // ignore rparen
}

Expression *ParseContext::parsePostfixExpression(int prec)
{
    SourceLocation loc = peek().loc;
    Expression *exp = parsePrimaryExpression();
    while((linePeek().getPostfixPrecidence()) > prec) {
        if(linePeek().is(tok::lparen)) // call
        {
            std::list<Expression*> args;
            parseArgumentList(args);
            exp = new CallExpression(exp, args, loc);
        } else if(linePeek().is(tok::lbracket)) //index/slice
        {
            ignore();
            Expression *index = parseExpression();
            if(!peek().is(tok::rbracket)) {
                emit_message(msg::ERROR, "expected ']' following index expression", peek().loc);
                return NULL;
            }
            ignore();
            exp = new IndexExpression(exp, index, loc);
        } else if(linePeek().is(tok::dot))
        {
            ignore(); //ignore .
            int n = 1;

            // look ahead and see if we can spot an 'infix cast'
            // an infix cast is a bit of a strange idea where you may cast to a type
            // along in a chain of dot-operators.
            // eg: base.SpecialClass: member.specialClassMember
            // The above is equivilent to (SpecialClass: base.member).specialClassMember
            // It is sort of a way to by pass the weird bracket stuff that shows up in C++
            // But, it might not be a good idea...
            while(lookAhead(n).isNot(tok::dot) && !lookAhead(n).followsNewline() &&
                    !lookAhead(n).isBinaryOp())
            {
                if(lookAhead(n).is(tok::colon)) // infix cast detected
                {
                    ASTType *type = parseType();
                    if(peek().isNot(tok::colon)){
                        emit_message(msg::ERROR,
                                "expected colon while parsing infix cast", peek().loc);
                        dropLine();
                        return NULL;
                    }
                    ignore(); //colon
                    loc = peek().loc;
                    exp = new DotExpression(exp, get().toString(), loc);
                    exp = new CastExpression(type, exp, loc);
                    return exp;
                }
                n++;
            }

            exp = new DotExpression(exp, get().toString(), loc);
        }
        else if(linePeek().is(tok::plusplus))
        {
            ignore(); // ignore ++
            exp = new PostfixOpExpression(exp, tok::plusplus, loc);
        } else if(linePeek().is(tok::minusminus))
        {
            ignore(); // ignore --
            exp = new PostfixOpExpression(exp, tok::minusminus, loc);
        } else
            emit_message(msg::FAILURE,
                    "this doesn't look like a postfix expresion to me!",
                    peek().loc);
    }

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

Expression *ParseContext::parseIdentifierExpression()
{
    SourceLocation loc = peek().loc;

    if(peek().isNot(tok::identifier) && peek().isNot(tok::kw_this) && peek().isNot(tok::dot))
    {
        emit_message(msg::ERROR, "expected identifier expression", loc);
        return NULL;
    }

    Identifier *id = getScope()->get(get().toString());
    IdentifierExpression *ret = new IdentifierExpression(id, loc);
    return ret;
}

Expression *ParseContext::parsePrimaryExpression() {
    int n = 1; //look ahead
    while(lookAhead(n).is(tok::caret))
        n++;
    if(lookAhead(n).is(tok::colon))
    {
        return parseCastExpression();
    }

    SourceLocation loc = peek().loc;
    if(peek().is(tok::charstring))
    {
        return new StringExpression(get().stringData(), loc);
    }

    if(peek().is(tok::kw_pack)) {
        ignore();
        if(peek().isNot(tok::charstring)) {
            emit_message(msg::ERROR, "pack expression expects filename string following keyword", loc);
            return NULL;
        }
        return new PackExpression(get().stringData(), loc);
    }

    if(peek().is(tok::lbracket))
    {
        return parseTupleExpression();
    }

    if(peek().is(tok::intNum))
    {
        return new IntExpression(ASTType::getLongTy(),
                (uint64_t) get().intData(), loc);
    }

    if(peek().is(tok::floatNum))
    {
        return new FloatExpression(ASTType::getDoubleTy(),
                get().floatData(), loc);
    }

    if(peek().is(tok::kw_true))
    {
        ignore();
        return new IntExpression(ASTType::getBoolTy(),
                (uint64_t) 1L, loc);
    }
    if(peek().is(tok::kw_false))
    {
        ignore();
        return new IntExpression(ASTType::getBoolTy(),
                (uint64_t) 0L, loc);
    }
    if(peek().is(tok::kw_null))
    {
        ignore();
        return new IntExpression(ASTType::getVoidTy()->getPointerTy(),
                (uint64_t) 0L, loc);
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

    // XXX dispite looking like a unary operator, unary dot is actually parsed here
    // this is because otherwise the recursive descent would fail on statements like
    // .v[0]
    if(peek().is(tok::dot)) {
        ignore();

        if(peek().isNot(tok::identifier) && peek().isNot(tok::kw_this)) {
            emit_message(msg::ERROR, "expected identifier expression", loc);
            return NULL;
        }

        //TODO: let lower in validate? create new ASTNode 'PrefixDotExpression'?
        //
        //XXX: lowering here makes 'this.func()' semantially equivilent to '.func()'
        // this is incorrect in the case of UFCS where '.func()' would signify that the function
        // is explicitly contained within 'this' or a superclass. 'this.func()' would still be vulnerable
        // to UFCS
        Expression *thisExp = new IdentifierExpression(getScope()->lookup("this"), loc);
        return new DotExpression(thisExp, get().toString());
    }

    if(peek().is(tok::identifier) || peek().is(kw_this) || peek().is(tok::dot)) {
        //TODO: in statement like "MyStruct[].sizeof", this will fail
        return parseIdentifierExpression();
    }

    if(peek().is(tok::kw_new)) {
        return parseNewExpression();
    }

    if(peek().isKeywordType())
    {
        return new TypeExpression(parseType(),loc);
    }

    emit_message(msg::FAILURE, "primary expression expected", peek().loc);
    return NULL;
}

Expression *ParseContext::parseBinaryExpression(int prec)
{
    SourceLocation lhsLoc = peek().loc;
    Expression *lhs = parseUnaryExpression(prec);

    tok::TokenKind op;
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

    if(!lhs) {
        emit_message(msg::FAILURE, "LHS of binary expression may not be valid expression", lhsLoc);
        return NULL;
    }

    return lhs;
}

void ParseContext::parseModule(ModuleDeclaration *module)
{
    this->module = module;
    currentPackage()->addPackage(module);

    pushScope(module->getScope());
    while(peek().isNot(tok::eof))
    {
        parseTopLevel(module); // modifies module
    }
    popScope();
    assert_message(!getScope(), msg::FAILURE, "invalid scope stack!", peek().loc);
}

void Parser::parseFile(ModuleDeclaration *module, File *file, SourceLocation l)
{
    // if we can't open the specified file, check if the file is in the 'lib' dir
    // if we still can't find it; throw a fit
    if(!file->exists())
    {
        emit_message(msg::ERROR, "unknown file '" + file->getName() + "'", l);
        return;
    }

    Lexer *lexer = new StreamLexer(file->getStream());
    lexer->setFilename(file->getName());
    ParseContext context(lexer, this, ast->getRootPackage());
    context.parseModule(module);
    delete lexer;
}
