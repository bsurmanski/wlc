SRC=main.cpp token.cpp lexer.cpp parser.cpp irCodegenContext.cpp identifier.cpp symbolTable.cpp ast.cpp irDebug.cpp

all:
	g++ $(SRC) `llvm-config --ldflags --cxxflags --libs` -ggdb -O0 -frtti -UNDEBUG -DDEBUG -o wlc

wlc: $(SRC)
	g++ $(SRC) `llvm-config --ldflags --cxxflags --libs` -ggdb -O0 -frtti -UNDEBUG -DDEBUG -o wlc

llvmir: test.wl wlc
	./wlc test.wl 2> test.ll

obj: llvmir 
	llc test.ll --filetype=obj -o test.o -mtriple="x86_64-unknown-linux" -O0

program: obj
	ld test.o /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc -lm -lSDL -o program -dynamic-linker /lib64/ld-linux-x86-64.so.2

clean:
	rm -f test.ll 
	rm -f test.o 
	rm -f program
