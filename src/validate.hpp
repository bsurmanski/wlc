#ifndef _VALIDATE_HPP
#define _VALIDATE_HPP

#include "ast.hpp"

enum Validity {
    UNCHECKED,
    VALID,
    INVALID
};

class ValidationVisitor : public ASTVisitor {
    bool valid;

    public:
    bool isValid() { return valid; }
    ValidationVisitor();
    virtual void visitPackage(Package *pak);
    virtual void visitTranslationUnit(TranslationUnit *tu);

    virtual void visitDeclaration(Declaration *decl);
    virtual void visitExpression(Expression *exp);
    virtual void visitStatement(Statement *stmt);

    virtual void visitFunctionDeclaration(FunctionDeclaration *decl);
    virtual void visitLabelDeclaration(LabelDeclaration *decl);
    virtual void visitVariableDeclaration(VariableDeclaration *decl);
    virtual void visitTypeDeclaration(TypeDeclaration *decl);
    virtual void visitStructUnionDeclaration(StructUnionDeclaration *decl);

    virtual void visitUnaryExpression(UnaryExpression *exp);
    virtual void visitBinaryExpression(BinaryExpression *exp);
    virtual void visitPrimaryExpression(PrimaryExpression *exp);
    virtual void visitCallExpression(CallExpression *exp);
    virtual void visitIndexExpression(IndexExpression *exp);
    virtual void visitIdentifierExpression(IdentifierExpression *exp);
    virtual void visitNumericExpression(NumericExpression *exp);
    virtual void visitStringExpression(StringExpression *exp);
    virtual void visitCompoundExpression(CompoundExpression *exp);
    virtual void visitBlockExpression(BlockExpression *exp);
    virtual void visitElseExpression(ElseExpression *exp);
    virtual void visitIfExpression(IfExpression *exp);
    virtual void visitLoopExpression(LoopExpression *exp);
    virtual void visitWhileExpression(WhileExpression *exp);
    virtual void visitForExpression(ForExpression *exp);
    virtual void visitSwitchExpression(SwitchExpression *exp);
    virtual void visitImportExpression(ImportExpression *exp);
    virtual void visitPackageExpression(PackageExpression *exp);
    virtual void visitCastExpression(CastExpression *exp);
    virtual void visitUseExpression(UseExpression *exp);
    virtual void visitTypeExpression(TypeExpression *exp);
    virtual void visitTupleExpression(TupleExpression *exp);
    virtual void visitDotExpression(DotExpression *exp);
    virtual void visitNewExpression(NewExpression *exp);
    virtual void visitDeleteExpression(DeleteExpression *exp);

    virtual void visitBreakStatement(BreakStatement *stmt);
    virtual void visitContinueStatement(ContinueStatement *stmt);
    virtual void visitLabelStatement(LabelStatement *stmt);
    virtual void visitCaseStatement(CaseStatement *stmt);
    virtual void visitGotoStatement(GotoStatement *stmt);
    virtual void visitDeclarationStatement(DeclarationStatement *stmt);
    virtual void visitReturnStatement(ReturnStatement *stmt);
};

#endif
