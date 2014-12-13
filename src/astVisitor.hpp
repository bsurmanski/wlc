#ifndef _ASTVISITOR_HPP
#define _ASTVISITOR_HPP

#include "ast.hpp"
#include "astScope.hpp"
#include <stack>

class ASTVisitor {
    //XXX have a stack of ASTNode*s
    std::stack<ASTScope*> scope;
    std::stack<FunctionDeclaration*> function;
    SourceLocation location;

    public:
    ASTScope *getScope() { return scope.top(); }
    void pushScope(ASTScope *sc) { scope.push(sc); }
    ASTScope *popScope() { ASTScope *sc = scope.top(); scope.pop(); return sc; }
    SourceLocation currentLocation() { return location; }

    FunctionDeclaration *getFunction() { return function.top(); }
    void pushFunction(FunctionDeclaration *fdecl) { function.push(fdecl); }
    FunctionDeclaration *popFunction() {
        FunctionDeclaration *fd = function.top();
        function.pop();
        return fd;
    }

    virtual void visitPackage(PackageDeclaration *pak) {}
    virtual void visitModule(ModuleDeclaration *mod) {}

    virtual void visitDeclaration(Declaration *decl){ location = decl->loc; }
    virtual void visitExpression(Expression *exp){ location = exp->loc; }
    virtual void visitStatement(Statement *stmt){ location = stmt->loc; }
    //virtual void visitType(ASTType *ty){}

    virtual void visitFunctionDeclaration(FunctionDeclaration *decl){}
    virtual void visitLabelDeclaration(LabelDeclaration *decl){}
    virtual void visitVariableDeclaration(VariableDeclaration *decl){}
    virtual void visitTypeDeclaration(TypeDeclaration *decl){}
    virtual void visitUserTypeDeclaration(UserTypeDeclaration *decl){}

    virtual void visitUnaryExpression(UnaryExpression *exp){}
    virtual void visitBinaryExpression(BinaryExpression *exp){}
    virtual void visitPostfixExpression(PostfixExpression *exp){}
    virtual void visitPostfixOpExpression(PostfixOpExpression *exp){}
    virtual void visitPrimaryExpression(PrimaryExpression *exp){}
    virtual void visitPackExpression(PackExpression *exp){}
    virtual void visitCallExpression(CallExpression *exp){}
    virtual void visitIndexExpression(IndexExpression *exp){}
    virtual void visitIdentifierExpression(IdentifierExpression *exp){}
    virtual void visitNumericExpression(NumericExpression *exp){}
    virtual void visitStringExpression(StringExpression *exp){}
    virtual void visitImportExpression(ImportExpression *exp){}
    virtual void visitPackageExpression(PackageExpression *exp){}
    virtual void visitCastExpression(CastExpression *exp){}
    virtual void visitUseExpression(UseExpression *exp){}
    virtual void visitTypeExpression(TypeExpression *exp){}
    virtual void visitTupleExpression(TupleExpression *exp){}
    virtual void visitDotExpression(DotExpression *exp){}
    virtual void visitNewExpression(NewExpression *exp){}
    virtual void visitAllocExpression(AllocExpression *exp) {}
    virtual void visitIdOpExpression(IdOpExpression *exp){}

    virtual void visitBreakStatement(BreakStatement *stmt){}
    virtual void visitContinueStatement(ContinueStatement *stmt){}
    virtual void visitLabelStatement(LabelStatement *stmt){}
    virtual void visitCaseStatement(CaseStatement *stmt){}
    virtual void visitGotoStatement(GotoStatement *stmt){}
    virtual void visitReturnStatement(ReturnStatement *stmt){}

    virtual void visitCompoundStatement(CompoundStatement *exp){}
    virtual void visitBlockStatement(BlockStatement *exp){}
    virtual void visitElseStatement(ElseStatement *exp){}
    virtual void visitIfStatement(IfStatement *exp){}
    virtual void visitLoopStatement(LoopStatement *exp){}
    virtual void visitWhileStatement(WhileStatement *exp){}
    virtual void visitForStatement(ForStatement *exp){}
    virtual void visitSwitchStatement(SwitchStatement *exp){}

    virtual void visitScope(ASTScope *scope){}
    virtual void visitType(ASTType *type){}
};

#endif
