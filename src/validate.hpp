#ifndef _VALIDATE_HPP
#define _VALIDATE_HPP

#include "ast.hpp"
#include "astVisitor.hpp"

#include <stack>

// can be removed?
/*
enum Validity {
    UNCHECKED,
    VALID,
    INVALID
};*/

// lower value is easier to reach
enum OverloadValidity {
    INVALID = 0,
    COERCE_MATCH = 1, // match, but type coercion requried
    DEFAULT_MATCH = 2, // match, but default parameter required
    FULL_MATCH = 3, //match, all parameters are exact type
};

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

    SourceLocation location;
    ASTScope *scope;
    //std::stack<ASTNode *> trace;

    public:
    bool isValid() { return valid; }
    ValidationVisitor();
    Identifier *resolveIdentifier(Identifier *id);
    ASTType *resolveType(ASTType *ty);
    Expression *coerceTo(ASTType *ty, Expression *exp);
    ASTType *commonType(ASTType *t1, ASTType *t2);
    virtual void visitPackage(PackageDeclaration *pak);
    virtual void visitModule(ModuleDeclaration *mod);

    virtual void visitDeclaration(Declaration *decl);
    virtual void visitExpression(Expression *exp);
    virtual void visitStatement(Statement *stmt);

    virtual void visitFunctionDeclaration(FunctionDeclaration *decl);
    virtual void visitLabelDeclaration(LabelDeclaration *decl);
    virtual void visitVariableDeclaration(VariableDeclaration *decl);
    virtual void visitTypeDeclaration(TypeDeclaration *decl);
    virtual void visitUserTypeDeclaration(UserTypeDeclaration *decl);

    virtual void visitUnaryExpression(UnaryExpression *exp);
    virtual void visitBinaryExpression(BinaryExpression *exp);
    virtual void visitPrimaryExpression(PrimaryExpression *exp);
    virtual void visitPackExpression(PackExpression *exp);
    virtual OverloadValidity resolveOverloadValidity(std::list<Expression*> args, ASTNode *overload);
    virtual ASTNode *resolveOverloadList(std::list<Expression*> args, std::list<ASTNode*>& overload);
    virtual void buildOverloadList(Expression *func, std::list<ASTNode*>& overload);
    virtual Expression *resolveCallArgument(ASTFunctionType *fty, unsigned i, Expression *arg, Expression *def);
    virtual void resolveCallArguments(FunctionExpression *func, std::list<Expression*>& args);
    virtual void visitCallExpression(CallExpression *exp);
    virtual void visitIndexExpression(IndexExpression *exp);
    virtual void visitIdentifierExpression(IdentifierExpression *exp);
    virtual void visitNumericExpression(NumericExpression *exp);
    virtual void visitStringExpression(StringExpression *exp);
    virtual void visitImportExpression(ImportExpression *exp);
    virtual void visitPackageExpression(PackageExpression *exp);
    virtual void visitCastExpression(CastExpression *exp);
    virtual void visitUseExpression(UseExpression *exp);
    virtual void visitTypeExpression(TypeExpression *exp);
    virtual void visitTupleExpression(TupleExpression *exp);
    virtual void visitDotExpression(DotExpression *exp);
    virtual void visitNewExpression(NewExpression *exp);
    virtual void visitAllocExpression(AllocExpression *exp);
    virtual void visitIdOpExpression(IdOpExpression *exp);

    virtual void visitBreakStatement(BreakStatement *stmt);
    virtual void visitContinueStatement(ContinueStatement *stmt);
    virtual void visitLabelStatement(LabelStatement *stmt);
    virtual void visitCaseStatement(CaseStatement *stmt);
    virtual void visitGotoStatement(GotoStatement *stmt);
    virtual void visitReturnStatement(ReturnStatement *stmt);

    virtual void visitCompoundStatement(CompoundStatement *exp);
    virtual void visitBlockStatement(BlockStatement *exp);
    virtual void visitElseStatement(ElseStatement *exp);
    virtual void visitIfStatement(IfStatement *exp);
    virtual void visitLoopStatement(LoopStatement *exp);
    virtual void visitWhileStatement(WhileStatement *exp);
    virtual void visitForStatement(ForStatement *exp);
    virtual void visitSwitchStatement(SwitchStatement *exp);

    virtual void visitScope(ASTScope *sc);
    virtual void visitType(ASTType *ty);
};

#endif
