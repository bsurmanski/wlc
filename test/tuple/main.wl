extern undecorated int printf(char^ fmt, ...);

[int, int] func() return [1, 2]

void tupleref([int, int]^ val) {
    (^val)[0] = 5
}

int main(int argc, char^^ argv)
{
    int a = 5
    int b = 6
    [a, b] = func()
    [int, int] c = func()
    tupleref(&c)
    printf("A: %d B: %d\n", a, b)
    printf("C1: %d C2: %d\n", c[0], c[1])
    return 0
}
