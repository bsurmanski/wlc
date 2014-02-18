package test

void ^memcpy(void^ dest, void^ src, long n);

int RAND_MAX = 2147483647

import "sdl.wl"
import "cstdlib.wl"
//import "cstdio.wl"
import(C) "/usr/include/stdio.h"

SDL_Surface^ surf = null
SDL_Surface^ back = null
void^ testval


struct SomeStruct
{
    int i
    int j
}

bool pixelIsWhite(SDL_Surface^ s, int i, int j)
{
    bool ret = s.pixels[i * s.format.BytesPerPixel + j * s.pitch]
    return ret
}

void setPixel(SDL_Surface^ s, int i, int j, int val) 
{
    s.pixels[i * s.format.BytesPerPixel + j * s.pitch] = val
}

void update()
{
    int n = 0 // neighbors
    for(int j = 1; j < 240; j++)
    {
        for(int i = 1; i < 320; ++i)
        {
            n = 0
            if (pixelIsWhite(surf, i-1, j-1)) n++
            if (pixelIsWhite(surf, i,   j-1)) n++
            if (pixelIsWhite(surf, i+1, j-1)) n++
            if (pixelIsWhite(surf, i-1, j))   n++
            if (pixelIsWhite(surf, i+1, j))   n++
            if (pixelIsWhite(surf, i-1, j+1)) n++
            if (pixelIsWhite(surf, i,   j+1)) n++
            if (pixelIsWhite(surf, i+1, j+1)) n++

            if(n < 2) setPixel(back, i, j, 0)
            if(n == 3) setPixel(back, i, j, 255)
            if(n > 3) setPixel(back, i, j, 0)
        }
    }
    memcpy(surf.pixels, back.pixels, 240 * surf.pitch)
}

void randomize(SDL_Surface ^s)
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
    return [124, 55]    
}

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
    MyNewStruct st = [1, 2, 3]

    [int, int] intTuple = myTupleFunc()

    float myfloat = 5.2
    printf("myFloat: %f, my tuple: %d\n", myfloat, intTuple[0])

    uint mySize = SDL_Surface.sizeof

    int iii = 5
    float j = float: iii
    MyNewStruct newstruct
    int[5] my5Array = [1,2,3,4,5]
    int[] darr = [1,2,3,4]

    printf("sizeof array's: %d %d\n", int[5].sizeof, int[].sizeof);

    printf("myArray: elem: sz: %d, %d %d %d %d %d\n",
    darr.size,
    darr[0],
    darr[1],
    darr[2],
    darr[3],
    darr[4],
    );

    int[] newarr = new int[5]
    delete newarr
    int^ mymalloc = new int
    delete mymalloc

    int8 spc = 0
    while(!spc) 
    {
        SDL_PumpEvents();
        int8^ keystate = SDL_GetKeyState(0)
        spc = keystate[SDLK_SPACE]
        SDL_Delay(32)
        SDL_Flip(surf)
        update()
    }

    return 0
}

struct MyNewStruct
{
    int a
    int b
    int c
}
