undecorated int printf(char ^fmt, ...);

class MyClass {
    int i
    void printCall() printf("PRINT\n")

    ~this() {
        printf("DELETING\n");
    }
}

int main(int argc, char^^ argv) {
    new MyClass().printCall()
}
