undecorated int printf(char^ fmt, ...);

class MyClass {
    int i

    this() {
        .i = 5
    }
}

void doStuff(MyClass cl) {
    printf("ufcs\n")
}

int main(int argc, char^^ argv) {
    MyClass cl = MyClass()

    cl.doStuff()
    return 0
}
