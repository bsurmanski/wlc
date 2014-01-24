package test

void ^memcpy(void^ dest, void^ src, long n);

int RAND_MAX = 2147483647

import "sdl.wl"
import "cstdlib.wl"

SDL_Surface^ surf = null
SDL_Surface^ back = null
void^ testval = null

bool pixelIsWhite(SDL_Surface^ s, int i, int j)
{
    bool ret = s.pixels[i * s.format.BytesPerPixel + j * s.pitch]
    return bool: ret
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
    char^ title = "Life"
    SDL_WM_SetCaption(title, null)

    int iii = 5
    float j = float: iii
    //MyNewStruct newstruct

    int8 spc = 0
    while(!spc) 
    {
        SDL_PumpEvents();
        int8^ keystate = SDL_GetKeyState(0)
        spc = keystate[SDL_SPACE]
        SDL_Delay(32)
        SDL_Flip(surf)
        update()
    }

    return 0
}

struct MyNewStruct
{
    int iiiiids    
}
