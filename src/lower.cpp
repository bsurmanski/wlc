#include <string>
#include "ast.hpp"
#include "astType.hpp"
#include "lower.hpp"
#include "message.hpp"
#include "token.hpp" // for getting operator precidence

using namespace std;

//
// Code for AST traversal, validation, and lowering
// if validate returns true, the AST should be in a consistant state, lowered, and ready for codegen;
// else if validate return false, the AST is invalid, and may be left in an inconsistant state.
//

//
// Lower
//

Lower::Lower() : ASTVisitor() {
    valid = true;
}

void Lower::visitFunctionDeclaration(FunctionDeclaration *decl) {
    if(decl->body)
        decl->body = decl->body->lower();
}

void Lower::visitVariableDeclaration(VariableDeclaration *decl) {
    if(decl->value) {
        decl->value = decl->value->lower();
        //XXX if statement is work around:
        // otherwise, on variable declaration of statically-sized arrays with tuple decl, wlc segfault
        // eg: "int[3] vals = [1, 2, 3]"
        if(decl->value &&  decl->value->getType() && !decl->value->getType()->isTuple())
            decl->value = decl->value->coerceTo(decl->getType());
    }
}

void Lower::visitUserTypeDeclaration(UserTypeDeclaration *decl) {
    //XXX lower constructor and destructor?
    for(int i = 0; i < decl->methods.size(); i++) {
        decl->methods[i] = dynamic_cast<FunctionDeclaration*>(decl->methods[i]->lower());
    }

    for(int i = 0; i < decl->members.size(); i++) {
        decl->members[i] = decl->members[i]->lower();
    }

    decl->scope->accept(this);
}

void Lower::visitUnaryExpression(UnaryExpression *exp) {
    //TODO: insert coersion cast
    exp->lhs = exp->lhs->lower();
}

void Lower::visitBinaryExpression(BinaryExpression *exp) {
    //TODO: insert coersion cast
    exp->lhs = exp->lhs->lower();
    exp->rhs = exp->rhs->lower();
}

void Lower::visitCallExpression(CallExpression *exp) {
    exp->function = exp->function->lower();

    std::list<Expression*>::iterator it = exp->args.begin();
    while(it != exp->args.end()) {
        if(!*it) emit_message(msg::FAILURE, "invalid or missing argument in call expression", exp->loc);
        else *it = (*it)->lower();
        it++;
    }
}

void Lower::visitIndexExpression(IndexExpression *exp) {
    exp->lhs = exp->lhs->lower();
    exp->index = exp->index->lower();
}

void Lower::visitCastExpression(CastExpression *exp) {
    exp->expression = exp->expression->lower();
}

void Lower::visitTupleExpression(TupleExpression *exp) {
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
void Lower::visitDotExpression(DotExpression *exp) {
    exp->lhs = exp->lhs->lower();
}

void Lower::visitNewExpression(NewExpression *exp) {
    //XXX newexp->function is resolved after this.
    // therefore it will *always* be null
    if(exp->function) exp->function = exp->function->lower()->functionExpression();

    std::list<Expression*>::iterator it = exp->args.begin();
    while(it != exp->args.end()) {
        if(!*it) emit_message(msg::FAILURE, "invalid or missing argument in call expression", exp->loc);
        else *it = (*it)->lower();
        it++;
    }
}

void Lower::visitCompoundStatement(CompoundStatement *stmt) {
    for(int i = 0; i < stmt->statements.size(); i++) {
        stmt->statements[i] = stmt->statements[i]->lower();
    }
}

void Lower::visitBlockStatement(BlockStatement *stmt) {
    if(stmt->body) {
        stmt->body = stmt->body->lower();
    }
}

void Lower::visitIfStatement(IfStatement *stmt) {
    stmt->condition = stmt->condition->lower();

    if(stmt->elsebr) {
        stmt->elsebr = dynamic_cast<ElseStatement*>(stmt->elsebr->lower());
    }
}

void Lower::visitLoopStatement(LoopStatement *stmt) {
    stmt->condition = stmt->condition->lower();

    if(stmt->update) {
        stmt->update = stmt->update->lower();
    }

    if(stmt->elsebr) {
        stmt->elsebr = dynamic_cast<ElseStatement*>(stmt->elsebr->lower());
    }
}

void Lower::visitForStatement(ForStatement *stmt) {
    if(stmt->decl) {
        stmt->decl = stmt->decl->lower();
    }
}

void Lower::visitSwitchStatement(SwitchStatement *stmt) {
    stmt->condition = stmt->condition->lower();
}

void Lower::visitReturnStatement(ReturnStatement *stmt) {
    if(stmt->expression) {
        stmt->expression = stmt->expression->lower();
    }
}
