class MyClass {
    int i
}

extern undecorated int printf(char ^c, ...);

int main(int argc, char^^ argv) {
    int ^i = new int
    delete i

    MyClass cl = new MyClass
    delete cl

    int[] newarr = new int[5]
    printf("%x\n", newarr.ptr)
    delete newarr
}
