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
    if(!cl or cl.i and true) {
        printf("true branch\n")
    } else {
        printf("false branch\n")
    }

    if(cl and cl.i) {
        printf("cli %d\n", cl.i)
    } else {
        printf("cl or i does not exist\n");
    }
    return 0
}
