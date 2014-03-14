all:
	wlc test.wl -lSDL -o program

#all: obj
#	ld test.o /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc -lm -lSDL -o program -dynamic-linker /lib64/ld-linux-x86-64.so.2

ctest: obj ctest.o
	ld test.o ctest.o /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc -lm -lSDL -o program -dynamic-linker /lib64/ld-linux-x86-64.so.2

ctest.o:
	gcc test.c -c -o ctest.o

llvmir: test.wl ../wlc
	../wlc test.wl

obj: llvmir 
	llc output.ll --filetype=obj -o test.o -mtriple="x86_64-unknown-linux" -O0

link:
	llc output.ll --filetype=obj -o test.o -mtriple="x86_64-unknown-linux" -O0
	ld test.o /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc -lm -lSDL -o program -dynamic-linker /lib64/ld-linux-x86-64.so.2

clean:
	rm -f test.ll 
	rm -f test.o 
	rm -f program
