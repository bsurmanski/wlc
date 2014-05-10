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
        if(children[i])
            children[i]->accept(v);
    v->popScope();
}

void TranslationUnit::accept(ASTVisitor *v) {
    Package::accept(v);
    v->visitTranslationUnit(this);
    for(int i = 0; i < types.size(); i++)
        if(types[i])
            types[i]->accept(v);

    for(int i = 0; i < globals.size(); i++)
        if(globals[i])
            globals[i]->accept(v);

    for(int i = 0; i < functions.size(); i++)
        if(functions[i])
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

void CompoundExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitCompoundExpression(this);
    v->pushScope(scope);
    for(int i = 0; i < statements.size(); i++)
        if(statements[i])
            statements[i]->accept(v);
    v->popScope();
}

void BlockExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitBlockExpression(this);
    v->pushScope(scope);
    if(body)
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
    if(elsebr)
        elsebr->accept(v);
}

void LoopExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitLoopExpression(this);
    if(condition)
        condition->accept(v);
    if(update)
        update->accept(v);
    if(elsebr)
        elsebr->accept(v);
}

void WhileExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitWhileExpression(this);
}

void ForExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitForExpression(this);
    if(decl)
        decl->accept(v);
}

void SwitchExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitSwitchExpression(this);
    if(condition)
        condition->accept(v);
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

void DeleteExpression::accept(ASTVisitor *v){
    Expression::accept(v);
    v->visitDeleteExpression(this);
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

void DeclarationStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitDeclarationStatement(this);
    if(declaration)
        declaration->accept(v);
}

void ReturnStatement::accept(ASTVisitor *v){
    Statement::accept(v);
    v->visitReturnStatement(this);
    if(expression)
        expression->accept(v);
}

