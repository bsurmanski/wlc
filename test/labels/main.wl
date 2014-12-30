undecorated int printf(char^ fmt, ...);

int main(int argc, char^^ argv){

    int i = 0

    goto mylabel
    printf("this should not print\n")
    label mylabel
    printf("%d\n", i)
    i++
    if(i < 3) goto mylabel

    return 0
}
