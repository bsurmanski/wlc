SRCFILES=main.cpp token.cpp lexer.cpp parser.cpp irCodegenContext.cpp identifier.cpp symbolTable.cpp ast.cpp irDebug.cpp message.cpp

SRC=$(foreach file, $(SRCFILES), src/$(file))

all:
	ctags -R -o .git/tags
	g++ $(SRC) `llvm-config --ldflags --cxxflags --libs` -lclang -ggdb -O0 -frtti -UNDEBUG -DDEBUG -o wlc

install: wlc
	sudo cp wlc /usr/local/bin/
	cp wl.vim ~/.vim/syntax/

installsyntax:
	cp wl.vim ~/.vim/syntax/
