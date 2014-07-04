//use "implicit_this"

extern nomangle int printf(char ^fmt, ...);


int main(int argc, char^^ argv)
{
    MyClass cl
    MySpecialClass scl
    cl.a = 5
    scl.svar = 11
    scl.a = 2
    scl.b = 11
    scl.printCall();
    int i = vcall(&scl)
    int j = vcall(&cl)
    printf("out %d\n", i);
    printf("out %d\n", j);
    cl.weirdFunc()
    return 0
}

MyClass^ weirdFunc(MyClass^ f) return f

int vcall(MyClass^ myclass) return myclass.myFunction(5,6);

class MySpecialClass : MyClass
{
    char svar

    void printCall() {
        printf("stuff\n");
    }

    int myFunction(int v1, int v2) {
        return v1 + v2 + .svar
    }
}

class MyClass
{
    int a
    int b

    int myFunction(int a, int b) {
        return a + b
    }
}
