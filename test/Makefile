all: obj
	ld test.o /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtn.o -lc -lm -lSDL -o program -dynamic-linker /lib64/ld-linux-x86-64.so.2

llvmir: test.wl ../wlc
	../wlc test.wl 2> test.ll

obj: llvmir 
	llc test.ll --filetype=obj -o test.o -mtriple="x86_64-unknown-linux" -O0

clean:
	rm -f test.ll 
	rm -f test.o 
	rm -f program
