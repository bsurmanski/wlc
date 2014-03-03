import(C) "/usr/include/GL/gl.h"
import(C) "/usr/include/GL/glext.h"
import(C) "/usr/include/SDL/SDL.h"
import(C) "/usr/include/SDL/SDL_keysym.h"

int tex;
void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);    
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    SDL_Surface^ surf = SDL_LoadBMP_RW(SDL_RWFromFile("apollo-2pow.bmp", "rb"), 1);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surf.w, surf.h, 
        0, GL_RGBA, GL_UNSIGNED_BYTE, surf.pixels);
}

int main(int argc, char^^ argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_Surface^ surf = SDL_SetVideoMode(320, 240, 32, SDL_OPENGL);
    bool spc = false;
    while(!spc)
    {
        SDL_PumpEvents();
        int8^ keystate = SDL_GetKeyState(0);
        spc = keystate[SDLK_SPACE];
        SDL_Delay(32);
        SDL_Flip(surf);
    }
}
