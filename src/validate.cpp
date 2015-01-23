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
// ValidationVisitor
//

ValidationVisitor::ValidationVisitor() : ASTVisitor() {
}

Identifier *ValidationVisitor::resolveIdentifier(Identifier *id) {
    if(id->isUndeclared()) {
        id = id->getScope()->resolveIdentifier(id);
    }

    if(id->isUndeclared()) {
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
                emit_message(msg::FAILURE, "undeclared user type", currentLocation());
                return NULL;
            }

            //location = ty->getDeclaration()->loc;
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
            emit_message(msg::ERROR, "invalid 0-tuple", currentLocation());
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
        emit_message(msg::FATAL, "invalid type", currentLocation());
    }

    return ty;
}

void ValidationVisitor::visitPackage(PackageDeclaration *pak) {
    if(!pak->getScope()) {
        emit_message(msg::ERROR, "package is missing scope", currentLocation());
    }
}

void ValidationVisitor::visitModule(ModuleDeclaration *mod) {
    ASTScope::iterator it;
    for(it = mod->importScope->begin(); it != mod->importScope->end(); it++) {
        if(!it->getDeclaration()->moduleDeclaration()) {
            emit_message(msg::ERROR, "invalid translation unit found, null import", currentLocation());
        }
    }
}

void ValidationVisitor::visitDeclaration(Declaration *decl) {
    ASTVisitor::visitDeclaration(decl);

    if(!decl->identifier) {
        emit_message(msg::ERROR, "null identifier in declaration", currentLocation());
    }

    if(decl->identifier->isUndeclared()) {
        emit_message(msg::ERROR, "inconsistant AST, undeclared identifier in definition", currentLocation());
    }

    if(decl->qualifier.weak && !decl->getType()->isClass()) {
        emit_message(msg::ERROR, "weak qualifier only applies to class type variables", currentLocation());
    }
}

void ValidationVisitor::visitFunctionDeclaration(FunctionDeclaration *decl) {
    //TODO: validate prototype
    if(!decl->getType()) {
        emit_message(msg::ERROR, "function declaration missing prototype", currentLocation());
    }

    if(!decl->getScope() && decl->body) {
        emit_message(msg::ERROR, "function has body without scope", currentLocation());
    }

    resolveType(decl->getType());
}

void ValidationVisitor::visitVariableDeclaration(VariableDeclaration *decl) {
    if(decl->getType()->kind == TYPE_DYNAMIC) {
        if(!decl->value) {
            emit_message(msg::ERROR,
                    "dynamically typed variables must have valid initializer", currentLocation());
        }

        decl->type = decl->value->getType();
    }

    resolveType(decl->getType());

    if(decl->qualifier.isStatic && decl->value && !decl->value->isConstant()) {
        emit_message(msg::ERROR, "static variable may only have constant initial value", currentLocation());
    }
}

void ValidationVisitor::visitTypeDeclaration(TypeDeclaration *decl) {
    if(!decl->getDeclaredType()) {
        emit_message(msg::ERROR, "invalid type declaration", currentLocation());
    }
}

void ValidationVisitor::visitUserTypeDeclaration(UserTypeDeclaration *decl) {
    if(!decl->type->isResolved()) {
        emit_message(msg::ERROR, "unresolved user type declaration", currentLocation());
    }

	ClassDeclaration *cldecl = decl->classDeclaration();
    if(cldecl) {
        if(cldecl->base) {
            cldecl->base = resolveIdentifier(cldecl->base);
            if(!cldecl->base->isClass() && !cldecl->base->isStruct()) {
                emit_message(msg::ERROR, "expected class type in base specifier", currentLocation());
                if(cldecl->base->isInterface()) {
                    emit_message(msg::ERROR, "interfaces are specified implicitly", currentLocation());
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

        idecl->populateVTable(); // this only registers vtable indices
    }

    //XXX lower constructor and destructor?
    for(int i = 0; i < decl->methods.size(); i++) {
        if(!decl->methods[i]) emit_message(msg::ERROR, "invalid method found in user type declaration", decl->loc);
    }

    for(int i = 0; i < decl->members.size(); i++) {
        if(!decl->members[i]) emit_message(msg::ERROR, "invalid member found in user type declaration", decl->loc);
    }

    for(int i = 0; i < decl->staticMembers.size(); i++) {
        if(!decl->staticMembers[i]) emit_message(msg::ERROR, "invalid static member found in user type declaration", decl->loc);
    }

    decl->scope->accept(this);
}

void ValidationVisitor::visitUnaryExpression(UnaryExpression *exp) {
    //TODO: insert coersion cast
    if(!exp->lhs) {
        emit_message(msg::ERROR, "unary operator is missing expression", currentLocation());
    }

    if(exp->op == tok::dot && !exp->lhs->identifierExpression()) {
        emit_message(msg::ERROR, "unary dot operator expects identifier following dot", currentLocation());
    }

    if(!getUnaryPrecidence((tok::TokenKind) exp->op)) {
        emit_message(msg::FAILURE, "unary expression found without unary operation", currentLocation());
    }

    switch(exp->op) {
        case tok::plusplus:
            if(!exp->lhs->isLValue()) {
                emit_message(msg::ERROR, "++ operator expects LValue", currentLocation());
            }

            if(!exp->lhs->getType()->isNumeric() && !exp->lhs->getType()->isPointer()) {
                emit_message(msg::ERROR, "++ operator is only valid on numeric expressions", currentLocation());
            }
            break;
        case tok::minusminus:
            if(!exp->lhs->isLValue()) {
                emit_message(msg::ERROR, "-- operator expects LValue", currentLocation());
            }

            if(!exp->lhs->getType()->isNumeric() && !exp->lhs->getType()->isPointer()) {
                emit_message(msg::ERROR, "-- operator is only valid on numeric expressions", currentLocation());
            }
            break;
        case tok::plus:
        case tok::minus:
            if(!exp->lhs->getType()->isNumeric()) {
                emit_message(msg::ERROR, "unary +- is only valid on numeric expressions", currentLocation());
            }
            break;
        case tok::bang:
            if(!exp->lhs->getType()->isBool()) {
                exp->lhs = exp->lhs->coerceTo(ASTType::getBoolTy());
            }
            break;
        case tok::tilde:
            emit_message(msg::UNIMPLEMENTED, "unimplemented unary operator '~'", currentLocation());
            break;
        case tok::caret:
            if(!exp->lhs->getType()->isPointer()) {
                emit_message(msg::ERROR, "attempt to dereference non-pointer type", currentLocation());
            }
            break;
        case tok::amp:
            if(!exp->lhs->isLValue()) {
                emit_message(msg::ERROR, "& operator expects LValue", currentLocation());
            }
            break;
    }
}

void ValidationVisitor::visitBinaryExpression(BinaryExpression *exp) {
    //TODO: insert coersion cast
    if(!exp->lhs) {
        emit_message(msg::ERROR, "binary operator is missing left hand expression", currentLocation());
    }

    if(!exp->rhs) {
        emit_message(msg::ERROR, "binary operator is missing right hand expression", currentLocation());
    }

    if(!exp->op.getBinaryPrecidence()) {
        emit_message(msg::FAILURE, "invalid operator found in binary expression", currentLocation());
    }

    // should never happen
    if(exp->op.kind == tok::colon) {
        emit_message(msg::FAILURE, "cast found as binary expression", currentLocation());
    }

    // should never happen
    if(exp->op.kind == tok::dot) {
        emit_message(msg::FAILURE, "dot found as binary expression", currentLocation());
    }
}

void ValidationVisitor::visitPackExpression(PackExpression *exp) {
    if(exp->filesize <= 0) {
        emit_message(msg::ERROR, "attempt to pack missing or empty file", currentLocation());
    }
}

void ValidationVisitor::visitCallExpression(CallExpression *exp) {
    if(!exp->function) {
        emit_message(msg::FAILURE, "invalid or missing function in call expression", exp->loc);
    } else {
        // would this be considered lowering?
        // tried putting that there, but i think the
        // lowering was not called... might need to look into that
        //
        // this is a call on a type. a stack constructor
        // eg. MyStruct st = MyStruct(1, 2, 3)
        if(exp->function->isType()) {
            ASTUserType *uty = exp->function->getDeclaredType()->asUserType();
            if(!uty) {
                emit_message(msg::ERROR, "invalid call on type", exp->loc);
            } else if(!uty->getConstructor()) {
                emit_message(msg::ERROR, "missing constructor for type " + uty->getName(), exp->loc);
            } else {
                /*
                exp->isConstructor = true;
                exp->function = new IdentifierExpression(uty->getConstructor()->getIdentifier(), exp->loc);
                Expression *this_exp = new StackAllocExpression(uty, exp->loc);
                if(uty->isStruct()) this_exp = new UnaryExpression(tok::amp, this_exp, currentLocation());
                exp->args.push_front(this_exp);
                */
            }
        }
    }
}

void ValidationVisitor::visitIndexExpression(IndexExpression *exp) {
    if(!exp->lhs) {
        emit_message(msg::ERROR, "invalid or missing base for index expression", exp->loc);
    }

    if(!exp->index) {
        emit_message(msg::ERROR, "invalid or missing index in index expression", exp->loc);
    }
}

void ValidationVisitor::visitIdentifierExpression(IdentifierExpression *exp) {
    // resolve identifier
    exp->id = resolveIdentifier(exp->id);

    if(exp->id->isUndeclared()){
        emit_message(msg::ERROR, "identifier is expected to be resolved at this point", currentLocation());
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

void ValidationVisitor::visitImportExpression(ImportExpression *exp) {
    if(!exp->expression) {
        emit_message(msg::ERROR, "import expression expects following expression", currentLocation());
    }

    if(!dynamic_cast<StringExpression*>(exp->expression)) {
        emit_message(msg::ERROR, "import expression expects following expression to be package string", currentLocation());
    }
}

void ValidationVisitor::visitCastExpression(CastExpression *exp) {
    resolveType(exp->type);
    if(!exp->expression) {
        emit_message(msg::ERROR, "missing value in cast expression", exp->loc);
    }
}

void ValidationVisitor::visitTupleExpression(TupleExpression *exp) {
    for(int i = 0; i < exp->members.size(); i++) {
        if(!exp->members[i]) emit_message(msg::ERROR, "expression in tuple expression", exp->loc);
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
    }

    if(exp->lhs->isValue()) {
        ASTType *lhstype = exp->lhs->getType();
        Identifier *rhsid = NULL;

        //TODO: check for UFCS
        // dereference pointer types on dot expression
        if(lhstype->isPointer() && lhstype->getPointerElementTy()->isStruct()) {
            rhsid = lhstype->getPointerElementTy()->getDeclaration()->lookup(exp->rhs);
            if(!rhsid) {
                //looks like ufcs
                //TODO: unify test test with below stuff
                Identifier *ufcs_id = getScope()->lookup(exp->rhs);
                ufcs_id = resolveIdentifier(ufcs_id);
                if(!ufcs_id->isFunction()) {
                    emit_message(msg::ERROR, "member '" + exp->rhs + "' not found in user type '" + lhstype->getName() + "'", currentLocation());
                }
                return;
            } else if(rhsid->isVariable()) {
                //XXX this should be in sema
                exp->lhs = new UnaryExpression(tok::caret, exp->lhs, currentLocation());
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
                    emit_message(msg::ERROR, "member '" + exp->rhs + "' not found in user type '" + lhstype->getName() + "'", currentLocation());
                }
            }

            //XXX hacky. in case rhs is userType, it's type can be
            // unresolved before used
            if(Declaration *rhsdecl = rhsid->getDeclaration()) {
                resolveType(rhsdecl->getType());
            }

            if(rhsid->isUndeclared()) {
                emit_message(msg::WARNING, "undeclared member identifier in validate", currentLocation());
            }
        } else if(lhstype->isArray()) {
            if(exp->rhs != "size" && exp->rhs != "ptr") {
                emit_message(msg::ERROR, "invalid property '" + exp->rhs + "' in array", currentLocation());
            }
        } else {
            emit_message(msg::ERROR, "invalid dot expression on non-user type", currentLocation());
        }

    } else if(exp->lhs->isType()) {
        ASTType *declty = exp->lhs->getDeclaredType();
        if(!declty) {
            emit_message(msg::ERROR, "invalid lhs of dot expression", currentLocation());
        }

        if(exp->rhs == "sizeof" || exp->rhs == "offsetof") {
            // i think we're fine
        } else {
            //TODO: look up static members / functions
            //TODO: validate static members
            //emit_message(msg::UNIMPLEMENTED, "no static members or function lookup availible", currentLocation());
        }
    } else {
        emit_message(msg::FAILURE, "unresolved dot expression", currentLocation());
    }
}

void ValidationVisitor::visitNewExpression(NewExpression *exp) {
    resolveType(exp->type);

    ASTUserType *uty = exp->type->asUserType();
    if(uty && exp->call) {
        if(!uty->getConstructor()) {
            emit_message(msg::ERROR, "invalid constructor call on type without constructor", currentLocation());
            return;
        }
    }

    if(exp->type->kind == TYPE_DYNAMIC_ARRAY) {
        emit_message(msg::ERROR, "cannot create unsized array in 'new' expression", currentLocation());
    }
}

void ValidationVisitor::visitAllocExpression(AllocExpression *exp) {
    exp->type = resolveType(exp->type);
}

void ValidationVisitor::visitLabelStatement(LabelStatement *stmt) {
    if(!stmt->identifier) {
        emit_message(msg::ERROR, "label statement expects following identifier", currentLocation());
    }
}

void ValidationVisitor::visitCaseStatement(CaseStatement *stmt) {
    for(int i = 0; i < stmt->values.size(); i++) {
        if(!stmt->values[i]) {
            emit_message(msg::ERROR, "invalid case value in case statement", stmt->loc);
        } else if(!stmt->values[i]->isConstant()) {
            emit_message(msg::ERROR, "case value must be constant", stmt->values[i]->loc);
        }
    }
}

void ValidationVisitor::visitGotoStatement(GotoStatement *stmt) {
    if(!stmt->identifier) {
        emit_message(msg::ERROR, "goto statement expects following identifier", stmt->loc);
    }
}

void ValidationVisitor::visitCompoundStatement(CompoundStatement *stmt) {
    if(!stmt->getScope() && stmt->statements.size()) {
        emit_message(msg::ERROR, "compound statement is missing scope", stmt->loc);
    }

    for(int i = 0; i < stmt->statements.size(); i++) {
        if(!stmt->statements[i]) {
            emit_message(msg::ERROR, "null statement in compound statement", stmt->loc);
        }
    }
}

void ValidationVisitor::visitBlockStatement(BlockStatement *stmt) {
#ifdef DEBUG
    if(!stmt->getScope() && stmt->body) {
        emit_message(msg::ERROR, "null scope in block statement", currentLocation());
    }
#endif
}

void ValidationVisitor::visitElseStatement(ElseStatement *stmt) {
    if(!stmt->body) {
        emit_message(msg::ERROR, "else statement missing body statement", currentLocation());
    }
}

void ValidationVisitor::visitIfStatement(IfStatement *stmt) {
    if(!stmt->body) {
        emit_message(msg::ERROR, "if statement missing body statement", currentLocation());
    }

    if(!stmt->condition) {
        emit_message(msg::ERROR, "if statement missing condition", currentLocation());
    } else {
        stmt->condition = stmt->condition->coerceTo(ASTType::getBoolTy());
    }
}

void ValidationVisitor::visitLoopStatement(LoopStatement *stmt) {
    if(!stmt->condition) {
        // fix loop without condition have true condition
        // eg while() --> while(true)
        stmt->condition = new IntExpression(ASTType::getBoolTy(), 1L, stmt->loc);
    }
}

void ValidationVisitor::visitSwitchStatement(SwitchStatement *stmt) {
    if(!stmt->condition) {
        emit_message(msg::ERROR, "switch statement missing condition", currentLocation());
    }
}

void ValidationVisitor::visitReturnStatement(ReturnStatement *stmt) {
    if(stmt->expression) {
        if(!stmt->expression->coercesTo(getFunction()->getReturnType())) {
            emit_message(msg::ERROR, "returned value can not be converted to return type", stmt->loc);
        } else if(!stmt->expression->getType()->is(getFunction()->getReturnType())) {
            stmt->expression = stmt->expression->coerceTo(getFunction()->getReturnType());
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
				emit_message(msg::ERROR, "could not resolve symbol in scope: " + id->getName(), currentLocation());
			}
        }

        //TODO: condition shouldnt be needed
        if(id->getType()) {
            resolveType(id->getType());
        }
    }
}
