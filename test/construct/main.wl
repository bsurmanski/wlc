//use "implicit_this"

extern undecorated int printf(char ^fmt, ...);

int main(int argc, char^^ argv)
{
    MyClass cl
    MySpecialClass scl

    cl = new MyClass
    scl = new MySpecialClass
    scl = new MySpecialClass(5)
    return 0
}

//MyClass weirdFunc(MyClass f) return f

//int vcall(MyClass myclass) return myclass.myFunction(5,6);

class MySpecialClass : MyClass
{
    char svar

    this() {
        .svar = 22
        .a = 5
        .b = 10
        printf("myspecialclass constructor\n");
    }

    this(int i) {
        .svar = i
        printf("myspecialclass special constructor\n");
    }

    ~this() {
        printf("special class destructor\n");
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

    ~this() {
        printf("class destructor\n");
    }
}
