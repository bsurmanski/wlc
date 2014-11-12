/**
 * lowering.cpp
 * wlc
 * Brandon Surmanski
 *
 * Contains all of the operation lowering for ASTNodes.
 * All implementations of the virtual ASTNode::lower method is contained here.
 *
 * 'lower' should be used to replace certain complex operations to simpler, more atomic operations.
 * for example: the statement representing 'a += b' should be lowered to 'a = a + b'
 *
 * lowering should happen after parsing and before code generation.
 **/

#include "ast.hpp"
#include "message.hpp"

Expression* BinaryExpression::lower() {
    if(op.isCompoundAssignOp()) {
        tok::TokenKind lowerOp;

        Expression *oprhs = NULL;

        // special case, cast expression
        if(op.kind == tok::colonequal) {
            oprhs = new CastExpression(this->lhs->getType(), this->rhs, this->loc);
        } else {
            switch(op.kind) {
                case tok::plusequal: lowerOp = tok::plus; break;
                case tok::minusequal: lowerOp = tok::minus; break;
                case tok::starequal: lowerOp = tok::star; break;
                case tok::slashequal: lowerOp = tok::slash; break;

                case tok::ampequal: lowerOp = tok::amp; break;
                case tok::barequal: lowerOp = tok::bar; break;
                case tok::caretequal: lowerOp = tok::caret; break;
                case tok::percentequal: lowerOp = tok::percent; break;
                default: emit_message(msg::FAILURE, "unknown compound assignment lowering");
            }

            oprhs = new BinaryExpression(lowerOp, this->lhs, this->rhs, this->loc);
        }
        return new BinaryExpression(tok::equal, this->lhs, oprhs, this->loc);
    }
    return this;
}

Expression *DotExpression::lower() {
    if(lhs->isType()) {
        // lower 'sizeof' expression to constant int
        if(rhs == "sizeof") {
            return new IntExpression(ASTType::getLongTy(), lhs->getDeclaredType()->getSize());
        }

        if(rhs == "offsetof") {
            ASTUserType *uty = lhs->getDeclaredType()->asUserType();
            if(uty) {
                emit_message(msg::UNIMPLEMENTED, "unimplemented 'offsetof' operator", loc);
            } else {
                emit_message(msg::ERROR, "invalid 'offsetof' on non-user type", loc);
            }
        }
    }

    return this;
}

Expression *CallExpression::lower() {
    /*
    if(function->dotExpression()) {
        DotExpression *dexp = function->dotExpression();
        Identifier *funcid = dexp->lhs->getType()->getDeclaration()->lookup(dexp->rhs);
        if(!funcid) {
            //TODO: lookup in callexpression scope for UFCS
        }
        function = new IdentifierExpression(funcid, dexp->loc);
        args.push_front(dexp->lhs);
    }*/

    return this;
}
