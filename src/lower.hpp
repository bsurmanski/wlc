#include "ast.hpp"
#include "astVisitor.hpp"

/*
 * calls 'lower' on all ASTNodes
 */
class Lower : public ASTVisitor {
    bool valid;

    ASTScope *scope;

    public:
    bool isValid() { return valid; }
    Lower();

    virtual void visitFunctionDeclaration(FunctionDeclaration *decl);
    virtual void visitVariableDeclaration(VariableDeclaration *decl);
    virtual void visitUserTypeDeclaration(UserTypeDeclaration *decl);

    virtual void visitUnaryExpression(UnaryExpression *exp);
    virtual void visitBinaryExpression(BinaryExpression *exp);
    virtual void visitCallExpression(CallExpression *exp);
    virtual void visitIndexExpression(IndexExpression *exp);
    virtual void visitCastExpression(CastExpression *exp);
    virtual void visitTupleExpression(TupleExpression *exp);
    virtual void visitDotExpression(DotExpression *exp);
    virtual void visitNewExpression(NewExpression *exp);
    virtual void visitReturnStatement(ReturnStatement *stmt);

    virtual void visitCompoundStatement(CompoundStatement *exp);
    virtual void visitBlockStatement(BlockStatement *exp);
    virtual void visitIfStatement(IfStatement *exp);
    virtual void visitLoopStatement(LoopStatement *exp);
    virtual void visitForStatement(ForStatement *exp);
    virtual void visitSwitchStatement(SwitchStatement *exp);
};


