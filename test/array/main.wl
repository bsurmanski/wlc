/*
use "importc"

import(C) "/usr/include/SDL/SDL.h"
import(C) "/usr/include/stdlib.h"
import(C) "/usr/include/stdio.h"
import(C) "/usr/include/math.h"
*/

extern undecorated int printf(char^ fmt, ...);
extern undecorated float sinf(float f);

int main(int argc, char^^ argv)
{
    int[2] i
    i[0] = 11;
    i[1] = 55;
    double x = sinf(3.14);
    printf("%x\n", i.ptr);
    printf("%d\n", i.size);
    printf("%d\n", i[0]);
    printf("%d\n", i[1]);
    printf("%f\n", x);
    return 0
}
