#include "irDebug.hpp"

#include <vector>
#include <string>
#include <sstream>

using namespace std;
using namespace llvm;

IRDebug::IRDebug(IRCodegenContext *c, IRTranslationUnit *u) : context(c), unit(u),
     di(*unit->llvmModule)
{
    diUnit = di.createCompileUnit(dwarf::DW_LANG_hi_user, u->mdecl->filenm, PROJDIR, CGSTR, false, "", 0);
    diFile = di.createFile(u->mdecl->filenm, PROJDIR);
}

llvm::Module *IRDebug::getModule() { return (llvm::Module*) unit->llvmModule; }

// added due to requirement debug info in LLVM 3.4
void createIdentMetadata(llvm::Module *m)
{
    llvm::NamedMDNode *identMD = m->getOrInsertNamedMetadata("llvm.ident");
    llvm::Value *identNode = { llvm::MDString::get(m->getContext(), CGSTR) };
    identMD->addOperand(llvm::MDNode::get(m->getContext(), identNode));

    m->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 4); //TODO: dwarf version as param?
    m->addModuleFlag(llvm::Module::Error, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
}

llvm::DIDescriptor IRDebug::currentScope()
{
    return context->getScope()->getDebug();
}

llvm::DIDescriptor IRDebug::createScope(llvm::DIDescriptor parent, SourceLocation loc)
{
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR == 5
    //TODO path discriminator (computed from DILocation di.computeNewDiscriminator(LLVMContext)
    return di.createLexicalBlock(parent, currentFile(), loc.line, loc.ch, 0 /*path discriminator*/);

#elif LLVM_VERSION_MAJOR == 3 && (LLVM_VERSION_MINOR == 4 || LLVM_VERSION_MINOR >= 6)
    return di.createLexicalBlock(parent, currentFile(), loc.line, loc.ch);
#endif
}

llvm::DICompositeType IRDebug::createDynamicArrayType(ASTType *ty)
{
    llvm::DIDescriptor DIContext(currentFile());
    ASTArrayType *arrty = (ASTArrayType*) ty;
    vector<Value *> vec;
    vec.push_back(di.createMemberType(DIContext,
                "ptr",
                currentFile(),
                0, //TODO line num
                64,
                64,
                0,
                0,
                createType(arrty->arrayOf)));

    vec.push_back(di.createMemberType(DIContext,
                "size",
                currentFile(),
                0, //TODO line num
                64,
                64,
                64,
                0,
                createType(ASTType::getULongTy())));
    DIArray arr = di.getOrCreateArray(vec);

    return di.createStructType(DIContext, //TODO: defined scope
            ty->getName(),
            currentFile(), //TODO: defined file
            0, //line num //TODO line num
            128,
            64,
            0, // flags
            llvm::DIType(),
            arr
            );
}

llvm::DICompositeType IRDebug::createArrayType(ASTType *ty)
{
    llvm::DIDescriptor DIContext(currentFile());
    ASTArrayType *arrty = (ASTArrayType*) ty;
    assert(!arrty->isDynamic());
    return di.createArrayType(arrty->length(), arrty->arrayOf->getAlign(),
            createType(arrty->arrayOf),
            DIArray());
}

llvm::DICompositeType IRDebug::createTupleType(ASTType *ty)
{
    llvm::DIDescriptor DIContext(currentFile());
    ASTTupleType *tupty = ty->asTuple();
    assert(tupty && "expected tuple");

    vector<Value *> vec;
    int offset = 0;
    for(int i = 0; i < tupty->types.size(); i++)
    {
        unsigned size = tupty->types[i]->getSize();
        unsigned align = tupty->types[i]->getAlign();
        if(offset % align)
            offset += (align - (offset % align));
        stringstream ss;
        ss << i;
        vec.push_back(di.createMemberType(
                    DIContext,
                    std::string("[") + ss.str() + "]",
                    currentFile(),
                    0, //TODO line num
                    size * 8,
                    align * 8,
                    offset * 8,
                    0,
                    createType(tupty->types[i])));
        offset += tupty->types[i]->getSize();
        //TODO: members
    }

    DIArray arr = di.getOrCreateArray(vec);

    return di.createStructType(DIContext, //TODO: defined scope
            ty->getName(),
            currentFile(), //TODO: defined file
            0, //line num //TODO line num
            ty->getSize() * 8,
            ty->getAlign() * 8,
            0, // flags
            llvm::DIType(),
            arr
            );
}

llvm::DICompositeType IRDebug::createUserType(ASTType *ty) //TODO: proper calculation
{
    llvm::DIDescriptor DIContext(currentFile());
    ASTUserType *userty = (ASTUserType*) ty;
    vector<Value *> vec;
    //int offset = 0;
    for(int i = 0; i < userty->length(); i++)
    {
        VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(userty->getMember(i));
        assert(vdecl);
        ASTType *vtype = userty->getMember(i)->getType();
        unsigned size = vtype->getSize();
        unsigned align = vtype->getAlign();
        vec.push_back(di.createMemberType(
                    DIContext,
                    vdecl->getName(),
                    currentFile(),
                    userty->getMember(i)->loc.line,
                    size * 8,
                    align * 8,
                    userty->getMemberOffset(i) * 8,
                    0,
                    createType(vdecl->getType())));
    }

    DIArray arr = di.getOrCreateArray(vec);

    return di.createStructType(DIContext, //TODO: defined scope
            ty->getName(),
            currentFile(), //TODO: defined file
            userty->getDeclaration()->loc.line, //line num
            ty->getSize() * 8,
            ty->getAlign() * 8,
            0, // flags
            llvm::DIType(),
            arr
            );
}

llvm::DICompositeType IRDebug::createStructType(ASTType *ty)
{
    assert(ty->isStruct() && "expected struct for debug info generation");
    llvm::DIDescriptor DIContext(currentFile());
    ASTUserType *userty = (ASTUserType*) ty;
    vector<Value *> vec;
    int offset = 0;
    for(int i = 0; i < userty->length(); i++)
    {
        VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(userty->getMember(i));
        assert(vdecl);
        unsigned size = vdecl->getType()->getSize();
        unsigned align = vdecl->getType()->getAlign();
        if(offset % align)
            offset += (align - (offset % align));
        vec.push_back(di.createMemberType(
                    DIContext,
                    vdecl->getName(),
                    currentFile(),
                    userty->getMember(i)->loc.line,
                    size * 8,
                    align * 8,
                    offset * 8,
                    0,
                    createType(vdecl->getType())));
        offset += vdecl->getType()->getSize();
        //TODO: members
    }

    DIArray arr = di.getOrCreateArray(vec);

    return di.createStructType(DIContext, //TODO: defined scope
            ty->getName(),
            currentFile(), //TODO: defined file
            userty->getDeclaration()->loc.line, //line num
            ty->getSize() * 8,
            ty->getAlign() * 8,
            0, // flags
            llvm::DIType(),
            arr
            );
}

llvm::DICompositeType IRDebug::createUnionType(ASTType *ty)
{
    assert(ty->isUnion() && "expected union for debug info generation");
    llvm::DIDescriptor DIContext(currentFile());
    ASTUserType *userty = (ASTUserType*) ty;
    vector<Value *> vec;
    for(int i = 0; i < userty->length(); i++)
    {
        VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(userty->getMember(i));
        assert(vdecl);
        unsigned size = vdecl->getType()->getSize();
        unsigned align = vdecl->getType()->getAlign();
        vec.push_back(di.createMemberType(
                    DIContext,
                    vdecl->getName(),
                    currentFile(),
                    userty->getMember(i)->loc.line,
                    size * 8,
                    align * 8,
                    0, //offset
                    0,
                    createType(vdecl->getType())));
        //TODO: members
    }

    DIArray arr = di.getOrCreateArray(vec);

    return di.createUnionType(DIContext, //TODO: defined scope
            ty->getName(),
            currentFile(), //TODO: defined file
            userty->getDeclaration()->loc.line, //line num
            ty->getSize() * 8,
            ty->getAlign() * 8,
            0, // flags
            arr
            );
}

llvm::DIType IRDebug::createClassType(ASTType *ty)
{
    assert(ty->isClass() && "expected struct for debug info generation");
    llvm::DIDescriptor DIContext(currentFile());
    ASTUserType *userty = (ASTUserType*) ty;
    vector<Value *> vec;


    vec.push_back(di.createMemberType(DIContext, "vtable", currentFile(),
                userty->getDeclaration()->loc.line, 8 * 8, 8 * 8, 0, 0,
                createType(ASTType::getVoidTy()->getPointerTy())));

    vec.push_back(di.createMemberType(DIContext, "refs", currentFile(),
                userty->getDeclaration()->loc.line, 8 * 8, 8 * 8, 8 * 8, 0,
                createType(ASTType::getLongTy())));

    int offset = 16;
    for(int i = 0; i < userty->length(); i++)
    {
        VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(userty->getMember(i));
        assert(vdecl);
        unsigned size = vdecl->getType()->getSize();
        unsigned align = vdecl->getType()->getAlign();
        if(offset % align)
            offset += (align - (offset % align));
        vec.push_back(di.createMemberType(
                    DIContext,
                    vdecl->getName(),
                    currentFile(),
                    userty->getDeclaration()->loc.line,
                    size * 8,
                    align * 8,
                    offset * 8,
                    0,
                    createType(vdecl->getType())));
        offset += vdecl->getType()->getSize();
        //TODO: members
    }

    DIArray arr = di.getOrCreateArray(vec);

    llvm::DICompositeType clty = di.createStructType(DIContext, //TODO: defined scope
            ty->getName(),
            currentFile(), //TODO: defined file
            userty->getDeclaration()->loc.line, //line num
            ty->getSize() * 8,
            ty->getAlign() * 8,
            0, // flags
            llvm::DIType(),
            arr
            );

    return di.createPointerType(clty, 64, 64);
}

llvm::DICompositeType IRDebug::createInterfaceType(ASTType *ty) {
    assert(ty->isInterface() && "expected interface for debug info generation");
    llvm::DIDescriptor DIContext(currentFile());
    ASTUserType *userty = (ASTUserType*) ty;
    vector<Value *> vec;
    /*
    int offset = 0;
    for(int i = 0; i < userty->length(); i++)
    {
        VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(userty->getMember(i));
        assert(vdecl);
        unsigned size = vdecl->getType()->getSize();
        unsigned align = vdecl->getType()->getAlign();
        if(offset % align)
            offset += (align - (offset % align));
        vec.push_back(di.createMemberType(
                    DIContext,
                    vdecl->getName(),
                    currentFile(),
                    userty->getMember(i)->loc.line,
                    size * 8,
                    align * 8,
                    offset * 8,
                    0,
                    createType(vdecl->getType())));
        offset += vdecl->getType()->getSize();
        //TODO: members
    }
    */

    DIArray arr = di.getOrCreateArray(vec);

    //TODO: correctly create interface debug info
    return di.createStructType(DIContext, //TODO: defined scope
            ty->getName(),
            currentFile(), //TODO: defined file
            userty->getDeclaration()->loc.line, //line num
            ty->getSize() * 8,
            ty->getAlign() * 8,
            0, // flags
            llvm::DIType(),
            arr
            );
}

llvm::DIType IRDebug::createType(ASTType *ty)
{
    //if(!ty->diType)
    {
        llvm::DIType dity;
        switch(ty->getKind())
        {
            case TYPE_BOOL:
                dity = di.createBasicType("bool", 8, 8, dwarf::DW_ATE_boolean);
                break;
            case TYPE_CHAR:
                dity = di.createBasicType("char", 8, 8, dwarf::DW_ATE_signed_char);
                break;
            case TYPE_UCHAR:
                dity = di.createBasicType("uchar", 8, 8, dwarf::DW_ATE_unsigned_char);
                break;
            case TYPE_SHORT:
                dity = di.createBasicType("int16", 16, 16, dwarf::DW_ATE_signed);
                break;
            case TYPE_USHORT:
                dity = di.createBasicType("uint16", 16, 16, dwarf::DW_ATE_unsigned);
                break;
            case TYPE_INT:
                dity = di.createBasicType("int32", 32, 32, dwarf::DW_ATE_signed);
                break;
            case TYPE_UINT:
                dity = di.createBasicType("uint32", 32, 32, dwarf::DW_ATE_unsigned);
                break;
            case TYPE_LONG:
                dity = di.createBasicType("int64", 64, 64, dwarf::DW_ATE_signed);
                break;
            case TYPE_ULONG:
                dity = di.createBasicType("uint64", 64, 64, dwarf::DW_ATE_unsigned);
                break;
            case TYPE_FLOAT:
                dity = di.createBasicType("float32", 32, 32, dwarf::DW_ATE_float);
                break;
            case TYPE_DOUBLE:
                dity = di.createBasicType("float64", 64, 64, dwarf::DW_ATE_float);
                break;
            case TYPE_VOID:
                dity = di.createBasicType("void", 8, 8, dwarf::DW_ATE_address);
                break;
            case TYPE_POINTER:
                dity = di.createPointerType(createType(ty->getPointerElementTy()), 64, 64);
                break;
            case TYPE_USER:
                if(typeMap.count(ty->getMangledName())) {
                    return typeMap[ty->getMangledName()];
                }

                /*
                 * forward declaration to prevent issues with recursive types
                 * TODO: tag with struct/class/union when appropriate
                 */
                typeMap[ty->getMangledName()] =
                    di.createReplaceableForwardDecl(
                            // separate tag for union, class?
                        llvm::dwarf::DW_TAG_structure_type,
                        ty->getName(),
                        currentScope(),
                        currentFile(),
                        ty->asUserType()->getDeclaration()->loc.line);

                if(ty->isStruct())
                    dity = createStructType(ty);
                else if(ty->isUnion())
                    dity = createUnionType(ty);
                else if(ty->isClass())
                    dity = createClassType(ty);
                else if(ty->isInterface())
                    dity = createInterfaceType(ty);
                else emit_message(msg::FAILURE, "debug builder - unknown user type");
                typeMap[ty->getMangledName()] = dity;
                break;
            case TYPE_ARRAY:
                return createArrayType(ty);
            case TYPE_DYNAMIC_ARRAY:
                return createDynamicArrayType(ty);
            case TYPE_TUPLE:
                dity = createTupleType(ty);
                break;
            case TYPE_FUNCTION:
                dity = createPrototype(ty);
                break;
            case TYPE_UNKNOWN: //XXX workaround
                dity = di.createBasicType("void", 8, 8, dwarf::DW_ATE_address);
                break;
            default:
                assert(false && "debug info not yet added for type");
        }
        ty->diType = dity;
    }

    return ty->diType;
}

llvm::DICompositeType IRDebug::createPrototype(ASTType *p)
{
    ASTFunctionType *astfty = p->asFunctionType();
    assert(astfty);

    vector<Value*> vec;
    vec.push_back(createType(astfty->ret));

    if(astfty->owner) {
        vec.push_back(createType(astfty->owner));
    }

    for(int i = 0; i < astfty->params.size(); i++)
    {
        vec.push_back(createType(astfty->params[i]));
    }

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 6
    DITypeArray arr = di.getOrCreateTypeArray(vec);
    return di.createSubroutineType(diFile, arr);
#else // llvm 3.5 or lower
    DIArray arr = di.getOrCreateArray(vec);
    return di.createSubroutineType(diFile, arr);
#endif
}

llvm::DISubprogram IRDebug::createFunction(FunctionDeclaration *f, Function *cgFunc) {
    //if(!f->diSubprogram)
    {
    llvm::DIDescriptor DIContext(currentFile());
    f->diSubprogram = di.createFunction(
            DIContext, //f->getScope()->getDebug(),
            f->getName(),
            f->getMangledName(),
            currentFile(),
            f->loc.line, //TODO
            createPrototype(f->getType()),
            false, //is local
            f->body, //is definition
            f->loc.line,
            0, //flags
            false, //isoptimized
            cgFunc);
    }
    return f->diSubprogram;
}

llvm::DIGlobalVariable IRDebug::createGlobal(VariableDeclaration *decl, ASTValue *val)
{
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 6
    return di.createGlobalVariable(currentScope(),
			decl->identifier->getName(),
			decl->identifier->getMangledName(),
            currentFile(),
            decl->loc.line,
            createType(decl->getType()),
            false, //TODO: local to unit?
            (Value*) val->value);
#else
    return di.createGlobalVariable(decl->identifier->getName(),
            currentFile(),
            decl->loc.line,
            createType(decl->getType()),
            false, //TODO: local to unit?
            (Value*) val->value);
#endif
}

llvm::Instruction *IRDebug::createVariable(std::string nm, ASTValue *v, BasicBlock *bb, SourceLocation loc, int argn)
{
    //TODO: name, line, type
    vector<Value*> addr;

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 6
	DIVariable div = di.createLocalVariable(
		argn ? dwarf::DW_TAG_arg_variable : dwarf::DW_TAG_auto_variable,
		currentScope(),
		nm,
		currentFile(),
		loc.line,
		createType(v->getType()));
#else
	DIVariable div = di.createComplexVariable(
		argn ? dwarf::DW_TAG_arg_variable : dwarf::DW_TAG_auto_variable,
		currentScope(),
		nm,
		currentFile(),
		loc.line,
		createType(v->getType()),
		addr,
		false);
#endif

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 6
    Instruction *idinst = di.insertDeclare((llvm::Value*) v->value, div, DIExpression(), bb);
#else // 3.5 or lower
    Instruction *idinst = di.insertDeclare((llvm::Value*) v->value, div, bb);
#endif
    return idinst;
}
