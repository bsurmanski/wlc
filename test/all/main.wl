//package test

use "importc"

import(C) "SDL/SDL.h"
import(C) "stdlib.h"
import(C) "stdio.h"
import(C) "string.h"

SDL_Surface^ surf = null
SDL_Surface^ back = null
void^ testval

int[] constarray = [1,2,3]

struct SomeStruct
{
    int i
    int j
}

union SomeUnion
{
    int myInt
    char myChar
    long myLong
}

bool pixelIsWhite(SDL_Surface^ s, int i, int j)
{
    int^ pxl = &s.pixels[i * s.format.BytesPerPixel + j * s.pitch]
    return ^pxl
}

void setPixel(SDL_Surface^ s, int i, int j, int val) 
{
    int^ pxl = &s.pixels[i * s.format.BytesPerPixel + j * s.pitch]
    ^pxl = val
}

int addFunc(int i = 0, int j = 5)
{
    return i + j 
}

void update()
{
    int n = 0 // neighbors
    int j
    int i
    for(j = 1; j < 239; j++)
    {
        for(i = 1; i < 319; ++i)
        {
            n = 0
            if (pixelIsWhite(surf, i-1, j-1)) n++
            if (pixelIsWhite(surf, i,   j-1)) n++
            if (pixelIsWhite(surf, i+1, j-1)) n++
            if (pixelIsWhite(surf, i-1, j))   n++
            if (pixelIsWhite(surf, i+1, j))   n++
            if (pixelIsWhite(surf, i-1, j+1)) n++
            if (pixelIsWhite(surf, i,   j+1)) n+=1
            if (pixelIsWhite(surf, i+1, j+1)) n+=1

            if(n < 2) setPixel(back, i, j, 0)
            if(n == 3) setPixel(back, i, j, 255)
            if(n > 3) setPixel(back, i, j, 0)
        }
    }
	
    memcpy(surf.pixels, back.pixels, 240 * surf.pitch)
}

void randomize(SDL_Surface^ s)
{
    for(int j = 0; j < 240; j++)
    {
        for(int i = 0; i < 320; ++i)
        {
            int set
            setPixel(s, i, j, rand())
        }
    }
    return
}

void^ somefunc()
{
    return null
    return testval
}

[int, int] myTupleFunc()
{
    return [int: 124, int: 55]
}

int[5] statArr = [1,2,3,4,5]
int main(int argc, char^^ argv)
{
    surf = SDL_SetVideoMode(320, 240, 0, SDL_SWSURFACE)
    back = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, surf.format.BitsPerPixel, 
                                surf.format.Rmask, surf.format.Gmask, surf.format.Bmask, 
                                surf.format.Amask)
    srand(100)
    randomize(back)
    randomize(surf)

    var title = "Life"
    SDL_WM_SetCaption(title, null)


    var tuple = [1, 2, 3]
    MyNewStruct st = [1, 2, 3, null]

    [int, int] intTuple = myTupleFunc()

    float myfloat = 0x1234
    printf("myFloat: %f, my tuple: %d, %d\n", myfloat, intTuple[0], intTuple[1])
    ++myfloat
    printf("myFloat2: %f\n", myfloat)
    myfloat = 0b1010
    printf("myFloatb: %f\n", myfloat)
    myfloat = 0o333
    printf("myFloato: %f\n", myfloat)

    int add = addFunc(5, 7)
    printf("addFunc: %d\n", addFunc(1,2))
    printf("addFunc: %d\n", addFunc(5,2))
    printf("addFunc: %d\n", addFunc(5, 7))

    uint mySize = SDL_Surface.sizeof

    MyNewStruct newstruct
    printf("class functions? %d\n", newstruct.getNum());

    SomeUnion myUnion
    myUnion.myLong = 4294967298 >> 10
    printf("union: %d %d %lld\n", myUnion.myChar, myUnion.myInt, myUnion.myLong);
    int[5] my5Array = [1,2,3,4,5]
    int[] darr = [1,2,3,4]

    printf("sizeof array's: %d %d\n", int[5].sizeof, int[].sizeof);
    printf("seek cur is: %f\n", SEEK_CUR);

    printf("myArray: elem: sz: %d, %d %d %d %d\n",
    darr.size,
    darr[0],
    darr[1],
    darr[2],
    darr[3],
    );

    int[] newarr = new int[5]
    //delete newarr
    int^ mymalloc = new int
    delete mymalloc

    void function() testFunc = update

    int8 spc = 0
    while(!spc) 
    {
        SDL_PumpEvents();
        uint8^ keystate = SDL_GetKeyState(null)
        spc = keystate[SDLK_SPACE]
        SDL_Delay(32)
        SDL_Flip(surf)
		
        testFunc()
        update()
    }

    return 0
}

struct MyNewStruct
{
    int a
    int b
    int c
    MyNewStruct ^st

    int getNum() return 5
}
