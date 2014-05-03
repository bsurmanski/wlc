#include "ast.hpp"
#include "astType.hpp"

//
// Code for AST traversal, validation, and
//

//
// AST
//

bool AST::validate() {
    return root->validate() == VALID;
}

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

/*
Validity TypeDeclaration::validate() {
    if(validity != UNCHECKED) return validity;
    validity = VALID;
    return INVALID; //TODO
} */

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
