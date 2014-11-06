undecorated int printf(char ^fmt, ...);

class MyClass {
    int i
    void printCall() printf("PRINT\n")
    this() .i = 0
    ~this() printf("DELETING\n")
}

int main(int argc, char^^ argv) {
    //new MyClass().printCall()
    //MyClass().printCall()
    MyClass cl = MyClass()
    cl.printCall()
}
