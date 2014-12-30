extern undecorated int printf(char^ str, ...);

class MyClass {
    int i
}

class MySubClass : MyClass {
}

int main(int argc, char^^ argv)
{
    int i = 5
    int j = 10
    float f = float: i * float: j
    f += 0.5f
    MyClass cl
    MySubClass msc = MySubClass: cl
    printf("%f\n", f)
    printf("%d\n", int: f)
}
