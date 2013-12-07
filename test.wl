package test

int printf(char^ str, ...);
long malloc(long sz);

import "testimport.wl"


int main()
{
    MyStruct st;
    for(int i = 0; i < 10; i++)
    {
        printf("WHOA %d\n", i)
        continue
        printf("uhoh")
    }
    return 0

}
