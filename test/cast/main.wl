extern undecorated int printf(char^ str, ...);

int main(int argc, char^^ argv)
{
    int i = 5
    int j = 10
    float f = float: i * float: j
    printf("%f\n", f)
}
