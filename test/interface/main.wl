//use "implicit_this"

extern undecorated int printf(char ^fmt, ...);


int main(int argc, char^^ argv)
{
    MyClass cl = new MyClass
    MySpecialClass scl = new MySpecialClass
    SomeFace face = cl
    cl.a = 5
    scl.svar = 11
    scl.a = 2
    scl.b = 11
    scl.printCall();
    int i = vcall(scl)
    int j = vcall(cl)
    printf("out %d\n", i);
    printf("out %d\n", j);
    cl.weirdFunc()
    someStruct s
    s.myfunc()
    return 0
}

MyClass weirdFunc(MyClass f) return f

int vcall(MyClass myclass) return myclass.myFunction(5,6);

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

struct someStruct {
    int i;
    int myfunc() return .i
}

interface SomeFace {
    void myFunc();
}
