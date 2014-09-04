extern undecorated int printf(char^ fmt, ...);

class MyClass {
    int i

    this() {
        .i = 5
    }
}

int main(int argc, char^^ argv)
{
    weak MyClass cl
    if(!cl or cl.i) {
        printf("true branch\n")
    } else {
        printf("false branch\n")
    }
    return 0
}
