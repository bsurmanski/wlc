int printf(char^ fmt, ...);

int main(int argc, char^^ argv)
{
    int a = 5
    int b = 6
    [a, b] = [1, 2]
    printf("A: %d B: %d\n", a, b);
    return 0
}
