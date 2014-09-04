#include <string>
#include "ast.hpp"
#include "astType.hpp"
#include "validate.hpp"
#include "message.hpp"

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
        emit_message(msg::ERROR, string("undeclared variable '") + id->getName()
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
            userty->getScope()->accept(this);
        }
    }

    if(ASTFunctionType *fty = ty->asFunctionType()){
        for(int i = 0; i < fty->params.size(); i++){
            fty->params[i] = resolveType(fty->params[i]);
        }

        if(fty->owner){
            resolveType(fty->owner);
        }

        resolveType(fty->ret);
    }

    if(ty->isPointer()){
        resolveType(ty->getPointerElementTy());
    }

    return ty;
}

void ValidationVisitor::visitPackage(Package *pak) {
    if(!pak->getScope()) valid = false;
}

void ValidationVisitor::visitTranslationUnit(TranslationUnit *tu) {
    for(int i = 0; i < tu->imports.size(); i++) {
        if(!tu->imports[i]) {
            valid = false;
            emit_message(msg::ERROR, "invalid translation unit found, null import");
        }
    }
}

void ValidationVisitor::visitDeclaration(Declaration *decl) {
    if(!decl->identifier) {
        valid = false;
        emit_message(msg::ERROR, "null identifier in declaration");
    }

    if(decl->identifier->isUndeclared()) {
        valid = false;
        emit_message(msg::ERROR, "inconsistant AST, undeclared identifier in definition");
    }

    if(decl->qualifier.weak && !decl->getType()->isClass()) {
        valid = false;
        emit_message(msg::ERROR, "weak qualifier only applies to class type variables");
    }
}

void ValidationVisitor::visitExpression(Expression *exp) {

}

void ValidationVisitor::visitStatement(Statement *stmt) {

}

void ValidationVisitor::visitFunctionDeclaration(FunctionDeclaration *decl) {
    //TODO: validate prototype
    if(!decl->getType()) {
        valid = false;
        emit_message(msg::ERROR, "function declaration missing prototype");
    }

    if(!decl->getScope() && decl->body) {
        valid = false;
        emit_message(msg::ERROR, "function has body without scope");
    }

    resolveType(decl->getType());

    if(decl->body)
        decl->body = decl->body->lower();
}

void ValidationVisitor::visitLabelDeclaration(LabelDeclaration *decl) {

}

void ValidationVisitor::visitVariableDeclaration(VariableDeclaration *decl) {
    if(decl->getType()->kind == TYPE_DYNAMIC) {
        if(!decl->value) {
            valid = false;
            emit_message(msg::ERROR,
                    "dynamically typed variables must have valid initializer", decl->loc);
        }
    }

    resolveType(decl->getType());

    if(decl->value) decl->value = decl->value->lower();
}

void ValidationVisitor::visitTypeDeclaration(TypeDeclaration *decl) {
    if(!decl->getDeclaredType()) {
        valid = false;
        emit_message(msg::ERROR, "invalid type declaration");
    }
}

void ValidationVisitor::visitUserTypeDeclaration(UserTypeDeclaration *decl) {
    if(!decl->type->isResolved()) {
        valid = false;
        emit_message(msg::ERROR, "unresolved user type declaration", decl->loc);
    }

    if(ClassDeclaration *cdecl = decl->classDeclaration()) {
        if(cdecl->base) {
            cdecl->base = resolveIdentifier(cdecl->base);
            if(!cdecl->base->isClass() && !cdecl->base->isStruct()) {
                emit_message(msg::ERROR, "expected class type in base specifier", cdecl->loc);
                if(cdecl->base->isInterface()) {
                    emit_message(msg::ERROR, "interfaces are specified implicitly", cdecl->loc);
                }
            }
        }

        cdecl->populateVTable();
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
        emit_message(msg::ERROR, "unary operator is missing expression");
    } else {
        exp->lhs = exp->lhs->lower();
    }

    if(exp->op == tok::dot && !exp->lhs->identifierExpression()){
        valid = false;
        emit_message(msg::ERROR, "unary dot operator expects identifier following dot");
    }
}

void ValidationVisitor::visitBinaryExpression(BinaryExpression *exp) {
    //TODO: insert coersion cast
    if(!exp->lhs) {
        valid = false;
        emit_message(msg::ERROR, "binary operator is missing left hand expression");
    } else {
        exp->lhs = exp->lhs->lower();
    }

    if(!exp->rhs) {
        valid = false;
        emit_message(msg::ERROR, "binary operator is missing right hand expression");
    } else {
        exp->rhs = exp->rhs->lower();
    }
}

void ValidationVisitor::visitPrimaryExpression(PrimaryExpression *exp) {

}

void ValidationVisitor::visitCallExpression(CallExpression *exp) {
    if(!exp->function) {
        emit_message(msg::FAILURE, "invalid or missing function in call expression", exp->loc);
    } else {
        exp->function = exp->function->lower();
    }

    for(int i = 0; i < exp->args.size(); i++) {
        if(!exp->args[i]) emit_message(msg::FAILURE, "invalid or missing argument in call expression", exp->loc);
        else exp->args[i] = exp->args[i]->lower();
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
        emit_message(msg::ERROR, "identifier is expected to be resolved at this point");
    }

    // resolve type of identifier if needed
    if(exp->id->getType()) //XXX temp?
        resolveType(exp->id->getType());

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
        emit_message(msg::ERROR, "import expression expects following expression");
    }

    if(!dynamic_cast<StringExpression*>(exp->expression)) {
        valid = false;
        emit_message(msg::ERROR, "import expression expects following expression to be package string");
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

void ValidationVisitor::visitDotExpression(DotExpression *exp) {
    if(!exp->lhs) {
        emit_message(msg::ERROR, "invalid base in dot expression", exp->loc);
    } else exp->lhs = exp->lhs->lower();
}

void ValidationVisitor::visitNewExpression(NewExpression *exp) {
    resolveType(exp->type);
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
        emit_message(msg::ERROR, "label statement expects following identifier");
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
        emit_message(msg::ERROR, "null scope in block statement", stmt->loc);
    }
#endif

    if(stmt->body) {
        stmt->body = stmt->body->lower();
    }

}

void ValidationVisitor::visitElseStatement(ElseStatement *stmt) {
    if(!stmt->body) {
        valid = false;
        emit_message(msg::ERROR, "else statement missing body statement", stmt->loc);
    }
}

void ValidationVisitor::visitIfStatement(IfStatement *stmt) {
    if(!stmt->body) {
        valid = false;
        emit_message(msg::ERROR, "if statement missing body statement", stmt->loc);
    }

    if(!stmt->condition) {
        emit_message(msg::ERROR, "if statement missing condition", stmt->loc);
    } else stmt->condition = stmt->condition->lower();

    if(stmt->elsebr) {
        stmt->elsebr = dynamic_cast<ElseStatement*>(stmt->elsebr->lower());
    }
}

void ValidationVisitor::visitLoopStatement(LoopStatement *stmt) {
    if(!stmt->condition) {
        emit_message(msg::ERROR, "loop statement missing condition", stmt->loc);
    } else stmt->condition = stmt->condition->lower();

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
        emit_message(msg::ERROR, "switch statement missing condition", stmt->loc);
    } else {
        stmt->condition = stmt->condition->lower();
    }
}

void ValidationVisitor::visitReturnStatement(ReturnStatement *stmt) {
    if(stmt->expression) stmt->expression = stmt->expression->lower();
    /*TODO: reenable when expression->getType() always works
    if(!stmt->expression->getType()->coercesTo(getFunction()->getReturnType())) {
        valid = false;
        emit_message(msg::ERROR, "returned value can not be converted to return type", stmt->loc);
    }*/
}

void ValidationVisitor::visitScope(ASTScope *sc){
    ASTScope::iterator end = sc->end();
    for(ASTScope::iterator it = sc->begin(); it != end; ++it){
        Identifier *id = *it;
        if(it->isUndeclared()) {
            id = sc->resolveIdentifier(id);
            if(!id || id->isUndeclared()) {
                emit_message(msg::ERROR, "could not resolve symbol in scope: " + id->getName());
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
