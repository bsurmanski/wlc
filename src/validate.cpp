#include "ast.hpp"
#include "astType.hpp"
#include "validate.hpp"

//
// Code for AST traversal, validation, and
//


//
// AST
//

bool AST::validate() {
}

/*
//
// Package
//

Validity Package::validate() {
    if(validity != UNCHECKED) return validity; // only check once

    validity = VALID; // avoid circular reference livelock

    for(int i = 0; i < children.size(); i++){
        if(children[i]->validate() != VALID)
            validity = INVALID;
    }
    return validity;
}

//
// TranslationUnit
//

Validity TranslationUnit::validate() {
    if(validity != UNCHECKED) return validity;

    validity = VALID;
    if(Package::validate() != VALID)
        validity = INVALID;

    for(int i = 0; i < imports.size(); i++) {
        if(imports[i]->validate() != VALID)
            validity = INVALID;
    }

    return validity;
}

//
// Declaration
//

Validity FunctionDeclaration::validate() {
    if(validity != UNCHECKED) return validity;
    validity = VALID;

    //XXX valdiate scope?
    if(body && body->validate() != VALID) validity = INVALID;
    if(!prototype || prototype->validate() != VALID) validity = INVALID;

    for(int i = 0; i < paramValues.size(); i++){
        if(paramValues[i]) {
            if(paramValues[i]->validate() != VALID) validity = INVALID;
        }
    }

    //TODO: validate uniqueness of paramNames

    //TODO
    return validity;
}

Validity VariableDeclaration::validate(){
    if(validity != UNCHECKED) return validity;
    validity = VALID;

    if(!type || type->validate() != VALID) validity = INVALID;
    if(value && value->validate() != VALID) validity = INVALID;

    return INVALID; //TODO
}

Validity LabelDeclaration::validate() {
    if(validity != UNCHECKED) return validity;
    validity = VALID;
    return validity; //TODO
}

Validity ArrayDeclaration::validate() {
    if(validity != UNCHECKED) return validity;
    return VariableDeclaration::validate();
}

Validity StructUnionDeclaration::validate() {
    if(validity != UNCHECKED) return validity;
    validity = VALID;
    if(!type || identifier->getDeclaredType() != type) validity = INVALID;
    if(type->validate() != VALID) validity = INVALID;
    return validity;
}

//
// Expression
//

//
// Statement
//

//
// ASTType
//
Validity DynamicTypeInfo::validate() {
    return VALID;
}

Validity FunctionTypeInfo::validate() {
    return VALID;
}

Validity PointerTypeInfo::validate() {
    return VALID;
}

Validity StaticArrayTypeInfo::validate() {
    return VALID;
}

Validity DynamicArrayTypeInfo::validate() {
    return VALID;
}

Validity HetrogenTypeInfo::validate() {
    return VALID;
}

Validity StructTypeInfo::validate() {
    return VALID;
}

Validity UnionTypeInfo::validate() {
    return VALID;
}

Validity ClassTypeInfo::validate() {
    return VALID;
}

Validity NamedUnknownInfo::validate() {
    return INVALID;
}

Validity AliasTypeInfo::validate() {
    return VALID;
}

Validity TupleTypeInfo::validate() {
    return VALID;
}

*/

void ValidationVisitor::visitPackage(Package *pak){

}

void ValidationVisitor::visitTranslationUnit(TranslationUnit *tu){

}

void ValidationVisitor::visitDeclaration(Declaration *decl){

}

void ValidationVisitor::visitExpression(Expression *exp){

}

void ValidationVisitor::visitStatement(Statement *stmt){

}

void ValidationVisitor::visitFunctionDeclaration(FunctionDeclaration *decl){

}

void ValidationVisitor::visitLabelDeclaration(LabelDeclaration *decl){

}

void ValidationVisitor::visitVariableDeclaration(VariableDeclaration *decl){

}

void ValidationVisitor::visitTypeDeclaration(TypeDeclaration *decl){

}

void ValidationVisitor::visitStructUnionDeclaration(StructUnionDeclaration *decl){

}

void ValidationVisitor::visitUnaryExpression(UnaryExpression *exp){

}

void ValidationVisitor::visitBinaryExpression(BinaryExpression *exp){

}

void ValidationVisitor::visitPrimaryExpression(PrimaryExpression *exp){

}

void ValidationVisitor::visitCallExpression(CallExpression *exp){

}

void ValidationVisitor::visitIndexExpression(IndexExpression *exp){

}

void ValidationVisitor::visitIdentifierExpression(IdentifierExpression *exp){

}

void ValidationVisitor::visitNumericExpression(NumericExpression *exp){

}

void ValidationVisitor::visitStringExpression(StringExpression *exp){

}

void ValidationVisitor::visitCompoundExpression(CompoundExpression *exp){

}

void ValidationVisitor::visitBlockExpression(BlockExpression *exp){

}

void ValidationVisitor::visitElseExpression(ElseExpression *exp){

}

void ValidationVisitor::visitIfExpression(IfExpression *exp){

}

void ValidationVisitor::visitLoopExpression(LoopExpression *exp){

}

void ValidationVisitor::visitWhileExpression(WhileExpression *exp){

}

void ValidationVisitor::visitForExpression(ForExpression *exp){

}

void ValidationVisitor::visitSwitchExpression(SwitchExpression *exp){

}

void ValidationVisitor::visitImportExpression(ImportExpression *exp){

}

void ValidationVisitor::visitPackageExpression(PackageExpression *exp){

}

void ValidationVisitor::visitCastExpression(CastExpression *exp){

}

void ValidationVisitor::visitUseExpression(UseExpression *exp){

}

void ValidationVisitor::visitTypeExpression(TypeExpression *exp){

}

void ValidationVisitor::visitTupleExpression(TupleExpression *exp){

}

void ValidationVisitor::visitDotExpression(DotExpression *exp){

}

void ValidationVisitor::visitNewExpression(NewExpression *exp){

}

void ValidationVisitor::visitDeleteExpression(DeleteExpression *exp){

}

void ValidationVisitor::visitBreakStatement(BreakStatement *stmt){

}

void ValidationVisitor::visitContinueStatement(ContinueStatement *stmt){

}

void ValidationVisitor::visitLabelStatement(LabelStatement *stmt){

}

void ValidationVisitor::visitCaseStatement(CaseStatement *stmt){

}

void ValidationVisitor::visitGotoStatement(GotoStatement *stmt){

}

void ValidationVisitor::visitDeclarationStatement(DeclarationStatement *stmt){

}

void ValidationVisitor::visitExpressionStatement(ExpressionStatement *stmt){

}

void ValidationVisitor::visitReturnStatement(ReturnStatement *stmt){

}
