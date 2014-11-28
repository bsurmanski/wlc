undecorated int printf(char^ fmt, ...);

[int, int] add([int, int] a, [int, int] b) {
    return [a[0] + b[0], a[1] + b[1]]
}

void main(int arg, char^^ argv) {
    [int, int] a = [1, 2]
    [int, int] b = [2, 3]

    var c = add(a, b)

    printf("Add results: %d, %d\n", c[0], c[1])
}
