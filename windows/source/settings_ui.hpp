// AiloEMU controller and appearance settings. GPL-2.0-or-later.
#pragma once
#include "core.hpp"
#include "controller_art.hpp"
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <xinput.h>
#include <array>
#include <vector>
#include <string>
#include <algorithm>

struct InterfaceTheme {
    COLORREF background=RGB(5,7,5);
    COLORREF panel=RGB(8,20,8);
    COLORREF accent=RGB(57,255,20);
    COLORREF primary=RGB(240,255,240);
    COLORREF secondary=RGB(170,205,170);
    void defaults(){*this=InterfaceTheme{};}
    void load(const fs::path& ini){
        background=COLORREF(GetPrivateProfileIntW(L"Theme",L"Background",background,ini.c_str()));
        panel=COLORREF(GetPrivateProfileIntW(L"Theme",L"Panel",panel,ini.c_str()));
        accent=COLORREF(GetPrivateProfileIntW(L"Theme",L"Accent",accent,ini.c_str()));
        primary=COLORREF(GetPrivateProfileIntW(L"Theme",L"PrimaryText",primary,ini.c_str()));
        secondary=COLORREF(GetPrivateProfileIntW(L"Theme",L"SecondaryText",secondary,ini.c_str()));
    }
    void save(const fs::path& ini)const{
        const wchar_t* names[]={L"Background",L"Panel",L"Accent",L"PrimaryText",L"SecondaryText"};
        const COLORREF colors[]={background,panel,accent,primary,secondary};
        for(size_t i=0;i<5;i++){auto value=std::to_wstring(colors[i]);WritePrivateProfileStringW(L"Theme",names[i],value.c_str(),ini.c_str());}
    }
};

enum PadBinding {
    PB_NONE=0,PB_X_A=1,PB_X_B,PB_X_X,PB_X_Y,PB_X_LB,PB_X_RB,PB_X_BACK,PB_X_START,
    PB_X_L3,PB_X_R3,PB_X_LT,PB_X_RT,PB_X_DU,PB_X_DD,PB_X_DL,PB_X_DR,
    PB_X_LU,PB_X_LD,PB_X_LL,PB_X_LR,PB_X_RU,PB_X_RD,PB_X_RL,PB_X_RR,
    PB_X_ANY_U=30,PB_X_ANY_D,PB_X_ANY_L,PB_X_ANY_R,
    PB_JOY_BUTTON=1000,PB_JOY_POV_U=1100,PB_JOY_POV_D,PB_JOY_POV_L,PB_JOY_POV_R,
    PB_JOY_AXIS=1200,PB_JOY_ANY_U=1300,PB_JOY_ANY_D,PB_JOY_ANY_L,PB_JOY_ANY_R,
    PB_KEY=2000
};

inline constexpr size_t ACTION_COUNT=16;
inline const wchar_t* const ACTION_NAMES[ACTION_COUNT]={
    L"Su",L"Giù",L"Sinistra",L"Destra",L"A",L"B",L"X",L"Y",
    L"L",L"R",L"L2 / ZL",L"R2 / ZR",L"L3",L"R3",L"Start",L"Select"
};
inline constexpr unsigned ACTION_RETRO_IDS[ACTION_COUNT]={
    RETRO_DEVICE_ID_JOYPAD_UP,RETRO_DEVICE_ID_JOYPAD_DOWN,RETRO_DEVICE_ID_JOYPAD_LEFT,RETRO_DEVICE_ID_JOYPAD_RIGHT,
    RETRO_DEVICE_ID_JOYPAD_A,RETRO_DEVICE_ID_JOYPAD_B,RETRO_DEVICE_ID_JOYPAD_X,RETRO_DEVICE_ID_JOYPAD_Y,
    RETRO_DEVICE_ID_JOYPAD_L,RETRO_DEVICE_ID_JOYPAD_R,RETRO_DEVICE_ID_JOYPAD_L2,RETRO_DEVICE_ID_JOYPAD_R2,
    RETRO_DEVICE_ID_JOYPAD_L3,RETRO_DEVICE_ID_JOYPAD_R3,RETRO_DEVICE_ID_JOYPAD_START,RETRO_DEVICE_ID_JOYPAD_SELECT
};

struct PadProfile {
    int device=-1;
    bool keyboardEnabled=true;
    std::array<int,ACTION_COUNT> binding{};
    std::array<int,ACTION_COUNT> keys{};
};

inline void defaultKeys(PadProfile& p,unsigned player){
    p.keys.fill(PB_NONE);p.keyboardEnabled=player<2;
    if(player==0)p.keys={PB_KEY+VK_UP,PB_KEY+VK_DOWN,PB_KEY+VK_LEFT,PB_KEY+VK_RIGHT,
        PB_KEY+'X',PB_KEY+'Z',PB_KEY+'C',PB_KEY+'V',PB_KEY+'Q',PB_KEY+'E',PB_NONE,PB_NONE,
        PB_NONE,PB_NONE,PB_KEY+VK_RETURN,PB_KEY+VK_RSHIFT};
    else if(player==1)p.keys={PB_KEY+'W',PB_KEY+'S',PB_KEY+'A',PB_KEY+'D',
        PB_KEY+'G',PB_KEY+'F',PB_KEY+'H',PB_KEY+'J',PB_KEY+'Y',PB_KEY+'U',PB_NONE,PB_NONE,
        PB_NONE,PB_NONE,PB_KEY+'T',PB_KEY+'R'};
}

inline void defaultBindings(PadProfile& p,bool winmm=false,Console console=Console::NES){
    if(winmm){
        p.binding={PB_JOY_ANY_U,PB_JOY_ANY_D,PB_JOY_ANY_L,PB_JOY_ANY_R,
            PB_JOY_BUTTON+0,PB_JOY_BUTTON+1,PB_JOY_BUTTON+2,PB_JOY_BUTTON+3,
            PB_JOY_BUTTON+4,PB_JOY_BUTTON+5,PB_JOY_BUTTON+6,PB_JOY_BUTTON+7,
            PB_JOY_BUTTON+10,PB_JOY_BUTTON+11,PB_JOY_BUTTON+9,PB_JOY_BUTTON+8};
    }else{
        p.binding={PB_X_ANY_U,PB_X_ANY_D,PB_X_ANY_L,PB_X_ANY_R,
            PB_X_A,PB_X_B,PB_X_Y,PB_X_X,PB_X_LB,PB_X_RB,PB_X_LT,PB_X_RT,
            PB_X_L3,PB_X_R3,PB_X_START,PB_X_BACK};
    }
    if(console==Console::SNES)std::swap(p.binding[4],p.binding[5]);
}

inline const wchar_t* consoleKey(Console c){
    switch(c){
    case Console::NES:return L"NES";case Console::SNES:return L"SNES";case Console::GB:return L"GB";case Console::GBC:return L"GBC";case Console::GBA:return L"GBA";
    case Console::Genesis:return L"Genesis";case Console::SMS:return L"MasterSystem";case Console::GameGear:return L"GameGear";case Console::SG1000:return L"SG1000";
    case Console::Sega32X:return L"Sega32X";case Console::Atari2600:return L"Atari2600";case Console::Atari7800:return L"Atari7800";
    case Console::NGP:return L"NGP";case Console::NGPC:return L"NGPC";case Console::WonderSwan:return L"WonderSwan";case Console::WonderSwanColor:return L"WonderSwanColor";
    case Console::PCEngine:return L"PCEngine";case Console::NintendoDS:return L"NintendoDS";case Console::Nintendo64:return L"Nintendo64";
    case Console::VirtualBoy:return L"VirtualBoy";case Console::PokemonMini:return L"PokemonMini";default:return L"None";
    }
}

struct ControllerSettings {
    std::array<std::array<PadProfile,4>,22> profiles{};
    ControllerSettings(){for(size_t c=1;c<profiles.size();++c)for(unsigned player=0;player<4;player++){defaultBindings(profiles[c][player],false,Console(c));defaultKeys(profiles[c][player],player);}}
    PadProfile& at(Console c,unsigned player){return profiles.at(size_t(c)).at(player);}
    const PadProfile& at(Console c,unsigned player)const{return profiles.at(size_t(c)).at(player);}
    static std::wstring section(Console c,unsigned player){return L"Pad."+std::wstring(consoleKey(c))+L".P"+std::to_wstring(player+1);}
    void load(const fs::path& ini){
        for(size_t ci=1;ci<profiles.size();++ci){Console c=Console(ci);for(unsigned p=0;p<4;p++){
            auto sec=section(c,p);auto& profile=profiles[ci][p];
            profile.device=int(GetPrivateProfileIntW(sec.c_str(),L"Device",profile.device,ini.c_str()));
            profile.keyboardEnabled=GetPrivateProfileIntW(sec.c_str(),L"KeyboardEnabled",profile.keyboardEnabled?1:0,ini.c_str())!=0;
            for(size_t a=0;a<ACTION_COUNT;a++){
                std::wstring padKey=L"Pad"+std::wstring(ACTION_NAMES[a]),keyKey=L"Key"+std::wstring(ACTION_NAMES[a]);
                int legacy=int(GetPrivateProfileIntW(sec.c_str(),ACTION_NAMES[a],profile.binding[a],ini.c_str()));
                profile.binding[a]=int(GetPrivateProfileIntW(sec.c_str(),padKey.c_str(),legacy,ini.c_str()));
                profile.keys[a]=int(GetPrivateProfileIntW(sec.c_str(),keyKey.c_str(),profile.keys[a],ini.c_str()));
            }
        }}
    }
    void save(const fs::path& ini)const{
        for(size_t ci=1;ci<profiles.size();++ci){Console c=Console(ci);for(unsigned p=0;p<4;p++){
            auto sec=section(c,p);const auto& profile=profiles[ci][p];auto device=std::to_wstring(profile.device);
            WritePrivateProfileStringW(sec.c_str(),L"Device",device.c_str(),ini.c_str());
            WritePrivateProfileStringW(sec.c_str(),L"KeyboardEnabled",profile.keyboardEnabled?L"1":L"0",ini.c_str());
            for(size_t a=0;a<ACTION_COUNT;a++){
                std::wstring padKey=L"Pad"+std::wstring(ACTION_NAMES[a]),keyKey=L"Key"+std::wstring(ACTION_NAMES[a]);
                auto padValue=std::to_wstring(profile.binding[a]),keyValue=std::to_wstring(profile.keys[a]);
                WritePrivateProfileStringW(sec.c_str(),padKey.c_str(),padValue.c_str(),ini.c_str());
                WritePrivateProfileStringW(sec.c_str(),keyKey.c_str(),keyValue.c_str(),ini.c_str());
            }
        }}
    }
};

inline bool xinputBindingPressed(const XINPUT_GAMEPAD& g,int code){
    switch(code){
    case PB_X_A:return (g.wButtons&XINPUT_GAMEPAD_A)!=0;case PB_X_B:return (g.wButtons&XINPUT_GAMEPAD_B)!=0;
    case PB_X_X:return (g.wButtons&XINPUT_GAMEPAD_X)!=0;case PB_X_Y:return (g.wButtons&XINPUT_GAMEPAD_Y)!=0;
    case PB_X_LB:return (g.wButtons&XINPUT_GAMEPAD_LEFT_SHOULDER)!=0;case PB_X_RB:return (g.wButtons&XINPUT_GAMEPAD_RIGHT_SHOULDER)!=0;
    case PB_X_BACK:return (g.wButtons&XINPUT_GAMEPAD_BACK)!=0;case PB_X_START:return (g.wButtons&XINPUT_GAMEPAD_START)!=0;
    case PB_X_L3:return (g.wButtons&XINPUT_GAMEPAD_LEFT_THUMB)!=0;case PB_X_R3:return (g.wButtons&XINPUT_GAMEPAD_RIGHT_THUMB)!=0;
    case PB_X_LT:return g.bLeftTrigger>80;case PB_X_RT:return g.bRightTrigger>80;
    case PB_X_DU:return (g.wButtons&XINPUT_GAMEPAD_DPAD_UP)!=0;case PB_X_DD:return (g.wButtons&XINPUT_GAMEPAD_DPAD_DOWN)!=0;
    case PB_X_DL:return (g.wButtons&XINPUT_GAMEPAD_DPAD_LEFT)!=0;case PB_X_DR:return (g.wButtons&XINPUT_GAMEPAD_DPAD_RIGHT)!=0;
    case PB_X_LU:return g.sThumbLY>16000;case PB_X_LD:return g.sThumbLY<-16000;case PB_X_LL:return g.sThumbLX<-16000;case PB_X_LR:return g.sThumbLX>16000;
    case PB_X_RU:return g.sThumbRY>16000;case PB_X_RD:return g.sThumbRY<-16000;case PB_X_RL:return g.sThumbRX<-16000;case PB_X_RR:return g.sThumbRX>16000;
    case PB_X_ANY_U:return (g.wButtons&XINPUT_GAMEPAD_DPAD_UP)||g.sThumbLY>16000;
    case PB_X_ANY_D:return (g.wButtons&XINPUT_GAMEPAD_DPAD_DOWN)||g.sThumbLY<-16000;
    case PB_X_ANY_L:return (g.wButtons&XINPUT_GAMEPAD_DPAD_LEFT)||g.sThumbLX<-16000;
    case PB_X_ANY_R:return (g.wButtons&XINPUT_GAMEPAD_DPAD_RIGHT)||g.sThumbLX>16000;
    default:return false;
    }
}

inline LONG joyAxis(const JOYINFOEX& j,unsigned axis){const LONG values[]={LONG(j.dwXpos),LONG(j.dwYpos),LONG(j.dwZpos),LONG(j.dwRpos),LONG(j.dwUpos),LONG(j.dwVpos)};return axis<6?values[axis]:32767;}
inline bool joyBindingPressed(const JOYINFOEX& j,int code){
    if(code>=PB_JOY_BUTTON&&code<PB_JOY_BUTTON+32)return (j.dwButtons&(1u<<(code-PB_JOY_BUTTON)))!=0;
    if(code>=PB_JOY_AXIS&&code<PB_JOY_AXIS+12){unsigned n=unsigned(code-PB_JOY_AXIS),axis=n/2;return (n&1)?joyAxis(j,axis)>49151:joyAxis(j,axis)<16384;}
    bool up=j.dwPOV!=JOY_POVCENTERED&&(j.dwPOV>=31500||j.dwPOV<=4500),down=j.dwPOV>=13500&&j.dwPOV<=22500;
    bool left=j.dwPOV>=22500&&j.dwPOV<=31500,right=j.dwPOV>=4500&&j.dwPOV<=13500;
    switch(code){case PB_JOY_POV_U:return up;case PB_JOY_POV_D:return down;case PB_JOY_POV_L:return left;case PB_JOY_POV_R:return right;
    case PB_JOY_ANY_U:return up||j.dwYpos<16384;case PB_JOY_ANY_D:return down||j.dwYpos>49151;
    case PB_JOY_ANY_L:return left||j.dwXpos<16384;case PB_JOY_ANY_R:return right||j.dwXpos>49151;default:return false;}
}

inline std::wstring keyboardBindingName(int vk){
    switch(vk){case VK_UP:return L"↑";case VK_DOWN:return L"↓";case VK_LEFT:return L"←";case VK_RIGHT:return L"→";
    case VK_RETURN:return L"Invio";case VK_SPACE:return L"Spazio";case VK_RSHIFT:return L"Shift D";case VK_LSHIFT:return L"Shift S";
    case VK_RCONTROL:return L"Ctrl D";case VK_LCONTROL:return L"Ctrl S";case VK_ESCAPE:return L"Esc";case VK_TAB:return L"Tab";case VK_BACK:return L"Backspace";}
    if(vk>='A'&&vk<='Z')return std::wstring(1,wchar_t(vk));
    if(vk>='0'&&vk<='9')return std::wstring(1,wchar_t(vk));
    UINT scan=MapVirtualKeyW(UINT(vk),MAPVK_VK_TO_VSC)<<16;if(vk==VK_RSHIFT||vk==VK_RCONTROL||vk==VK_RMENU)scan|=1u<<24;
    wchar_t name[64]={};if(GetKeyNameTextW(LONG(scan),name,64)>0)return name;return L"VK "+std::to_wstring(vk);
}

inline std::wstring bindingName(int code){
    static const wchar_t* xnames[]={L"",L"A",L"B",L"X",L"Y",L"LB",L"RB",L"View / Back",L"Menu / Start",L"L3",L"R3",L"LT",L"RT",L"D-pad ↑",L"D-pad ↓",L"D-pad ←",L"D-pad →",L"Stick S ↑",L"Stick S ↓",L"Stick S ←",L"Stick S →",L"Stick D ↑",L"Stick D ↓",L"Stick D ←",L"Stick D →"};
    if(code==PB_NONE)return L"Non assegnato";
    if(code>=PB_KEY&&code<PB_KEY+256)return keyboardBindingName(code-PB_KEY);
    if(code>0&&code<int(std::size(xnames)))return xnames[code];
    if(code>=PB_X_ANY_U&&code<=PB_X_ANY_R){const wchar_t* n[]={L"D-pad / Stick ↑",L"D-pad / Stick ↓",L"D-pad / Stick ←",L"D-pad / Stick →"};return n[code-PB_X_ANY_U];}
    if(code>=PB_JOY_BUTTON&&code<PB_JOY_BUTTON+32)return L"Pulsante "+std::to_wstring(code-PB_JOY_BUTTON+1);
    if(code>=PB_JOY_POV_U&&code<=PB_JOY_POV_R){const wchar_t* n[]={L"POV ↑",L"POV ↓",L"POV ←",L"POV →"};return n[code-PB_JOY_POV_U];}
    if(code>=PB_JOY_AXIS&&code<PB_JOY_AXIS+12){static const wchar_t* axes[]={L"X",L"Y",L"Z",L"R",L"U",L"V"};unsigned n=unsigned(code-PB_JOY_AXIS);return std::wstring(L"Asse ")+axes[n/2]+((n&1)?L" +":L" -");}
    if(code>=PB_JOY_ANY_U&&code<=PB_JOY_ANY_R){const wchar_t* n[]={L"POV / Asse ↑",L"POV / Asse ↓",L"POV / Asse ←",L"POV / Asse →"};return n[code-PB_JOY_ANY_U];}
    return L"Sconosciuto";
}

class SettingsDialog {
    enum {NAV_CONTROLLERS=2001,NAV_APPEARANCE,COMBO_CONSOLE,COMBO_PLAYER,COMBO_DEVICE,REFRESH_DEVICES,RESET_PROFILE,
        SOURCE_KEYBOARD,SOURCE_GAMEPAD,KEYBOARD_ENABLE,COLOR_BASE=2200,RESET_THEME=2210,STATUS=2220};
    HWND owner=nullptr,window=nullptr,consoleBox=nullptr,playerBox=nullptr,deviceBox=nullptr,status=nullptr,keyboardCheck=nullptr,keyboardSource=nullptr,gamepadSource=nullptr;
    std::array<HWND,5> colorButtons{};std::array<RECT,ACTION_COUNT> actionRects{};
    std::vector<HWND> controllerPage,appearancePage;HFONT font=nullptr,bold=nullptr,tiny=nullptr;HBRUSH panelBrush=nullptr;
    ControllerSettings workingControls;InterfaceTheme workingTheme;bool done=false,accepted=false,controllerVisible=true,editKeyboard=true;
    int capture=-1;DWORD captureStart=0;
    using XInputFn=DWORD(WINAPI*)(DWORD,XINPUT_STATE*);HMODULE xinput=nullptr;XInputFn xget=nullptr;
    static std::wstring fromUtf8(const std::string& s){if(s.empty())return {};int n=MultiByteToWideChar(CP_UTF8,0,s.data(),int(s.size()),nullptr,0);std::wstring w(size_t(n),L'\0');MultiByteToWideChar(CP_UTF8,0,s.data(),int(s.size()),w.data(),n);return w;}
    static std::wstring colorText(COLORREF c){wchar_t b[16];swprintf(b,16,L"#%02X%02X%02X",GetRValue(c),GetGValue(c),GetBValue(c));return b;}
    static void fill(HDC dc,RECT r,COLORREF c){HBRUSH b=CreateSolidBrush(c);FillRect(dc,&r,b);DeleteObject(b);}
    static void box(HDC dc,RECT r,COLORREF inside,COLORREF border=RGB(0,0,0),int thickness=4){fill(dc,r,border);InflateRect(&r,-thickness,-thickness);fill(dc,r,inside);}
    HWND add(const wchar_t* cls,const wchar_t* text,DWORD style,int x,int y,int w,int h,int id,std::vector<HWND>* page=nullptr){
        HWND c=CreateWindowExW(0,cls,text,WS_CHILD|WS_VISIBLE|style,x,y,w,h,window,reinterpret_cast<HMENU>(INT_PTR(id)),GetModuleHandleW(nullptr),nullptr);
        SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);if(page)page->push_back(c);return c;
    }
    Console selectedConsole()const{int i=int(SendMessageW(consoleBox,CB_GETCURSEL,0,0));return i<0?Console::NES:Console(SendMessageW(consoleBox,CB_GETITEMDATA,i,0));}
    unsigned selectedPlayer()const{int i=int(SendMessageW(playerBox,CB_GETCURSEL,0,0));return i<0?0u:unsigned(i);}
    PadProfile& profile(){return workingControls.at(selectedConsole(),selectedPlayer());}
    const std::array<int,ACTION_COUNT>& shownBindings()const{const auto& p=workingControls.at(selectedConsole(),selectedPlayer());return editKeyboard?p.keys:p.binding;}
    void cancelCapture(){if(capture>=0){capture=-1;KillTimer(window,1);InvalidateRect(window,nullptr,FALSE);}}
    void page(bool controllers){cancelCapture();controllerVisible=controllers;for(HWND h:controllerPage)ShowWindow(h,controllers?SW_SHOW:SW_HIDE);for(HWND h:appearancePage)ShowWindow(h,controllers?SW_HIDE:SW_SHOW);InvalidateRect(window,nullptr,TRUE);}
    void fillConsoles(){
        for(unsigned c=unsigned(Console::NES);c<=unsigned(Console::PokemonMini);++c){int i=int(SendMessageW(consoleBox,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(fromUtf8(consoleName(Console(c))).c_str())));SendMessageW(consoleBox,CB_SETITEMDATA,i,c);}
        SendMessageW(consoleBox,CB_SETCURSEL,0,0);fillPlayers();
    }
    void fillPlayers(){
        cancelCapture();int old=std::max(0,int(SendMessageW(playerBox,CB_GETCURSEL,0,0)));SendMessageW(playerBox,CB_RESETCONTENT,0,0);
        unsigned count=consolePlayers(selectedConsole());for(unsigned p=0;p<count;p++){auto s=L"Porta "+std::to_wstring(p+1);SendMessageW(playerBox,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(s.c_str()));}
        SendMessageW(playerBox,CB_SETCURSEL,std::min(old,int(count-1)),0);fillDevices();SendMessageW(keyboardCheck,BM_SETCHECK,profile().keyboardEnabled?BST_CHECKED:BST_UNCHECKED,0);InvalidateRect(window,nullptr,FALSE);
    }
    void fillDevices(){
        int wanted=profile().device;SendMessageW(deviceBox,CB_RESETCONTENT,0,0);
        auto item=[&](const std::wstring& s,int value){int i=int(SendMessageW(deviceBox,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(s.c_str())));SendMessageW(deviceBox,CB_SETITEMDATA,i,value);if(value==wanted)SendMessageW(deviceBox,CB_SETCURSEL,i,0);};
        item(L"Automatico (porta corrispondente)",-1);item(L"Nessun gamepad",0);
        for(unsigned i=0;i<4;i++){XINPUT_STATE s{};bool connected=xget&&xget(i,&s)==ERROR_SUCCESS;item(L"XInput "+std::to_wstring(i+1)+(connected?L" — connesso":L" — non rilevato"),int(i+1));}
        UINT count=joyGetNumDevs();for(UINT id=0;id<count;id++){JOYCAPSW caps{};JOYINFOEX j{};j.dwSize=sizeof(j);j.dwFlags=JOY_RETURNALL;if(joyGetDevCapsW(id,&caps,sizeof(caps))==JOYERR_NOERROR&&joyGetPosEx(id,&j)==JOYERR_NOERROR)item(std::wstring(caps.szPname)+L" (WinMM)",100+int(id));}
        if(SendMessageW(deviceBox,CB_GETCURSEL,0,0)==CB_ERR)SendMessageW(deviceBox,CB_SETCURSEL,0,0);
    }
    void refreshColors(){COLORREF colors[]={workingTheme.background,workingTheme.panel,workingTheme.accent,workingTheme.primary,workingTheme.secondary};for(size_t i=0;i<5;i++)SetWindowTextW(colorButtons[i],colorText(colors[i]).c_str());if(panelBrush)DeleteObject(panelBrush);panelBrush=CreateSolidBrush(workingTheme.panel);InvalidateRect(window,nullptr,TRUE);}
    int detectBinding(int device){
        if(device==-1)device=int(selectedPlayer()+1);
        if(device>=1&&device<=4&&xget){XINPUT_STATE state{};if(xget(unsigned(device-1),&state)!=ERROR_SUCCESS)return 0;for(int c=PB_X_A;c<=PB_X_RR;c++)if(xinputBindingPressed(state.Gamepad,c))return c;}
        if(device>=100){JOYINFOEX j{};j.dwSize=sizeof(j);j.dwFlags=JOY_RETURNALL;if(joyGetPosEx(UINT(device-100),&j)!=JOYERR_NOERROR)return 0;for(int i=0;i<32;i++)if(joyBindingPressed(j,PB_JOY_BUTTON+i))return PB_JOY_BUTTON+i;for(int c=PB_JOY_POV_U;c<=PB_JOY_POV_R;c++)if(joyBindingPressed(j,c))return c;for(int c=PB_JOY_AXIS;c<PB_JOY_AXIS+12;c++)if(joyBindingPressed(j,c))return c;}
        return 0;
    }
    controller_art::Sprite sprite;
    Console spriteConsole=Console::None;
    COLORREF spriteBackground=CLR_INVALID;
    void layoutActions(Console c){
        if(spriteConsole!=c||spriteBackground!=workingTheme.background){
            COLORREF b=workingTheme.background;
            sprite=controller_art::make(c,(uint32_t(GetRValue(b))<<16)|(uint32_t(GetGValue(b))<<8)|GetBValue(b));
            spriteConsole=c;spriteBackground=b;
        }
        for(size_t a=0;a<ACTION_COUNT;++a){
            auto r=sprite.buttons[a].bounds;
            actionRects[a]=r.w>0?RECT{217+r.x*3,174+r.y*3,217+(r.x+r.w)*3,174+(r.y+r.h)*3}:RECT{};
        }
    }
    static std::wstring compactBinding(int code){
        if(code==PB_NONE)return L"—";
        if(code>=PB_KEY){
            int key=code-PB_KEY;
            switch(key){
            case VK_UP:return L"↑";case VK_DOWN:return L"↓";case VK_LEFT:return L"←";case VK_RIGHT:return L"→";
            case VK_RETURN:return L"Enter";case VK_RSHIFT:return L"RSh";case VK_LSHIFT:return L"LSh";
            case VK_SPACE:return L"Space";case VK_LCONTROL:return L"LCtrl";case VK_RCONTROL:return L"RCtrl";
            default:return bindingName(code);
            }
        }
        if(code>=PB_X_ANY_U&&code<=PB_X_ANY_R){const wchar_t* n[]={L"↑",L"↓",L"←",L"→"};return n[code-PB_X_ANY_U];}
        if(code==PB_X_BACK)return L"Back";
        if(code==PB_X_START)return L"Start";
        if(code>=PB_X_DU&&code<=PB_X_RR){
            const wchar_t* arrows[]={L"↑",L"↓",L"←",L"→"};int n=code-PB_X_DU;
            return std::wstring(n<4?L"D":n<8?L"L":L"R")+arrows[n%4];
        }
        if(code>=PB_JOY_BUTTON&&code<PB_JOY_BUTTON+32)return L"B"+std::to_wstring(code-PB_JOY_BUTTON+1);
        if(code>=PB_JOY_POV_U&&code<=PB_JOY_POV_R){const wchar_t* n[]={L"P↑",L"P↓",L"P←",L"P→"};return n[code-PB_JOY_POV_U];}
        if(code>=PB_JOY_ANY_U&&code<=PB_JOY_ANY_R){const wchar_t* n[]={L"↑",L"↓",L"←",L"→"};return n[code-PB_JOY_ANY_U];}
        if(code>=PB_JOY_AXIS&&code<PB_JOY_AXIS+12){const wchar_t* n[]={L"X",L"Y",L"Z",L"R",L"U",L"V"};int i=code-PB_JOY_AXIS;return std::wstring(n[i/2])+((i&1)?L"+":L"-");}
        return bindingName(code);
    }
    void drawAction(HDC dc,size_t a){
        RECT r=actionRects[a];if(IsRectEmpty(&r))return;
        if(capture==int(a)){
            HBRUSH brush=CreateSolidBrush(workingTheme.accent);RECT edge=r;InflateRect(&edge,3,3);FrameRect(dc,&edge,brush);DeleteObject(brush);
        }
        std::wstring assigned=capture==int(a)?L"…":compactBinding(shownBindings()[a]);
        auto old=SelectObject(dc,tiny);SetBkMode(dc,TRANSPARENT);
        // Fit to the physical key. Full unabridged binding is shown on hover.
        SIZE sz{};GetTextExtentPoint32W(dc,assigned.c_str(),int(assigned.size()),&sz);
        while(assigned.size()>1&&sz.cx>r.right-r.left-2){
            assigned.pop_back();GetTextExtentPoint32W(dc,assigned.c_str(),int(assigned.size()),&sz);
        }
        if(r.bottom-r.top>=39){
            RECT caption=r;caption.bottom=caption.top+13;
            RECT shade=caption;OffsetRect(&shade,1,1);SetTextColor(dc,RGB(0,0,0));DrawTextW(dc,sprite.buttons[a].label.c_str(),-1,&shade,DT_CENTER|DT_SINGLELINE);
            SetTextColor(dc,RGB(255,255,255));DrawTextW(dc,sprite.buttons[a].label.c_str(),-1,&caption,DT_CENTER|DT_SINGLELINE);
            r.top+=12;
        }
        RECT shadow=r;OffsetRect(&shadow,1,1);SetTextColor(dc,RGB(0,0,0));
        DrawTextW(dc,assigned.c_str(),-1,&shadow,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        SetTextColor(dc,RGB(255,255,255));DrawTextW(dc,assigned.c_str(),-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);SelectObject(dc,old);
    }
    void paintController(HDC dc){
        RECT art={205,148,950,550};box(dc,art,workingTheme.background,workingTheme.accent,2);
        layoutActions(selectedConsole());
        BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=controller_art::W;
        info.bmiHeader.biHeight=-controller_art::H;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
        int oldMode=SetStretchBltMode(dc,COLORONCOLOR);
        StretchDIBits(dc,217,174,controller_art::W*3,controller_art::H*3,0,0,controller_art::W,controller_art::H,sprite.pixels.data(),&info,DIB_RGB_COLORS,SRCCOPY);
        SetStretchBltMode(dc,oldMode);
        for(size_t a=0;a<ACTION_COUNT;a++)drawAction(dc,a);
        RECT name={220,150,935,173};SetBkMode(dc,TRANSPARENT);SetTextColor(dc,workingTheme.primary);auto old=SelectObject(dc,font);
        DrawTextW(dc,fromUtf8(consoleName(selectedConsole())).c_str(),-1,&name,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        RECT mode={220,517,935,544};SetTextColor(dc,workingTheme.secondary);
        std::wstring line=editKeyboard?L"Tastiera: clicca un tasto disegnato, poi premi il tasto da assegnare.":L"Gamepad: clicca un tasto disegnato, poi premi il comando fisico.";
        DrawTextW(dc,line.c_str(),-1,&mode,DT_CENTER|DT_VCENTER|DT_SINGLELINE);SelectObject(dc,old);
    }
    void chooseColor(unsigned index){
        COLORREF* colors[]={&workingTheme.background,&workingTheme.panel,&workingTheme.accent,&workingTheme.primary,&workingTheme.secondary};static COLORREF custom[16]{};
        CHOOSECOLORW cc{};cc.lStructSize=sizeof(cc);cc.hwndOwner=window;cc.rgbResult=*colors[index];cc.lpCustColors=custom;cc.Flags=CC_FULLOPEN|CC_RGBINIT;
        if(ChooseColorW(&cc)){*colors[index]=cc.rgbResult;refreshColors();}
    }
    void startCapture(int action){
        if(action<0||action>=int(ACTION_COUNT))return;
        cancelCapture();capture=action;captureStart=GetTickCount();SetFocus(window);
        if(editKeyboard)SetWindowTextW(status,L"Premi il tasto da mostrare nel pulsante. Canc rimuove, Esc annulla.");
        else {SetWindowTextW(status,L"Premi un pulsante, grilletto, POV o direzione dell'asse. Canc rimuove.");SetTimer(window,1,25,nullptr);}
        InvalidateRect(window,nullptr,FALSE);
    }
    void finishCapture(int code){
        if(capture<0)return;
        auto& p=profile();(editKeyboard?p.keys:p.binding)[size_t(capture)]=code;capture=-1;KillTimer(window,1);
        SetWindowTextW(status,L"Assegnazione aggiornata. Puoi configurare un altro pulsante.");InvalidateRect(window,nullptr,FALSE);
    }
    void create(){
        font=CreateFontW(-16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        bold=CreateFontW(-19,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        tiny=CreateFontW(-12,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,FIXED_PITCH,L"Consolas");panelBrush=CreateSolidBrush(workingTheme.panel);
        add(L"BUTTON",L"Controller",BS_PUSHBUTTON,18,24,160,44,NAV_CONTROLLERS);add(L"BUTTON",L"Aspetto",BS_PUSHBUTTON,18,76,160,44,NAV_APPEARANCE);
        HWND title=add(L"STATIC",L"Controller pixel art",SS_LEFT,210,16,700,28,0,&controllerPage);SendMessageW(title,WM_SETFONT,reinterpret_cast<WPARAM>(bold),TRUE);
        add(L"STATIC",L"Console",SS_LEFT,210,52,70,24,0,&controllerPage);consoleBox=add(L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL,280,48,210,300,COMBO_CONSOLE,&controllerPage);
        add(L"STATIC",L"Giocatore",SS_LEFT,510,52,75,24,0,&controllerPage);playerBox=add(L"COMBOBOX",L"",CBS_DROPDOWNLIST,585,48,145,240,COMBO_PLAYER,&controllerPage);
        keyboardSource=add(L"BUTTON",L"Tastiera",BS_AUTORADIOBUTTON|WS_GROUP,210,88,105,30,SOURCE_KEYBOARD,&controllerPage);gamepadSource=add(L"BUTTON",L"Gamepad",BS_AUTORADIOBUTTON,322,88,105,30,SOURCE_GAMEPAD,&controllerPage);SendMessageW(keyboardSource,BM_SETCHECK,BST_CHECKED,0);
        keyboardCheck=add(L"BUTTON",L"Tastiera attiva insieme al gamepad",BS_AUTOCHECKBOX,445,90,255,26,KEYBOARD_ENABLE,&controllerPage);
        add(L"STATIC",L"Gamepad",SS_LEFT,210,120,70,22,0,&controllerPage);deviceBox=add(L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL,280,116,555,300,COMBO_DEVICE,&controllerPage);add(L"BUTTON",L"Aggiorna",BS_PUSHBUTTON,842,115,88,28,REFRESH_DEVICES,&controllerPage);
        add(L"BUTTON",L"Ripristina sorgente",BS_PUSHBUTTON,210,558,175,32,RESET_PROFILE,&controllerPage);status=add(L"STATIC",L"Clicca un pulsante disegnato sul controller per assegnarlo.",SS_LEFT,405,560,525,40,STATUS,&controllerPage);
        HWND appearanceTitle=add(L"STATIC",L"Colori dell'interfaccia",SS_LEFT,210,20,620,30,0,&appearancePage);SendMessageW(appearanceTitle,WM_SETFONT,reinterpret_cast<WPARAM>(bold),TRUE);
        const wchar_t* colorNames[]={L"Sfondo",L"Pannelli e barra di stato",L"Colore evidenziazione",L"Testo principale",L"Testo secondario"};
        for(size_t i=0;i<5;i++){add(L"STATIC",colorNames[i],SS_LEFT,230,90+int(i)*62,250,28,0,&appearancePage);colorButtons[i]=add(L"BUTTON",L"",BS_PUSHBUTTON,500,84+int(i)*62,180,34,COLOR_BASE+int(i),&appearancePage);}
        add(L"BUTTON",L"Ripristina nero e verde",BS_PUSHBUTTON,230,420,230,34,RESET_THEME,&appearancePage);add(L"STATIC",L"Le modifiche vengono applicate dopo aver premuto OK.",SS_LEFT,230,475,590,35,0,&appearancePage);
        add(L"BUTTON",L"OK",BS_DEFPUSHBUTTON,735,620,96,34,IDOK);add(L"BUTTON",L"Annulla",BS_PUSHBUTTON,842,620,96,34,IDCANCEL);
        fillConsoles();refreshColors();page(true);
    }
    void command(int id,int notification){
        if(id==NAV_CONTROLLERS){page(true);return;}if(id==NAV_APPEARANCE){page(false);return;}
        if(id==COMBO_CONSOLE&&notification==CBN_SELCHANGE){fillPlayers();return;}if(id==COMBO_PLAYER&&notification==CBN_SELCHANGE){fillDevices();SendMessageW(keyboardCheck,BM_SETCHECK,profile().keyboardEnabled?BST_CHECKED:BST_UNCHECKED,0);InvalidateRect(window,nullptr,FALSE);return;}
        if(id==SOURCE_KEYBOARD){cancelCapture();editKeyboard=true;SendMessageW(keyboardSource,BM_SETCHECK,BST_CHECKED,0);SendMessageW(gamepadSource,BM_SETCHECK,BST_UNCHECKED,0);SetWindowTextW(status,L"Tastiera selezionata: clicca un pulsante pixel art e premi un tasto.");InvalidateRect(window,nullptr,FALSE);return;}
        if(id==SOURCE_GAMEPAD){cancelCapture();editKeyboard=false;SendMessageW(keyboardSource,BM_SETCHECK,BST_UNCHECKED,0);SendMessageW(gamepadSource,BM_SETCHECK,BST_CHECKED,0);SetWindowTextW(status,L"Gamepad selezionato: clicca un pulsante pixel art e premi un comando.");InvalidateRect(window,nullptr,FALSE);return;}
        if(id==KEYBOARD_ENABLE){profile().keyboardEnabled=SendMessageW(keyboardCheck,BM_GETCHECK,0,0)==BST_CHECKED;return;}
        if(id==COMBO_DEVICE&&notification==CBN_SELCHANGE){int i=int(SendMessageW(deviceBox,CB_GETCURSEL,0,0));if(i>=0){int old=profile().device,value=int(SendMessageW(deviceBox,CB_GETITEMDATA,i,0));profile().device=value;if((old>=100)!=(value>=100))defaultBindings(profile(),value>=100,selectedConsole());InvalidateRect(window,nullptr,FALSE);}return;}
        if(id==REFRESH_DEVICES){fillDevices();return;}
        if(id==RESET_PROFILE){if(editKeyboard)defaultKeys(profile(),selectedPlayer());else defaultBindings(profile(),profile().device>=100,selectedConsole());SendMessageW(keyboardCheck,BM_SETCHECK,profile().keyboardEnabled?BST_CHECKED:BST_UNCHECKED,0);InvalidateRect(window,nullptr,FALSE);return;}
        if(id>=COLOR_BASE&&id<COLOR_BASE+5){chooseColor(unsigned(id-COLOR_BASE));return;}if(id==RESET_THEME){workingTheme.defaults();refreshColors();return;}
        if(id==IDOK){accepted=true;done=true;DestroyWindow(window);return;}if(id==IDCANCEL){done=true;DestroyWindow(window);return;}
    }
    LRESULT event(UINT msg,WPARAM wp,LPARAM lp){
        switch(msg){
        case WM_CREATE:create();return 0;case WM_COMMAND:command(LOWORD(wp),HIWORD(wp));return 0;
        case WM_PAINT:{PAINTSTRUCT ps{};HDC dc=BeginPaint(window,&ps);if(controllerVisible)paintController(dc);EndPaint(window,&ps);return 0;}
        case WM_LBUTTONDOWN:if(controllerVisible){POINT p{GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};layoutActions(selectedConsole());for(size_t a=0;a<ACTION_COUNT;a++)if(PtInRect(&actionRects[a],p)){startCapture(int(a));return 0;}}break;
        case WM_MOUSEMOVE:if(controllerVisible&&capture<0){
            POINT p{GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};layoutActions(selectedConsole());
            std::wstring hint=L"Clicca un pulsante disegnato sul controller per assegnarlo.";
            for(size_t a=0;a<ACTION_COUNT;++a)if(PtInRect(&actionRects[a],p)){
                hint=sprite.buttons[a].label+L" → "+bindingName(shownBindings()[a]);break;
            }
            SetWindowTextW(status,hint.c_str());
        }return 0;
        case WM_TIMER:if(capture>=0&&!editKeyboard&&GetTickCount()-captureStart>250){int code=detectBinding(profile().device);if(code)finishCapture(code);}return 0;
        case WM_KEYDOWN:case WM_SYSKEYDOWN:
            if(capture>=0){
                if(wp==VK_ESCAPE){cancelCapture();SetWindowTextW(status,L"Assegnazione annullata.");return 0;}
                if(wp==VK_DELETE){finishCapture(PB_NONE);return 0;}
                if(editKeyboard){int vk=int(wp);if(wp==VK_SHIFT||wp==VK_CONTROL||wp==VK_MENU){UINT scan=UINT((lp>>16)&0xFF);if(lp&(1LL<<24))scan|=0xE000;UINT specific=MapVirtualKeyW(scan,MAPVK_VSC_TO_VK_EX);if(specific)vk=int(specific);}finishCapture(PB_KEY+vk);return 0;}
                return 0;
            }
            if(wp==VK_ESCAPE){command(IDCANCEL,0);return 0;}break;
        case WM_CTLCOLORSTATIC:{HDC dc=reinterpret_cast<HDC>(wp);SetTextColor(dc,workingTheme.primary);SetBkColor(dc,workingTheme.panel);return reinterpret_cast<LRESULT>(panelBrush);}
        case WM_ERASEBKGND:{RECT r{};GetClientRect(window,&r);FillRect(reinterpret_cast<HDC>(wp),&r,panelBrush);return 1;}
        case WM_CLOSE:done=true;DestroyWindow(window);return 0;case WM_DESTROY:window=nullptr;return 0;
        }
        return DefWindowProcW(window,msg,wp,lp);
    }
    static LRESULT CALLBACK proc(HWND h,UINT m,WPARAM w,LPARAM l){SettingsDialog* d=reinterpret_cast<SettingsDialog*>(GetWindowLongPtrW(h,GWLP_USERDATA));if(m==WM_NCCREATE){d=static_cast<SettingsDialog*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);d->window=h;SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(d));}return d?d->event(m,w,l):DefWindowProcW(h,m,w,l);}
public:
    SettingsDialog(HWND parent,const ControllerSettings& controls,const InterfaceTheme& theme):owner(parent),workingControls(controls),workingTheme(theme){
        xinput=LoadLibraryExW(L"xinput1_4.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!xinput)xinput=LoadLibraryExW(L"xinput9_1_0.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(xinput){auto address=GetProcAddress(xinput,"XInputGetState");static_assert(sizeof(xget)==sizeof(address));memcpy(&xget,&address,sizeof(xget));}
    }
    ~SettingsDialog(){if(xinput)FreeLibrary(xinput);if(font)DeleteObject(font);if(bold)DeleteObject(bold);if(tiny)DeleteObject(tiny);if(panelBrush)DeleteObject(panelBrush);}
    bool show(ControllerSettings& controls,InterfaceTheme& theme,bool appearance=false){
        static bool registered=false;if(!registered){WNDCLASSEXW wc{};wc.cbSize=sizeof(wc);wc.lpfnWndProc=proc;wc.hInstance=GetModuleHandleW(nullptr);wc.lpszClassName=L"AiloEMUSettings";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=GetSysColorBrush(COLOR_WINDOW);if(!RegisterClassExW(&wc)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return false;registered=true;}
        RECT parent{};GetWindowRect(owner,&parent);int w=980,h=720,x=parent.left+(parent.right-parent.left-w)/2,y=parent.top+(parent.bottom-parent.top-h)/2;
        EnableWindow(owner,FALSE);window=CreateWindowExW(WS_EX_DLGMODALFRAME|WS_EX_CONTROLPARENT,L"AiloEMUSettings",L"AiloEMU - Impostazioni",WS_CAPTION|WS_SYSMENU|WS_VISIBLE,x,y,w,h,owner,nullptr,GetModuleHandleW(nullptr),this);
        if(!window){EnableWindow(owner,TRUE);return false;}if(appearance)page(false);
        MSG msg{};while(!done&&GetMessageW(&msg,nullptr,0,0)>0){
            if(capture>=0&&editKeyboard&&(msg.message==WM_KEYDOWN||msg.message==WM_SYSKEYDOWN)){SendMessageW(window,msg.message,msg.wParam,msg.lParam);continue;}
            if(!IsDialogMessageW(window,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}
        }
        EnableWindow(owner,TRUE);SetForegroundWindow(owner);if(accepted){controls=workingControls;theme=workingTheme;}return accepted;
    }
};
