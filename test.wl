package test

int printf(char^ str, ...);

long ^malloc(int sz);

import "sdl.wl"

SDL_Surface^ surf = null

void update()
{
    int i
    int j
    for(i = 0; i < 320; ++i)
    {
        for(j = 0; j < 240; j++)
        {
            surf.pixels[i * surf.format.BytesPerPixel + j * surf.pitch] = 255
        }
    }
}

int main(int argc, char^^ argv)
{
    surf = SDL_SetVideoMode(320, 240, 0, SDL_SWSURFACE)
    char^ title = "Life"
    SDL_WM_SetCaption(title, null)
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

