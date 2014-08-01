//use "implicit_this"

extern undecorated int printf(char ^fmt, ...);

int main(int argc, char^^ argv)
{
    MyClass cl
    MySpecialClass scl

    cl = new MyClass
    scl = new MySpecialClass
    return 0
}

//MyClass weirdFunc(MyClass f) return f

//int vcall(MyClass myclass) return myclass.myFunction(5,6);

class MySpecialClass : MyClass
{
    char svar

    this() {
        this.svar = 22
        printf("myspecialclass constructor\n");
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
        printf("myclass constructor\n");
    }
}
