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
    for(int i = 0; i < children.size(); i++)
        children[i]->accept(v);
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

void Declaration::accept(ASTVisitor *v) {
    v->visitDeclaration(this);

    if(FunctionDeclaration *fdecl = dynamic_cast<FunctionDeclaration*>(this))
        fdecl->body->accept(v);
    if(VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(this))
        vdecl->value->accept(v);
    if(TypeDeclaration *tdecl = dynamic_cast<TypeDeclaration*>(this))
        tdecl->getType()->accept(v);
}

void Expression::accept(ASTVisitor *v) {
    v->visitExpression(this);
    if(CompoundExpression* cexp = dynamic_cast<CompoundExpression*>(this))
        for(int i = 0; i < cexp->statements.size(); i++)
            cexp->statements[i]->accept(v);

    if(BlockExpression *bexp = dynamic_cast<BlockExpression*>(this))
        bexp->body->accept(v);

}

void Statement::accept(ASTVisitor *v) {
    v->visitStatement(this);

    if(DeclarationStatement *dstmt = dynamic_cast<DeclarationStatement*>(this))
        dstmt->declaration->accept(v);

    if(ExpressionStatement *estmt = dynamic_cast<ExpressionStatement*>(this))
        estmt->expression->accept(v);

    if(ReturnStatement *rstmt = dynamic_cast<ReturnStatement*>(this))
        rstmt->expression->accept(v);

}
