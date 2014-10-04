SRCFILES:=main.cpp token.cpp lexer.cpp parser.cpp irCodegenContext.cpp identifier.cpp astScope.cpp astType.cpp irDebug.cpp message.cpp parsec.cpp ast.cpp validate.cpp astVisitor.cpp lowering.cpp


# additional clang libraries to build
# only used for clang 3.4, to build additional static libraries
CLANGLIBS=
CLANGLIBS_34:=/usr/lib/libclang.a\
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
/usr/lib/libclangFormat.a \
/usr/lib/libLLVMSupport.a

NODEPS:=clean install installsyntax
SRC:=$(foreach file, $(SRCFILES), src/$(file))
OBJ:=$(foreach file, $(SRCFILES), build/$(file:.cpp=.o))
DEP:=$(foreach file, $(SRCFILES), build/$(file:.cpp=.d))

LLVMVERSION=$(shell llvm-config --version)

LLVMLDFLAGS=

TESTVER=1.2.3

ifeq ($(LLVMVERSION),3.5.0)
LLVMLDFLAGS += -lLLVM-3.5 -lclang -lz -lpthread -lcurses -ldl
endif

ifeq ($(LLVMVERSION),3.4.0)
LLVMLDFLAGS += -lLLVM-3.4
CLANGLIBS=$(CLANGLIBS_34)
endif

CXXFLAGS=`llvm-config --cxxflags` -ggdb -O0 -frtti -UNDEBUG -DDEBUG -I/usr/local/include
LDFLAGS=`llvm-config --ldflags --libs` $(LLVMLDFLAGS)
.PHONY: clean all install installsyntax

all: build wlc

clean:
	rm -rf build

wlc: $(OBJ)
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
	sudo mkdir -p /usr/local/include/wl
	sudo cp -R lib/* /usr/local/include/wl

installsyntax:
	cp wl.vim ~/.vim/syntax/

-include $(DEP)
