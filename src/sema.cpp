#include <string>
#include "ast.hpp"
#include "astType.hpp"
#include "sema.hpp"
#include "message.hpp"
#include "token.hpp" // for getting operator precidence

using namespace std;

//
// Code for AST traversal, validation, and lowering
// if validate returns true, the AST should be in a consistant state, lowered, and ready for codegen;
// else if validate return false, the AST is invalid, and may be left in an inconsistant state.
//

//
// Sema
//

Sema::Sema() : ASTVisitor() {
    valid = true;
}

/**
 * get the type which both t1 and t2 may promote to
 */
ASTType *Sema::commonType(ASTType *t1, ASTType *t2) {
    if(t1->is(t2)) return t1;


    if(t1->isUserType() || t2->isUserType()) {
        if(t1->extends(t2)) return t2;
        if(t2->extends(t1)) return t1;

        if(t1->isUserType() != t2->isUserType()) {
            emit_message(msg::ERROR, "attempt to cast user type to non-user type", location);
        }
    }

    if(t1->isNumeric() && t2->isNumeric()) {
        if(t1->getPriority() > t2->getPriority()) return t1;
        else return t2;
    }

    return NULL;
}

void Sema::visitFunctionDeclaration(FunctionDeclaration *decl) {
    if(decl->body)
        decl->body = decl->body->lower();
}

void Sema::visitVariableDeclaration(VariableDeclaration *decl) {
    if(decl->value) decl->value = decl->value->lower();
}

void Sema::visitUserTypeDeclaration(UserTypeDeclaration *decl) {
    //XXX lower constructor and destructor?
    for(int i = 0; i < decl->methods.size(); i++) {
        decl->methods[i] = dynamic_cast<FunctionDeclaration*>(decl->methods[i]->lower());
    }

    for(int i = 0; i < decl->members.size(); i++) {
        decl->members[i] = decl->members[i]->lower();
    }

    decl->scope->accept(this);
}

void Sema::visitUnaryExpression(UnaryExpression *exp) {
    //TODO: insert coersion cast
    exp->lhs = exp->lhs->lower();
}

void Sema::visitBinaryExpression(BinaryExpression *exp) {
    //TODO: insert coersion cast
    exp->lhs = exp->lhs->lower();
    exp->rhs = exp->rhs->lower();
}

Expression *Sema::resolveCallArgument(ASTFunctionType *fty, unsigned i, Expression *arg, Expression *def) {
    ASTType *argty = NULL;
    ASTType *paramty = NULL;

    if(arg) argty = arg->getType();
    if(i < fty->params.size()) paramty = fty->params[i];

    if(fty->isVararg() && i >= fty->params.size()) {
        if(argty->isNumeric()) {
            if(argty->isFloating()) {
                return arg->coerceTo(ASTType::getDoubleTy());
            } else if(argty->isInteger() && argty->getSize() < 4) {
                return arg->coerceTo(ASTType::getIntTy());
            } else {
                return arg;
            }
        }

        // arrays cannot directly convert to void^.
        // strings (char[]) can convert to char^, so try void^: char^: arr
        // This should only allow strings as varargs
        if(argty->isArray() && argty->getPointerElementTy()->coercesTo(ASTType::getCharTy())) {
            return
               arg->coerceTo(ASTType::getCharTy()->getPointerTy())->
                    coerceTo(ASTType::getVoidTy()->getPointerTy());
        }

        if(argty->coercesTo(ASTType::getVoidTy()->getPointerTy())) {
            return arg->coerceTo(ASTType::getVoidTy()->getPointerTy());
        }

        emit_message(msg::ERROR, "invalid argument passed as vararg argument", location);
        return NULL;
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

void Sema::resolveCallArguments(FunctionExpression *func, std::list<Expression*>& funcargs) {
    std::list<Expression*> resolvedArgs;

    if(func) {
        // dereference function pointer

        ASTFunctionType *fty = NULL;
        FunctionDeclaration *fdecl = NULL;
        if(func->fpointer) {
            fty = func->fpointer->getType()->getPointerElementTy()->asFunctionType();
            if(!fty) {
                emit_message(msg::ERROR, "non-function pointer used in function pointer context", location);
                return;
            }
            //func = new UnaryExpression(tok::caret, func, location);
        } else {
            fty = func->overload->getType()->asFunctionType();
            fdecl = func->overload;

            if(!fty) {
                emit_message(msg::ERROR, "attempt to call non-function type", location);
                return;
            }
        }

        std::list<Expression*>::iterator args = funcargs.begin();

        while(resolvedArgs.size() < fty->params.size() ||
                (fty->isVararg() && args != funcargs.end())) {
            Expression *arg = NULL;
            Expression *defaultArg = NULL;
            ASTType *paramty = NULL;
            //XXX below will break on varargs
            paramty  = fty->params[resolvedArgs.size()];

            if(args != funcargs.end()) {
                arg = *args;
            }

            if(fdecl) {
                defaultArg = fdecl->getDefaultParameter(resolvedArgs.size());
            }

            Expression *resolvedArg = resolveCallArgument(fty, resolvedArgs.size(), arg, defaultArg);

            if(!resolvedArg) {
                std::stringstream ss;
                ss << "cannot resolve argument " <<  resolvedArgs.size() << " of function call. cannot convert argument of type '" << arg->getType()->getName() << "'" << "to '" << paramty->getName() << "'";
                emit_message(msg::ERROR, ss.str(), func->loc);
            }

            resolvedArgs.push_back(resolvedArg);

            args++;
        }

        if(args != funcargs.end()) {
            emit_message(msg::ERROR, "too many arguments passed for function", location);
        }

        funcargs = resolvedArgs;
    } else if(func->isType()) {
        emit_message(msg::FAILURE, "call expression on type should be lowered by now", location);
        //TODO: properly resolve arguments
        //XXX: should this be in lowering?

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

OverloadValidity Sema::resolveOverloadValidity(std::list<Expression*> args, ASTNode *overload) {
    ASTFunctionType *fty = NULL;
    FunctionDeclaration *fdecl = NULL;

    OverloadValidity ret = OverloadValidity::FULL_MATCH;

    if(overload->declaration()) {
        fdecl = overload->declaration()->functionDeclaration();
        fty = fdecl->getType();
    } else if(overload->expression()) {
        ASTType *expty = overload->expression()->getType();
        if(expty) {
            if(expty->isFunctionType()) {
                fty = expty->asFunctionType();
            }
            if(expty->isFunctionPointer()) {
                fty = expty->getPointerElementTy()->asFunctionType();
            }
        }
    }

    if(!fty) return OverloadValidity::INVALID;
    if(fty->params.size() < args.size() && !fty->isVararg()) return OverloadValidity::INVALID;

    std::list<Expression*>::iterator args_it = args.begin();
    for(int i = 0; i < fty->params.size(); i++) {
        if(args_it == args.end()) {
            if(fdecl && fdecl->getDefaultParameter(i)) {
                if(ret > OverloadValidity::DEFAULT_MATCH)
                    ret = OverloadValidity::DEFAULT_MATCH;
                continue;
            } else {
                return OverloadValidity::INVALID;
            }
        }

        if((*args_it)->getType()->is(fty->params[i])) {
            // do nothing
            // break to it++
        } else if((*args_it)->getType()->coercesTo(fty->params[i])) {
            if(ret > OverloadValidity::COERCE_MATCH) ret = OverloadValidity::COERCE_MATCH;
            // break to it++
        }

        args_it++;
    }

    return ret;
}

ASTNode *Sema::resolveOverloadList(std::list<Expression*> args, std::list<ASTNode*>& overload) {
    std::list<ASTNode*>::iterator it = overload.begin();

    bool ambiguous = false; // if we get multiple resolutions of same validity
    OverloadValidity validity = OverloadValidity::INVALID;
    ASTNode *current = NULL;

    while(it != overload.end()) {
        OverloadValidity olval = resolveOverloadValidity(args, *it);
        if(olval > validity) {
            validity = olval;
            current = *it;
            ambiguous = false;
        } else if(olval == validity) {
            ambiguous = true;
        }
        it++;
    }

    if(ambiguous) return NULL;
    return current;
}

void Sema::buildOverloadList(Expression *func, std::list<ASTNode*>& overload) {
    if(func->getType()->isFunctionPointer()) {
        overload.push_back(func);
    } else if(func->identifierExpression()) {
        //TODO: find overloads
        FunctionDeclaration *decl = func->identifierExpression()->getDeclaration()->functionDeclaration();
        while(decl) {
            overload.push_back(decl);
            decl = decl->getNextOverload();
        }
    } else if(func->dotExpression()) {
        emit_message(msg::ERROR, "dot expression should be lowered by now", location);
    }
}

void Sema::visitCallExpression(CallExpression *exp) {
    if(!exp->function) {
        emit_message(msg::FAILURE, "invalid or missing function in call expression", exp->loc);
    } else {
        exp->function = exp->function->lower();

        // would this be considered lowering?
        // tried putting that there, but i think the
        // lowering was not called... might need to look into that
        //
        // this is a call on a type. a stack constructor
        // eg. MyStruct st = MyStruct(1, 2, 3)
        if(exp->function->isType()) {
            ASTUserType *uty = exp->function->getDeclaredType()->asUserType();
            if(uty) {
                if(!uty->getConstructor()) {
                    emit_message(msg::ERROR, "missing constructor for type " + uty->getName(), exp->loc);
                } else {
                    // swap out function for constructor, and push an alloca'd struct as the passed arg
                    exp->isConstructor = true;
                    exp->function = new IdentifierExpression(uty->getConstructor()->getIdentifier(), exp->loc);
                    exp->args.push_front(new StackAllocExpression(uty, exp->loc));
                }
            } else {
                emit_message(msg::ERROR, "invalid call on type", exp->loc);
            }
        }

        // if lhs is dot expression, this is either method call or UFCS.
        // Decide which, and push 'this' variable as function argument if required
        //
        // its unfortunate that we need to make a slight exception for dot expressions,
        // but probably necessary.
        DotExpression *dexp = exp->function->dotExpression();
        if(dexp) {
            if(dexp->isValue()) {
                // this is so that structs literals are passed as 'this' as a pointer
                if(dexp->lhs->getType()->isStruct() && dexp->lhs->isLValue()) {
                    exp->args.push_front(new UnaryExpression(tok::amp, dexp->lhs, location));
                } else {
                    exp->args.push_front(dexp->lhs);
                }

                // lower dot expression from dotExpression to identifierExpression
                if(dexp->lhs->getType()->isUserType()) {
                    Identifier *fid = dexp->lhs->getType()->asUserType()->getScope()->lookup(dexp->rhs);
                    exp->function = new IdentifierExpression(fid, location);
                } else if(dexp->lhs->getType()->isUserTypePointer()) {
                    // also lower dot expression where LHS is pointer to user type
                    // XXX should not do this if pointer to class?
                    ASTUserType *uty = dexp->lhs->getType()->asPointer()->getPointerElementTy()->asUserType();
                    Identifier *fid = uty->getScope()->lookup(dexp->rhs);
                    exp->function = new IdentifierExpression(fid, location);
                }

            } else { // UFCS
                Identifier *fid = getScope()->lookup(dexp->rhs);
                exp->function = new IdentifierExpression(fid, location);
                exp->args.push_front(dexp->lhs);
                emit_message(msg::DEBUGGING, "ufcs", location);
            }
        }

        std::list<ASTNode*> overloads;
        buildOverloadList(exp->function, overloads);
        ASTNode *callfunc = resolveOverloadList(exp->args, overloads);
        if(!callfunc) {
            emit_message(msg::ERROR, "no valid overload found", location);
            return;
        }

        exp->resolvedFunction = new FunctionExpression;
        if(callfunc->expression()) {
            exp->resolvedFunction->fpointer = callfunc->expression();
        } else if(callfunc->declaration()) {
            exp->resolvedFunction->overload = callfunc->declaration()->functionDeclaration();
        }

        resolveCallArguments(exp->resolvedFunction, exp->args);
    }

    // only lower arguments if we don't already have an error
    if(currentErrorLevel() < msg::ERROR) {
        std::list<Expression*>::iterator it = exp->args.begin();
        while(it != exp->args.end()) {
            if(!*it) emit_message(msg::FAILURE, "invalid or missing argument in call expression", exp->loc);
            else *it = (*it)->lower();
            it++;
        }
    }
}

void Sema::visitIndexExpression(IndexExpression *exp) {
    exp->lhs = exp->lhs->lower();
    exp->index = exp->index->lower();
}

void Sema::visitCastExpression(CastExpression *exp) {
    exp->expression = exp->expression->lower();
}

void Sema::visitTupleExpression(TupleExpression *exp) {
    for(int i = 0; i < exp->members.size(); i++) {
        exp->members[i] = exp->members[i]->lower();
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
void Sema::visitDotExpression(DotExpression *exp) {
    exp->lhs = exp->lhs->lower();

    if(exp->lhs->isValue()) {
        ASTType *lhstype = exp->lhs->getType();
        Identifier *rhsid = NULL;

        //TODO: check for UFCS
        // dereference pointer types on dot expression
        if(lhstype->isPointer() && lhstype->getPointerElementTy()->isStruct()) {
            rhsid = lhstype->getPointerElementTy()->getDeclaration()->lookup(exp->rhs);
            if(rhsid->isVariable()) {
                exp->lhs = new UnaryExpression(tok::caret, exp->lhs, location);
                lhstype = exp->lhs->getType();
            } else if(rhsid->isFunction()) {
                // functions take pointer
                return;
            }
        }
    }
}

void Sema::visitNewExpression(NewExpression *exp) {
    // push alloc as 'new' argument
    exp->args.push_front(exp->alloc);

    ASTUserType *uty = exp->type->asUserType();
    if(uty && exp->call) {
        exp->function = new FunctionExpression(uty->getConstructor(), location);
        //resolveType(exp->function->getType()); //XXX temp. some constructor args-type would not be defined.

        resolveCallArguments(exp->function, exp->args);
    }
}

void Sema::visitCompoundStatement(CompoundStatement *stmt) {
    for(int i = 0; i < stmt->statements.size(); i++) {
        stmt->statements[i] = stmt->statements[i]->lower();
    }
}

void Sema::visitBlockStatement(BlockStatement *stmt) {
    if(stmt->body) {
        stmt->body = stmt->body->lower();
    }
}

void Sema::visitIfStatement(IfStatement *stmt) {
    stmt->condition = stmt->condition->lower();
    stmt->condition = stmt->condition->coerceTo(ASTType::getBoolTy());

    if(stmt->elsebr) {
        stmt->elsebr = dynamic_cast<ElseStatement*>(stmt->elsebr->lower());
    }
}

void Sema::visitLoopStatement(LoopStatement *stmt) {
    stmt->condition = stmt->condition->lower();

    if(stmt->update) {
        stmt->update = stmt->update->lower();
    }

    if(stmt->elsebr) {
        stmt->elsebr = dynamic_cast<ElseStatement*>(stmt->elsebr->lower());
    }
}

void Sema::visitForStatement(ForStatement *stmt) {
    if(stmt->decl) {
        stmt->decl = stmt->decl->lower();
    }
}

void Sema::visitSwitchStatement(SwitchStatement *stmt) {
    stmt->condition = stmt->condition->lower();
}

void Sema::visitReturnStatement(ReturnStatement *stmt) {
    if(stmt->expression) {
        stmt->expression = stmt->expression->lower();
    }
}
