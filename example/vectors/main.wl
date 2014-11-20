undecorated int printf(char^ fmt, ...);

[int, int] add([int, int] a, [int, int] b) {
    [int, int] ret = [a[0] + b[0], a[1] + b[1]]
    return ret
}

void main(int arg, char^^ argv) {
    [int, int] a = [1, 2]
    [int, int] b = [2, 3]

    [int, int] c = add(a, b)

    int a1 = c[0]
    int a2 = c[1]
    printf("Add results: %d, %d\n", a1, a2)
}
