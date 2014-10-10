SRCFILES:=main.cpp token.cpp lexer.cpp parser.cpp irCodegenContext.cpp identifier.cpp astScope.cpp astType.cpp irDebug.cpp message.cpp parsec.cpp ast.cpp validate.cpp astVisitor.cpp lowering.cpp

# additional clang libraries to build
CLANGLIBS= /usr/lib/libclang.a\
/usr/lib/libclangIndex.a\
/usr/lib/libclangFrontend.a\
/usr/lib/libclangDriver.a\
/usr/lib/libclangParse.a\
/usr/lib/libclangSema.a\
/usr/lib/libclangEdit.a\
/usr/lib/libclangTooling.a\
/usr/lib/libclangRewriteFrontend.a\
/usr/lib/libclangAST.a\
/usr/lib/libclangFormat.a\
/usr/lib/libclangBasic.a\
/usr/lib/libclangLex.a\
/usr/lib/libclangSerialization.a\
/usr/lib/libclangAnalysis.a\

# only used for clang 3.4, to build additional static libraries
CLANGLIBS_34:=/usr/lib/libclangRewriteCore.a\
/usr/lib/libLLVMSupport.a

CLANGLIBS_35:=/usr/lib/libclangRewrite.a

NODEPS:=clean install installsyntax
SRC:=$(foreach file, $(SRCFILES), src/$(file))
OBJ:=$(foreach file, $(SRCFILES), build/$(file:.cpp=.o))
DEP:=$(foreach file, $(SRCFILES), build/$(file:.cpp=.d))

# get llvm version in format of '3.4'
# llvm config will get version, and cut will extract only the
# major and minor version, ignoring any patch or svn qualifier
LLVMVERSION=$(shell llvm-config --version | cut -c -3)

LLVMLDFLAGS=

ifeq ($(LLVMVERSION),3.5)
LLVMLDFLAGS += -lLLVM-3.5 -lclang -lz -lpthread -lcurses -ldl
CLANGLIBS:=$(CLANGLIBS) $(CLANGLIBS_35)
endif

ifeq ($(LLVMVERSION),3.4)
LLVMLDFLAGS += -lLLVM-3.4
CLANGLIBS:=$(CLANGLIBS) $(CLANGLIBS_34)
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

build/%.o: src/%.cpp
	g++ $< -c $(CXXFLAGS) -ggdb -O0 -frtti -UNDEBUG -DDEBUG -o $@
	g++ $(CXXFLAGS) -MM -MT '$@' $< -MF build/$*.d

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
