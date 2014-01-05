#include "irDebug.hpp"

#include <vector>

using namespace std;
using namespace llvm;


llvm::DIDescriptor IRDebug::currentScope()
{
    return context->getScope()->getDebug();
}


llvm::DICompositeType IRDebug::createStructType(ASTType *ty)
{
    assert(ty->isStruct() && "expected struct");
    StructTypeInfo *sti = (StructTypeInfo*) ty->info;
    vector<Value *> vec;
    for(int i = 0; i < sti->members.size(); i++)
    {
        
    }

    DIArray arr = di.getOrCreateArray(vec);

    return di.createStructType(currentScope(), //TODO: defined scope
            ty->getName(), 
            currentFile(), //TODO: defined file 
            0, 
            ty->size(),
            ty->size(),
            0, 
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
    f->diSubprogram = di.createFunction(currentScope(), f->getName(), f->getName(),
            currentFile(), 
            0, //TODO
            createPrototype(f->prototype), false, true, 0, 0, false, (Function*) f->cgValue);
    }
    return f->diSubprogram;
}
