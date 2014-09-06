//use "implicit_this"

extern undecorated int printf(char ^fmt, ...);

int main(int argc, char^^ argv)
{
    MyStruct st = MyStruct()
    printf("st %d %d\n", st.a, st.b)

    int i = st.myFunction(5,5)
    printf("%d\n", i)
    return 0
}

struct MyStruct
{
    int a
    int b

    int myFunction(int a, int b) {
        return a + b
    }

    this() {
        .a = 1
        .b = 2
    }
}
