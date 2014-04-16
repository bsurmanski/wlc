int printf(char ^fmt, ...);

int main(int argc, char^^ argv)
{
    switch(argc)
    {
        printf("default here!\n")
        case 1
            printf("1 argument passed!\n")
        case 2,3,4
            printf("2 argument passed!\n")
    }
    return 0
}
