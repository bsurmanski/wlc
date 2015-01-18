
int[] negatives = [1, 2, -3, 4]

undecorated int printf(char^ fmt, ...);

int main(int argc, char^^ argv) {
    for(int i = 0; i < negatives.size; i++) {
        printf("%d ", negatives[i])
    }
    printf("\n")
    return 0
}
