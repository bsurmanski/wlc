class MyClass {
    int i
}

int main(int argc, char^^ argv) {
    int ^i = new int
    delete i

    MyClass cl = new MyClass
    delete cl

    int[] newarr = new int[5]
    delete newarr
}
