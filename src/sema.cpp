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

        emit_message(msg::ERROR, "invalid argument passed as vararg argument", currentLocation());
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
                emit_message(msg::ERROR, "non-function pointer used in function pointer context", currentLocation());
                return;
            }
            //func = new UnaryExpression(tok::caret, func, currentLocation());
        } else {
            fty = func->overload->getType()->asFunctionType();
            fdecl = func->overload;

            if(!fty) {
                emit_message(msg::ERROR, "attempt to call non-function type", currentLocation());
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
			if (!fty->isVararg()) {
				paramty = fty->params[resolvedArgs.size()];
			}
			else {
				paramty = (*args)->getType();
			}

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
            emit_message(msg::ERROR, "too many arguments passed for function", currentLocation());
        }

        funcargs = resolvedArgs;
    } else if(func->isType()) {
        emit_message(msg::FAILURE, "call expression on type should be lowered by now", currentLocation());
    } else {
        emit_message(msg::ERROR, "invalid function call", currentLocation());
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
        emit_message(msg::FAILURE, "dot expression should be lowered by now", currentLocation());
    }
}

void Sema::visitCallExpression(CallExpression *exp) {
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
                // this is so that structs literals are passed as 'this' as a pointer
                if(dexp->lhs->getType()->isStruct() && dexp->lhs->isLValue()) {
                    exp->args.push_front(new UnaryExpression(tok::amp, dexp->lhs, currentLocation()));
                } else {
                    exp->args.push_front(dexp->lhs);
                }

                // lower dot expression from dotExpression to identifierExpression
                if(dexp->lhs->getType()->isUserType()) {
                    Identifier *fid = dexp->lhs->getType()->asUserType()->getScope()->lookup(dexp->rhs);
                    exp->function = new IdentifierExpression(fid, currentLocation());
                } else if(dexp->lhs->getType()->isUserTypePointer()) {
                    // also lower dot expression where LHS is pointer to user type
                    // XXX should not do this if pointer to class?
                    ASTUserType *uty = dexp->lhs->getType()->asPointer()->getPointerElementTy()->asUserType();
                    Identifier *fid = uty->getScope()->lookup(dexp->rhs);
                    exp->function = new IdentifierExpression(fid, currentLocation());
                }

            } else { // UFCS
                Identifier *fid = getScope()->lookup(dexp->rhs);
                exp->function = new IdentifierExpression(fid, currentLocation());
                exp->args.push_front(dexp->lhs);
                //emit_message(msg::DEBUGGING, "ufcs", currentLocation());
            }
        }

        std::list<ASTNode*> overloads;
        buildOverloadList(exp->function, overloads);
        ASTNode *callfunc = resolveOverloadList(exp->args, overloads);
        if(!callfunc) {
            emit_message(msg::ERROR, "no valid overload found for function '" + exp->function->asString() + "'", currentLocation());
            return;
        }

        exp->resolvedFunction = new FunctionExpression(exp->loc);
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

void Sema::visitNewExpression(NewExpression *exp) {
    // push alloc as 'new' argument

    //XXX kinda messy
    exp->args.push_front(new DummyExpression(exp->type->getReferenceTy()));

    ASTUserType *uty = exp->type->asUserType();
    if(uty && exp->call) {
        exp->function = new FunctionExpression(uty->getConstructor(), currentLocation());
        resolveCallArguments(exp->function, exp->args);
    }

    if(!exp->function && exp->args.size() > 1) {
        emit_message(msg::ERROR, "unknown constructor call", currentLocation());
    }

    //XXX pop 'this'. CG will be responsible to push it again
    exp->args.pop_front();
}

void Sema::visitIfStatement(IfStatement *stmt) {
    stmt->condition = stmt->condition->coerceTo(ASTType::getBoolTy());
}

void Sema::visitBinaryExpression(BinaryExpression *bexp) {
    if(bexp->op.kind == tok::equal) {
        bexp->rhs = bexp->rhs->coerceTo(bexp->lhs->getType());
    }
}
