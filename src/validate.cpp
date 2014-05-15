#include <string>
#include "ast.hpp"
#include "astType.hpp"
#include "validate.hpp"
#include "message.hpp"

using namespace std;

//
// Code for AST traversal, validation, and
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
        id = getScope()->lookup(id->getName());
    }

    if(id->isUndeclared()) {
        valid = false;
        emit_message(msg::ERROR, string("undeclared variable '") + id->getName()
                + string("' in scope"));
    }

    return id;
}

ASTType *ValidationVisitor::resolveType(ASTType *ty) {
    if(!ty->isResolved()) {
        if(ASTUserType *userty = ty->userType()) {
            userty->identifier = resolveIdentifier(userty->identifier);
        }
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
}

void ValidationVisitor::visitExpression(Expression *exp) {

}

void ValidationVisitor::visitStatement(Statement *stmt) {

}

void ValidationVisitor::visitFunctionDeclaration(FunctionDeclaration *decl) {
    //TODO: validate prototype
    if(!decl->prototype) {
        valid = false;
        emit_message(msg::ERROR, "function declaration missing prototype");
    }

    if(!decl->getScope() && decl->body) {
        valid = false;
        emit_message(msg::ERROR, "function has body without scope");
    }

    resolveType(decl->prototype);
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
}

void ValidationVisitor::visitUnaryExpression(UnaryExpression *exp) {
    //TODO: insert coersion cast
    if(!exp->lhs) {
        valid = false;
        emit_message(msg::ERROR, "unary operator is missing expression");
    }
}

void ValidationVisitor::visitBinaryExpression(BinaryExpression *exp) {
    //TODO: insert coersion cast
    if(!exp->lhs) {
        valid = false;
        emit_message(msg::ERROR, "binary operator is missing left hand expression");
    }

    if(!exp->rhs) {
        valid = false;
        emit_message(msg::ERROR, "binary operator is missing right hand expression");
    }
}

void ValidationVisitor::visitPrimaryExpression(PrimaryExpression *exp) {

}

void ValidationVisitor::visitCallExpression(CallExpression *exp) {

}

void ValidationVisitor::visitIndexExpression(IndexExpression *exp) {

}

void ValidationVisitor::visitIdentifierExpression(IdentifierExpression *exp) {
    // resolve identifier
    exp->id = resolveIdentifier(exp->id);

    // resolve type of identifier if needed
    resolveType(exp->id->getType());
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
}

void ValidationVisitor::visitUseExpression(UseExpression *exp) {

}

void ValidationVisitor::visitTypeExpression(TypeExpression *exp) {

}

void ValidationVisitor::visitTupleExpression(TupleExpression *exp) {

}

void ValidationVisitor::visitDotExpression(DotExpression *exp) {

}

void ValidationVisitor::visitNewExpression(NewExpression *exp) {
    resolveType(exp->type);
}

void ValidationVisitor::visitDeleteExpression(DeleteExpression *exp) {

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
            emit_message(msg::ERROR, "invalid case value");
        }
    }
}

void ValidationVisitor::visitGotoStatement(GotoStatement *stmt) {
    if(!stmt->identifier) {
        valid = false;
        emit_message(msg::ERROR, "goto statement expects following identifier");
    }
}

void ValidationVisitor::visitDeclarationStatement(DeclarationStatement *stmt) {
#ifdef DEBUG
    if(!stmt->declaration) {
        valid = false;
        emit_message(msg::ERROR, "null declaration in declaration statement");
    }
#endif

}

void ValidationVisitor::visitCompoundStatement(CompoundStatement *stmt) {
    if(!stmt->getScope() && stmt->statements.size()) {
        valid = false;
        emit_message(msg::ERROR, "compound stmtression is missing scope");
    }

#ifdef DEBUG
    for(int i = 0; i < stmt->statements.size(); i++) {
        if(!stmt->statements[i]) {
            valid = false;
            emit_message(msg::ERROR, "null statement in compound statement");
        }
    }
#endif
}

void ValidationVisitor::visitBlockStatement(BlockStatement *stmt) {
#ifdef DEBUG
    if(!stmt->getScope() && stmt->body) {
        valid = false;
        emit_message(msg::ERROR, "null scope in block statement");
    }
#endif

}

void ValidationVisitor::visitElseStatement(ElseStatement *stmt) {
    if(!stmt->body) {
        valid = false;
        emit_message(msg::ERROR, "else stmtression stmtects body statement");
    }
}

void ValidationVisitor::visitIfStatement(IfStatement *stmt) {
    if(!stmt->body) {
        valid = false;
        emit_message(msg::ERROR, "if stmtression stmtects body statement");
    }
}

void ValidationVisitor::visitLoopStatement(LoopStatement *stmt) {

}

void ValidationVisitor::visitWhileStatement(WhileStatement *stmt) {

}

void ValidationVisitor::visitForStatement(ForStatement *stmt) {

}

void ValidationVisitor::visitSwitchStatement(SwitchStatement *stmt) {
    if(!stmt->condition) {
        valid = false;
        emit_message(msg::ERROR, "switch stmtression stmtects condition");
    }
}

void ValidationVisitor::visitReturnStatement(ReturnStatement *stmt) {
    /*TODO: reenable when expression->getType() always works
    if(!stmt->expression->getType()->coercesTo(getFunction()->getReturnType())) {
        valid = false;
        emit_message(msg::ERROR, "returned value can not be converted to return type", stmt->loc);
    }*/
}

void ValidationVisitor::visitType(ASTType *ty) {
}
