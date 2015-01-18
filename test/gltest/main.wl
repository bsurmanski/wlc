use "importc"

import(C) "/usr/include/GL/gl.h"
import(C) "/usr/include/GL/glext.h"
import(C) "/usr/include/SDL/SDL.h"
import(C) "/usr/include/SDL/SDL_keysym.h"

extern undecorated double sin(double f);

float tick = 0
uint gltick
uint tex


void init()
{
    SDL_Init(SDL_INIT_VIDEO)
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8)
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8)
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8)
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16)
    SDL_Surface^ surf = SDL_SetVideoMode(512, 256, 32, SDL_OPENGL)
    SDL_Surface^ img = SDL_LoadBMP_RW(SDL_RWFromFile("apollo-2pow.bmp", "rb"), 1)

    if(!img)
    {
        printf("ERROR LOADING IMG\n")    
    }

    glGenTextures(1, &tex)
    glBindTexture(GL_TEXTURE_2D, tex)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img.w, img.h,
        0, GL_RGB, GL_UNSIGNED_BYTE, img.pixels)
    SDL_FreeSurface(img)

    initShaders()
    glDisable(GL_DEPTH_TEST)
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D)
    glDisable(GL_LIGHTING)
    glViewport(0,0,512,256)
    glClear(GL_COLOR_BUFFER_BIT)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glOrtho(0,512,256,0,-1,1)
}

void initShaders()
{
    char^ vshader = pack "shader.vert"
    char^ fshader = pack "shader.frag"
    
    uint vs = glCreateShader(GL_VERTEX_SHADER)
    uint fs = glCreateShader(GL_FRAGMENT_SHADER)
    glShaderSource(vs, 1, &vshader, null)
    glShaderSource(fs, 1, &fshader, null)
    glCompileShader(vs)
    glCompileShader(fs)

    char^ TEST = malloc(512);

    int err
    glGetShaderiv(vs, GL_COMPILE_STATUS, &err)
    if(err != GL_TRUE)
    {
        glGetShaderInfoLog(vs, 511, null, TEST)
        printf("VS ERR: %s\n", TEST)    
    }

    glGetShaderiv(fs, GL_COMPILE_STATUS, &err)
    if(err != GL_TRUE)
    {
        printf("FS ERR\n")    
    }

    uint prog = glCreateProgram()
    glAttachShader(prog, vs)
    glAttachShader(prog, fs)
    glLinkProgram(prog)
    glUseProgram(prog)
    gltick = glGetUniformLocation(prog, "tick")

}

void draw()
{
    glUniform1f(gltick, sin(tick) / 4)
    glBindTexture(GL_TEXTURE_2D, tex)
    glBegin(GL_QUADS)
    glTexCoord2i(0,0)
    glVertex2i(0,0)

    glTexCoord2i(1,0)
    glVertex2i(512,0)

    glTexCoord2i(1,1)
    glVertex2i(512,256)

    glTexCoord2i(0,1)
    glVertex2i(0,256)
    glEnd();

    SDL_GL_SwapBuffers()
}

int main(int argc, char^^ argv)
{
    bool spc = false

    init()
    while(!spc)
    {
        SDL_PumpEvents()
        uint8^ keystate = SDL_GetKeyState(null)
        spc = keystate[SDLK_SPACE]
        draw()
        SDL_Delay(32)
        tick=tick+0.1f
    }
}
