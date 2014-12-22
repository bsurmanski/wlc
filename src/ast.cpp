#include "ast.hpp"
#include "validate.hpp"
#include "sema.hpp"
#include "lower.hpp"


#if defined WIN32
char *realpath(const char *path, char *resolve) {
	return _fullpath(resolve, path, PATH_MAX);
}
std::string getFilebase(std::string s)
{
	std::tr2::sys::path filepath(s);
	return basename(filepath);
}
#elif defined __APPLE__
#include <libgen.h>
std::string getFilebase(std::string s)
{
	size_t lastDot = s.find_last_of(".");
	if (lastDot != std::string::npos)
		s = s.substr(0, lastDot);
	char *buf = (char*) malloc(s.size()+1);
	memcpy(buf, s.c_str(), s.size()+1);
	std::string ret = basename(buf);
	free(buf);
	return ret;
}
#else //linux
std::string getFilebase(std::string s)
{
	size_t lastDot = s.find_last_of(".");
	if (lastDot != std::string::npos)
		s = s.substr(0, lastDot);
	return basename(s.c_str());
}
#endif

unsigned getFileSize(std::string filenm) {
    struct stat st;
    if(stat(filenm.c_str(), &st)) {
        return 0;
    }
    return st.st_size;
}

//
// AST
//

AST::AST(){
    root = new PackageDeclaration(NULL, NULL, SourceLocation(), DeclarationQualifier());
    Identifier *id = root->getScope()->getInScope("__root");
    id->addDeclaration(root, Identifier::ID_PACKAGE);
    root->identifier = id; //XXX bit hacky
}

AST::~AST() {
    delete root;
}

bool AST::validate() {
    ValidationVisitor validate;
    Lower lowering;
    Sema sema;
    PackageDeclaration *pdecl = getRootPackage();

    pdecl->accept(&validate);
    if(currentErrorLevel() < msg::ERROR) {
        pdecl->accept(&lowering);
        if(currentErrorLevel() < msg::ERROR) {
            pdecl->accept(&sema);
        }
    }
    return currentErrorLevel() < msg::ERROR;
}

ASTFunctionType *FunctionDeclaration::getType() {
    if(!prototype) {
        std::vector<ASTType *> paramTy;

        if(owner) {
            paramTy.push_back(owner);
        }

        for(int i = 0; i < parameters.size(); i++) {
            paramTy.push_back(parameters[i]->getType());
        }

        prototype = ASTType::getFunctionTy(returnTy, paramTy, vararg);
    }
    return prototype;
} //TODO: 'prototype' should be 'type'?

int FunctionDeclaration::minExpectedParameters() {
    int n = 0;

    for(int i = 0; i < parameters.size(); i++) {
        if(!parameters[i]->value) {
            n++;
        }
    }

    return n;
}

int FunctionDeclaration::maxExpectedParameters() {
    if(isVararg()) return 999; //arbitrarily large; shouldn't need more than 999 arguments, right?
    return parameters.size();
}

Expression *FunctionDeclaration::getDefaultParameter(unsigned parami) {
    if(parami >= parameters.size()) return NULL;
    return parameters[parami]->value;
}

size_t UserTypeDeclaration::getAlign() const {
    size_t align = 0;
    VariableDeclaration *vd;
    for(int i = 0; i < members.size(); i++){
        vd = members[i]->variableDeclaration();
        assert(vd && "expected variable decl, found something else");
        if(vd->getType()->getAlign() > align)
            align = vd->getType()->getAlign();
    }
    if(align == 0) return 1; // TODO: empty struct, bad!
    return align; //TODO: handle packed
}

long ClassDeclaration::getMemberIndex(std::string member){
    for(int i = 0; i < members.size(); i++){
        if(members[i]->identifier->getName() == member){
            return i;
        }
    }
    return -1;
}

long StructDeclaration::getMemberIndex(std::string member){
    for(int i = 0; i < members.size(); i++){
        if(members[i]->identifier->getName() == member){
            return i;
        }
    }
    return -1;
}

size_t StructDeclaration::getSize() const {
    size_t sz = 0;
    VariableDeclaration *vd;
    unsigned align;
    for(int i = 0; i < members.size(); i++) {
        vd = members[i]->variableDeclaration();
        if(!vd) continue;

        align = vd->getType()->getAlign();
        if(!packed && sz % align)
            sz += (align - (sz % align));
        sz += vd->type->getSize();
    }
    return sz;
}

size_t UnionDeclaration::getSize() const {
    size_t sz = 0;
    VariableDeclaration *vd;
    for(int i = 0; i < members.size(); i++){
        vd = members[i]->variableDeclaration();
        if(vd->getType()->getSize() > sz)
            sz = vd->getType()->getSize();
    }
    return sz;
}

size_t ClassDeclaration::getSize() const {
    size_t sz = base ? base->getDeclaredType()->getSize() : 0;
    VariableDeclaration *vd;
    unsigned align; //TODO: padding past base?
    for(int i = 0; i < members.size(); i++) {
        vd = members[i]->variableDeclaration();
        if(!vd) continue;
        align = vd->getType()->getAlign();
        if(sz % align)
            sz += (align - (sz % align));
        //reference types treated as pointers
        if(vd->getType()->isReference()) sz += 8;
        else sz += vd->type->getSize();
    }
    return sz;
}

void ClassDeclaration::populateVTable() {
    if(vtable.size() > 0) return; //already populated

    if(base && base->getDeclaration() && base->getDeclaration()->classDeclaration()) {
        ClassDeclaration *bclass = base->getDeclaration()->classDeclaration();
        bclass->populateVTable();

        // copy in base members
        for(int i = 0; i < bclass->vtable.size(); i++) {
            vtable.push_back(bclass->vtable[i]);
        }

        for(int i = 0; i < methods.size(); i++) {
            for(int j = 0; j < vtable.size(); j++) {
                // if overridden method
                if(methods[i]->getName() == vtable[j]->getName()) {
                    vtable[j] = methods[i];
                    goto CONTINUE;
                }
            }

            //not overridden, put at vtable end
            vtable.push_back(methods[i]);
CONTINUE:;
        }
    }
}

Expression *Expression::coerceTo(ASTType *ty) {
    ASTType *expty = getType();

    if(expty->is(ty)) {
        return this;
    }

    if(coercesTo(ty)) {
        //TODO: note in AST that this is implicit
        return new CastExpression(ty, this, loc);
    }

    emit_message(msg::ERROR, "attempt to coerce expression of type '" + expty->getName() +
            "' to incompatible type '" + ty->getName() + "'", loc);

    return NULL;
}

ASTType *BinaryExpression::getType() {
    switch(op.kind){
        case tok::equal:
        case tok::colonequal:
        case tok::plusequal:
        case tok::minusequal:
        case tok::starequal:
        case tok::slashequal:
        case tok::ampequal:
        case tok::barequal:
        case tok::caretequal:
        case tok::percentequal:
            return lhs->getType();
        case tok::barbar:
        case tok::kw_or:
        case tok::ampamp:
        case tok::kw_and:
        case tok::bangequal:
        case tok::equalequal:
        case tok::less:
        case tok::lessequal:
        case tok::greater:
        case tok::greaterequal:
            return ASTType::getBoolTy();
        case tok::plus:
        case tok::minus:
        case tok::lessless:
        case tok::greatergreater:
        case tok::star:
        case tok::slash:
        case tok::percent:
        case tok::bar:
        case tok::amp:
            //TODO: resolve type
            if(lhs->getType()->getPriority() > rhs->getType()->getPriority()) {
                return lhs->getType();
            } else {
                return rhs->getType();
            }
        default: return NULL;
    }
return NULL;
}


//TODO XXX
ASTType *DotExpression::getType() {
    if(rhs == "sizeof" || rhs == "offsetof") {
        return ASTType::getLongTy();
    }

    ASTType *lhstype = lhs->getType();

    if(rhs == "ptr" && lhstype->isArray()) {
        return lhstype->asArray()->arrayOf->getPointerTy();
    }

    if(rhs == "size" && lhstype->isArray()) {
        return ASTType::getLongTy();
    }


    //if type is pointer, implicit dereference on dot expression
    if(lhstype->asPointer()) {
        lhstype = lhstype->asPointer()->ptrTo;
    }

    if(ASTUserType *uty = lhstype->asUserType()) {
        Identifier *dotid = uty->getDeclaration()->lookup(rhs);
        if(dotid) return dotid->getType();
    }

    return NULL;
}


ASTTupleType *TupleExpression::getType() {
    std::vector<ASTType*> tupty;
    for(int i = 0; i < members.size(); i++) {
        tupty.push_back(members[i]->getType());
    }
    return ASTType::getTupleTy(tupty);
}

bool TupleExpression::coercesTo(ASTType *ty) {
    ASTType *this_ty = getType();

    if(getType()->coercesTo(ty)) return true;
    if(isConstant()) {
        if(ASTTupleType *tupty = ty->asTuple()) {
            if(tupty->length() != this_ty->length()) return false;
            for(int i = 0; i < members.size(); i++) {
                if(!members[i]->coercesTo(tupty->getMemberType(i))) return false;
            }
            return true;
        }

        // duplicate of above, but for static array destination
        if(ASTStaticArrayType *sarr = ty->asSArray()) {
            if(sarr->length() != this_ty->length()) return false;
            for(int i = 0; i < members.size(); i++) {
                if(!members[i]->coercesTo(sarr->getMemberType(i))) return false;
            }
            return true;
        }
    }
    return false;
}

ASTType *StringExpression::getType() {
    return ASTType::getStringTy(string.length());
}

ASTType *PackExpression::getType() {
    return ASTType::getStringTy(filesize);
}
