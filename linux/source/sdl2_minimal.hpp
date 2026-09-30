// Minimal SDL2 ABI declarations used by AiloEMU's Linux frontend.
// SDL is dynamically linked; this file intentionally avoids a build-time SDL SDK dependency.
#pragma once
#include <cstdint>
using Uint8=uint8_t; using Sint16=int16_t; using Uint16=uint16_t; using Sint32=int32_t; using Uint32=uint32_t; using Uint64=uint64_t;
struct SDL_Window; struct SDL_Renderer; struct SDL_Texture; struct SDL_GameController; struct SDL_Joystick;
using SDL_AudioDeviceID=Uint32;
struct SDL_Rect{int x,y,w,h;};
struct SDL_Keysym{Sint32 scancode; Sint32 sym; Uint16 mod; Uint32 unused;};
struct SDL_KeyboardEvent{Uint32 type,timestamp,windowID;Uint8 state,repeat,padding2,padding3;SDL_Keysym keysym;};
struct SDL_DropEvent{Uint32 type,timestamp;char* file;Uint32 windowID;};
struct SDL_ControllerDeviceEvent{Uint32 type,timestamp;Sint32 which;};
struct SDL_ControllerAxisEvent{Uint32 type,timestamp;Sint32 which;Uint8 axis,padding1,padding2,padding3;Sint16 value;Uint16 padding4;};
struct SDL_ControllerButtonEvent{Uint32 type,timestamp;Sint32 which;Uint8 button,state,padding1,padding2;};
struct SDL_MouseButtonEvent{Uint32 type,timestamp,windowID,which;Uint8 button,state,clicks,padding1;Sint32 x,y;};
struct SDL_MouseMotionEvent{Uint32 type,timestamp,windowID,which,state;Sint32 x,y,xrel,yrel;};
union SDL_Event{Uint32 type;SDL_KeyboardEvent key;SDL_DropEvent drop;SDL_ControllerDeviceEvent cdevice;SDL_ControllerAxisEvent caxis;SDL_ControllerButtonEvent cbutton;SDL_MouseButtonEvent button;SDL_MouseMotionEvent motion;Uint8 padding[56];};
struct SDL_AudioSpec{int freq;Uint16 format;Uint8 channels,silence;Uint16 samples,padding;Uint32 size;void(*callback)(void*,Uint8*,int);void* userdata;};
extern "C" {
int SDL_Init(Uint32); void SDL_Quit(); const char* SDL_GetError();
SDL_Window* SDL_CreateWindow(const char*,int,int,int,int,Uint32); void SDL_DestroyWindow(SDL_Window*);
SDL_Renderer* SDL_CreateRenderer(SDL_Window*,int,Uint32);void SDL_DestroyRenderer(SDL_Renderer*);
int SDL_SetRenderDrawColor(SDL_Renderer*,Uint8,Uint8,Uint8,Uint8);int SDL_RenderClear(SDL_Renderer*);int SDL_RenderFillRect(SDL_Renderer*,const SDL_Rect*);
void SDL_RenderPresent(SDL_Renderer*);int SDL_RenderSetLogicalSize(SDL_Renderer*,int,int);int SDL_RenderCopy(SDL_Renderer*,SDL_Texture*,const SDL_Rect*,const SDL_Rect*);
SDL_Texture* SDL_CreateTexture(SDL_Renderer*,Uint32,int,int,int);void SDL_DestroyTexture(SDL_Texture*);int SDL_UpdateTexture(SDL_Texture*,const SDL_Rect*,const void*,int);
int SDL_PollEvent(SDL_Event*);void SDL_free(void*);Uint32 SDL_GetTicks();void SDL_Delay(Uint32);
int SDL_NumJoysticks();int SDL_IsGameController(int);SDL_GameController* SDL_GameControllerOpen(int);void SDL_GameControllerClose(SDL_GameController*);SDL_Joystick* SDL_GameControllerGetJoystick(SDL_GameController*);Sint32 SDL_JoystickInstanceID(SDL_Joystick*);
SDL_AudioDeviceID SDL_OpenAudioDevice(const char*,int,const SDL_AudioSpec*,SDL_AudioSpec*,int);void SDL_CloseAudioDevice(SDL_AudioDeviceID);void SDL_PauseAudioDevice(SDL_AudioDeviceID,int);int SDL_QueueAudio(SDL_AudioDeviceID,const void*,Uint32);Uint32 SDL_GetQueuedAudioSize(SDL_AudioDeviceID);void SDL_ClearQueuedAudio(SDL_AudioDeviceID);
int SDL_SetWindowFullscreen(SDL_Window*,Uint32);void SDL_SetWindowTitle(SDL_Window*,const char*);char* SDL_GetBasePath();char* SDL_GetPrefPath(const char*,const char*);
}
constexpr Uint32 SDL_INIT_AUDIO=0x10,SDL_INIT_VIDEO=0x20,SDL_INIT_GAMECONTROLLER=0x2000,SDL_INIT_EVENTS=0x4000;
constexpr Uint32 SDL_WINDOWPOS_CENTERED=0x2FFF0000u,SDL_WINDOW_RESIZABLE=0x20,SDL_WINDOW_ALLOW_HIGHDPI=0x2000,SDL_WINDOW_FULLSCREEN_DESKTOP=0x1001;
constexpr Uint32 SDL_RENDERER_ACCELERATED=0x2,SDL_RENDERER_PRESENTVSYNC=0x4;constexpr int SDL_TEXTUREACCESS_STREAMING=1;
constexpr Uint32 SDL_PIXELFORMAT_ARGB8888=0x16362004;constexpr Uint16 AUDIO_S16LSB=0x8010;
constexpr Uint32 SDL_QUIT=0x100,SDL_KEYDOWN=0x300,SDL_KEYUP=0x301,SDL_MOUSEMOTION=0x400,SDL_MOUSEBUTTONDOWN=0x401,SDL_MOUSEBUTTONUP=0x402;
constexpr Uint32 SDL_CONTROLLERAXISMOTION=0x650,SDL_CONTROLLERBUTTONDOWN=0x651,SDL_CONTROLLERBUTTONUP=0x652,SDL_CONTROLLERDEVICEADDED=0x653,SDL_CONTROLLERDEVICEREMOVED=0x654,SDL_DROPFILE=0x1000;
constexpr int SDL_SCANCODE_A=4,SDL_SCANCODE_B=5,SDL_SCANCODE_O=18,SDL_SCANCODE_S=22,SDL_SCANCODE_X=27,SDL_SCANCODE_Z=29,SDL_SCANCODE_RETURN=40,SDL_SCANCODE_ESCAPE=41,SDL_SCANCODE_BACKSPACE=42,SDL_SCANCODE_TAB=43,SDL_SCANCODE_SPACE=44,SDL_SCANCODE_F1=58,SDL_SCANCODE_F5=62,SDL_SCANCODE_F9=66,SDL_SCANCODE_F11=68,SDL_SCANCODE_RIGHT=79,SDL_SCANCODE_LEFT=80,SDL_SCANCODE_DOWN=81,SDL_SCANCODE_UP=82,SDL_SCANCODE_LCTRL=224,SDL_SCANCODE_LSHIFT=225;
constexpr Uint16 KMOD_CTRL=0x00C0;
constexpr Uint8 SDL_CONTROLLER_AXIS_LEFTX=0,SDL_CONTROLLER_AXIS_LEFTY=1,SDL_CONTROLLER_AXIS_RIGHTX=2,SDL_CONTROLLER_AXIS_RIGHTY=3;
constexpr Uint8 SDL_CONTROLLER_BUTTON_A=0,SDL_CONTROLLER_BUTTON_B=1,SDL_CONTROLLER_BUTTON_X=2,SDL_CONTROLLER_BUTTON_Y=3,SDL_CONTROLLER_BUTTON_BACK=4,SDL_CONTROLLER_BUTTON_START=6,SDL_CONTROLLER_BUTTON_LEFTSHOULDER=9,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER=10,SDL_CONTROLLER_BUTTON_DPAD_UP=11,SDL_CONTROLLER_BUTTON_DPAD_DOWN=12,SDL_CONTROLLER_BUTTON_DPAD_LEFT=13,SDL_CONTROLLER_BUTTON_DPAD_RIGHT=14;
