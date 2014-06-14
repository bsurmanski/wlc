extern nomangle int printf(char ^fmt, ...);

int main(int argc, char^^ argv)
{
    MyClass cl
    MySpecialClass scl
    cl.a = 5
    scl.svar = 10
    scl.a = 2
    scl.b = 11
    printf("out %d\n", scl.myFunction(1,2));
    cl.weirdFunc()
    return 0    
}

MyClass^ weirdFunc(MyClass^ f) return f


class MySpecialClass : MyClass
{
    char svar

    int myFunction(int c, int b) {
        return c + b + this.svar
    }
}

class MyClass
{
    int a
    int b
}
