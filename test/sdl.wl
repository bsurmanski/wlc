// SDL

int SDL_INIT_TIMER = 1
int SDL_INIT_AUDIO = 16
int SDL_INIT_VIDEO = 32
int SDL_INIT_CDROM = 256
int SDL_INIT_JOYSTICK = 512
int SDL_INIT_EVERYTHING = 65535

int SDL_Init(int flags);
int SDL_InitSubSystem(int flags);
void SDL_QuitSubSystem(int flags);
int SDL_WasInit(int flags);
void SDL_Quit();

// SDL_active
int SDL_APPMOUSEFOCUS = 1
int SDL_APPINPUTFOCUS = 2
int SDL_APPACTIVE = 4

uint8 SDL_GetAppState();

// SDL_audio
/*
struct SDL_AudioSpec
{
    int freq
    uint16 format
    uint8 channels
    uint8 silence
    uint16 samples
    uint16 padding
    uint32 size
    void^ callback //TODO: need function pointers
    void^ userdata
}

int AUDIO_U8 = 8
int AUDIO_S8 = 32776
int AUDIO_U16LSB = 16
int AUDIO_S16LSB = 32784
//TODO audio formats if needed

struct SDL_AudioCVT
{
    int needed
    uint16 src_format
    uint16 dst_format
    double rate_incr
    uint8^ buf
    int len
    int len_cvt
    int len_mult
    double len_ratio
    void ^filters //TODO callback
    int filter_index
}

int SDL_AudioInit(char ^driver_nm);
void SDL_AudioQuit();
char ^SDL_AudioDriverName(char ^nmbuf, int maxlen);
int SDL_OpenAudio(SDL_AudioSpec^ desired, SDL_AudioSpec^ obtained);

int SDL_AUDIO_STOPPED = 0
int SDL_AUDIO_PLAYING = 1
int SDL_AUDIO_PAUSED = 2

int SDL_GetAudioStatus();
void SDL_PauseAudio(int pause_on);
void SDL_MixAudio(uint8^ dst, uint^ src, uint32 len, int vol);
void SDL_LockAudio();
void SDL_UnlockAudio();
void SDL_CloseAudio();

// SDL_error
void SDL_SetError(char ^fmt, ...);
char^ SDL_GetError();
void SDL_ClearError();
*/

// SDL_events.h

struct SDL_KeyboardEvent 
{
    uint8 type
    uint8 which
    uint8 state
    SDL_keysym keysym
}

struct SDL_Event;
int SDL_PeepEvents(SDL_Event^ events, int numevents, int action, uint32 mask);
void SDL_PumpEvents();
int8 SDL_EventState(uint8 type, int state);

// SDL_image
int IMG_INIT_JPG = 1
int IMG_INIT_PNG = 2
int IMG_INIT_TIF = 4
int IMG_INIT_WEBP = 8

int IMG_Init(int flags);
void IMG_Quit();
SDL_Surface^ IMG_Load(char ^filnm);
int IMG_InvertAlpha(int on);

// SDL_keyboard
struct SDL_keysym
{
    int8 scancode
    int sym
    int mod
    uint16 unicode
}

int SDL_DEFAULT_REPEAT_DELAY = 500
int SDL_DEFAULT_REPEAT_INTERVAL = 30

int SDL_EnableUNICODE(int enable);
int SDL_EnableKeyRepeat(int delay, int interval);
void SDL_GetKeyRepeat(int^ delay, int^ interval);
uint8^ SDL_GetKeyState(int^ numkeys);
char^ SDL_GetKeyName(int key);

// SDL_keysym
int SDLK_BACKSPACE		= 8
int SDLK_TAB		= 9
int SDLK_CLEAR		= 12
int SDLK_RETURN		= 13
int SDLK_PAUSE		= 19
int SDLK_ESCAPE		= 27
int SDLK_SPACE		= 32
int SDLK_EXCLAIM		= 33
int SDLK_QUOTEDBL		= 34
int SDLK_HASH		= 35
int SDLK_DOLLAR		= 36
int SDLK_AMPERSAND		= 38
int SDLK_QUOTE		= 39
int SDLK_LEFTPAREN		= 40
int SDLK_RIGHTPAREN		= 41
int SDLK_ASTERISK		= 42
int SDLK_PLUS		= 43
int SDLK_COMMA		= 44
int SDLK_MINUS		= 45
int SDLK_PERIOD		= 46
int SDLK_SLASH		= 47
int SDLK_0			= 48
int SDLK_1			= 49
int SDLK_2			= 50
int SDLK_3			= 51
int SDLK_4			= 52
int SDLK_5			= 53
int SDLK_6			= 54
int SDLK_7			= 55
int SDLK_8			= 56
int SDLK_9			= 57
int SDLK_COLON		= 58
int SDLK_SEMICOLON		= 59
int SDLK_LESS		= 60
int SDLK_EQUALS		= 61
int SDLK_GREATER		= 62
int SDLK_QUESTION		= 63
int SDLK_AT			= 64

int SDLK_LEFTBRACKET	= 91
int SDLK_BACKSLASH		= 92
int SDLK_RIGHTBRACKET	= 93
int SDLK_CARET		= 94
int SDLK_UNDERSCORE		= 95
int SDLK_BACKQUOTE		= 96
int SDLK_a			= 97
int SDLK_b			= 98
int SDLK_c			= 99
int SDLK_d			= 100
int SDLK_e			= 101
int SDLK_f			= 102
int SDLK_g			= 103
int SDLK_h			= 104
int SDLK_i			= 105
int SDLK_j			= 106
int SDLK_k			= 107
int SDLK_l			= 108
int SDLK_m			= 109
int SDLK_n			= 110
int SDLK_o			= 111
int SDLK_p			= 112
int SDLK_q			= 113
int SDLK_r			= 114
int SDLK_s			= 115
int SDLK_t			= 116
int SDLK_u			= 117
int SDLK_v			= 118
int SDLK_w			= 119
int SDLK_x			= 120
int SDLK_y			= 121
int SDLK_z			= 122
int SDLK_DELETE		= 127

int SDL_SPACE = 32


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
SDL_Surface^ SDL_CreateRGBSurface(uint flags, int width, int height, int depth,
                                  int32 rmask, int32 gmask, int32 bmask, int32 amask);
int SDL_Flip(SDL_Surface^ surf);

// SDL_timer.h

void SDL_Delay(int32 ms);
