#include "ast.hpp"
#include "validate.hpp"
#include "sema.hpp"
#include "lower.hpp"
#include "codegenContext.hpp"


#if defined WIN32
#include<Windows.h>
#undef ERROR
char *realpath(const char *path, char *resolve) {
	return _fullpath(resolve, path, PATH_MAX);
}
std::string getFilebase(std::string s)
{
	std::tr2::sys::path filepath(s);
	return basename(filepath);
}

unsigned getFileSize(std::string filenm) {
	HANDLE hFile = CreateFile(filenm.c_str(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1; // error condition, could call GetLastError to find out more

	LARGE_INTEGER size;
	if (!GetFileSizeEx(hFile, &size))
	{
		CloseHandle(hFile);
		return -1; // error condition, could call GetLastError to find out more
	}

	CloseHandle(hFile);
	return size.QuadPart;
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

unsigned getFileSize(std::string filenm) {
	struct stat st;
	if(stat(filenm.c_str(), &st)) {
		return 0;
	}
	return st.st_size;
}

#else //linux

std::string getFilebase(std::string s)
{
	size_t lastDot = s.find_last_of(".");
	if (lastDot != std::string::npos)
		s = s.substr(0, lastDot);
	return basename(s.c_str());
}

unsigned getFileSize(std::string filenm) {
	struct stat st;
	if (stat(filenm.c_str(), &st)) {
		return 0;
	}
	return st.st_size;
}

#endif


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

        if(owner && !isStatic()) {
            if(owner->isInterface()) paramTy.push_back(ASTType::getVoidTy()->getPointerTy());
            else if(owner->isStruct()) paramTy.push_back(owner->getReferenceTy());
            else paramTy.push_back(owner);
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

FunctionDeclaration *UserTypeDeclaration::getMethod(std::string name, ASTFunctionType *opt_ty) {
    for(int i = 0; i < methods.size(); i++) {
        if(name == methods[i]->getName()) {
            FunctionDeclaration *fdecl = methods[i];
            while(fdecl) {
                //XXX using 'coercesTo' so that first parameter 'this' may convert to proper type
                if(!opt_ty || fdecl->getType()->coercesTo(opt_ty)) {
                    return fdecl;
                }
                fdecl = fdecl->getNextOverload();
            }
        }
    }
    return NULL;
}

FunctionDeclaration *ClassDeclaration::getMethod(std::string name, ASTFunctionType *opt_ty) {
    FunctionDeclaration *fdecl = UserTypeDeclaration::getMethod(name, opt_ty);
    if(!fdecl && base) {
        UserTypeDeclaration *bdecl = base->getDeclaration()->userTypeDeclaration();
        assert(bdecl);
        return bdecl->getMethod(name, opt_ty);
    }

    return fdecl;
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
    }

    //XXX static methods are populated in vtable.
    // I like the idea, but I haven't tested it's validity yet
    for(int i = 0; i < methods.size(); i++) {
        for(int j = 0; j < vtable.size(); j++) {
            // if overridden method
            if(methods[i]->getName() == vtable[j]->getName() &&
                    methods[i]->getType()->coercesTo(vtable[j]->getType())) {
                //TODO: double check that function return type can be varient
                methods[i]->setVTableIndex(j);
                vtable[j] = methods[i];
                goto CONTINUE;
            }
        }

        //not overridden, put at vtable end
        methods[i]->setVTableIndex(vtable.size());
        vtable.push_back(methods[i]);
CONTINUE:;
    }
}

// does not actually populate any vtable. each type that gets converted to an
// interface type will have it's own vtable. BUT this populates the method indices
// that are constant for every type converted to interface
void InterfaceDeclaration::populateVTable() {
    for(int i = 0; i < methods.size(); i++) {
            methods[i]->setVTableIndex(i);
    }
}

//
// Expression
//

ASTValue *Expression::getValue(CodegenContext *ctx) {
    //TODO: do not cache value if global
    if(!value) {
        value = ctx->codegenExpression(this);
    }
    return value;
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

    // if coercing to composite type, and all members will coerce to respective type, we are fine
    if(ASTCompositeType *cty = ty->asCompositeType()) {
        if(cty->isClass()) return false;
        if((cty->isTuple() || cty->isSArray()) &&
                cty->length() != this_ty->length()) {
            return false;
        }
        for(int i = 0; i < members.size(); i++) {
            if(!members[i]->coercesTo(cty->getMemberType(i))) return false;
        }
        return true;
    }

    return false;
}

ASTType *StringExpression::getType() {
    return ASTType::getStringTy(string.length());
}

ASTType *PackExpression::getType() {
    return ASTType::getStringTy(filesize);
}
