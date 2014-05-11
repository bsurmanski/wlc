SRCFILES=main.cpp token.cpp lexer.cpp parser.cpp irCodegenContext.cpp identifier.cpp symbolTable.cpp astType.cpp irDebug.cpp message.cpp parsec.cpp ast.cpp validate.cpp astVisitor.cpp

CLANGLIBS=\
/usr/lib/libclang.a\
/usr/lib/libclangIndex.a\
/usr/lib/libclangFrontend.a\
/usr/lib/libclangDriver.a\
/usr/lib/libclangTooling.a\
/usr/lib/libclangSerialization.a\
/usr/lib/libclangParse.a\
/usr/lib/libclangSema.a\
/usr/lib/libclangARCMigrate.a\
/usr/lib/libclangRewriteFrontend.a\
/usr/lib/libclangRewriteCore.a\
/usr/lib/libclangAnalysis.a\
/usr/lib/libclangEdit.a\
/usr/lib/libclangAST.a\
/usr/lib/libclangLex.a\
/usr/lib/libclangBasic.a\
/usr/lib/libclangFormat.a

#/usr/lib/libclangASTMatchers.a\
/usr/lib/libclangCodeGen.a\
/usr/lib/libclangDynamicASTMatchers.a\
/usr/lib/libclangFrontendTool.a\
/usr/lib/libclangStaticAnalyzerCheckers.a\
/usr/lib/libclangStaticAnalyzerCore.a\
/usr/lib/libclangStaticAnalyzerFrontend.a\

SRC=$(foreach file, $(SRCFILES), src/$(file))

all:
	ctags -R -o .git/tags
	g++ $(SRC) $(CLANGLIBS) `llvm-config --ldflags --cxxflags --libs` -lLLVM-3.4 -ggdb -O0 -frtti -UNDEBUG -DDEBUG -o wlc

install: wlc
	sudo cp wlc /usr/local/bin/
	cp wl.vim ~/.vim/syntax/
	sudo cp -R lib/* /usr/local/include/wl

installsyntax:
	cp wl.vim ~/.vim/syntax/
