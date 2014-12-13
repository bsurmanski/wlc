#ifndef _VALIDATE_HPP
#define _VALIDATE_HPP

#include "ast.hpp"
#include "astVisitor.hpp"

#include <stack>


/*
 * ValidationVisitor is used to as a semantics checking
 * and lowering pass. Should be run on AST after parsing is fully
 * complete. will traverse AST and emit error if AST is invalid.
 * At the same time, it will lower any complex operations (eg +=)
 *
 * TODO: perhaps create a pass for validation without lowering?
 */
class ValidationVisitor : public ASTVisitor {
    bool valid;
    ASTScope *scope;
    //std::stack<ASTNode *> trace;

    public:
    bool isValid() { return valid; }
    ValidationVisitor();
    Identifier *resolveIdentifier(Identifier *id);
    ASTType *resolveType(ASTType *ty);
    Expression *coerceTo(ASTType *ty, Expression *exp);
    virtual void visitPackage(PackageDeclaration *pak);
    virtual void visitModule(ModuleDeclaration *mod);

    virtual void visitDeclaration(Declaration *decl);
    virtual void visitFunctionDeclaration(FunctionDeclaration *decl);
    virtual void visitVariableDeclaration(VariableDeclaration *decl);
    virtual void visitTypeDeclaration(TypeDeclaration *decl);
    virtual void visitUserTypeDeclaration(UserTypeDeclaration *decl);

    virtual void visitUnaryExpression(UnaryExpression *exp);
    virtual void visitBinaryExpression(BinaryExpression *exp);
    virtual void visitPackExpression(PackExpression *exp);
    virtual void visitCallExpression(CallExpression *exp);
    virtual void visitIndexExpression(IndexExpression *exp);
    virtual void visitIdentifierExpression(IdentifierExpression *exp);
    virtual void visitNumericExpression(NumericExpression *exp);
    virtual void visitImportExpression(ImportExpression *exp);
    virtual void visitCastExpression(CastExpression *exp);
    virtual void visitTupleExpression(TupleExpression *exp);
    virtual void visitDotExpression(DotExpression *exp);
    virtual void visitNewExpression(NewExpression *exp);
    virtual void visitAllocExpression(AllocExpression *exp);

    virtual void visitLabelStatement(LabelStatement *stmt);
    virtual void visitCaseStatement(CaseStatement *stmt);
    virtual void visitGotoStatement(GotoStatement *stmt);
    virtual void visitReturnStatement(ReturnStatement *stmt);

    virtual void visitCompoundStatement(CompoundStatement *exp);
    virtual void visitBlockStatement(BlockStatement *exp);
    virtual void visitElseStatement(ElseStatement *exp);
    virtual void visitIfStatement(IfStatement *exp);
    virtual void visitLoopStatement(LoopStatement *exp);
    virtual void visitSwitchStatement(SwitchStatement *exp);

    virtual void visitScope(ASTScope *sc);
};

#endif
