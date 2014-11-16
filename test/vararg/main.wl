undecorated int printf(char^ fmt, ...);

int main(int argc, char^^ argv) {
    printf("test a string: %s\n", "a string!")
    printf("test some numbers: %d %d %d\n", 1, 2, 3)

    char a = 4
    short b = 5
    long c = 6
    float f = 0.6f
    printf("test promotion: %d %d %lld %f\n", a, b, c, f)
    return 0
}
