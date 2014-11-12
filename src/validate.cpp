#include <string>
#include "ast.hpp"
#include "astType.hpp"
#include "validate.hpp"
#include "message.hpp"
#include "token.hpp" // for getting operator precidence

using namespace std;

//
// Code for AST traversal, validation, and lowering
// if validate returns true, the AST should be in a consistant state, lowered, and ready for codegen;
// else if validate return false, the AST is invalid, and may be left in an inconsistant state.
//

//
// AST
//

bool AST::validate() {
    ValidationVisitor visitor;
    getRootPackage()->accept(&visitor);
    return visitor.isValid();
}

//
// ValidationVisitor
//

ValidationVisitor::ValidationVisitor() : ASTVisitor() {
    valid = true;
}

Identifier *ValidationVisitor::resolveIdentifier(Identifier *id) {
    if(id->isUndeclared()) {
        id = id->getScope()->resolveIdentifier(id);
    }

    if(id->isUndeclared()) {
        valid = false;
        emit_message(msg::ERROR, string("undeclared identifier '") + id->getName()
                + string("' in scope"));
    }

    if(id->isExpression()) {
        id->getExpression()->accept(this);
    }

    return id;
}

ASTType *ValidationVisitor::resolveType(ASTType *ty) {
    if(!ty->isResolved()) {
        if(ASTUserType *userty = ty->asUserType()) {
            userty->identifier = resolveIdentifier(userty->identifier);

            if(!ty->getDeclaration()) {
                emit_message(msg::FAILURE, "undeclared user type", location);
                return NULL;
            }

            location = ty->getDeclaration()->loc;
            //userty->getScope()->accept(this);
        }
    }

    if(ASTFunctionType *fty = ty->asFunctionType()) {
        for(int i = 0; i < fty->params.size(); i++){
            fty->params[i] = resolveType(fty->params[i]);
        }

        if(fty->owner){
            resolveType(fty->owner);
        }

        resolveType(fty->ret);
    }

    if(ty->isPointer()) {
        resolveType(ty->getPointerElementTy());
    }

    if(ty->isTuple()) {
        ASTTupleType *tupty = ty->asTuple();
        if(!tupty->types.size()) {
            emit_message(msg::ERROR, "invalid 0-tuple", location);
        }

        for(int i = 0; i < tupty->types.size(); i++) {
            resolveType(tupty->types[i]);
        }
    }

    if(ty->isArray()) {
        resolveType(ty->asArray()->arrayOf);
    }

    if(ty->isUserType()) {
        //XXX requried?
        ((ASTUserType*)ty)->identifier = resolveIdentifier(ty->asUserType()->identifier);
    }

    if(ty->kind == TYPE_UNKNOWN || ty->kind == TYPE_UNKNOWN_USER) {
        emit_message(msg::FATAL, "invalid type", location);
    }

    return ty;
}

Expression *ValidationVisitor::coerceTo(ASTType *ty, Expression *exp) {
    ASTType *expty = exp->getType();

    if(expty->is(ty)) {
        return exp;
    }

    if(expty->coercesTo(ty)) {
        //TODO: note in AST that this is implicit
        return new CastExpression(ty, exp, exp->loc);
    }

    emit_message(msg::ERROR, "attempt to coerce expression of type '" + expty->getName() +
            "' to incompatible type '" + ty->getName() + "'", location);

    return NULL;
}

/**
 * get the type which both t1 and t2 may promote to
 */
ASTType *ValidationVisitor::commonType(ASTType *t1, ASTType *t2) {
    if(t1->is(t2)) return t1;


    if(t1->isUserType() || t2->isUserType()) {
        if(t1->extends(t2)) return t2;
        if(t2->extends(t1)) return t1;

        if(t1->isUserType() != t2->isUserType()) {
            emit_message(msg::ERROR, "attempt to cast user type to non-user type");
        }
    }

    if(t1->isNumeric() && t2->isNumeric()) {
        if(t1->getPriority() > t2->getPriority()) return t1;
        else return t2;
    }

    return NULL;
}

void ValidationVisitor::visitPackage(Package *pak) {
    if(!pak->getScope()) valid = false;
}

void ValidationVisitor::visitTranslationUnit(TranslationUnit *tu) {
    for(int i = 0; i < tu->imports.size(); i++) {
        if(!tu->imports[i]) {
            valid = false;
            emit_message(msg::ERROR, "invalid translation unit found, null import", location);
        }
    }
}

void ValidationVisitor::visitDeclaration(Declaration *decl) {
    location = decl->loc;

    if(!decl->identifier) {
        valid = false;
        emit_message(msg::ERROR, "null identifier in declaration", location);
    }

    if(decl->identifier->isUndeclared()) {
        valid = false;
        emit_message(msg::ERROR, "inconsistant AST, undeclared identifier in definition", location);
    }

    if(decl->qualifier.weak && !decl->getType()->isClass()) {
        valid = false;
        emit_message(msg::ERROR, "weak qualifier only applies to class type variables", location);
    }
}

void ValidationVisitor::visitExpression(Expression *exp) {
    location = exp->loc;
}

void ValidationVisitor::visitStatement(Statement *stmt) {
    location = stmt->loc;
}

void ValidationVisitor::visitFunctionDeclaration(FunctionDeclaration *decl) {
    //TODO: validate prototype
    if(!decl->getType()) {
        valid = false;
        emit_message(msg::ERROR, "function declaration missing prototype", location);
    }

    if(!decl->getScope() && decl->body) {
        valid = false;
        emit_message(msg::ERROR, "function has body without scope", location);
    }

    resolveType(decl->getType());

    if(decl->body)
        decl->body = decl->body->lower();
}

void ValidationVisitor::visitLabelDeclaration(LabelDeclaration *decl) {

}

void ValidationVisitor::visitVariableDeclaration(VariableDeclaration *decl) {
    if(decl->value) decl->value = decl->value->lower();

    if(decl->getType()->kind == TYPE_DYNAMIC) {
        if(!decl->value) {
            valid = false;
            emit_message(msg::ERROR,
                    "dynamically typed variables must have valid initializer", location);
        }

        decl->type = decl->value->getType();
    }

    resolveType(decl->getType());

}

void ValidationVisitor::visitTypeDeclaration(TypeDeclaration *decl) {
    if(!decl->getDeclaredType()) {
        valid = false;
        emit_message(msg::ERROR, "invalid type declaration", location);
    }
}

void ValidationVisitor::visitUserTypeDeclaration(UserTypeDeclaration *decl) {
    if(!decl->type->isResolved()) {
        valid = false;
        emit_message(msg::ERROR, "unresolved user type declaration", location);
    }

	ClassDeclaration *cldecl = decl->classDeclaration();
    if(cldecl) {
        if(cldecl->base) {
            cldecl->base = resolveIdentifier(cldecl->base);
            if(!cldecl->base->isClass() && !cldecl->base->isStruct()) {
                emit_message(msg::ERROR, "expected class type in base specifier", location);
                if(cldecl->base->isInterface()) {
                    emit_message(msg::ERROR, "interfaces are specified implicitly", location);
                }
            }
        }

        cldecl->populateVTable();
    }

    if(InterfaceDeclaration *idecl = dynamic_cast<InterfaceDeclaration*>(decl)){
        for(int i = 0; i < idecl->methods.size(); i++){
            if(idecl->methods[i]->body){
                emit_message(msg::ERROR, "interface methods should not declare method body (did you forget ';'?)",
                        idecl->methods[i]->loc);
            }
        }
    }

    //XXX lower constructor and destructor?
    for(int i = 0; i < decl->methods.size(); i++) {
        if(!decl->methods[i]) emit_message(msg::ERROR, "invalid method found in user type declaration", decl->loc);
        decl->methods[i] = dynamic_cast<FunctionDeclaration*>(decl->methods[i]->lower());
    }

    for(int i = 0; i < decl->members.size(); i++) {
        if(!decl->members[i]) emit_message(msg::ERROR, "invalid member found in user type declaration", decl->loc);
        decl->members[i] = decl->members[i]->lower();
    }

    decl->scope->accept(this);
}

void ValidationVisitor::visitUnaryExpression(UnaryExpression *exp) {
    //TODO: insert coersion cast
    if(!exp->lhs) {
        valid = false;
        emit_message(msg::ERROR, "unary operator is missing expression", location);
    } else {
        exp->lhs = exp->lhs->lower();
    }

    if(exp->op == tok::dot && !exp->lhs->identifierExpression()) {
        valid = false;
        emit_message(msg::ERROR, "unary dot operator expects identifier following dot", location);
    }

    if(!getUnaryPrecidence((tok::TokenKind) exp->op)) {
        emit_message(msg::FAILURE, "unary expression found without unary operation", location);
    }

    switch(exp->op) {
        case tok::plusplus:
            if(!exp->lhs->isLValue()) {
                emit_message(msg::ERROR, "++ operator expects LValue", location);
            }

            if(!exp->lhs->getType()->isNumeric() && !exp->lhs->getType()->isPointer()) {
                emit_message(msg::ERROR, "++ operator is only valid on numeric expressions", location);
            }
            break;
        case tok::minusminus:
            if(!exp->lhs->isLValue()) {
                emit_message(msg::ERROR, "-- operator expects LValue", location);
            }

            if(!exp->lhs->getType()->isNumeric() && !exp->lhs->getType()->isPointer()) {
                emit_message(msg::ERROR, "-- operator is only valid on numeric expressions", location);
            }
            break;
        case tok::plus:
        case tok::minus:
            if(!exp->lhs->getType()->isNumeric()) {
                emit_message(msg::ERROR, "unary +- is only valid on numeric expressions", location);
            }
            break;
        case tok::bang:
            if(!exp->lhs->getType()->isBool()) {
                exp->lhs = coerceTo(ASTType::getBoolTy(), exp->lhs);
            }
            break;
        case tok::tilde:
            emit_message(msg::UNIMPLEMENTED, "unimplemented unary operator '~'", location);
            break;
        case tok::caret:
            if(!exp->lhs->getType()->isPointer()) {
                emit_message(msg::ERROR, "attempt to dereference non-pointer type", location);
            }
            break;
        case tok::amp:
            if(!exp->lhs->isLValue()) {
                emit_message(msg::ERROR, "& operator expects LValue", location);
            }
            break;
    }
}

void ValidationVisitor::visitBinaryExpression(BinaryExpression *exp) {
    //TODO: insert coersion cast
    if(!exp->lhs) {
        valid = false;
        emit_message(msg::ERROR, "binary operator is missing left hand expression", location);
    } else {
        exp->lhs = exp->lhs->lower();
    }

    if(!exp->rhs) {
        valid = false;
        emit_message(msg::ERROR, "binary operator is missing right hand expression", location);
    } else {
        exp->rhs = exp->rhs->lower();
    }

    if(!exp->op.getBinaryPrecidence()) {
        emit_message(msg::FAILURE, "invalid operator found in binary expression", location);
    }

    // should never happen
    if(exp->op.kind == tok::colon) {
        emit_message(msg::FAILURE, "cast found as binary expression", location);
    }

    // should never happen
    if(exp->op.kind == tok::dot) {
        emit_message(msg::FAILURE, "dot found as binary expression", location);
    }

}

void ValidationVisitor::visitPrimaryExpression(PrimaryExpression *exp) {

}

Expression *ValidationVisitor::resolveCallArgument(ASTFunctionType *fty, unsigned i, Expression *arg, Expression *def) {
    ASTType *argty = arg->getType();
    ASTType *paramty  = fty->params[i];

    if(fty->isVararg() && i >= fty->params.size()) {
        return arg; // TODO: resolve to vararg type
    } else if(argty && argty->is(paramty)) {
        return arg;
    } else if (argty && argty->coercesTo(paramty)) {
        return new CastExpression(paramty, arg, arg->loc);
    } else if(def) {
        if(def->getType()->coercesTo(paramty)) {
            return new CastExpression(paramty, def, arg->loc);
        }
    }

    return NULL;

}

void ValidationVisitor::resolveCallArguments(Expression *func, std::list<Expression*>& funcargs) {
    std::list<Expression*> resolvedArgs;

    if(func->isValue()) {
        // dereference function pointer

        ASTFunctionType *fty = NULL;
        FunctionDeclaration *fdecl = NULL;
        if(func->getType()->isPointer()) {
            fty = func->getType()->getPointerElementTy()->asFunctionType();
            if(!fty) {
                emit_message(msg::ERROR, "non-function pointer used in function pointer context", location);
                return;
            }
            //func = new UnaryExpression(tok::caret, func, location);
        } else {
            fty = func->getType()->asFunctionType();

            if(func->identifierExpression()) {
                fdecl = func->identifierExpression()->getDeclaration()->functionDeclaration();
            }

            if(!fty) {
                emit_message(msg::ERROR, "attempt to call non-function type", location);
                return;
            }
        }

        std::list<Expression*>::iterator args = funcargs.begin();

        //TODO: while condition will not work on varargs
        while(resolvedArgs.size() < fty->params.size()) {
            Expression *arg = NULL;
            Expression *defaultArg = NULL;
            ASTType *argty = NULL;
            ASTType *paramty = NULL;
            //XXX below will break on varargs
            paramty  = fty->params[resolvedArgs.size()];

            if(args != funcargs.end()) {
                arg = *args;
                argty = arg->getType();
            }

            if(fdecl) {
                defaultArg = fdecl->getDefaultParameter(resolvedArgs.size());
            }

            Expression *resolvedArg = resolveCallArgument(fty, resolvedArgs.size(), arg, defaultArg);

            if(!resolvedArg) {
                emit_message(msg::ERROR, "cannot resolve argument %d of function call; does not coerce to expected type '" + paramty->getName() + "'", func->loc);
            }

            resolvedArgs.push_back(resolvedArg);

            args++;
        }

        funcargs = resolvedArgs;
    } else if(func->isType()) {
        // this is a type declaration.
        // we are calling a type. eg a constructor call
        // of style MyStruct(1, 2, 3)

        ASTUserType *uty = func->getDeclaredType()->asUserType();
        if(!uty) {
            emit_message(msg::ERROR, "constructor syntax on non-user type '" +
                    func->getDeclaredType()->getName() + "'", location);
        }

        //TODO: currently breaks because codegen needs to alloc 'this'
        //func = new IdentifierExpression(uty->getConstructor()->getIdentifier(), location);
    } else {
        emit_message(msg::ERROR, "invalid function call", location);
    }

}

void ValidationVisitor::visitCallExpression(CallExpression *exp) {
    if(!exp->function) {
        emit_message(msg::FAILURE, "invalid or missing function in call expression", exp->loc);
    } else {
        exp->function = exp->function->lower();

        // if lhs is dot expression, this is either method call or UFCS.
        // Decide which, and push 'this' variable as function argument if required
        //
        // its unfortunate that we need to make a slight exception for dot expressions,
        // but probably necessary.
        DotExpression *dexp = exp->function->dotExpression();
        if(dexp) {
            if(dexp->isValue()) {
                if(dexp->lhs->getType()->isStruct() && dexp->lhs->isLValue()) {
                    exp->args.push_front(new UnaryExpression(tok::amp, dexp->lhs, location));
                } else {
                    exp->args.push_front(dexp->lhs);
                }
            } else { // UFCS
                Identifier *fid = getScope()->lookup(dexp->rhs);
                exp->function = new IdentifierExpression(fid, location);
                exp->args.push_front(dexp->lhs);
                emit_message(msg::DEBUGGING, "ufcs", location);
            }
        }

        resolveCallArguments(exp->function, exp->args);

        //TODO: resolve overload
    }

    std::list<Expression*>::iterator it = exp->args.begin();
    while(it != exp->args.end()) {
        if(!*it) emit_message(msg::FAILURE, "invalid or missing argument in call expression", exp->loc);
        else *it = (*it)->lower();
        it++;
    }
}

void ValidationVisitor::visitIndexExpression(IndexExpression *exp) {
    if(!exp->lhs) {
        emit_message(msg::ERROR, "invalid or missing base for index expression", exp->loc);
    } else {
        exp->lhs = exp->lhs->lower();
    }

    if(!exp->index) {
        emit_message(msg::ERROR, "invalid or missing index in index expression", exp->loc);
    } else {
        exp->index = exp->index->lower();
    }
}

void ValidationVisitor::visitIdentifierExpression(IdentifierExpression *exp) {
    // resolve identifier
    exp->id = resolveIdentifier(exp->id);

    if(exp->id->isUndeclared()){
        emit_message(msg::ERROR, "identifier is expected to be resolved at this point", location);
    }

    // resolve type of identifier if needed
    if(exp->id->getType()) //XXX temp?
    {
        resolveType(exp->id->getType());
    }

    if(exp->id->getDeclaredType())
        exp->id->astType = resolveType(exp->id->getDeclaredType());
}

void ValidationVisitor::visitNumericExpression(NumericExpression *exp) {
    // TODO: validate type
}

void ValidationVisitor::visitStringExpression(StringExpression *exp) {

}

void ValidationVisitor::visitImportExpression(ImportExpression *exp) {
    if(!exp->expression) {
        valid = false;
        emit_message(msg::ERROR, "import expression expects following expression", location);
    }

    if(!dynamic_cast<StringExpression*>(exp->expression)) {
        valid = false;
        emit_message(msg::ERROR, "import expression expects following expression to be package string", location);
    }
}

void ValidationVisitor::visitPackageExpression(PackageExpression *exp) {

}

void ValidationVisitor::visitCastExpression(CastExpression *exp) {
    resolveType(exp->type);
    if(!exp->expression) {
        emit_message(msg::ERROR, "missing value in cast expression", exp->loc);
    } else {
        exp->expression = exp->expression->lower();
    }
}

void ValidationVisitor::visitUseExpression(UseExpression *exp) {

}

void ValidationVisitor::visitTypeExpression(TypeExpression *exp) {

}

void ValidationVisitor::visitTupleExpression(TupleExpression *exp) {
    for(int i = 0; i < exp->members.size(); i++) {
        if(!exp->members[i]) emit_message(msg::ERROR, "expression in tuple expression", exp->loc);
        else exp->members[i] = exp->members[i]->lower();
    }
}

/*
 * Dot expression is either:
 *
 * a) a member lookup
 * b) static member
 * c) a scope specifier
 * d) a method lookup (which must be followed by a call)
 * e) a UFCS call
 *
 * (a), (b), and (c) are similar in function; so are (d) and (e)
 *
 * if LHS is a UserType variable (or pointer to), and RHS is a valid member, we have (a)
 * if RHS is a static member, we have (b)
 * else if RHS is a valid method name, we have (d)
 * else if LHS is a package (or some scope token), we have (b)
 * else if RHS is a valid function name in scope, and it's first parameter matches LHS's type, we have (e)
 */
void ValidationVisitor::visitDotExpression(DotExpression *exp) {
    if(!exp->lhs) {
        emit_message(msg::ERROR, "invalid base in dot expression", exp->loc);
    } else exp->lhs = exp->lhs->lower();

    if(exp->lhs->isValue()) {
        ASTType *lhstype = exp->lhs->getType();
        Identifier *rhsid = NULL;

        //TODO: check for UFCS
        // dereference pointer types on dot expression
        if(lhstype->isPointer() && lhstype->getPointerElementTy()->isStruct()) {
            rhsid = lhstype->getPointerElementTy()->getDeclaration()->lookup(exp->rhs);
            if(!rhsid) {
                //TODO UFCS
            } else if(rhsid->isVariable()) {
                exp->lhs = new UnaryExpression(tok::caret, exp->lhs, location);
                lhstype = exp->lhs->getType();
            } else if(rhsid->isFunction()) {
                // functions take pointer
                return;
            }
        }

        // if user type or pointer to user type
        if(lhstype->isUserType()) {
            rhsid = lhstype->getDeclaration()->lookup(exp->rhs);

            if(!rhsid) {
                // member not found in userType; check if this is a 'Uniform Function Call Syntax' thing
                rhsid = getScope()->lookup(exp->rhs);

                if(!rhsid || !rhsid->isFunction()) {
                    // member is not a UFCS. member not found in scope, or found identifier is not a function
                    emit_message(msg::ERROR, "member '" + exp->rhs + "' not found in user type '" + lhstype->getName() + "'", location);
                }
            }

            //XXX hacky. in case rhs is userType, it's type can be
            // unresolved before used
            if(Declaration *rhsdecl = rhsid->getDeclaration()) {
                resolveType(rhsdecl->getType());
            }

            if(rhsid->isUndeclared()) {
                emit_message(msg::WARNING, "undeclared member identifier in validate", location);
            }
        } else if(lhstype->isArray()) {
            if(exp->rhs != "size" && exp->rhs != "ptr") {
                emit_message(msg::ERROR, "invalid property '" + exp->rhs + "' in array", location);
            }
        } else {
            emit_message(msg::ERROR, "invalid dot expression on non-user type", location);
        }

    } else if(exp->lhs->isType()) {
        ASTType *declty = exp->lhs->getDeclaredType();
        if(!declty) {
            emit_message(msg::ERROR, "invalid lhs of dot expression", location);
        }

        if(exp->rhs == "sizeof" || exp->rhs == "offsetof") {
            // i think we're fine
        } else {
            //TODO: look up static members / functions
            emit_message(msg::UNIMPLEMENTED, "no static members or function lookup availible", location);
        }
    } else {
        emit_message(msg::FAILURE, "unresolved dot expression", location);
    }
}

void ValidationVisitor::visitNewExpression(NewExpression *exp) {
    resolveType(exp->type);

    ASTUserType *uty = exp->type->asUserType();
    if(uty && exp->call) {
        exp->function = new IdentifierExpression(uty->getConstructor()->identifier, location);

        //resolveCallArguments(exp->function, exp->args);
    }

    if(!exp->function && exp->args.size() > 0) {
        emit_message(msg::ERROR, "unknown constructor call", location);
    }

    if(exp->type->kind == TYPE_DYNAMIC_ARRAY) {
        emit_message(msg::ERROR, "cannot create unsized array in 'new' expression", location);
    }
}

void ValidationVisitor::visitIdOpExpression(IdOpExpression *exp) {

}

void ValidationVisitor::visitBreakStatement(BreakStatement *stmt) {

}

void ValidationVisitor::visitContinueStatement(ContinueStatement *stmt) {

}

void ValidationVisitor::visitLabelStatement(LabelStatement *stmt) {
    if(!stmt->identifier) {
        valid = false;
        emit_message(msg::ERROR, "label statement expects following identifier", location);
    }
}

void ValidationVisitor::visitCaseStatement(CaseStatement *stmt) {
    for(int i = 0; i < stmt->values.size(); i++) {
        if(!stmt->values[i]) {
            valid = false;
            emit_message(msg::ERROR, "invalid case value in case statement", stmt->loc);
        } else if(!stmt->values[i]->isConstant()) {
            valid = false;
            emit_message(msg::ERROR, "case value must be constant", stmt->values[i]->loc);
        }
    }
}

void ValidationVisitor::visitGotoStatement(GotoStatement *stmt) {
    if(!stmt->identifier) {
        valid = false;
        emit_message(msg::ERROR, "goto statement expects following identifier", stmt->loc);
    }
}

void ValidationVisitor::visitCompoundStatement(CompoundStatement *stmt) {
    if(!stmt->getScope() && stmt->statements.size()) {
        valid = false;
        emit_message(msg::ERROR, "compound statement is missing scope", stmt->loc);
    }

    for(int i = 0; i < stmt->statements.size(); i++) {
        if(!stmt->statements[i]) {
            valid = false;
            emit_message(msg::ERROR, "null statement in compound statement", stmt->loc);
        } else stmt->statements[i] = stmt->statements[i]->lower();
    }
}

void ValidationVisitor::visitBlockStatement(BlockStatement *stmt) {
#ifdef DEBUG
    if(!stmt->getScope() && stmt->body) {
        valid = false;
        emit_message(msg::ERROR, "null scope in block statement", location);
    }
#endif

    if(stmt->body) {
        stmt->body = stmt->body->lower();
    }
}

void ValidationVisitor::visitElseStatement(ElseStatement *stmt) {
    if(!stmt->body) {
        valid = false;
        emit_message(msg::ERROR, "else statement missing body statement", location);
    }
}

void ValidationVisitor::visitIfStatement(IfStatement *stmt) {
    if(!stmt->body) {
        valid = false;
        emit_message(msg::ERROR, "if statement missing body statement", location);
    }

    if(!stmt->condition) {
        emit_message(msg::ERROR, "if statement missing condition", location);
    } else {
        stmt->condition = stmt->condition->lower();
        stmt->condition = coerceTo(ASTType::getBoolTy(), stmt->condition);
    }

    if(stmt->elsebr) {
        stmt->elsebr = dynamic_cast<ElseStatement*>(stmt->elsebr->lower());
    }
}

void ValidationVisitor::visitLoopStatement(LoopStatement *stmt) {
    if(!stmt->condition) {
        // lower loop without condition have true condition
        // eg while() --> while(true)
        stmt->condition = new IntExpression(ASTType::getBoolTy(), 1L, stmt->loc);
    } else {
        stmt->condition = stmt->condition->lower();
        stmt->condition = coerceTo(ASTType::getBoolTy(), stmt->condition);
    }

    if(stmt->update) {
        stmt->update = stmt->update->lower();
    }

    if(stmt->elsebr) {
        stmt->elsebr = dynamic_cast<ElseStatement*>(stmt->elsebr->lower());
    }
}

void ValidationVisitor::visitWhileStatement(WhileStatement *stmt) {

}

void ValidationVisitor::visitForStatement(ForStatement *stmt) {
    if(stmt->decl) {
        stmt->decl = stmt->decl->lower();
    }
}

void ValidationVisitor::visitSwitchStatement(SwitchStatement *stmt) {
    if(!stmt->condition) {
        valid = false;
        emit_message(msg::ERROR, "switch statement missing condition", location);
    } else {
        stmt->condition = stmt->condition->lower();
    }
}

void ValidationVisitor::visitReturnStatement(ReturnStatement *stmt) {
    if(stmt->expression) {
        stmt->expression = stmt->expression->lower();

        if(!stmt->expression->getType()->coercesTo(getFunction()->getReturnType())) {
            valid = false;
            emit_message(msg::ERROR, "returned value can not be converted to return type", stmt->loc);
        } else if(!stmt->expression->getType()->is(getFunction()->getReturnType())){
            stmt->expression = coerceTo(getFunction()->getReturnType(), stmt->expression);
        }
    }
}

void ValidationVisitor::visitScope(ASTScope *sc){
	if (sc->empty()) {
		return;
	}
    //ASTScope::iterator end = sc->end();
    for(ASTScope::iterator it = sc->begin(); it != sc->end();){
        Identifier *id = *it;
		if (it != sc->end()) // wtf is this? why isn't ++it just at the top? Windows is a whiner. need to increment before we modify id
			++it;
        if(id->isUndeclared()) {
            id = sc->resolveIdentifier(id);
			if (!id || id->isUndeclared()) {
				emit_message(msg::ERROR, "could not resolve symbol in scope: " + id->getName(), location);
			}
        }

        //TODO: condition shouldnt be needed
        if(id->getType()) {
            resolveType(id->getType());
        }
    }
}

void ValidationVisitor::visitType(ASTType *ty) {

}
