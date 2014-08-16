extern undecorated int printf(char ^c, ...);

class MyClass {
    int i

    ~this() {
        printf("destructor\n");
    }
}

class MyExtendClass : MyClass {
    ~this() {
        printf("extend destructor\n");
    }
}

int main(int argc, char^^ argv) {
    int ^i = new int
    delete i

    MyClass cl = new MyClass
    release cl

    MyExtendClass ecl = new MyExtendClass
    retain ecl
    release ecl
    release ecl

    MyExtendClass ecl2 = new MyExtendClass

    int[] newarr = new int[5]
    printf("%x\n", newarr.ptr)
    delete newarr
}
