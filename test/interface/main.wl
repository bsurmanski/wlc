//use "implicit_this"

extern undecorated int printf(char ^fmt, ...);

void callFunctions(SomeFace face) {
    face.func1()
    face.func2()
}

int main(int argc, char^^ argv)
{
    MyClass cl = new MyClass
    MySpecialClass scl = new MySpecialClass
    SomeFace face1 = cl
    SomeFace face2 = scl
    callFunctions(face1)
    callFunctions(face2)
    cl.func1()
    return 0
}

class MySpecialClass : MyClass
{
    void func1() {
        printf("MySpecialClass func1\n")
    }

    void func2() {
        printf("TODO: retrieve virtual base methods in interface cast")
    }
}

class MyClass
{
    void func1() {
        printf("MyClass func1\n")
    }

    void func2() {
        printf("MyClass func2\n")
    }
}

interface SomeFace {
    void func1();
    void func2();
}
