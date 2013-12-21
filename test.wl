package test

int printf(char^ str, ...);

long ^malloc(int sz);

import "sdl.wl"

SDL_Surface^ surf

void update()
{
    int i
    int j
    surf.pixels[1] = 1
    $
    for(i = 0; i < 320; ++i)
    {
        //for(j = 0; j < 240; j++)
        //{
            //surf.pixels[1] = 1 //i * surf.format.BytesPerPixel + j * s.pitch] = 1
        //}
    }
}

int main(int argc, char^^ argv)
{
    surf = SDL_SetVideoMode(320, 240, 0, SDL_SWSURFACE)
    char^ title = "Life"
    SDL_WM_SetCaption(title, 0)
    int8 spc = 0
    while(!spc)
    {
        SDL_PumpEvents();
        int8^ keystate = SDL_GetKeyState(0)
        spc = keystate[SDL_SPACE]
        SDL_Delay(32)
        update()
    }
    return 0
}

