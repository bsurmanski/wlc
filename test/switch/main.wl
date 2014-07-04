use "importc"

import(C) "/usr/include/stdio.h"

int main(int argc, char^^ argv)
{
    switch(argc)
    {
        printf("default here!\n")
        case 1
            int i = argc
            printf("1 argument passed! %d\n", i)
        case 2,3,4
            printf("2 argument passed!\n")
    }
    return 0
}
