// Additional core-linking permission: ../licenses/AiloEMU-Core-Linking-Exception.txt
// AiloEMU 2.2.4 - Windows frontend. GPL-2.0-or-later.
#ifndef UNICODE
#define UNICODE
#endif
#define _UNICODE
#define NOMINMAX
#include "core.hpp"
#include "diagnostics.hpp"
#include "resampler.hpp"
#include "multisystem_tests.hpp"
#include "settings_ui.hpp"
#include <windowsx.h>
#include <commdlg.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <xinput.h>
#include <chrono>
#include <memory>
#include <cmath>

enum {ID_OPEN=100,ID_DEMO,ID_CLOSE,ID_EXIT,ID_PAUSE,ID_RESET,ID_SAVE,ID_LOAD,
      ID_FULL,ID_PIXEL,ID_MUTE,ID_FOLDER,ID_HELP,ID_ABOUT,ID_SCREENSHOT,
      ID_CONTROLLERS,ID_APPEARANCE,ID_VOLUME=200,ID_SLOT=300,ID_REGION=400,ID_DEMO_SNES=500,ID_DEMO_GB,ID_DEMO_GBC,ID_DEMO_GBA};
using Clock=std::chrono::steady_clock;
static const wchar_t* HELP=
    L"AILOEMU 2.2.1 - SISTEMI A CARTUCCIA SENZA BIOS ESTERNO\n\n"
    L"GIOCATORE 1\nFrecce: movimento | X: A | Z: B\nInvio: Start | Maiusc destro: Select\nC = X, V = Y, Q = L, E = R\n\n"
    L"GIOCATORE 2\nW A S D: movimento | G: A | F: B\nT: Start | R: Select\nH = X, J = Y, Y = L, U = R\n\n"
    L"CONTROLLER E TASTIERA\nXInput, controller generici Windows/WinMM e tastiera. Fino a 4 porte N64.\n"
    L"Impostazioni > Controller mostra il controller pixel art della console:\n"
    L"clicca un suo pulsante e premi il tasto o comando fisico desiderato.\n"
    L"Tastiera e gamepad hanno associazioni separate e funzionano insieme.\n"
    L"Nintendo DS: clic/tocco con il pulsante sinistro del mouse.\n\n"
    L"Ctrl+O: apri ROM | P: pausa | Ctrl+R: reset\nF5: salva stato | F8: carica stato | F11: schermo intero\nM: audio | F12: screenshot | Esc: esci da schermo intero\n\n"
    L"Formati: .nes .sfc .smc .gb .gbc .gba .md .gen .sms .gg .sg .sg1000\n"
    L".32x .a26 .a78 .ngp .ngc .ws .wsc .pce .nds .z64 .n64 .v64 .vb .min.\n"
    L"Estrai prima gli ZIP. Non usare .bin: l'estensione e' ambigua.\n"
    L"Salvataggi batteria/RTC automatici ogni 15 secondi e alla chiusura.";

static std::wstring widen(const std::string& s){
    if(s.empty())return {};
    int n=MultiByteToWideChar(CP_UTF8,0,s.data(),int(s.size()),nullptr,0);
    std::wstring w(n,L'\0');MultiByteToWideChar(CP_UTF8,0,s.data(),int(s.size()),w.data(),n);return w;
}

// waveOut uses only Windows system libraries; sample storage stays alive until WHDR_DONE.
class AudioOut {
    HWAVEOUT device=nullptr;
    Resampler resampler;
    struct Block{WAVEHDR header{};std::array<int16_t,8192> samples{};bool prepared=false;};
    std::array<Block,8> blocks{};
public:
    bool open(unsigned rate){
        close();WAVEFORMATEX format{};format.wFormatTag=WAVE_FORMAT_PCM;format.nChannels=2;
        format.nSamplesPerSec=rate;format.wBitsPerSample=16;format.nBlockAlign=4;format.nAvgBytesPerSec=rate*4;
        return waveOutOpen(&device,WAVE_MAPPER,&format,0,0,CALLBACK_NULL)==MMSYSERR_NOERROR;
    }
    bool available()const{return device!=nullptr;}
    void clear(){resampler.clear();if(device)waveOutReset(device);}
    void close(){
        resampler.clear();
        if(!device)return;
        waveOutReset(device);
        for(auto& b:blocks)if(b.prepared){waveOutUnprepareHeader(device,&b.header,sizeof(b.header));b.prepared=false;b.header={};}
        waveOutClose(device);device=nullptr;
    }
    void submit(const std::vector<int16_t>& source,int volume,double sourceRate){
        if(!device||source.empty())return;
        auto data=resampler.convert(source,sourceRate);
        size_t offset=0;
        while(offset<data.size()){
            Block* free=nullptr;unsigned pending=0;
            for(auto& b:blocks){
                if(!b.prepared||(b.header.dwFlags&WHDR_DONE)){if(!free)free=&b;}
                else ++pending;
            }
            if(!free||pending>=5)return; // bound latency after a slow frame
            auto& b=*free;
            if(b.prepared)waveOutUnprepareHeader(device,&b.header,sizeof(b.header));
            size_t n=std::min(data.size()-offset,b.samples.size());
            for(size_t i=0;i<n;++i)b.samples[i]=int16_t(int(data[offset+i])*volume/100);
            b.header={};b.header.lpData=reinterpret_cast<LPSTR>(b.samples.data());b.header.dwBufferLength=DWORD(n*2);
            b.prepared=waveOutPrepareHeader(device,&b.header,sizeof(b.header))==MMSYSERR_NOERROR;
            if(b.prepared&&waveOutWrite(device,&b.header,sizeof(b.header))!=MMSYSERR_NOERROR){clear();return;}
            offset+=n;
        }
    }
    ~AudioOut(){close();}
};

class App {
    HWND window=nullptr;HMENU menu=nullptr;HFONT titleFont=nullptr,bodyFont=nullptr,smallFont=nullptr;
    HBRUSH background=nullptr;
    std::unique_ptr<Core> core;AudioOut audio;
    fs::path base,dataRoot,ini;
    HMODULE xinput=nullptr;
    using XInputFn=DWORD(WINAPI*)(DWORD,XINPUT_STATE*);
    XInputFn xget=nullptr;
    bool paused=false,focused=true,modal=false,full=false,pixel=false,muted=false,quitting=false;
    unsigned slot=1,region=0;int volume=75;
    ControllerSettings controllerSettings;InterfaceTheme theme;
    RECT oldRect{};LONG_PTR oldStyle=0;
    std::wstring notice=L"Pronto. Apri una ROM o prova la demo inclusa.";
    Clock::time_point noticeUntil=Clock::now()+std::chrono::seconds(6),nextFrame=Clock::now(),lastSave=Clock::now();
    bool running()const{return core&&core->loaded&&!paused&&focused&&!modal&&!IsIconic(window);}
    void message(const std::wstring& s){notice=s;noticeUntil=Clock::now()+std::chrono::seconds(5);InvalidateRect(window,nullptr,FALSE);}
    void dialog(const std::wstring& text,const wchar_t* title=L"AiloEMU",UINT flags=MB_OK|MB_ICONINFORMATION){
        bool before=modal;modal=true;audio.clear();MessageBoxW(window,text.c_str(),title,flags);modal=before;nextFrame=Clock::now();
    }
    void error(const std::exception& e){dialog(widen(e.what()),L"AiloEMU - Attenzione",MB_OK|MB_ICONWARNING);}
    bool ask(const wchar_t* text){
        modal=true;audio.clear();int result=MessageBoxW(window,text,L"AiloEMU",MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2);
        modal=false;nextFrame=Clock::now();return result==IDYES;
    }
    void readSettings(){
        volume=std::clamp(int(GetPrivateProfileIntW(L"Audio",L"Volume",75,ini.c_str())),0,100);
        muted=GetPrivateProfileIntW(L"Audio",L"Mute",0,ini.c_str())!=0;
        pixel=GetPrivateProfileIntW(L"Video",L"PixelInteger",0,ini.c_str())!=0;
        region=std::min(2u,GetPrivateProfileIntW(L"System",L"Region",0,ini.c_str()));
        controllerSettings.load(ini);theme.load(ini);
    }
    void settings(){
        auto v=std::to_wstring(volume);
        WritePrivateProfileStringW(L"Audio",L"Volume",v.c_str(),ini.c_str());
        WritePrivateProfileStringW(L"Audio",L"Mute",muted?L"1":L"0",ini.c_str());
        WritePrivateProfileStringW(L"Video",L"PixelInteger",pixel?L"1":L"0",ini.c_str());
        auto r=std::to_wstring(region);WritePrivateProfileStringW(L"System",L"Region",r.c_str(),ini.c_str());
        controllerSettings.save(ini);theme.save(ini);
    }
    void input(){
        core->buttons={};core->analog={};
        if(!running())return;
        for(unsigned p=0;p<consolePlayers(core->kind);++p){
            const PadProfile& profile=controllerSettings.at(core->kind,p);int device=profile.device;
            if(profile.keyboardEnabled)for(size_t a=0;a<ACTION_COUNT;a++){
                int binding=profile.keys[a];if(binding>=PB_KEY&&binding<PB_KEY+256&&(GetAsyncKeyState(binding-PB_KEY)&0x8000))core->buttons[p]|=uint16_t(1u<<ACTION_RETRO_IDS[a]);
            }
            if(device==-1)device=int(p+1);
            if(device>=1&&device<=4){
                XINPUT_STATE state{};
                if(xget&&xget(unsigned(device-1),&state)==ERROR_SUCCESS){
                    const auto& g=state.Gamepad;
                    for(size_t a=0;a<ACTION_COUNT;a++)if(xinputBindingPressed(g,profile.binding[a]))core->buttons[p]|=uint16_t(1u<<ACTION_RETRO_IDS[a]);
                    core->analog[p][0]=g.sThumbLX;core->analog[p][1]=int16_t(-g.sThumbLY);
                    core->analog[p][2]=g.sThumbRX;core->analog[p][3]=int16_t(-g.sThumbRY);
                }
            }else if(device>=100){
                JOYINFOEX state{};state.dwSize=sizeof(state);state.dwFlags=JOY_RETURNALL;
                if(joyGetPosEx(UINT(device-100),&state)==JOYERR_NOERROR){
                    for(size_t a=0;a<ACTION_COUNT;a++)if(joyBindingPressed(state,profile.binding[a]))core->buttons[p]|=uint16_t(1u<<ACTION_RETRO_IDS[a]);
                    JOYCAPSW caps{};
                    if(joyGetDevCapsW(UINT(device-100),&caps,sizeof(caps))==JOYERR_NOERROR){
                        auto normalized=[](DWORD value,UINT low,UINT high){
                            if(high<=low)return int16_t(0);
                            double n=(double(value)-double(low))*65534.0/double(high-low)-32767.0;
                            int result=std::clamp(int(std::lround(n)),-32767,32767);return int16_t(std::abs(result)<4096?0:result);
                        };
                        core->analog[p][0]=normalized(state.dwXpos,caps.wXmin,caps.wXmax);core->analog[p][1]=normalized(state.dwYpos,caps.wYmin,caps.wYmax);
                        if((caps.wCaps&JOYCAPS_HASZ)&&(caps.wCaps&JOYCAPS_HASR)){
                            core->analog[p][2]=normalized(state.dwZpos,caps.wZmin,caps.wZmax);core->analog[p][3]=normalized(state.dwRpos,caps.wRmin,caps.wRmax);
                        }
                    }
                }
            }
            if(!core->analog[p][0]&&!core->analog[p][1]){
                if(core->buttons[p]&(1u<<RETRO_DEVICE_ID_JOYPAD_LEFT))core->analog[p][0]=-32767;
                if(core->buttons[p]&(1u<<RETRO_DEVICE_ID_JOYPAD_RIGHT))core->analog[p][0]=32767;
                if(core->buttons[p]&(1u<<RETRO_DEVICE_ID_JOYPAD_UP))core->analog[p][1]=-32767;
                if(core->buttons[p]&(1u<<RETRO_DEVICE_ID_JOYPAD_DOWN))core->analog[p][1]=32767;
            }
            // Never send impossible opposite directions.
            for(unsigned mask:{(1u<<4)|(1u<<5),(1u<<6)|(1u<<7)})
                if((core->buttons[p]&mask)==mask)core->buttons[p]&=uint16_t(~mask);
        }
    }
    void menus(){
        if(!menu)return;
        bool loaded=core&&core->loaded;
        for(UINT id:{ID_CLOSE,ID_PAUSE,ID_RESET,ID_SAVE,ID_LOAD,ID_SCREENSHOT})EnableMenuItem(menu,id,MF_BYCOMMAND|(loaded?MF_ENABLED:MF_GRAYED));
        CheckMenuItem(menu,ID_PAUSE,MF_BYCOMMAND|(paused?MF_CHECKED:MF_UNCHECKED));
        CheckMenuItem(menu,ID_MUTE,MF_BYCOMMAND|(muted?MF_CHECKED:MF_UNCHECKED));
        CheckMenuItem(menu,ID_PIXEL,MF_BYCOMMAND|(pixel?MF_CHECKED:MF_UNCHECKED));
        CheckMenuItem(menu,ID_FULL,MF_BYCOMMAND|(full?MF_CHECKED:MF_UNCHECKED));
        CheckMenuRadioItem(menu,ID_SLOT+1,ID_SLOT+5,ID_SLOT+slot,MF_BYCOMMAND);
        for(UINT id=ID_REGION;id<=ID_REGION+2;id++)EnableMenuItem(menu,id,MF_BYCOMMAND|(core&&hasRegionMenu(core->kind)?MF_ENABLED:MF_GRAYED));
        CheckMenuRadioItem(menu,ID_REGION,ID_REGION+2,ID_REGION+region,MF_BYCOMMAND);
        CheckMenuRadioItem(menu,ID_VOLUME+25,ID_VOLUME+100,ID_VOLUME+volume,MF_BYCOMMAND);
    }
    void buildMenu(){
        menu=CreateMenu();auto file=CreatePopupMenu(),game=CreatePopupMenu(),video=CreatePopupMenu(),sound=CreatePopupMenu(),configuration=CreatePopupMenu(),help=CreatePopupMenu(),slots=CreatePopupMenu();
        AppendMenuW(file,MF_STRING,ID_OPEN,L"&Apri ROM...\tCtrl+O");AppendMenuW(file,MF_STRING,ID_DEMO,L"Demo NES originale");
        AppendMenuW(file,MF_STRING,ID_DEMO_SNES,L"Demo SNES originale");
        AppendMenuW(file,MF_STRING,ID_DEMO_GB,L"Demo Game Boy originale");
        AppendMenuW(file,MF_STRING,ID_DEMO_GBC,L"Demo Game Boy Color originale");
        AppendMenuW(file,MF_STRING,ID_DEMO_GBA,L"Demo GBA originale");
        AppendMenuW(file,MF_STRING,ID_CLOSE,L"Chiudi ROM");AppendMenuW(file,MF_SEPARATOR,0,nullptr);
        AppendMenuW(file,MF_STRING,ID_FOLDER,L"Apri cartella salvataggi");AppendMenuW(file,MF_STRING,ID_EXIT,L"Esci");
        AppendMenuW(game,MF_STRING,ID_PAUSE,L"Pausa\tP");AppendMenuW(game,MF_STRING,ID_RESET,L"Reset console\tCtrl+R");
        AppendMenuW(game,MF_SEPARATOR,0,nullptr);AppendMenuW(game,MF_STRING,ID_SAVE,L"Salva stato\tF5");AppendMenuW(game,MF_STRING,ID_LOAD,L"Carica stato\tF8");
        for(unsigned s=1;s<=5;s++){auto label=L"Slot "+std::to_wstring(s);AppendMenuW(slots,MF_STRING,ID_SLOT+s,label.c_str());}
        AppendMenuW(game,MF_POPUP,reinterpret_cast<UINT_PTR>(slots),L"Slot di salvataggio");
        auto regions=CreatePopupMenu();AppendMenuW(regions,MF_STRING,ID_REGION,L"Automatico");
        AppendMenuW(regions,MF_STRING,ID_REGION+1,L"NTSC (60 Hz)");AppendMenuW(regions,MF_STRING,ID_REGION+2,L"PAL (50 Hz)");
        AppendMenuW(game,MF_POPUP,reinterpret_cast<UINT_PTR>(regions),L"Regione della console");
        AppendMenuW(video,MF_STRING,ID_FULL,L"Schermo intero\tF11");AppendMenuW(video,MF_STRING,ID_PIXEL,L"Pixel interi (1:1)");
        AppendMenuW(video,MF_STRING,ID_SCREENSHOT,L"Screenshot BMP\tF12");
        AppendMenuW(sound,MF_STRING,ID_MUTE,L"Disattiva audio\tM");AppendMenuW(sound,MF_SEPARATOR,0,nullptr);
        for(int v:{25,50,75,100}){auto label=std::to_wstring(v)+L"%";AppendMenuW(sound,MF_STRING,ID_VOLUME+v,label.c_str());}
        AppendMenuW(configuration,MF_STRING,ID_CONTROLLERS,L"Controller e comandi...");
        AppendMenuW(configuration,MF_STRING,ID_APPEARANCE,L"Colori dell'interfaccia...");
        AppendMenuW(help,MF_STRING,ID_HELP,L"Controlli e istruzioni\tF1");AppendMenuW(help,MF_STRING,ID_ABOUT,L"Informazioni e crediti");
        AppendMenuW(menu,MF_POPUP,reinterpret_cast<UINT_PTR>(file),L"&File");AppendMenuW(menu,MF_POPUP,reinterpret_cast<UINT_PTR>(game),L"&Gioco");
        AppendMenuW(menu,MF_POPUP,reinterpret_cast<UINT_PTR>(video),L"&Video");AppendMenuW(menu,MF_POPUP,reinterpret_cast<UINT_PTR>(sound),L"&Audio");
        AppendMenuW(menu,MF_POPUP,reinterpret_cast<UINT_PTR>(configuration),L"&Impostazioni");AppendMenuW(menu,MF_POPUP,reinterpret_cast<UINT_PTR>(help),L"&Aiuto");
        SetMenu(window,menu);menus();
    }
    void openGame(const fs::path& p){
        Console type=detectConsole(p);
        validateROM(readBytes(p),type);
        audio.clear();core->message.clear();
        if(core->kind!=type){
            fs::path dll=base/coreFile(type);
            if(!fs::exists(dll))throw std::runtime_error("DLL mancante: "+dll.filename().u8string());
            core->close();core.reset();core=std::make_unique<Core>();
            fs::path saves=dataRoot/L"saves";
            if(*saveFolder(type))saves/=saveFolder(type);
            try{core=std::make_unique<Core>(dll,saves,dataRoot/L"system",type);}
            catch(...){menus();InvalidateRect(window,nullptr,FALSE);throw;}
        }
        core->setRegion(region==1?"NTSC":region==2?"PAL":"Auto");
        try{core->load(p);}catch(...){menus();SetWindowTextW(window,L"AiloEMU");InvalidateRect(window,nullptr,FALSE);throw;}
        paused=false;nextFrame=Clock::now();lastSave=Clock::now();
        auto title=L"AiloEMU 2.2.4  |  "+widen(consoleName(type))+L"  |  "+p.filename().wstring();SetWindowTextW(window,title.c_str());
        bool sound=audio.open(48000);
        menus();
        if(!sound)message(L"ROM avviata. Dispositivo audio non disponibile: controlla l'uscita audio di Windows.");
        else if(!core->message.empty())message(widen(core->message));
        else message(p.filename().wstring()+L"  |  "+widen(consoleName(type))+L"  |  "+std::to_wstring(int(std::lround(core->av.timing.fps)))+L" Hz");
        core->message.clear();
    }
    void chooseGame(){
        wchar_t path[32768]={};OPENFILENAMEW ofn{};ofn.lStructSize=sizeof(ofn);ofn.hwndOwner=window;
        ofn.lpstrFilter=L"Tutte le ROM supportate\0*.nes;*.sfc;*.smc;*.gb;*.gbc;*.gba;*.md;*.mdx;*.gen;*.smd;*.68k;*.sgd;*.sms;*.gg;*.sg;*.sg1000;*.32x;*.pco;*.a26;*.a78;*.cdf;*.ngp;*.ngc;*.ngpc;*.npc;*.ws;*.wsc;*.pc2;*.pce;*.nds;*.z64;*.n64;*.v64;*.u1;*.vb;*.vboy;*.min\0Nintendo\0*.nes;*.sfc;*.smc;*.gb;*.gbc;*.gba;*.nds;*.z64;*.n64;*.v64;*.u1\0Sega\0*.md;*.mdx;*.gen;*.smd;*.68k;*.sgd;*.sms;*.gg;*.sg;*.sg1000;*.32x;*.pco\0Atari\0*.a26;*.a78;*.cdf\0Altri portatili\0*.ngp;*.ngc;*.ngpc;*.npc;*.ws;*.wsc;*.pc2;*.vb;*.vboy;*.min\0PC Engine\0*.pce\0\0";
        ofn.lpstrFile=path;ofn.nMaxFile=32768;ofn.lpstrTitle=L"Apri una ROM ottenuta legalmente";
        ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_EXPLORER;
        std::wstring initial=(base/L"roms").wstring();ofn.lpstrInitialDir=initial.c_str();
        modal=true;audio.clear();BOOL selected=GetOpenFileNameW(&ofn);modal=false;nextFrame=Clock::now();
        if(selected)openGame(path);
    }
    void fullscreen(){
        full=!full;
        if(full){
            GetWindowRect(window,&oldRect);oldStyle=GetWindowLongPtrW(window,GWL_STYLE);
            MONITORINFO monitor{};monitor.cbSize=sizeof(monitor);GetMonitorInfoW(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&monitor);
            SetMenu(window,nullptr);SetWindowLongPtrW(window,GWL_STYLE,oldStyle&~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window,HWND_TOP,monitor.rcMonitor.left,monitor.rcMonitor.top,
                monitor.rcMonitor.right-monitor.rcMonitor.left,monitor.rcMonitor.bottom-monitor.rcMonitor.top,SWP_FRAMECHANGED);
        }else{
            SetWindowLongPtrW(window,GWL_STYLE,oldStyle);SetMenu(window,menu);
            SetWindowPos(window,nullptr,oldRect.left,oldRect.top,oldRect.right-oldRect.left,oldRect.bottom-oldRect.top,SWP_FRAMECHANGED|SWP_NOZORDER);
        }
        menus();InvalidateRect(window,nullptr,FALSE);
    }
    void screenshot(){
        if(core->pixels.empty())return;
        BITMAPFILEHEADER file{};BITMAPINFOHEADER info{};
        info.biSize=sizeof(info);info.biWidth=LONG(core->width);info.biHeight=-LONG(core->height);
        info.biPlanes=1;info.biBitCount=32;info.biSizeImage=DWORD(core->pixels.size()*4);
        file.bfType=0x4D42;file.bfOffBits=sizeof(file)+sizeof(info);file.bfSize=file.bfOffBits+info.biSizeImage;
        Bytes b(file.bfSize);memcpy(b.data(),&file,sizeof(file));memcpy(b.data()+sizeof(file),&info,sizeof(info));memcpy(b.data()+file.bfOffBits,core->pixels.data(),info.biSizeImage);
        SYSTEMTIME t{};GetLocalTime(&t);wchar_t name[128];
        swprintf(name,128,L"Ailo-%04u%02u%02u-%02u%02u%02u-%03u.bmp",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
        writeAtomic(dataRoot/L"screenshots"/name,b.data(),b.size());message(L"Screenshot salvato in screenshots.");
    }
    void closeGame(){
        core->close();audio.close();
        paused=false;SetWindowTextW(window,L"AiloEMU");menus();message(L"ROM chiusa. I salvataggi batteria sono stati conservati.");
    }
    void configure(bool appearance){
        modal=true;audio.clear();SettingsDialog dialog(window,controllerSettings,theme);
        if(dialog.show(controllerSettings,theme,appearance)){
            HBRUSH next=CreateSolidBrush(theme.background);SetClassLongPtrW(window,GCLP_HBRBACKGROUND,reinterpret_cast<LONG_PTR>(next));
            if(background)DeleteObject(background);
            background=next;settings();InvalidateRect(window,nullptr,TRUE);
            message(appearance?L"Colori dell'interfaccia aggiornati.":L"Configurazione controller aggiornata.");
        }
        modal=false;nextFrame=Clock::now();
    }
    void command(unsigned id){
        if(id>=ID_REGION&&id<=ID_REGION+2){
            if(core->loaded&&!hasRegionMenu(core->kind))return;
            if(core->loaded&&!ask(L"Cambiare regione riavvia il gioco. Continuare?"))return;
            region=id-ID_REGION;menus();settings();
            if(core->loaded){auto path=core->gamePath;openGame(path);}return;
        }
        if(id>ID_SLOT&&id<=ID_SLOT+5){slot=id-ID_SLOT;menus();message(L"Selezionato slot "+std::to_wstring(slot));return;}
        if(id>=ID_VOLUME+25&&id<=ID_VOLUME+100){volume=int(id-ID_VOLUME);muted=false;menus();settings();return;}
        switch(id){
        case ID_OPEN:chooseGame();break;
        case ID_DEMO:openGame(base/L"roms"/L"Ailo-Demo.nes");break;
        case ID_DEMO_SNES:openGame(base/L"roms"/L"Ailo-SNES-Demo.sfc");break;
        case ID_DEMO_GB:openGame(base/L"roms"/L"Ailo-GB-Demo.gb");break;
        case ID_DEMO_GBC:openGame(base/L"roms"/L"Ailo-GBC-Demo.gbc");break;
        case ID_DEMO_GBA:openGame(base/L"roms"/L"Ailo-GBA-Demo.gba");break;
        case ID_CLOSE:closeGame();break;
        case ID_EXIT:SendMessageW(window,WM_CLOSE,0,0);break;
        case ID_PAUSE:if(core->loaded){paused=!paused;audio.clear();nextFrame=Clock::now();menus();message(paused?L"In pausa. Premi P per continuare.":L"Ripresa.");}break;
        case ID_RESET:if(core->loaded&&ask(L"Riavviare la console? I progressi non salvati nel gioco andranno persi.")){core->saveRAM();core->restart();audio.clear();message(L"Console riavviata.");}break;
        case ID_SAVE:if(core->loaded){core->saveState(slot);message(L"Stato salvato nello slot "+std::to_wstring(slot));}break;
        case ID_LOAD:if(core->loaded){core->loadState(slot);audio.clear();nextFrame=Clock::now();message(L"Stato caricato dallo slot "+std::to_wstring(slot));}break;
        case ID_FULL:fullscreen();break;
        case ID_PIXEL:pixel=!pixel;menus();settings();InvalidateRect(window,nullptr,FALSE);break;
        case ID_MUTE:muted=!muted;audio.clear();menus();settings();message(muted?L"Audio disattivato.":L"Audio attivato.");break;
        case ID_FOLDER:ShellExecuteW(window,L"open",(dataRoot/L"saves").c_str(),nullptr,nullptr,SW_SHOWNORMAL);break;
        case ID_SCREENSHOT:screenshot();break;
        case ID_CONTROLLERS:configure(false);break;
        case ID_APPEARANCE:configure(true);break;
        case ID_HELP:dialog(HELP,L"AiloEMU - Controlli");break;
        case ID_ABOUT:dialog(L"AiloEMU 2.2.4 - Windows x64\n\n21 sistemi a cartuccia, selezionati automaticamente.\nController pixel art interattivi, tastiera e gamepad configurabili insieme.\nTemi colore personalizzabili e nessun BIOS esterno richiesto.\n\nInterfaccia C++ personalizzata; nuclei libretro di progetti\nindipendenti. Sorgenti, revisioni e licenze inclusi.\nSnes9x, Genesis Plus GX e PicoDrive: solo uso non commerciale.\n\nNon include giochi commerciali, firmware proprietario o BIOS.",L"Informazioni su AiloEMU");break;
        }
    }
    static void rectangle(HDC dc,RECT r,COLORREF color){HBRUSH brush=CreateSolidBrush(color);FillRect(dc,&r,brush);DeleteObject(brush);}
    void text(HDC dc,const std::wstring& s,RECT r,COLORREF color,HFONT font,UINT align=DT_LEFT|DT_VCENTER|DT_SINGLELINE){
        auto old=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,color);DrawTextW(dc,s.c_str(),-1,&r,align);SelectObject(dc,old);
    }
    RECT gameRect()const{
        RECT r{};GetClientRect(window,&r);int bottom=r.bottom-(full?0:36);
        if(!core||!core->loaded||!core->width||!core->height)return {0,0,r.right,bottom};
        bool nativeAspect=isHandheld(core->kind)||core->kind==Console::VirtualBoy;
        float ratio=pixel||nativeAspect?float(core->width)/core->height:4.0f/3.0f;
        int w=r.right,h=int(w/ratio);if(h>bottom){h=bottom;w=int(h*ratio);}
        if(pixel){int scale=std::min(int(r.right)/int(core->width),bottom/int(core->height));if(scale>=1){w=int(core->width)*scale;h=int(core->height)*scale;}}
        return {(r.right-w)/2,(bottom-h)/2,(r.right+w)/2,(bottom+h)/2};
    }
    void pointer(LPARAM lp,bool pressed){
        if(!core||!core->loaded||core->kind!=Console::NintendoDS)return;
        RECT v=gameRect();int x=GET_X_LPARAM(lp),y=GET_Y_LPARAM(lp);
        x=std::clamp(x,int(v.left),int(v.right-1));y=std::clamp(y,int(v.top),int(v.bottom-1));
        int64_t nx=int64_t(x-v.left)*65535/std::max(1L,v.right-v.left-1)-32768;
        int64_t ny=int64_t(y-v.top)*65535/std::max(1L,v.bottom-v.top-1)-32768;
        core->pointerX=int16_t(nx);core->pointerY=int16_t(ny);core->pointerPressed=pressed;
    }
    void paint(){
        PAINTSTRUCT ps{};HDC screen=BeginPaint(window,&ps);RECT r;GetClientRect(window,&r);
        if(r.right<=0||r.bottom<=0){EndPaint(window,&ps);return;}
        HDC dc=CreateCompatibleDC(screen);HBITMAP bitmap=CreateCompatibleBitmap(screen,r.right,r.bottom);auto old=SelectObject(dc,bitmap);
        rectangle(dc,r,theme.background);int footer=full?0:36;
        RECT viewport={0,0,r.right,r.bottom-footer};
        if(core&&core->loaded&&!core->pixels.empty()){
            RECT game=gameRect();int x=game.left,y=game.top,w=game.right-game.left,h=game.bottom-game.top;
            BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=LONG(core->width);info.bmiHeader.biHeight=-LONG(core->height);
            info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;SetStretchBltMode(dc,COLORONCOLOR);
            StretchDIBits(dc,x,y,w,h,0,0,core->width,core->height,core->pixels.data(),&info,DIB_RGB_COLORS,SRCCOPY);
            if(paused||!focused){RECT badge={viewport.right/2-130,viewport.bottom/2-24,viewport.right/2+130,viewport.bottom/2+24};rectangle(dc,badge,theme.panel);text(dc,L"IN PAUSA",badge,theme.accent,bodyFont,DT_CENTER|DT_VCENTER|DT_SINGLELINE);}
        }else{
            int cx=r.right/2,cy=(r.bottom-footer)/2;
            RECT tag={cx-64,cy-155,cx+64,cy-115};rectangle(dc,tag,theme.accent);text(dc,L"21 SISTEMI",tag,theme.background,bodyFont,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            text(dc,L"AILOEMU",{20,cy-98,r.right-20,cy-28},theme.primary,titleFont,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            text(dc,L"NINTENDO  /  SEGA  /  ATARI  /  NEC  /  SNK",{10,cy-20,r.right-10,cy+12},theme.secondary,bodyFont,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            RECT button={cx-160,cy+42,cx+160,cy+98};rectangle(dc,button,theme.accent);text(dc,L"APRI ROM   /   CTRL + O",button,theme.background,bodyFont,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            text(dc,L"Trascina qui una ROM estratta e ottenuta legalmente.",{10,cy+112,r.right-10,cy+142},theme.secondary,smallFont,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            text(dc,L"File > Demo  per provare subito",{10,cy+154,r.right-10,cy+184},theme.accent,smallFont,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        }
        if(!full){
            RECT status={0,r.bottom-footer,r.right,r.bottom};rectangle(dc,status,theme.panel);
            std::wstring s=Clock::now()<noticeUntil?notice:L"Ctrl+O Apri   |   P Pausa   |   F5 Salva   |   F8 Carica   |   F1 Controlli";
            text(dc,s,{14,status.top,r.right-110,status.bottom},theme.primary,smallFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
            text(dc,L"SLOT "+std::to_wstring(slot),{r.right-96,status.top,r.right-12,status.bottom},theme.accent,smallFont,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
        }else if(Clock::now()<noticeUntil){
            RECT overlay={12,12,std::min(r.right-12,850L),48};rectangle(dc,overlay,theme.panel);overlay.left+=10;text(dc,notice,overlay,theme.primary,smallFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
        }
        BitBlt(screen,0,0,r.right,r.bottom,dc,0,0,SRCCOPY);SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);EndPaint(window,&ps);
    }
    LRESULT event(UINT msg,WPARAM wp,LPARAM lp){
        switch(msg){
        case WM_COMMAND:command(LOWORD(wp));return 0;
        case WM_PAINT:paint();return 0;
        case WM_ERASEBKGND:return 1;
        case WM_SIZE:InvalidateRect(window,nullptr,FALSE);return 0;
        case WM_GETMINMAXINFO:{auto* m=reinterpret_cast<MINMAXINFO*>(lp);m->ptMinTrackSize={560,520};return 0;}
        case WM_ACTIVATEAPP:focused=wp!=0;if(!focused){audio.clear();if(core)core->buttons={};}nextFrame=Clock::now();InvalidateRect(window,nullptr,FALSE);return 0;
        case WM_ENTERMENULOOP:case WM_ENTERSIZEMOVE:modal=true;audio.clear();return 0;
        case WM_EXITMENULOOP:case WM_EXITSIZEMOVE:modal=false;nextFrame=Clock::now();return 0;
        case WM_DROPFILES:{
            HDROP drop=reinterpret_cast<HDROP>(wp);UINT n=DragQueryFileW(drop,0,nullptr,0);std::wstring path(n+1,L'\0');
            DragQueryFileW(drop,0,path.data(),n+1);DragFinish(drop);path.resize(n);if(n)openGame(path);return 0;
        }
        case WM_MOUSEMOVE:if(core&&core->kind==Console::NintendoDS)pointer(lp,(wp&MK_LBUTTON)!=0);return 0;
        case WM_LBUTTONDOWN:if(core&&core->loaded){pointer(lp,true);SetCapture(window);}return 0;
        case WM_LBUTTONUP:
            if(core&&core->loaded){pointer(lp,false);if(GetCapture()==window)ReleaseCapture();}
            else chooseGame();
            return 0;
        case WM_CAPTURECHANGED:if(core)core->pointerPressed=false;return 0;
        case WM_SYSKEYDOWN:if(wp==VK_RETURN&&!(lp&(1LL<<30))){fullscreen();return 0;}break;
        case WM_KEYDOWN:{
            if(lp&(1LL<<30))return 0;
            bool ctrl=(GetKeyState(VK_CONTROL)&0x8000)!=0;
            if(ctrl&&wp=='O')command(ID_OPEN);else if(ctrl&&wp=='R')command(ID_RESET);
            else if(wp==VK_F1)command(ID_HELP);else if(wp==VK_F5)command(ID_SAVE);
            else if(wp==VK_F8)command(ID_LOAD);else if(wp==VK_F11)command(ID_FULL);
            else if(wp==VK_F12)command(ID_SCREENSHOT);else if(wp==VK_ESCAPE&&full)fullscreen();
            else if(wp=='P')command(ID_PAUSE);else if(wp=='M')command(ID_MUTE);
            return 0;
        }
        case WM_CLOSE:
            try{if(core&&core->loaded)core->saveRAM();}catch(const std::exception& e){error(e);if(!ask(L"Il salvataggio non e' riuscito. Uscire comunque?"))return 0;}
            settings();audio.close();quitting=true;DestroyWindow(window);return 0;
        case WM_DESTROY:PostQuitMessage(0);return 0;
        }
        return DefWindowProcW(window,msg,wp,lp);
    }
    static LRESULT CALLBACK windowProc(HWND h,UINT msg,WPARAM wp,LPARAM lp){
        App* a=reinterpret_cast<App*>(GetWindowLongPtrW(h,GWLP_USERDATA));
        if(msg==WM_NCCREATE){a=static_cast<App*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);a->window=h;SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(a));}
        if(a){try{return a->event(msg,wp,lp);}catch(const std::exception& e){a->error(e);return 0;}}
        return DefWindowProcW(h,msg,wp,lp);
    }
public:
    explicit App(fs::path folder):base(std::move(folder)),dataRoot(base){
        // Portable first; fall back to the user's local profile if extraction is read-only.
        try{uint8_t v=0;writeAtomic(base/L"saves"/L".write-test",&v,1);fs::remove(base/L"saves"/L".write-test");}
        catch(...){wchar_t local[32768]={};DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",local,32768);if(!n||n>=32768)throw std::runtime_error("Cartella non scrivibile: estrai AiloEMU in una cartella personale.");dataRoot=fs::path(local)/L"AiloEMU";}
        ini=dataRoot/L"AiloEMU.ini";readSettings();
        core=std::make_unique<Core>();
        fs::create_directories(dataRoot/L"saves");
        xinput=LoadLibraryExW(L"xinput1_4.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(!xinput)xinput=LoadLibraryExW(L"xinput9_1_0.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(xinput){auto address=GetProcAddress(xinput,"XInputGetState");static_assert(sizeof(xget)==sizeof(address));memcpy(&xget,&address,sizeof(xget));}
        titleFont=CreateFontW(-48,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        bodyFont=CreateFontW(-18,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        smallFont=CreateFontW(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        background=CreateSolidBrush(theme.background);
    }
    ~App(){if(xinput)FreeLibrary(xinput);DeleteObject(titleFont);DeleteObject(bodyFont);DeleteObject(smallFont);DeleteObject(background);if(menu)DestroyMenu(menu);}
    int start(HINSTANCE instance,const fs::path& initial){
        WNDCLASSEXW wc{};wc.cbSize=sizeof(wc);wc.lpfnWndProc=windowProc;wc.hInstance=instance;
        wc.lpszClassName=L"AiloEMUWindow";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=background;
        wc.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(1));wc.hIconSm=wc.hIcon;
        if(!RegisterClassExW(&wc))throw std::runtime_error("Registrazione finestra non riuscita.");
        window=CreateWindowExW(WS_EX_ACCEPTFILES,wc.lpszClassName,L"AiloEMU",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,900,770,nullptr,nullptr,instance,this);
        if(!window)throw std::runtime_error("Impossibile creare la finestra.");
        buildMenu();ShowWindow(window,SW_SHOW);UpdateWindow(window);
        if(!initial.empty()){try{openGame(initial);}catch(const std::exception& e){error(e);}}
        if(dataRoot!=base)message(L"Salvataggi in %LOCALAPPDATA%\\AiloEMU. La cartella del programma non e' scrivibile.");
        timeBeginPeriod(1);
        MSG msg{};
        while(!quitting){
            while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){if(msg.message==WM_QUIT){quitting=true;break;}TranslateMessage(&msg);DispatchMessageW(&msg);}
            if(quitting)break;
            auto now=Clock::now();
            if(running()&&now>=nextFrame){
                try{
                    input();core->frame();audio.submit(core->audio,muted?0:volume,core->av.timing.sample_rate);InvalidateRect(window,nullptr,FALSE);
                    if(!core->message.empty()){message(widen(core->message));core->message.clear();}
                    if(core->shutdownRequested){paused=true;menus();message(L"La ROM ha richiesto l'arresto.");}
                    if(now-lastSave>std::chrono::seconds(15)){lastSave=now;core->saveRAM();}
                }catch(const std::exception& e){paused=true;menus();error(e);}
                auto duration=std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1.0/core->av.timing.fps));
                nextFrame+=duration;if(Clock::now()-nextFrame>duration*3)nextFrame=Clock::now()+duration;
            }else{
                DWORD wait=running()?DWORD(std::clamp<long long>(std::chrono::duration_cast<std::chrono::milliseconds>(nextFrame-now).count(),1,16)):50;
                MsgWaitForMultipleObjectsEx(0,nullptr,wait,QS_ALLINPUT,MWMO_INPUTAVAILABLE);
            }
        }
        timeEndPeriod(1);return 0;
    }
};

int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,LPWSTR,int){
    SetProcessDPIAware();
    wchar_t exe[32768]={};GetModuleFileNameW(nullptr,exe,32768);fs::path base=fs::path(exe).parent_path();
    int argc=0;LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(),&argc);
    fs::path initial;if(argc>1)initial=argv[1];LocalFree(argv);
    try{
        if(initial==L"--self-test"){
            int nes=runDiagnostics(base/L"fceumm_libretro.dll",base/L"roms"/L"Ailo-Demo.nes",base/L"test-results");
            int extra=runExtraTests(base/L"snes9x_libretro.dll",base/L"mgba_libretro.dll",base/L"roms",base/L"test-results");
            std::ofstream report(base/L"test-results"/L"report.txt",std::ios::app);
            std::ifstream more(base/L"test-results"/L"multisystem.txt");report<<"\nSNES / GB / GBC / GBA / AUDIO\n"<<more.rdbuf();
            return nes||extra;
        }
        App app(base);return app.start(instance,initial);
    }catch(const std::exception& e){MessageBoxW(nullptr,widen(e.what()).c_str(),L"AiloEMU - Errore",MB_OK|MB_ICONERROR);return 1;}
}
