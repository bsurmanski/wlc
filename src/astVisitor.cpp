#include "ast.hpp"
#include "astVisitor.hpp"

void AST::accept(ASTVisitor *v) {
    getRootPackage()->accept(v);
}

void PackageDeclaration::accept(ASTVisitor *v) {
    v->visitPackage(this);
    v->visitScope(scope);
    v->pushScope(this->getScope());
    for(int i = 0; i < children.size(); i++)
        if(children[i]) children[i]->accept(v);
    v->popScope();
}

void ModuleDeclaration::accept(ASTVisitor *v) {
    PackageDeclaration::accept(v);
    v->visitModule(this);
    for(ASTScope::iterator it = getScope()->begin(); it != getScope()->end(); it++){
        if(it->getDeclaration()) {
            it->getDeclaration()->accept(v);
        } else if (it->getExpression()) {
            //XXX TODO: validate expression
        } else {
            emit_message(msg::ERROR, "invalidate symbol in scope: " + it->getName());
        }
    }
}

//
// Declaration
//

void Declaration::accept(ASTVisitor *v) {
    Statement::accept(v);
    v->visitDeclaration(this);
}

void FunctionDeclaration::accept(ASTVisitor *v) {
    Declaration::accept(v);
    if(scope) {
        v->visitScope(scope);
        v->visitFunctionDeclaration(this);
        v->pushScope(this->getScope());
        v->pushFunction(this);
        if(body) body->accept(v);
        v->popFunction();
        v->popScope();
    }

    if(!scope && body) {
        emit_message(msg::FAILURE, "function with scope and no body");
    }

    if(nextoverload) {
        nextoverload->accept(v);
    }
}

void LabelDeclaration::accept(ASTVisitor *v){
    Declaration::accept(v);
    v->visitLabelDeclaration(this);
}

void VariableDeclaration::accept(ASTVisitor *v){
    Declaration::accept(v);
    v->visitVariableDeclaration(this);
    if(value) value->accept(v);
}

void TypeDeclaration::accept(ASTVisitor *v){
    Declaration::accept(v);
    v->visitTypeDeclaration(this);
}

void UserTypeDeclaration::accept(ASTVisitor *v){
    TypeDeclaration::accept(v);
    v->visitScope(scope);
    v->visitUserTypeDeclaration(this);
    v->visitType(type);

    if(constructor) constructor->accept(v);

    if(destructor) destructor->accept(v);

    for(int i = 0; i < methods.size(); i++) {
        methods[i]->accept(v);
    }

    for(int i = 0; i < members.size(); i++) {
        members[i]->accept(v);
    }

    for(int i = 0; i < staticMembers.size(); i++) {
        staticMembers[i]->accept(v);
    }
}

//
// Expression
//


void Expression::accept(ASTVisitor *v) {
    Statement::accept(v);
    v->visitExpression(this);
}

void PostfixExpression::accept(ASTVisitor *v) {
    Expression::accept(v);
    v->visitPostfixExpression(this);
}

void PostfixOpExpression::accept(ASTVisitor *v) {
    Expression::accept(v);
    if(lhs) lhs->accept(v);
    v->visitPostfixOpExpression(this);
}

void UnaryExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    if(lhs) lhs->accept(v);
    v->visitUnaryExpression(this);
}

void BinaryExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    if(lhs) lhs->accept(v);
    if(rhs) rhs->accept(v);
    v->visitBinaryExpression(this);
}

void PrimaryExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitPrimaryExpression(this);
}

void PackExpression::accept(ASTVisitor *v){
    PrimaryExpression::accept(v);
    v->visitPackExpression(this);
}

void CallExpression::accept(ASTVisitor *v){
    PostfixExpression::accept(v);
    if(function) function->accept(v);

    std::list<Expression*>::iterator it = args.begin();
    while(it != args.end()) {
        (*it)->accept(v);
        it++;
    }

    v->visitCallExpression(this);
}

void IndexExpression::accept(ASTVisitor *v){
    PostfixExpression::accept(v);
    if(lhs) lhs->accept(v);
    if(index) index->accept(v);
    v->visitIndexExpression(this);
}

void IdentifierExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitIdentifierExpression(this);
}

void NumericExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitNumericExpression(this);
}

void StringExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitStringExpression(this);
}


void ImportExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitImportExpression(this);
}

void PackageExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    if(package) package->accept(v);
    v->visitPackageExpression(this);
}

void CastExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    if(expression) expression->accept(v);
    v->visitCastExpression(this);
}

void UseExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitUseExpression(this);
}

void TypeExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitTypeExpression(this);
}

void TupleExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    for(int i = 0; i < members.size(); i++)
        if(members[i])
            members[i]->accept(v);
    v->visitTupleExpression(this);
}

void DotExpression::accept(ASTVisitor *v){
    PostfixExpression::accept(v);
    if(lhs) lhs->accept(v);
    v->visitDotExpression(this);
}

void AllocExpression::accept(ASTVisitor *v) {
    PrimaryExpression::accept(v);

    v->visitAllocExpression(this);
}

void NewExpression::accept(ASTVisitor *v){
    Expression::accept(v);

    std::list<Expression*>::iterator it = args.begin();
    while(it != args.end()) {
        (*it)->accept(v);
        it++;
    }

    v->visitNewExpression(this);
}

void IdOpExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    if(expression) expression->accept(v);
    v->visitIdOpExpression(this);
}

//
// Statement
//

void Statement::accept(ASTVisitor *v) {
    v->visitStatement(this);
}

void BreakStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitBreakStatement(this);
}

void ContinueStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitContinueStatement(this);
}

void LabelStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitLabelStatement(this);
}

void CaseStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    for(int i = 0; i < values.size(); i++)
        if(values[i]) values[i]->accept(v);
    v->visitCaseStatement(this);
}

void GotoStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitGotoStatement(this);
}

void ReturnStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    if(expression) expression->accept(v);
    v->visitReturnStatement(this);
}

void CompoundStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->pushScope(scope);
    for(int i = 0; i < statements.size(); i++)
        if(statements[i]) statements[i]->accept(v);
    v->popScope();
    v->visitCompoundStatement(this);
}

void BlockStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitScope(scope);
    v->pushScope(scope);
    if(body) body->accept(v);
    v->popScope();
    v->visitBlockStatement(this);
}

void ElseStatement::accept(ASTVisitor *v){
    BlockStatement::accept(v);
    v->visitElseStatement(this);
}

void IfStatement::accept(ASTVisitor *v){
    BlockStatement::accept(v);
    condition->accept(v);
    if(elsebr) elsebr->accept(v);
    v->visitIfStatement(this);
}

void LoopStatement::accept(ASTVisitor *v){
    BlockStatement::accept(v);
    if(condition) condition->accept(v);
    if(update) update->accept(v);
    if(elsebr) elsebr->accept(v);
    v->visitLoopStatement(this);
}

void WhileStatement::accept(ASTVisitor *v){
    LoopStatement::accept(v);
    v->visitWhileStatement(this);
}

void ForStatement::accept(ASTVisitor *v){
    LoopStatement::accept(v);
    if(decl) decl->accept(v);
    v->visitForStatement(this);
}

void SwitchStatement::accept(ASTVisitor *v){
    BlockStatement::accept(v);
    if(condition) condition->accept(v);
    v->visitSwitchStatement(this);
}
