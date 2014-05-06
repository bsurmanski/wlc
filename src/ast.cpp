#include "ast.hpp"

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
    v->pushScope(this->getScope());
    for(int i = 0; i < children.size(); i++)
        children[i]->accept(v);
    v->popScope();
}

void TranslationUnit::accept(ASTVisitor *v) {
    Package::accept(v);
    v->visitTranslationUnit(this);
    for(int i = 0; i < types.size(); i++)
        types[i]->accept(v);

    for(int i = 0; i < globals.size(); i++)
        globals[i]->accept(v);

    for(int i = 0; i < functions.size(); i++)
        functions[i]->accept(v);
}

//
// Declaration
//

void Declaration::accept(ASTVisitor *v) {
    v->visitDeclaration(this);
}

void FunctionDeclaration::accept(ASTVisitor *v) {
    Declaration::accept(v);
    v->visitFunctionDeclaration(this);
    v->pushScope(this->getScope());
    v->pushFunction(this);
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
}

void TypeDeclaration::accept(ASTVisitor *v){
    Declaration::accept(v);
    v->visitTypeDeclaration(this);
}

void StructUnionDeclaration::accept(ASTVisitor *v){
    Declaration::accept(v);
    v->visitStructUnionDeclaration(this);
}

//
// Expression
//


void Expression::accept(ASTVisitor *v) {
    v->visitExpression(this);
    if(CompoundExpression* cexp = dynamic_cast<CompoundExpression*>(this)){
        v->pushScope(cexp->getScope());
        for(int i = 0; i < cexp->statements.size(); i++)
            cexp->statements[i]->accept(v);
        v->popScope();
    }

    if(BlockExpression *bexp = dynamic_cast<BlockExpression*>(this)){
        v->pushScope(bexp->getScope());
        bexp->body->accept(v);
        v->popScope();
    }
}

void PostfixExpression::accept(ASTVisitor *v) {
    Expression::accept(v);
    v->visitPostfixExpression(this);
}

void PostfixOpExpression::accept(ASTVisitor *v) {
    Expression::accept(v);
    v->visitPostfixOpExpression(this);
    lhs->accept(v);
}

void UnaryExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitUnaryExpression(this);
    lhs->accept(v);
}

void BinaryExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitBinaryExpression(this);
    lhs->accept(v);
    rhs->accept(v);
}

void PrimaryExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitPrimaryExpression(this);
}

void CallExpression::accept(ASTVisitor *v){
    PostfixExpression::accept(v);
    v->visitCallExpression(this);
    function->accept(v);
}

void IndexExpression::accept(ASTVisitor *v){
    PostfixExpression::accept(v);
    v->visitIndexExpression(this);
    lhs->accept(v);
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

void CompoundExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitCompoundExpression(this);
    v->pushScope(scope);
    for(int i = 0; i < statements.size(); i++)
        statements[i]->accept(v);
    v->popScope();
}

void BlockExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitBlockExpression(this);
    v->pushScope(scope);
    body->accept(v);
    v->popScope();
}

void ElseExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitElseExpression(this);
}

void IfExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitIfExpression(this);
    condition->accept(v);
    elsebr->accept(v);
}

void LoopExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitLoopExpression(this);
    condition->accept(v);
    update->accept(v);
    elsebr->accept(v);
}

void WhileExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitWhileExpression(this);
}

void ForExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitForExpression(this);
    decl->accept(v);
}

void SwitchExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitSwitchExpression(this);
    condition->accept(v);
}

void ImportExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitImportExpression(this);
}

void PackageExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitPackageExpression(this);
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
        members[i]->accept(v);
}

void DotExpression::accept(ASTVisitor *v){
    PostfixExpression::accept(v);
    v->visitDotExpression(this);
    lhs->accept(v);
}

void NewExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitNewExpression(this);
}

void DeleteExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitDeleteExpression(this);
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
        values[i]->accept(v);
}

void GotoStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitGotoStatement(this);
}

void DeclarationStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitDeclarationStatement(this);
    declaration->accept(v);
}

void ExpressionStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitExpressionStatement(this);
    expression->accept(v);
}

void ReturnStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitReturnStatement(this);
    if(expression)
        expression->accept(v);
}

