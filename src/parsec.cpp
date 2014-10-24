/// C Parsing fun stuff

#include <cstdio>
#include <fcntl.h>

#ifdef WIN32
#include <io.h>
#include <sys/stat.h>
#define dup _dup
#define dup2 _dup2
#define open _open
#define close _close
#define basename _basename
#define access _access
#define fileno _fileno
#define S_IWUSR _S_IWRITE
#define S_IWGRP _S_IWRITE
#define S_IWOTH _S_IWRITE
#define S_IRUSR _S_IREAD
#define S_IRGRP _S_IREAD
#define S_IROTH _S_IREAD
#define F_OK 04 //file exists and is readible
#else
#include <unistd.h>
#endif

#include <clang-c/Index.h>
#include <vector>
#include <iostream>

#include "parsec.hpp"

using namespace std;

SourceLocation SourceLocationFromCLocation(CXSourceLocation cxloc)
{
    CXFile file;
    unsigned line, column;
    clang_getSpellingLocation(cxloc, &file, &line, &column, 0);
    const char *filenm = clang_getCString(clang_getFileName(file));
    return SourceLocation(filenm, line, column);
}

struct StructVisitorArg
{
    TranslationUnit *unit;
    std::vector<Declaration *> *members;
    ASTScope *scope;
};

ASTType *ASTTypeFromCType(TranslationUnit *unit, CXType ctype);

CXChildVisitResult StructVisitor(CXCursor cursor, CXCursor parent, void *svarg)
{
    StructVisitorArg *m = (StructVisitorArg*) svarg;

    TranslationUnit *unit = m->unit;
    std::vector<Declaration *> *members = m->members;
    ASTScope *scope = m->scope;

    CXSourceLocation cxloc = clang_getCursorLocation(cursor);
    SourceLocation loc = SourceLocationFromCLocation(cxloc);

    DeclarationQualifier dqual;
    dqual.decorated = false;

	if (clang_isAttribute(cursor.kind)) {
		char message[256];
		CXString cxname = clang_getCursorDisplayName(cursor);
		string name = clang_getCString(cxname);
		
		sprintf(message, "declaration attribute ignored: %s", name.c_str());
		emit_message(msg::WARNING, message);
	} else if(cursor.kind == CXCursor_FieldDecl)
    {
        CXString cxname = clang_getCursorSpelling(cursor);
        string name = clang_getCString(cxname);

        Identifier *id = scope->getInScope(name);
        ASTType *ty = ASTTypeFromCType(unit, clang_getCursorType(cursor));
        if(!ty) return CXChildVisit_Break;
        VariableDeclaration *vdecl = new VariableDeclaration(ty, id, 0, loc, dqual);
        id->addDeclaration(vdecl, Identifier::ID_VARIABLE);
        members->push_back(vdecl);
    } else if(cursor.kind == CXCursor_UnionDecl || cursor.kind == CXCursor_StructDecl)
    {
        ASTType *ty = ASTTypeFromCType(unit, clang_getCursorType(cursor));
        if(!ty) return CXChildVisit_Break;
        Identifier *id = scope->get("___" + ty->getName()); // TODO: proper scope name
        VariableDeclaration *vdecl = new VariableDeclaration(ty, id, 0, loc, dqual);
        id->addDeclaration(vdecl, Identifier::ID_VARIABLE);
        //members->push_back(vdecl);
    } else
    {
		char message[256];
		sprintf(message, "unknown field in C record: %d, %s:%d", cursor.kind, loc.filenm, loc.line);
        emit_message(msg::FAILURE, message, loc);
    }

    return CXChildVisit_Continue;
}

ASTType *ASTRecordTypeFromCType(TranslationUnit *unit, CXType ctype)
{
    vector<Declaration*> members;
    ASTScope *tbl = new ASTScope(unit->getScope());
    CXString cxname = clang_getTypeSpelling(ctype);
    CXCursor typeDecl = clang_getTypeDeclaration(ctype);
    string name = clang_getCString(cxname);
    string sstruct = "struct ";
    string sunion = "union ";
    if(name.compare(0, sstruct.length(), sstruct) == 0) //remove 'struct' keyword
    {
        name = name.substr(sstruct.length());
    } else if(name.compare(0, sunion.length(), sunion) == 0) //remove 'union' keyword
    {
        name = name.substr(sunion.length());
    }

    Identifier *id = unit->getScope()->lookup(name);

    UserTypeDeclaration *utdecl = 0;
    if(!id || id->isUndeclared()) //TODO: should just use 'get' above?
    {
        id = unit->getScope()->get(name);

        StructVisitorArg svarg = { unit, &members, tbl };
        clang_visitChildren(typeDecl, StructVisitor, &svarg);

        if(typeDecl.kind == CXCursor_StructDecl)
        {
            //TODO: correct source loc
            utdecl = new StructDeclaration(id, tbl, SourceLocation(), DeclarationQualifier());
        } else {// is union
            utdecl = new UnionDeclaration(id, tbl, SourceLocation(), DeclarationQualifier());
        }

        for(int i = 0; i < members.size(); i++) {
            utdecl->addMember(members[i]);
        }

        return utdecl->getDeclaredType();
    }

    return id->getDeclaredType();
}


ASTType *ASTTypeFromCType(TranslationUnit *unit, CXType ctype)
{
    ASTType *ty = 0;

    switch(ctype.kind)
    {
        case CXType_Void:
            return ASTType::getVoidTy();
        case CXType_Unexposed: //TODO: create a specific type for unexposed?
            return ASTType::getVoidTy()->getPointerTy();
        case CXType_Bool: return ASTType::getBoolTy();
        case CXType_Char_U:
        case CXType_UChar: return ASTType::getUCharTy();
        case CXType_Char_S:
        case CXType_SChar: return ASTType::getCharTy();
        case CXType_UShort: return ASTType::getUShortTy();
        case CXType_Short: return ASTType::getShortTy();
        case CXType_UInt: return ASTType::getUIntTy();
        case CXType_Int: return ASTType::getIntTy();
        case CXType_Long:
        case CXType_LongLong:
            return ASTType::getLongTy();
        case CXType_ULong:
        case CXType_ULongLong:
            return ASTType::getULongTy();
        case CXType_Float:
            return ASTType::getFloatTy();
        case CXType_Double:
            return ASTType::getDoubleTy();
        case CXType_Pointer:
        case CXType_VariableArray:
            ty = ASTTypeFromCType(unit, clang_getPointeeType(ctype));
            if(ty) return ty->getPointerTy();
            return NULL;
        case CXType_Record:
            return ASTRecordTypeFromCType(unit, ctype);
        case CXType_Typedef:
            return ASTTypeFromCType(unit, clang_getCanonicalType(ctype));
        case CXType_Enum:
            return ASTType::getULongTy();
        case CXType_ConstantArray:
            return ASTTypeFromCType(unit,
                    clang_getElementType(ctype))->getArrayTy(clang_getArraySize(ctype));
        case CXType_FunctionProto:
            return NULL; //TODO
        default:
            emit_message(msg::WARNING, "failed conversion of CType to WLType: " +
                    string(clang_getCString(clang_getTypeSpelling(ctype))));
            return NULL;
    }
}

#include<clang/Lex/MacroInfo.h>
#include<clang/Lex/Preprocessor.h>

namespace clang {
    class MacroDefinition;
    
	namespace cxcursor {
        const MacroDefinition *getCursorMacroDefinition(CXCursor c);
	}

    namespace cxindex {
        const MacroInfo *getMacroInfo(const MacroDefinition *def, CXTranslationUnit TU);
    }
}

const clang::MacroInfo *getCursorMacroInfo(CXCursor c)
{
    const clang::MacroDefinition *definition = clang::cxcursor::getCursorMacroDefinition(c);
    const CXTranslationUnit TU = clang_Cursor_getTranslationUnit(c);
    return clang::cxindex::getMacroInfo(definition, TU);
}

static CXTranslationUnit *cxUnit = 0;
CXChildVisitResult CVisitor(CXCursor cursor, CXCursor parent, void *tUnit)
{
    TranslationUnit* unit = (TranslationUnit *) tUnit;

    CXSourceLocation cxloc = clang_getCursorLocation(cursor);
    SourceLocation loc = SourceLocationFromCLocation(cxloc);

    if(cursor.kind == CXCursor_FunctionDecl)
    {
        CXString cxname = clang_getCursorSpelling(cursor);
        std::string name = clang_getCString(cxname);

        CXType fType = clang_getCursorType(cursor);
        int nargs = clang_getNumArgTypes(fType);

        vector<ASTType*> params;
        vector<VariableDeclaration*> parameters;

        for(int i = 0; i < nargs; i++)
        {
            ASTType *astArgTy =
                    ASTTypeFromCType(unit,
                            clang_getCanonicalType(clang_getArgType(fType, i)));
            if(!astArgTy) goto ERR;

            params.push_back(astArgTy);
            parameters.push_back(new VariableDeclaration(astArgTy, NULL, NULL, loc, DeclarationQualifier()));
        }

        ASTType *rType = ASTTypeFromCType(unit, clang_getResultType(fType));

        if(!rType) goto ERR;

        Identifier *id = unit->getScope()->get(name);

        DeclarationQualifier dqual;
        dqual.decorated = false;

        FunctionDeclaration *fdecl = new FunctionDeclaration(id, NULL, rType,
               parameters, clang_isFunctionTypeVariadic(fType), 0, 0, loc, dqual);

        id->addDeclaration(fdecl, Identifier::ID_FUNCTION);

        //unit->functions.push_back(fdecl);

    } else if(cursor.kind == CXCursor_VarDecl)
    {
        CXType type = clang_getCursorType(cursor);
        CXString cxname = clang_getCursorSpelling(cursor);
        std::string name = clang_getCString(cxname);

        ASTType *wlType = ASTTypeFromCType(unit, type);
        if(!wlType) goto ERR;

        Identifier *id = unit->getScope()->get(name);
        CXLinkageKind linkage = clang_getCursorLinkage(cursor);
        DeclarationQualifier dqual;
        dqual.external = linkage == CXLinkage_External || linkage == CXLinkage_UniqueExternal;
        dqual.decorated = false;
        VariableDeclaration *vdecl = new VariableDeclaration(wlType, id, 0, loc, dqual);
        id->addDeclaration(vdecl, Identifier::ID_VARIABLE);
        //unit->globals.push_back(vdecl);

    } else if(cursor.kind == CXCursor_MacroDefinition)
    {
        const clang::MacroInfo *MI = getCursorMacroInfo(cursor);
        if(!MI) goto MACRO_ERR;
        if(MI->isObjectLike())
        {
            if(MI->getNumTokens() == 1 && MI->getReplacementToken(0).isLiteral()) // literal value
            {
                clang::Token tok = MI->getReplacementToken(0);
                if(tok.getKind() == clang::tok::numeric_constant)
                {
                    string name = string(clang_getCString(clang_getCursorSpelling(cursor)));
                    Identifier *id = unit->getScope()->get(name);
                    if(id->getDeclaration())
                    {
                        printf("Macro redefines value, ");
                        goto MACRO_ERR;
                    }
                    NumericExpression *val = new NumericExpression(NumericExpression::DOUBLE,
                            ASTType::getDoubleTy(), atof(tok.getLiteralData()));
                    //VariableDeclaration *vdecl = new VariableDeclaration(ASTType::getDoubleTy(),
                    //        id, val, loc, false);
                    id->setExpression(val);
                    //unit->globals.push_back(vdecl);
                        //printf("MACRO: %s: %f\n", name.c_str(), val->floatValue);
                } else goto MACRO_ERR;
            } else if(MI->getNumTokens() == 1 && MI->getReplacementToken(0).isAnyIdentifier()) { // identifier alias
                string name = string(clang_getCString(clang_getCursorSpelling(cursor)));
                string alias = string(MI->getReplacementToken(0).getIdentifierInfo()->getName().str());

                Identifier *idName = unit->getScope()->get(name);
                Identifier *idAlias = unit->getScope()->get(alias);
                if(idName->getDeclaration())
                {
                    printf("Macro redefines value");
                    goto MACRO_ERR;
                }

                IdentifierExpression *val = new IdentifierExpression(idAlias);
                idName->setExpression(val);
            } else
            {
                goto MACRO_ERR;
            }
        } else goto MACRO_ERR;
    } else if(cursor.kind == CXCursor_EnumDecl)
    {
        return CXChildVisit_Recurse;
    } else if(cursor.kind == CXCursor_EnumConstantDecl)
    {
        CXString cxname = clang_getCursorSpelling(cursor);
        std::string name = clang_getCString(cxname);
        Identifier *id = unit->getScope()->get(name);
        int64_t val = clang_getEnumConstantDeclValue(cursor);
                NumericExpression *nval = new NumericExpression(NumericExpression::INT,
                        ASTType::getLongTy(), (uint64_t) val);

        id->setExpression(nval);
    } else if(cursor.kind == CXCursor_StructDecl)
    {
        // will generate struct ASTType
        ASTTypeFromCType(unit, clang_getCursorType(cursor));
    } else if(cursor.kind == CXCursor_TypedefDecl) {
        /*
        // XXX FROM OTHER ATTEMPT ON VULCAN
        Identifier *id = unit->getScope()->get(clang_getCString(clang_getCursorSpelling(cursor)));
        ASTType *typedefty = ASTTypeFromCType(unit, clang_getTypedefDeclUnderlyingType(cursor));
        id->setExpression(new TypeExpression(typedefty, loc));
        */
        goto ERR; //TODO
        CXString cxname = clang_getCursorSpelling(cursor);
        std::string name = clang_getCString(cxname);
        CXType ctype = clang_getTypedefDeclUnderlyingType(cursor);
        ASTType *ty = ASTTypeFromCType(unit, ctype);
        if(!ty) goto ERR;
        Identifier *id = unit->getScope()->get(name);
        id->setKind(Identifier::ID_TYPE);
        id->setDeclaredType(ty);
    }

    return CXChildVisit_Continue;
MACRO_ERR:
    emit_message(msg::WARNING, "failed to convert macro into WL symbol: " +
            string(clang_getCString(clang_getCursorSpelling(cursor))));
    return CXChildVisit_Continue;
ERR:
    emit_message(msg::WARNING, "failed to convert symbol to WL typing: " +
            string(clang_getCString(clang_getCursorSpelling(cursor))));
    return CXChildVisit_Continue;
}

#ifdef WIN32
std::string getAbsoluteIncludePath(std::string file) {
	char *ienv = getenv("INCLUDE");
	if (!ienv) return "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/include/" + file; // default location
	//printf("INCLUDE: %s\n", ienv); 
	//TODO split ienv delimiter, search for any existing files
	return "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/include/" + file;
}
#else
std::string getAbsoluteIncludePath(std::string file) {
	return "/usr/include/" + file;
}
#endif

void parseCImport(TranslationUnit *unit,
        std::string filenm,
        SourceLocation loc)
{
    // redirect stderr
    int ofilenm = fileno(stderr);
    int oerr, nerr;
    fflush(stderr);
    nerr = open("stderr.log", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if(nerr > 0) { // there was no error opening stderr.log
        oerr = dup(fileno(stderr));
        dup2(nerr, ofilenm);
        close(nerr);
    }

    if(access(filenm.c_str(), F_OK) == -1)
    {
		filenm = getAbsoluteIncludePath(filenm);
        if(access(filenm.c_str(), F_OK) == -1) {
            emit_message(msg::ERROR, "imported C file does not exist: " + filenm, loc);
        }
    }

    const char *commandArgs[] = {
        "-triple", "x86_64-unknown-linux-gnu",
        "-resource-dir", "/usr/lib/clang/3.4",
        "-internal-isystem /usr/include",
        "-c-isystem /usr/include",
        "-DGL_GLEXT_PROTOTYPES", //TODO: tmp
        "-v",
        0,
    };
    //pushScope(unit->getScope());

    CXIndex Idx = clang_createIndex(1,1);
    CXTranslationUnit Unit = clang_createTranslationUnitFromSourceFile(
            Idx,
            filenm.c_str(),
            (sizeof(commandArgs) / sizeof(commandArgs[0]) - 1),
            commandArgs,
            0,
            0);

    cxUnit = &Unit;

    clang_visitChildren(clang_getTranslationUnitCursor(Unit), CVisitor, unit);

    //currentPackage()->addPackage(unit);

    //popScope();

    // restore stderr if we changed it earlier (if there was no error opening stderr.log
    if(nerr > 0) {
        fflush(stderr);
        dup2(oerr, ofilenm);
        close(oerr);
    }
}
