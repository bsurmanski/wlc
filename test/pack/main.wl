
char[] mystr = pack "hello.txt"

undecorated int printf(char^ fmt, ...);
int main(int argc, char^^ argv) {
    printf(mystr);
}
