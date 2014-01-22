SRCFILES=main.cpp token.cpp lexer.cpp parser.cpp irCodegenContext.cpp identifier.cpp symbolTable.cpp ast.cpp irDebug.cpp message.cpp

SRC=$(foreach file, $(SRCFILES), src/$(file))

all:
	g++ $(SRC) `llvm-config --ldflags --cxxflags --libs` -ggdb -O0 -frtti -UNDEBUG -DDEBUG -o wlc

wlc: $(SRC)
	g++ $(SRC) `llvm-config --ldflags --cxxflags --libs` -ggdb -O0 -frtti -UNDEBUG -DDEBUG -o wlc

install: wlc
	sudo cp wlc /usr/local/bin/
	cp wl.vim ~/.vim/syntax/

