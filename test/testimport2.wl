import "testimport.wl"

int printf(char^ fmt, ...);

void myfunc()
{
    printf("myfunc! %d\n", myimportedint)
    myimportedint = 22
    printf("myfunc! %d\n", myimportedint)
}
