#include "irDebug.hpp"

#include <vector>
#include <string>
#include <sstream>

using namespace std;
using namespace llvm;

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

llvm::DICompositeType IRDebug::createTupleType(ASTType *ty)
{

    assert(ty->type == TYPE_TUPLE && "expected struct");
    llvm::DIDescriptor DIContext(currentFile());
    TupleTypeInfo *tti = (TupleTypeInfo*) ty->info;
    vector<Value *> vec;
    int offset = 0;
    for(int i = 0; i < tti->types.size(); i++)
    {
        unsigned size = tti->types[i]->size();
        unsigned align = tti->types[i]->align();
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
                    createType(tti->types[i])));
        offset += tti->types[i]->size();
        //TODO: members 
    }

    DIArray arr = di.getOrCreateArray(vec);

    return di.createStructType(DIContext, //TODO: defined scope
            ty->getName(), 
            currentFile(), //TODO: defined file 
            0, //line num //TODO line num
            ty->size() * 8,
            ty->align() * 8,
            0, // flags
            llvm::DIType(),
            arr
            ); 
}

llvm::DICompositeType IRDebug::createStructType(ASTType *ty)
{
    assert(ty->isStruct() && "expected struct");
    llvm::DIDescriptor DIContext(currentFile());
    StructTypeInfo *sti = (StructTypeInfo*) ty->info;
    vector<Value *> vec;
    int offset = 0;
    for(int i = 0; i < sti->members.size(); i++)
    {
        VariableDeclaration *vdecl = dynamic_cast<VariableDeclaration*>(sti->members[i]);
        assert(vdecl);
        unsigned size = vdecl->getType()->size();
        unsigned align = vdecl->getType()->align();
        if(offset % align)
            offset += (align - (offset % align));
        vec.push_back(di.createMemberType(
                    DIContext,
                    vdecl->getName(),
                    currentFile(),
                    sti->members[0]->loc.line,
                    size * 8,
                    align * 8,
                    offset * 8,
                    0, 
                    createType(vdecl->getType())));
        offset += vdecl->getType()->size();
        //TODO: members 
    }

    DIArray arr = di.getOrCreateArray(vec);

    return di.createStructType(DIContext, //TODO: defined scope
            ty->getName(), 
            currentFile(), //TODO: defined file 
            sti->getDeclaration()->loc.line, //line num
            ty->size() * 8,
            ty->align() * 8,
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
        switch(ty->type)
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
                dity = di.createPointerType(createType(ty->getReferencedTy()), 64, 64);
                break;
            case TYPE_STRUCT:
                dity = createStructType(ty);
                break;
            case TYPE_ARRAY:
                return DIType(); //TODO
            case TYPE_TUPLE:
                dity = createTupleType(ty);
                break;
            default:
                assert(false && "debug info not yet added for type");
        }
        ty->diType = dity;
    }

    return ty->diType;
}

llvm::DICompositeType IRDebug::createPrototype(FunctionPrototype *p)
{
    vector<Value*> vec;

    vec.push_back(createType(p->returnType));
    for(int i = 0; i < p->parameters.size(); i++)
    {
        vec.push_back(createType(p->parameters[i].first));
    }

    DIArray arr = di.getOrCreateArray(vec);
    return di.createSubroutineType(diFile, arr);
}

llvm::DISubprogram IRDebug::createFunction(FunctionDeclaration *f)
{
    //if(!f->diSubprogram)
    {
    llvm::DIDescriptor DIContext(currentFile());
    f->diSubprogram = di.createFunction(
            DIContext, //f->getScope()->getDebug(), 
            f->getName(),
            f->getName(),
            currentFile(), 
            f->loc.line, //TODO
            createPrototype(f->prototype), 
            false, //is local
            f->body, //is definition
            f->loc.line,
            0, //flags
            false, //isoptimized
            (Function*) f->cgValue);

    for(int i = 0; i < f->prototype->parameters.size(); i++)
    {
    }
    }
    return f->diSubprogram;
}

llvm::DIGlobalVariable IRDebug::createGlobal(VariableDeclaration *decl, ASTValue *val)
{
    return di.createGlobalVariable(decl->identifier->getName(), 
            currentFile(), 
            decl->loc.line,
            createType(decl->getType()),
            false, //TODO: local to unit?
            (Value*) val->cgValue);
}

llvm::Instruction *IRDebug::createVariable(std::string nm, ASTValue *v, BasicBlock *bb, SourceLocation loc, int argn)
{
    //TODO: name, line, type
    vector<Value*> addr;

    DIVariable div = di.createComplexVariable(
                argn ? dwarf::DW_TAG_arg_variable : dwarf::DW_TAG_auto_variable,
                currentScope(),
                nm,
                currentFile(),
                loc.line,
                createType(v->getType()),
                addr,
                false);

    Instruction *idinst = di.insertDeclare((llvm::Value*) v->cgValue, div, bb);
    return idinst;
}
