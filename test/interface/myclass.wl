extern undecorated int printf(char ^fmt, ...);

class MySpecialClass : MyClass
{
    void func1() {
        printf("MySpecialClass func1\n")
    }
}

class MyClass
{
    void func1() {
        printf("MyClass func1\n")
    }

    void func2(int i) {
        printf("magic number %d\n", i)
    }

    void func2() {
        printf("MyClass func2\n")
    }

}
