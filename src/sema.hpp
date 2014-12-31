#include "ast.hpp"
#include "astVisitor.hpp"

// lower value is easier to reach
enum OverloadValidity {
    INVALID = 0,
    COERCE_MATCH = 1, // match, but type coercion requried
    DEFAULT_MATCH = 2, // match, but default parameter required
    FULL_MATCH = 3, //match, all parameters are exact type
};

/*
 * semantic validation. simply makes all
 * identifiers and types refer to valid values
 */
class Sema : public ASTVisitor {
    bool valid;

    ASTScope *scope;

    public:
    bool isValid() { return valid; }
    Sema();

    virtual OverloadValidity resolveOverloadValidity(std::list<Expression*> args, ASTNode *overload);
    virtual ASTNode *resolveOverloadList(std::list<Expression*> args, std::list<ASTNode*>& overload);
    virtual void buildOverloadList(Expression *func, std::list<ASTNode*>& overload);
    virtual Expression *resolveCallArgument(ASTFunctionType *fty, unsigned i, Expression *arg, Expression *def);
    virtual void resolveCallArguments(FunctionExpression *func, std::list<Expression*>& args);
    virtual void visitCallExpression(CallExpression *exp);
    virtual void visitNewExpression(NewExpression *exp);
    virtual void visitCastExpression(CastExpression *cexp);
    virtual void visitIfStatement(IfStatement *exp);
    virtual void visitBinaryExpression(BinaryExpression *bexp);
};
