SRCFILES=main.cpp token.cpp lexer.cpp parser.cpp irCodegenContext.cpp identifier.cpp astScope.cpp astType.cpp irDebug.cpp message.cpp parsec.cpp ast.cpp validate.cpp astVisitor.cpp

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
OBJ=$(foreach file, $(SRCFILES), build/$(file:.cpp=.o))
DEP=$(foreach file, $(SRCFILES), build/$(file:.cpp=.d))

CXXFLAGS=`llvm-config --cxxflags` -ggdb -O0 -frtti -UNDEBUG -DDEBUG
LDFLAGS=`llvm-config --ldflags --libs` -lLLVM-3.4

.PHONY: clean all

all: wlc

clean:
	rm -rf build

wlc: build $(OBJ) $(SRC)
	ctags -R -o .git/tags
	g++ $(OBJ) $(CLANGLIBS) $(CXXFLAGS) $(LDFLAGS) -o wlc

build/%.o: src/%.cpp build/%.d
	g++ $< -c $(CXXFLAGS) -ggdb -O0 -frtti -UNDEBUG -DDEBUG -o $@

build/%.d: src/%.cpp
	g++ $(CXXFLAGS) -MM -MT '$(patsubst %.d,%.o,$@)' $< -MF $@

build:
	mkdir -p build

install: wlc
	sudo cp wlc /usr/local/bin/
	cp wl.vim ~/.vim/syntax/
	sudo cp -R lib/* /usr/local/include/wl

installsyntax:
	cp wl.vim ~/.vim/syntax/

-include $(DEP)
