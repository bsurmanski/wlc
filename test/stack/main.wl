extern undecorated int printf(char^ str, ...);

class MyClass {
    int i

    this() { 
        .i = 5
    }

    void func() {
        printf("%d!", .i)
    }
}

int main(int argc, char^^ argv) {
    //MyClass cl = MyClass()

    (MyClass()).func()

    //printf("%d\n", cl.i)
}
