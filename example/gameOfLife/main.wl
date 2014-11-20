//package test

use "importc"

import(C) "SDL/SDL.h"
import(C) "stdlib.h"
import(C) "stdio.h"
import(C) "string.h"

SDL_Surface^ surf = null
SDL_Surface^ back = null
void^ testval

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
    SDL_WM_SetCaption(title.ptr, null)

    int8 spc = 0
    while(!spc) 
    {
        SDL_PumpEvents();
        int8^ keystate = SDL_GetKeyState(null)
        spc = keystate[SDLK_SPACE]
        SDL_Delay(32)
        SDL_Flip(surf)
		
        update()
    }

    return 0
}
