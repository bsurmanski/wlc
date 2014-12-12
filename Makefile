SRCFILES:=main.cpp token.cpp lexer.cpp parser.cpp irCodegenContext.cpp identifier.cpp astScope.cpp astType.cpp irDebug.cpp message.cpp parsec.cpp ast.cpp validate.cpp sema.cpp astVisitor.cpp lowering.cpp

# additional clang libraries to build
llvm_prefix=/usr
ifeq ($(shell if [ -e /usr/local/lib/libclang.a ]; then echo "EXISTS"; fi ),EXISTS)
	llvm_prefix=/usr/local
endif

CLANGLIBS = libclang.a\
libclangIndex.a\
libclangFrontend.a\
libclangDriver.a\
libclangParse.a\
libclangSema.a\
libclangEdit.a\
libclangTooling.a\
libclangRewriteFrontend.a\
libclangAST.a\
libclangFormat.a\
libclangBasic.a\
libclangLex.a\
libclangSerialization.a\
libclangAnalysis.a

# only used for clang 3.4, to build additional static libraries
CLANGLIBS_34:=libclangRewriteCore.a\
libLLVMSupport.a

CLANGLIBS_35:=libclangRewrite.a

NODEPS:=clean install installsyntax
SRC:=$(foreach file, $(SRCFILES), src/$(file))
OBJ:=$(foreach file, $(SRCFILES), build/$(file:.cpp=.o))
DEP:=$(foreach file, $(SRCFILES), build/$(file:.cpp=.d))

# get llvm version in format of '3.4'
# llvm config will get version, and cut will extract only the
# major and minor version, ignoring any patch or svn qualifier
LLVMVERSION=$(shell llvm-config --version | cut -c -3)

LLVMLDFLAGS=

ifeq ($(shell uname -s),Darwin)
LLVMLDFLAGS += -lllvm
endif

ifeq ($(LLVMVERSION),3.5)
ifeq ($(shell uname -s),Linux)
LLVMLDFLAGS += -lLLVM-3.5
endif
LLVMLDFLAGS += -lclang -lz -lpthread -lcurses -ldl
CLANGLIBS:=$(CLANGLIBS) $(CLANGLIBS_35)
endif

ifeq ($(LLVMVERSION),3.4)
ifeq ($(shell uname -s),Linux)
LLVMLDFLAGS += -lLLVM-3.4
endif
CLANGLIBS:=$(CLANGLIBS) $(CLANGLIBS_34)
endif

CLANGLIBS:=$(foreach file, $(CLANGLIBS), $(llvm_prefix)/lib/$(file))

CXXFLAGS=`llvm-config --cxxflags` -ggdb -O0 -frtti -UNDEBUG -DDEBUG -I/usr/local/include -Wall -Wno-sign-compare -Wno-reorder
LDFLAGS=`llvm-config --ldflags --libs` $(LLVMLDFLAGS)
.PHONY: clean all install installsyntax

all: build wlc

clean:
	rm -rf build

wlc: $(OBJ)
	-ctags -R -o .tags
	g++ $(OBJ) $(CLANGLIBS) $(CXXFLAGS) $(LDFLAGS) -o wlc

build/%.o: src/%.cpp
	g++ $< -c $(CXXFLAGS) -o $@
	g++ $(CXXFLAGS) -MM -MT '$@' $< -MF build/$*.d

build:
	mkdir -p build

install: wlc
	sudo cp wlc /usr/local/bin/
	-cp wl.vim ~/.vim/syntax/
	sudo mkdir -p /usr/local/include/wl
	sudo cp -R lib/* /usr/local/include/wl

installsyntax:
	cp wl.vim ~/.vim/syntax/

-include $(DEP)
