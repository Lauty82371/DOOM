#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

#include <emscripten.h>
#include <SDL.h>

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"
#include "d_main.h"
#include "g_game.h"
#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "v_video.h"

static SDL_Window* doom_window = NULL;
static SDL_Renderer* doom_renderer = NULL;
static SDL_Texture* doom_texture = NULL;
static Uint32* argb_buffer = NULL;
static int palette_ready = 0;
static Uint32 palette32[256];
static Uint32 basetime = 0;

extern byte* screens[5];
extern int usemouse;
extern int mousemoved;

static int translate_key(SDL_Keycode key)
{
    switch (key)
    {
        case SDLK_LEFT: return KEY_LEFTARROW;
        case SDLK_RIGHT: return KEY_RIGHTARROW;
        case SDLK_UP: return KEY_UPARROW;
        case SDLK_DOWN: return KEY_DOWNARROW;
        case SDLK_ESCAPE: return KEY_ESCAPE;
        case SDLK_RETURN: return KEY_ENTER;
        case SDLK_TAB: return KEY_TAB;
        case SDLK_F1: return KEY_F1;
        case SDLK_F2: return KEY_F2;
        case SDLK_F3: return KEY_F3;
        case SDLK_F4: return KEY_F4;
        case SDLK_F5: return KEY_F5;
        case SDLK_F6: return KEY_F6;
        case SDLK_F7: return KEY_F7;
        case SDLK_F8: return KEY_F8;
        case SDLK_F9: return KEY_F9;
        case SDLK_F10: return KEY_F10;
        case SDLK_F11: return KEY_F11;
        case SDLK_F12: return KEY_F12;
        case SDLK_BACKSPACE: return KEY_BACKSPACE;
        case SDLK_EQUALS: return KEY_EQUALS;
        case SDLK_MINUS: return KEY_MINUS;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT: return KEY_RSHIFT;
        case SDLK_LCTRL:
        case SDLK_RCTRL: return KEY_RCTRL;
        case SDLK_LALT:
        case SDLK_RALT: return KEY_RALT;
        default:
            if (key >= 32 && key <= 126)
            {
                if (key >= 'A' && key <= 'Z') return key - 'A' + 'a';
                return key;
            }
            return 0;
    }
}

void I_StartFrame(void) {}
void I_StartTic(void) {}

void I_GetEvent(void)
{
    SDL_Event ev;
    event_t doom_ev;

    while (SDL_PollEvent(&ev))
    {
        memset(&doom_ev, 0, sizeof(doom_ev));

        switch (ev.type)
        {
            case SDL_QUIT:
                I_Quit();
                break;

            case SDL_KEYDOWN:
                doom_ev.type = ev_keydown;
                doom_ev.data1 = translate_key(ev.key.keysym.sym);
                if (doom_ev.data1) D_PostEvent(&doom_ev);
                break;

            case SDL_KEYUP:
                doom_ev.type = ev_keyup;
                doom_ev.data1 = translate_key(ev.key.keysym.sym);
                if (doom_ev.data1) D_PostEvent(&doom_ev);
                break;

            case SDL_MOUSEMOTION:
                doom_ev.type = ev_mouse;
                doom_ev.data1 = 0;
                doom_ev.data2 = ev.motion.xrel << 2;
                doom_ev.data3 = (-ev.motion.yrel) << 2;
                if (doom_ev.data2 || doom_ev.data3) D_PostEvent(&doom_ev);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                doom_ev.type = ev_mouse;
                doom_ev.data1 = (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT) ? 1 : 0)
                              | (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT) ? 2 : 0)
                              | (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE) ? 4 : 0);
                doom_ev.data2 = 0;
                doom_ev.data3 = 0;
                D_PostEvent(&doom_ev);
                break;
        }
    }
}

void I_UpdateNoBlit(void) {}

void I_FinishUpdate(void)
{
    int i;
    byte* src = screens[0];
    if (!src || !doom_texture) return;

    for (i = 0; i < SCREENWIDTH * SCREENHEIGHT; ++i)
        argb_buffer[i] = palette32[src[i]];

    SDL_UpdateTexture(doom_texture, NULL, argb_buffer, SCREENWIDTH * sizeof(Uint32));
    SDL_RenderClear(doom_renderer);
    SDL_RenderCopy(doom_renderer, doom_texture, NULL, NULL);
    SDL_RenderPresent(doom_renderer);
}

void I_ReadScreen(byte* scr)
{
    memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

void I_SetPalette(byte* palette)
{
    for (int i = 0; i < 256; ++i)
    {
        int r = gammatable[usegamma][*palette++];
        int g = gammatable[usegamma][*palette++];
        int b = gammatable[usegamma][*palette++];
        palette32[i] = 0xff000000 | (r << 16) | (g << 8) | b;
    }
    palette_ready = 1;
}

void I_InitGraphics(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
        I_Error("SDL_Init failed: %s", SDL_GetError());

    doom_window = SDL_CreateWindow("DOOM HTML", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   SCREENWIDTH * 2, SCREENHEIGHT * 2, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    doom_renderer = SDL_CreateRenderer(doom_window, -1, SDL_RENDERER_ACCELERATED);
    doom_texture = SDL_CreateTexture(doom_renderer, SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_STREAMING, SCREENWIDTH, SCREENHEIGHT);
    argb_buffer = (Uint32*)malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(Uint32));

    SDL_RenderSetLogicalSize(doom_renderer, SCREENWIDTH, SCREENHEIGHT);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    usemouse = 1;
}

void I_ShutdownGraphics(void)
{
    free(argb_buffer);
    if (doom_texture) SDL_DestroyTexture(doom_texture);
    if (doom_renderer) SDL_DestroyRenderer(doom_renderer);
    if (doom_window) SDL_DestroyWindow(doom_window);
    SDL_Quit();
}

void I_InitNetwork(void) {}
void I_NetCmd(void) {}
void I_InitSound(void) {}
void I_SubmitSound(void) {}
void I_ShutdownSound(void) {}
void I_SetChannels(void) {}
int I_GetSfxLumpNum(sfxinfo_t* sfx) { return 0; }
int I_StartSound(int id, int vol, int sep, int pitch, int priority) { return 0; }
void I_StopSound(int handle) {}
int I_SoundIsPlaying(int handle) { return 0; }
void I_UpdateSoundParams(int handle, int vol, int sep, int pitch) {}
void I_InitMusic(void) {}
void I_ShutdownMusic(void) {}
void I_SetMusicVolume(int volume) {}
void I_PauseSong(int handle) {}
void I_ResumeSong(int handle) {}
int I_RegisterSong(void* data) { return 0; }
void I_PlaySong(int handle, int looping) {}
void I_StopSong(int handle) {}
void I_UnRegisterSong(int handle) {}

int I_GetTime(void)
{
    Uint32 now = SDL_GetTicks();
    if (!basetime) basetime = now;
    return (int)(((now - basetime) * TICRATE) / 1000);
}

void I_WaitVBL(int count)
{
    SDL_Delay((count * 1000) / 70);
}

byte* I_ZoneBase(int* size)
{
    *size = 16 * 1024 * 1024;
    return (byte*)malloc(*size);
}

int I_GetHeapSize(void) { return 16 * 1024 * 1024; }
byte* I_AllocLow(int length) { return (byte*)calloc(1, length); }
void I_Tactile(int on, int off, int total) {}
void I_BeginRead(void) {}
void I_EndRead(void) {}

void I_Error(char* error, ...)
{
    va_list argptr;
    char buffer[1024];
    va_start(argptr, error);
    vsnprintf(buffer, sizeof(buffer), error, argptr);
    va_end(argptr);
    emscripten_run_script_int("console.error('DOOM error')");
    fprintf(stderr, "%s\n", buffer);
    emscripten_cancel_main_loop();
}

void I_Quit(void)
{
    emscripten_cancel_main_loop();
}
