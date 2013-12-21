int SDL_Init(int flags);
int SDL_InitSubSystem(int flags);
void SDL_QuitSubSystem(int flags);
int SDL_WasInit(int flags);
void SDL_Quit();

// SDL_Video

void SDL_WM_SetCaption(char^ title, char^ icon);

struct SDL_Rect
{
    int16 x
    int16 y
    uint16 w
    uint16 h
}

struct SDL_Color
{
    int8 r
    int8 g
    int8 b
    int8 unused
}

struct SDL_Palette
{
    int ncolors
    SDL_Color^ colors
}

struct SDL_PixelFormat
{
    SDL_Palette^ palette    
    uint8 BitsPerPixel
    uint8 BytesPerPixel
    uint8 Rloss
    uint8 Gloss
    uint8 Bloss
    uint8 Aloss
    uint8 Rshift
    uint8 Gshift
    uint8 Bshift
    uint8 Ashift
    uint32 Rmask
    uint32 Gmask
    uint32 Bmask
    uint32 Amask
}

//struct private_hwdata;
//struct SDL_BlitMap;
struct SDL_Surface
{
    int flags
    SDL_PixelFormat^ format
    int w
    int h
    int16 pitch
    void^ pixels
    int offset
    //private_hwdata^ hwdata
    void^ hwdata
    SDL_Rect clip_rect
    int unused1
    int locked
    //SDL_BlitMap^ map
    void^ map
    uint format_version
    int refcount
}

int SDL_SWSURFACE = 0

SDL_Surface^ SDL_SetVideoMode(int w, int h, int bpp, int flags);
int SDL_Flip(SDL_Surface^ surf);

// SDL_timer.h

void SDL_Delay(int32 ms);

// SDL_keyboard.h
int SDL_SPACE = 32
struct SDL_keysym
{
    int8 scancode
    int sym
    int mod
    uint16 unicode
}

uint8^ SDL_GetKeyState(int^ numkeys);

// SDL_events.h

void SDL_PumpEvents();
