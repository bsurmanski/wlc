extern undecorated int printf(char ^c, ...);

class MyClass {
    int i

    ~this() {
        printf("destructor\n");
    }
}

class MyExtendClass : MyClass {
    this() {
    }

    ~this() {
        printf("extend destructor\n");
    }
}

int main(int argc, char^^ argv) {
    int ^i = new int
    delete i

    MyClass cl = new MyClass
    delete cl

    MyExtendClass ecl = new MyExtendClass
    delete ecl

    int[] newarr = new int[5]
    //printf("%x\n", newarr.ptr)
    delete newarr
}
