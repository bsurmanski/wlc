int printf(char ^fmt, ...);

int main(int argc, char^^ argv)
{
    switch(argc)
    {
        case 1
            printf("1 argument passed!\n");
            break
        case 2
            printf("2 argument passed!\n");
            break
    }
    return 0
}
