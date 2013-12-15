SRC=main.cpp token.cpp lexer.cpp parser.cpp irCodegenContext.cpp identifier.cpp symbolTable.cpp ast.cpp

all:
	g++ $(SRC) `llvm-config --ldflags --cxxflags --libs` -ggdb -O0 -frtti -UNDEBUG -DDEBUG -o wlc

wlc: $(SRC)
	g++ $(SRC) `llvm-config --ldflags --cxxflags --libs` -ggdb -O0 -frtti -UNDEBUG -DDEBUG -o wlc

llvmir: test.wl wlc
	./wlc 2> llvmir

obj: llvmir 
	llc llvmir --filetype=obj -o obj -mtriple="x86_64-unknown-linux"

program: obj
	ld obj /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc -lm -o program -dynamic-linker /lib64/ld-linux-x86-64.so.2

clean:
	rm -f llvmir 
	rm -f obj
	rm -f program
