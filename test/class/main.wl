//use "implicit_this"

extern undecorated int printf(char ^fmt, ...);

int main(int argc, char^^ argv)
{
    MyClass cl
    MySpecialClass scl

    cl = new MyClass
    scl = new MySpecialClass

    printf("scl svar: %d\n", scl.svar);
    printf("cl a: %d\n", cl.a);
    cl.a = 5
    printf("cl a: %d\n", cl.a);
    scl.svar = 11
    scl.a = 2
    scl.b = 11
    scl.printCall()
    scl.printCall(5)
    SomeStruct s
    s.i = 5
    printf("struct %d\n", s.myfunc())
    return 0
}

//MyClass weirdFunc(MyClass f) return f

//int vcall(MyClass myclass) return myclass.myFunction(5,6);

class MySpecialClass : MyClass
{
    char svar

    void printCall() {
        printf("stuff\n");
    }

    void printCall(int i) {
        printf("printing magic %d\n", i);
    }

    int myFunction(int v1, int v2) {
        return v1 + v2 + .svar
    }

    this() {
        this.svar = 22
    }
}

class MyClass
{
    int a
    int b

    int myFunction(int a, int b) {
        return a + b
    }

    this() {
        .a = 0
        .b = 0
    }
}

struct SomeStruct {
    int i;
    int myfunc() return .i
}
