all: obj
	ld test.o /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc -lm -lSDL -lGL -lGLEW -o program -dynamic-linker /lib64/ld-linux-x86-64.so.2

ctest.o:
	gcc test.c -c -o ctest.o

llvmir: 
	wlc main.wl

obj: llvmir 
	llc output.ll --filetype=obj -o test.o -mtriple="x86_64-unknown-linux" -O0

link:
	llc output.ll --filetype=obj -o test.o -mtriple="x86_64-unknown-linux" -O0
	ld test.o /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc -lm -lSDL -o program -dynamic-linker /lib64/ld-linux-x86-64.so.2

clean:
	rm -f test.ll 
	rm -f test.o 
	rm -f program
