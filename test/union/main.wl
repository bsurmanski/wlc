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

    void^ ptr1 = &un.k
    void^ ptr2 = &un.l

    int^ intp1 = ptr1
    int^ intp2 = ptr2
    printf("%x == %x\n", ^intp1, ^intp2);
    return 0
}
