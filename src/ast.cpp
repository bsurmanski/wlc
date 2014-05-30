#include "ast.hpp"

Package::Package(Package *par, std::string nm) : parent(par), scope(NULL), cgValue(NULL),
    identifier(NULL)
{
    name = nm;
    if(parent) {
        identifier = parent->getScope()->getInScope(name);
        identifier->setDeclaration(new PackageDeclaration(this, identifier,
                    SourceLocation(nm.c_str(), 1)), Identifier::ID_PACKAGE);
    }
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
        sz += vd->type->getSize();
    }
    return sz;
}
