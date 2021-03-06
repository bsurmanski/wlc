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

void func() {
    MyClass cl = new MyClass
    printf("\nautorelease below?\n")
}

void func2() {
    MyClass cl = new MyClass
    retain cl
    printf("\nno autorelease below\n")
}

void func3(MyClass param) {
    printf("param passed!\n");
}

int main(int argc, char^^ argv) {
    int ^i = new int
    delete i

    MyClass cl = new MyClass
    printf("refcount: %d\n", cl.refcount)
    func3(cl);
    printf("refcount: %d\n", cl.refcount)
    printf("destructor below\n");
    release cl
    printf("cl: %x\n", cl)

    MyExtendClass ecl = new MyExtendClass
    printf("extended destructor below\n");
    retain ecl
    release ecl
    release ecl

    MyExtendClass ecl2 = new MyExtendClass

    func()
    printf("autorelease above\n\n")

    func2()
    printf("no autorelease above\n\n")


    int[] newarr = new int[5]
    printf("%x\n", newarr.ptr)
    delete newarr

    printf("autorelease below\n")
}
