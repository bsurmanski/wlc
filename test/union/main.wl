extern undecorated int printf(char ^fmt, ...);

union MyUnion {
    int i
    int j
    long k
    double l
}

int main(int argc, char^^ argv)
{
    MyUnion un
    un.i = 5
    printf("%d == %d\n", un.i, un.j);
    un.k = 10
    printf("%d == %d\n", un.k, un.i);
    printf("%x == %x\n", un.k, un.l);
    return 0
}
