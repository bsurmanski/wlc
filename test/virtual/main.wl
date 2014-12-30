undecorated int printf(char^ fmt, ...);

class BaseClass {
    void print() {
        printf("baseClass\n")
    }
}

class DerivedClass : BaseClass {
    void print() {
        printf("derivedClass\n")
    }
}

void doPrint(BaseClass cl) {
    cl.print()
}

int main(int argc, char^^ argv) {
    BaseClass cl = new BaseClass
    DerivedClass dcl = new DerivedClass
    doPrint(cl)
    doPrint(dcl)
    return 0
}
