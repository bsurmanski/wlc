#include "ast.hpp"
#include "astVisitor.hpp"

AST::AST(){
    root = new Package;
}

AST::~AST() {
    delete root;
}

void AST::accept(ASTVisitor *v) {
    getRootPackage()->accept(v);
}

void Package::accept(ASTVisitor *v) {
    v->visitPackage(this);
    v->visitScope(scope);
    v->pushScope(this->getScope());
    for(int i = 0; i < children.size(); i++)
        if(children[i])
            children[i]->accept(v);
    v->popScope();
}

void TranslationUnit::accept(ASTVisitor *v) {
    Package::accept(v);
    v->visitTranslationUnit(this);
    for(ASTScope::iterator it = getScope()->begin(); it != getScope()->end(); it++){
        it->getDeclaration()->accept(v);
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
    v->visitScope(scope);
    v->visitFunctionDeclaration(this);
    v->pushScope(this->getScope());
    v->pushFunction(this);
    if(body)
        body->accept(v);
    v->popFunction();
    v->popScope();
}

void LabelDeclaration::accept(ASTVisitor *v){
    Declaration::accept(v);
    v->visitLabelDeclaration(this);
}

void VariableDeclaration::accept(ASTVisitor *v){
    Declaration::accept(v);
    v->visitVariableDeclaration(this);
    if(value)
        value->accept(v);
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
    v->visitPostfixOpExpression(this);
    if(lhs)
        lhs->accept(v);
}

void UnaryExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitUnaryExpression(this);
    if(lhs)
        lhs->accept(v);
}

void BinaryExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitBinaryExpression(this);
    if(lhs)
        lhs->accept(v);
    if(rhs)
        rhs->accept(v);
}

void PrimaryExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitPrimaryExpression(this);
}

void CallExpression::accept(ASTVisitor *v){
    PostfixExpression::accept(v);
    v->visitCallExpression(this);
    if(function)
        function->accept(v);
}

void IndexExpression::accept(ASTVisitor *v){
    PostfixExpression::accept(v);
    v->visitIndexExpression(this);
    if(lhs)
        lhs->accept(v);
    if(index)
        index->accept(v);
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
    v->visitPackageExpression(this);
    if(package)
        package->accept(v);
}

void CastExpression::accept(ASTVisitor *v){
    Expression::accept(v);
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
    v->visitTupleExpression(this);
    for(int i = 0; i < members.size(); i++)
        if(members[i])
            members[i]->accept(v);
}

void DotExpression::accept(ASTVisitor *v){
    PostfixExpression::accept(v);
    v->visitDotExpression(this);
    if(lhs)
        lhs->accept(v);
}

void NewExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitNewExpression(this);
}

void IdOpExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitIdOpExpression(this);
    if(expression)
        expression->accept(v);
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
    v->visitCaseStatement(this);
    for(int i = 0; i < values.size(); i++)
        if(values[i])
            values[i]->accept(v);
}

void GotoStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitGotoStatement(this);
}

void ReturnStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitReturnStatement(this);
    if(expression)
        expression->accept(v);
}

void CompoundStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitCompoundStatement(this);
    v->pushScope(scope);
    for(int i = 0; i < statements.size(); i++)
        if(statements[i])
            statements[i]->accept(v);
    v->popScope();
}

void BlockStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitScope(scope);
    v->visitBlockStatement(this);
    v->pushScope(scope);
    if(body)
        body->accept(v);
    v->popScope();
}

void ElseStatement::accept(ASTVisitor *v){
    BlockStatement::accept(v);
    v->visitElseStatement(this);
}

void IfStatement::accept(ASTVisitor *v){
    BlockStatement::accept(v);
    v->visitIfStatement(this);
    condition->accept(v);
    if(elsebr)
        elsebr->accept(v);
}

void LoopStatement::accept(ASTVisitor *v){
    BlockStatement::accept(v);
    v->visitLoopStatement(this);
    if(condition)
        condition->accept(v);
    if(update)
        update->accept(v);
    if(elsebr)
        elsebr->accept(v);
}

void WhileStatement::accept(ASTVisitor *v){
    LoopStatement::accept(v);
    v->visitWhileStatement(this);
}

void ForStatement::accept(ASTVisitor *v){
    LoopStatement::accept(v);
    v->visitForStatement(this);
    if(decl)
        decl->accept(v);
}

void SwitchStatement::accept(ASTVisitor *v){
    BlockStatement::accept(v);
    v->visitSwitchStatement(this);
    if(condition)
        condition->accept(v);
}
