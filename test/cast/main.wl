/*
use "importc"

import(C) "/usr/include/SDL/SDL.h"
import(C) "/usr/include/stdlib.h"
import(C) "/usr/include/stdio.h"
import(C) "/usr/include/math.h"
*/

extern undecorated int printf(char^ str, ...);

int main(int argc, char^^ argv)
{
    int i = 5
    int j = 10
    float f = float: i * float: j
    printf("%f\n", f)
}
