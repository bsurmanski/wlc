import(C) "/usr/include/GL/gl.h"
import(C) "/usr/include/GL/glext.h"
import(C) "/usr/include/SDL/SDL.h"
import(C) "/usr/include/SDL/SDL_keysym.h"

float sin(float f);

float tick = 0;
int gltick;
int tex;
void init()
{
    SDL_Surface^ surf = SDL_LoadBMP_RW(SDL_RWFromFile("apollo-2pow.bmp", "rb"), 1);

    if(!surf)
    {
        printf("ERROR LOADING IMG\n");    
    }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, surf.w, surf.h, 
        0, GL_RGB, GL_UNSIGNED_BYTE, surf.pixels);
    SDL_FreeSurface(surf);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glViewport(0,0,512,256);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,512,256,0,0-1,1);
    initShaders();
}

char^ fileToString(char^ filenm)
{
    void^ file = fopen(filenm, "r");
    fseek(file, 0, SEEK_END);
    int filesz = ftell(file);
    fseek(file, 0, SEEK_SET);
    char^ str = malloc(filesz + 1);
    str[filesz] = 0;
    fread(str, filesz, 1, file);
    return str;
}

void initShaders()
{
    char^ vshader = fileToString("shader.frag"); 
    char^ fshader = fileToString("shader.vert"); 
    
    if(!vshader or !fshader)
    {
        printf("ERROR LOADING SHADERS\n");    
    }

    int vs = glCreateShader(GL_VERTEX_SHADER);
    int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vs, 1, &vshader, 0);
    glShaderSource(fs, 1, &fshader, 0);
    glCompileShader(vs);
    glCompileShader(fs);
    int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glUseProgram(prog);
    gltick = glGetUniformLocation(prog, "tick");
}

void draw()
{
    glUniform1f(gltick, sin(tick) / 4);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
    glTexCoord2i(0,0);
    glVertex2i(0,0);

    glTexCoord2i(1,0);
    glVertex2i(512,0);

    glTexCoord2i(1,1);
    glVertex2i(512,256);

    glTexCoord2i(0,1);
    glVertex2i(0,256);

    SDL_GL_SwapBuffers();
}

int main(int argc, char^^ argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_Surface^ surf = SDL_SetVideoMode(512, 256, 32, SDL_OPENGL);
    bool spc = false;

    init();
    while(!spc)
    {
        SDL_PumpEvents();
        int8^ keystate = SDL_GetKeyState(0);
        spc = keystate[SDLK_SPACE];
        draw();
        SDL_Delay(32);
        tick=tick+0.1f;
    }
}
