package test

int printf(char^ str, ...);

long malloc(long sz);
import "testimport.wl"
import "testimport2.wl"

int someconstant = 5;

int main(int argc, char^^ argv)
{
    printf("ARGV0: %s\n", ^argv)
    MyStruct st
    MyStruct^ stptr = &st
    for(int i = 0; i < 10; i++)
    {
        printf("WHOA %d\n", i)
        continue
        printf("uhoh")
    }
    myfunc()
    return 0
}

