package test

import test2
import thing.module

int printf(char^ str, ...);
long malloc(long sz);

char^ myString = "test1"

long iiii = 1

struct mystruct
{
    int i
    int j
}

int main()
{
    char^ mystr = "this is a string!"
    myfunc(50, mystr)
    return 0
}

void myfunc(int i, char^ somestr)
{
    long sz = 8
    while(sz > 5)
    {
        printf("Hello World! %d %s\n", sz, somestr)
        sz = sz - 1
        if(sz == 6) return
    }
}

//char thangyglobal;
