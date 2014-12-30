//use "implicit_this"

extern undecorated int printf(char ^fmt, ...);


int main(int argc, char^^ argv)
{
    MyClass cl = new MyClass
    MySpecialClass scl = new MySpecialClass
    SomeFace face1 = cl
    SomeFace face2 = scl
    face1.func1()
    face1.func2()
    face2.func1()
    face2.func2()
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
