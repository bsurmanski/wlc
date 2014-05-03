class MyClass
{
    int a
    int b
}

class MySpecialClass : MyClass
{
    char svar

    int myFunction(int c, int b) {
        return c + b + svar
    }
}

int main(int argc, char^^ argv)
{
    MyClass cl
    MySpecialClass scl
    cl.a = 5
    scl.svar = 10
    scl.a = 2
    scl.b = 11
    return 0    
}
