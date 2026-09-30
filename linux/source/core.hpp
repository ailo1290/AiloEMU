// Additional core-linking permission: ../licenses/AiloEMU-Core-Linking-Exception.txt
// AiloEMU frontend - Copyright (C) 2026. GPL-2.0-or-later.
#pragma once
#include "libretro.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#endif
namespace fs = std::filesystem;
using Bytes = std::vector<uint8_t>;
inline Bytes readBytes(const fs::path& p, size_t limit = 512ull*1024*1024) {
    std::ifstream f(p, std::ios::binary | std::ios::ate);
    if (!f) throw std::runtime_error("Impossibile aprire il file.");
    auto n = f.tellg();
    if (n < 0 || uint64_t(n) > limit) throw std::runtime_error("Dimensione del file non valida (massimo 512 MB).");
    Bytes b(static_cast<size_t>(n)); f.seekg(0);
    if (!b.empty() && !f.read(reinterpret_cast<char*>(b.data()), b.size())) throw std::runtime_error("Lettura del file incompleta.");
    return b;
}
inline void writeAtomic(const fs::path& p, const void* data, size_t size) {
    fs::create_directories(p.parent_path());
    fs::path tmp=p; tmp += ".tmp";
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f || !f.write(static_cast<const char*>(data), size) || !f.flush())
            throw std::runtime_error("Salvataggio fallito. Controlla spazio e permessi della cartella.");
    }
#ifdef _WIN32
    if (!MoveFileExW(tmp.c_str(), p.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        throw std::runtime_error("Impossibile completare il salvataggio.");
#else
    fs::rename(tmp,p);
#endif
}
inline uint64_t hash64(const void* ptr, size_t n) {
    uint64_t h=14695981039346656037ULL;
    const auto* b=static_cast<const uint8_t*>(ptr);
    for(size_t i=0;i<n;i++) h=(h^b[i])*1099511628211ULL;
    return h;
}
inline uint32_t crc32(const void* ptr, size_t n) {
    uint32_t c=~0u; const auto* b=static_cast<const uint8_t*>(ptr);
    for(size_t i=0;i<n;i++){c^=b[i];for(int j=0;j<8;j++)c=(c>>1)^(0xEDB88320u & (0u-(c&1)));}
    return ~c;
}
inline std::string hex64(uint64_t v) {
    const char* digits="0123456789abcdef"; std::string s(16,'0');
    for(int i=15;i>=0;--i){s[i]=digits[v&15];v>>=4;} return s;
}
enum class Console { None, NES, SNES, GB, GBC, GBA, Genesis, SMS, GameGear, SG1000,
    Sega32X, Atari2600, Atari7800, NGP, NGPC, WonderSwan, WonderSwanColor,
    PCEngine, NintendoDS, Nintendo64, VirtualBoy, PokemonMini };
inline bool isGameBoy(Console c){return c==Console::GB||c==Console::GBC||c==Console::GBA;}
inline bool isHandheld(Console c){return isGameBoy(c)||c==Console::GameGear||c==Console::NGP||c==Console::NGPC||c==Console::WonderSwan||c==Console::WonderSwanColor||c==Console::NintendoDS||c==Console::VirtualBoy||c==Console::PokemonMini;}
inline unsigned consolePlayers(Console c){return c==Console::Nintendo64?4u:isHandheld(c)?1u:2u;}
inline bool hasRegionMenu(Console c){return c==Console::NES||c==Console::SNES;}
inline const char* saveFolder(Console c){
    switch(c){case Console::SNES:return "snes";case Console::GB:return "gb";case Console::GBC:return "gbc";case Console::GBA:return "gba";
    case Console::Genesis:return "megadrive";case Console::SMS:return "mastersystem";case Console::GameGear:return "gamegear";case Console::SG1000:return "sg1000";case Console::Sega32X:return "32x";
    case Console::Atari2600:return "atari2600";case Console::Atari7800:return "atari7800";case Console::NGP:return "ngp";case Console::NGPC:return "ngpc";
    case Console::WonderSwan:return "wonderswan";case Console::WonderSwanColor:return "wonderswancolor";case Console::PCEngine:return "pcengine";
    case Console::NintendoDS:return "nds";case Console::Nintendo64:return "n64";case Console::VirtualBoy:return "virtualboy";case Console::PokemonMini:return "pokemonmini";default:return "";}
}
inline const char* consoleName(Console c){
    switch(c){case Console::NES:return "NES";case Console::SNES:return "SNES";case Console::GB:return "GB";case Console::GBC:return "GBC";case Console::GBA:return "GBA";
    case Console::Genesis:return "Mega Drive";case Console::SMS:return "Master System";case Console::GameGear:return "Game Gear";case Console::SG1000:return "SG-1000";case Console::Sega32X:return "Sega 32X";
    case Console::Atari2600:return "Atari 2600";case Console::Atari7800:return "Atari 7800";case Console::NGP:return "Neo Geo Pocket";case Console::NGPC:return "Neo Geo Pocket Color";
    case Console::WonderSwan:return "WonderSwan";case Console::WonderSwanColor:return "WonderSwan Color";case Console::PCEngine:return "PC Engine";case Console::NintendoDS:return "Nintendo DS";
    case Console::Nintendo64:return "Nintendo 64";case Console::VirtualBoy:return "Virtual Boy";case Console::PokemonMini:return "Pokemon Mini";default:return "";}
}
inline const char* coreFile(Console c){
#ifdef _WIN32
    constexpr const char* ext=".dll";
#else
    constexpr const char* ext=".so";
#endif
    const char* stem="fceumm_libretro";
    switch(c){case Console::SNES:stem="snes9x_libretro";break;case Console::GB:case Console::GBC:case Console::GBA:stem="mgba_libretro";break;
    case Console::Genesis:case Console::SMS:case Console::GameGear:case Console::SG1000:stem="genesis_plus_gx_libretro";break;case Console::Sega32X:stem="picodrive_libretro";break;
    case Console::Atari2600:stem="stella2014_libretro";break;case Console::Atari7800:stem="prosystem_libretro";break;case Console::NGP:case Console::NGPC:stem="mednafen_ngp_libretro";break;
    case Console::WonderSwan:case Console::WonderSwanColor:stem="mednafen_wswan_libretro";break;case Console::PCEngine:stem="mednafen_pce_fast_libretro";break;
    case Console::NintendoDS:stem="desmume2015_libretro";break;case Console::Nintendo64:stem="parallel_n64_libretro";break;case Console::VirtualBoy:stem="mednafen_vb_libretro";break;
    case Console::PokemonMini:stem="pokemini_libretro";break;default:break;}
    static thread_local std::string filename;
    filename=stem; filename+=ext; return filename.c_str();
}
inline Console detectConsole(const fs::path& p){
    std::string e=p.extension().u8string();
    for(char& ch:e)ch=char(std::tolower(static_cast<unsigned char>(ch)));
    if(e==".nes")return Console::NES;
    if(e==".sfc"||e==".smc")return Console::SNES;
    if(e==".gb")return Console::GB;
    if(e==".gbc")return Console::GBC;
    if(e==".gba")return Console::GBA;
    if(e==".md"||e==".mdx"||e==".gen"||e==".smd"||e==".68k"||e==".sgd")return Console::Genesis;
    if(e==".sms")return Console::SMS;
    if(e==".gg")return Console::GameGear;
    if(e==".sg"||e==".sg1000")return Console::SG1000;
    if(e==".32x"||e==".pco")return Console::Sega32X;
    if(e==".a26")return Console::Atari2600;
    if(e==".a78"||e==".cdf")return Console::Atari7800;
    if(e==".ngp")return Console::NGP;
    if(e==".ngc"||e==".ngpc"||e==".npc")return Console::NGPC;
    if(e==".ws")return Console::WonderSwan;
    if(e==".wsc"||e==".pc2")return Console::WonderSwanColor;
    if(e==".pce")return Console::PCEngine;
    if(e==".nds")return Console::NintendoDS;
    if(e==".z64"||e==".n64"||e==".v64"||e==".u1")return Console::Nintendo64;
    if(e==".vb"||e==".vboy")return Console::VirtualBoy;
    if(e==".min")return Console::PokemonMini;
    throw std::runtime_error("Formato non supportato. Consulta LEGGIMI.txt per l'elenco delle estensioni.");
}
inline void validateROM(const Bytes& data,Console type){
    if(type==Console::NES){
        if(data.size()<16||memcmp(data.data(),"NES\x1a",4))throw std::runtime_error("Intestazione NES non valida.");
        if((data[7]&0x0C)!=0x08){
            size_t expected=16+((data[6]&4)?512:0)+size_t(data[4])*16384+size_t(data[5])*8192;
            if(!data[4]||data.size()<expected)throw std::runtime_error("ROM iNES troncata.");
        }
    }else if(type==Console::SNES){
        if(data.size()<32768||data.size()>16*1024*1024||(data.size()%1024!=0&&data.size()%1024!=512))
            throw std::runtime_error("Dimensione ROM SNES non valida. Usa una cartuccia .sfc/.smc estratta.");
    }else if(type==Console::GB||type==Console::GBC){
        if(data.size()<32768||data.size()>8*1024*1024||data.size()%16384)
            throw std::runtime_error("Dimensione ROM Game Boy non valida.");
    }else if(type==Console::GBA){
        if(data.size()<192||data.size()>32*1024*1024||data[0xB2]!=0x96)
            throw std::runtime_error("ROM GBA troncata o intestazione non valida.");
    }else if(type==Console::Nintendo64){
        if(data.size()<4096||data.size()>64*1024*1024)throw std::runtime_error("Dimensione ROM Nintendo 64 non valida.");
        uint32_t magic=(uint32_t(data[0])<<24)|(uint32_t(data[1])<<16)|(uint32_t(data[2])<<8)|data[3];
        if(magic!=0x80371240&&magic!=0x37804012&&magic!=0x40123780)throw std::runtime_error("Intestazione Nintendo 64 non valida.");
    }else if(type==Console::NintendoDS){
        if(data.size()<512||data.size()>512ull*1024*1024)throw std::runtime_error("Dimensione ROM Nintendo DS non valida.");
        auto le32=[&](size_t offset){return uint32_t(data[offset])|(uint32_t(data[offset+1])<<8)|(uint32_t(data[offset+2])<<16)|(uint32_t(data[offset+3])<<24);};
        const uint32_t arm9Offset=le32(0x20),arm9Size=le32(0x2C);
        const uint32_t arm7Offset=le32(0x30),arm7Size=le32(0x3C);
        auto segmentFits=[&](uint32_t offset,uint32_t size){
            return size>0&&offset>=0x200&&uint64_t(offset)+uint64_t(size)<=data.size();
        };
        if(data[0x12]>3||!segmentFits(arm9Offset,arm9Size)||!segmentFits(arm7Offset,arm7Size))
            throw std::runtime_error("Header Nintendo DS non valido o dump troncato.");
    }else if(data.size()<128||data.size()>64*1024*1024){
        throw std::runtime_error("Dimensione ROM non valida per la console selezionata.");
    }
}
class Core {
    inline static Core* active=nullptr; // libretro itself is a single-instance API
#ifdef _WIN32
    HMODULE module=nullptr;
#else
    void* module=nullptr;
#endif
    bool initialized=false;
    std::map<std::string,std::string> options;
    std::vector<retro_memory_descriptor> memoryMap;
    std::string sysdir,savedir,contentPath,contentDir,contentName,contentExt;
    retro_game_info_ext extended{};
    Bytes rom;
    uint64_t romID=0;
    fs::path saveRoot,systemRoot,contentCache;
    unsigned pixelFormat=RETRO_PIXEL_FORMAT_0RGB1555,rotation=0;
    template<typename T> void bind(T& function,const char* name) {
#ifdef _WIN32
        FARPROC address=GetProcAddress(module,name);
        static_assert(sizeof(function)==sizeof(address));
        memcpy(&function,&address,sizeof(function));
#else
        function=reinterpret_cast<T>(dlsym(module,name));
#endif
        if(!function)throw std::runtime_error(std::string("Funzione del core mancante: ")+name);
    }
#define CORE_API(name) decltype(&retro_##name) name=nullptr;
    CORE_API(init) CORE_API(deinit) CORE_API(api_version)
    CORE_API(set_environment) CORE_API(set_video_refresh) CORE_API(set_audio_sample)
    CORE_API(set_audio_sample_batch) CORE_API(set_input_poll) CORE_API(set_input_state)
    CORE_API(get_system_info) CORE_API(get_system_av_info) CORE_API(set_controller_port_device)
    CORE_API(load_game) CORE_API(unload_game) CORE_API(run) CORE_API(reset)
    CORE_API(serialize_size) CORE_API(serialize) CORE_API(unserialize)
    CORE_API(get_memory_data) CORE_API(get_memory_size)
#undef CORE_API
    static bool environment(unsigned cmd,void* data) {
        Core& c=*active;
        switch(cmd){
        case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION: *static_cast<unsigned*>(data)=2; return true;
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2:
            c.setOptions(static_cast<retro_core_options_v2*>(data)); return true;
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL:
            c.setOptions(static_cast<retro_core_options_v2_intl*>(data)->us); return true;
        case RETRO_ENVIRONMENT_SET_VARIABLES: {
            auto* v=static_cast<retro_variable*>(data);
            for(;v&&v->key;++v){std::string value=v->value?v->value:"";auto start=value.find("; ");
                if(start!=std::string::npos){value=value.substr(start+2);value=value.substr(0,value.find('|'));}
                c.options[v->key]=value;
            }
            c.preferredOptions();return true;
        }
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS: {
            auto* d=static_cast<retro_core_option_definition*>(data);
            for(;d&&d->key;++d)c.options[d->key]=d->default_value?d->default_value:d->values[0].value;
            c.preferredOptions();return true;
        }
        case RETRO_ENVIRONMENT_GET_VARIABLE: {
            auto* v=static_cast<retro_variable*>(data); auto it=c.options.find(v->key);
            v->value=it==c.options.end()?nullptr:it->second.c_str(); return v->value!=nullptr;
        }
        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: *static_cast<bool*>(data)=false; return true;
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
            *static_cast<const char**>(data)=c.sysdir.c_str(); return true;
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
            *static_cast<const char**>(data)=c.savedir.c_str(); return true;
        case RETRO_ENVIRONMENT_GET_GAME_INFO_EXT:
            if(c.rom.empty())return false;
            *static_cast<const retro_game_info_ext**>(data)=&c.extended;return true;
        case RETRO_ENVIRONMENT_SET_MEMORY_MAPS: {
            auto* map=static_cast<retro_memory_map*>(data);
            c.memoryMap.assign(map->descriptors,map->descriptors+map->num_descriptors);return true;
        }
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
            unsigned f=*static_cast<unsigned*>(data);
            if(f>RETRO_PIXEL_FORMAT_RGB565)return false;
            c.pixelFormat=f;return true;
        }
        case RETRO_ENVIRONMENT_GET_CAN_DUPE: *static_cast<bool*>(data)=true;return true;
        case RETRO_ENVIRONMENT_GET_LANGUAGE: *static_cast<unsigned*>(data)=RETRO_LANGUAGE_ENGLISH;return true;
        case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS: return true;
        case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES:
            *static_cast<uint64_t*>(data)=(1ULL<<RETRO_DEVICE_JOYPAD)|(1ULL<<RETRO_DEVICE_ANALOG)|(1ULL<<RETRO_DEVICE_POINTER);return true;
        case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS: *static_cast<unsigned*>(data)=4;return true;
        case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE: *static_cast<int*>(data)=3;return true;
        case RETRO_ENVIRONMENT_SET_GEOMETRY: c.av.geometry=*static_cast<retro_game_geometry*>(data);return true;
        case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: c.av=*static_cast<retro_system_av_info*>(data);return true;
        case RETRO_ENVIRONMENT_SET_MESSAGE: {
            auto* m=static_cast<retro_message*>(data);if(m&&m->msg)c.message=m->msg;return true;
        }
        case RETRO_ENVIRONMENT_SHUTDOWN: c.shutdownRequested=true;return true;
        case RETRO_ENVIRONMENT_SET_ROTATION:
            c.rotation=*static_cast<const unsigned*>(data)&3u;return true;
        case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS: return true;
        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
        case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
        case RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE:
        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
        case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL: return true;
        default: return false;
        }
    }
    void setOptions(const retro_core_options_v2* defs) {
        if(!defs||!defs->definitions)return;
        for(auto* d=defs->definitions;d->key;++d)
            options[d->key]=d->default_value?d->default_value:d->values[0].value;
        preferredOptions();
    }
    void preferredOptions(){
        options["fceumm_sndrate_hint"]="48KHz";
        options["fceumm_sndquality"]="High";
        options["fceumm_aspect"]="4:3";
        options["fceumm_overscan_h_left"]="0";
        options["fceumm_overscan_h_right"]="0";
        options["fceumm_overscan_v_top"]="0";
        options["fceumm_overscan_v_bottom"]="0";
        options["mgba_use_bios"]="OFF"; // mGBA's built-in HLE, no Nintendo BIOS distributed
        options["mgba_skip_bios"]="ON";
        options["mgba_frameskip"]="disabled";
        options["snes9x_overclock_superfx"]="100%";
        options["desmume_internal_resolution"]="256x192";
        options["desmume_screens_layout"]="top/bottom";
        options["desmume_pointer_mouse"]="enabled";
        options["desmume_pointer_type"]="touch";
        options["desmume_load_to_memory"]="enabled";
        options["parallel-n64-gfxplugin"]="angrylion";
        options["parallel-n64-rspplugin"]="hle";
        options["parallel-n64-cpucore"]="cached_interpreter";
        options["parallel-n64-angrylion-multithread"]="all threads";
        options["parallel-n64-alt-map"]="enabled"; // Every drawn C key has its own binding.

    }
    static void video(const void* data,unsigned w,unsigned h,size_t pitch){
        Core& c=*active;
        if(!data||w==0||h==0||w>2048||h>2048)return;
        unsigned outW=(c.rotation&1)?h:w,outH=(c.rotation&1)?w:h;
        c.width=outW;c.height=outH;c.pixels.resize(size_t(outW)*outH);
        for(unsigned y=0;y<h;++y){
            const uint8_t* row=static_cast<const uint8_t*>(data)+y*pitch;
            for(unsigned x=0;x<w;++x){
                uint32_t rgb;
                if(c.pixelFormat==RETRO_PIXEL_FORMAT_XRGB8888)memcpy(&rgb,row+x*4,4);
                else {uint16_t p;memcpy(&p,row+x*2,2);unsigned r,g,b;
                    if(c.pixelFormat==RETRO_PIXEL_FORMAT_RGB565){r=(p>>11)&31;g=(p>>5)&63;b=p&31;g=(g<<2)|(g>>4);}
                    else{r=(p>>10)&31;g=(p>>5)&31;b=p&31;g=(g<<3)|(g>>2);}
                    r=(r<<3)|(r>>2);b=(b<<3)|(b>>2);rgb=(r<<16)|(g<<8)|b;
                }
                unsigned dx=x,dy=y;
                if(c.rotation==1){dx=y;dy=w-1-x;}
                else if(c.rotation==2){dx=w-1-x;dy=h-1-y;}
                else if(c.rotation==3){dx=h-1-y;dy=x;}
                c.pixels[size_t(dy)*outW+dx]=rgb&0xFFFFFFu;
            }
        }
        ++c.videoFrames;
    }
    static size_t audioBatch(const int16_t* samples,size_t frames){
        if(samples&&frames<=96000)active->audio.insert(active->audio.end(),samples,samples+frames*2);
        return frames;
    }
    static void audioSample(int16_t l,int16_t r){int16_t a[]={l,r};audioBatch(a,1);}
    static void poll(){}
    static int16_t input(unsigned port,unsigned device,unsigned index,unsigned id){
        if(port>=4)return 0;
        unsigned type=device&RETRO_DEVICE_MASK;
        if(type==RETRO_DEVICE_JOYPAD){if(index)return 0;if(id==RETRO_DEVICE_ID_JOYPAD_MASK)return static_cast<int16_t>(active->buttons[port]);return id<16?((active->buttons[port]>>id)&1):0;}
        if(type==RETRO_DEVICE_ANALOG&&index<=RETRO_DEVICE_INDEX_ANALOG_RIGHT&&id<2)return active->analog[port][index*2+id];
        if(type==RETRO_DEVICE_POINTER&&port==0&&index==0){if(id==RETRO_DEVICE_ID_POINTER_X)return active->pointerX;if(id==RETRO_DEVICE_ID_POINTER_Y)return active->pointerY;if(id==RETRO_DEVICE_ID_POINTER_PRESSED)return active->pointerPressed?1:0;}
        return 0;
    }
public:
    Console kind=Console::None;
    bool loaded=false,shutdownRequested=false;
    retro_system_av_info av{};
    std::array<uint16_t,4> buttons{};
    std::array<std::array<int16_t,4>,4> analog{};
    int16_t pointerX=0,pointerY=0;bool pointerPressed=false;
    std::vector<uint32_t> pixels;
    std::vector<int16_t> audio;
    unsigned width=256,height=240;
    uint64_t videoFrames=0;
    std::string message,version;
    fs::path gamePath;
    Core()=default;
    Core(const fs::path& dll,const fs::path& saves,const fs::path& system,Console type=Console::NES):saveRoot(saves),systemRoot(system),kind(type){
        if(active)throw std::runtime_error("Un core e' gia' attivo.");
        fs::create_directories(saves);fs::create_directories(system);
        sysdir=system.u8string();savedir=saves.u8string();
#ifdef _WIN32
        module=LoadLibraryExW(dll.c_str(),nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
#else
        module=dlopen(dll.c_str(),RTLD_NOW|RTLD_LOCAL);
#endif
        if(!module)throw std::runtime_error("Nucleo non caricato: "+dll.filename().u8string()+". Estrai tutta la cartella ZIP accanto ad AiloEMU.exe.");
        try{
#define BIND(name) bind(name,"retro_" #name)
            BIND(init);BIND(deinit);BIND(api_version);BIND(set_environment);BIND(set_video_refresh);
            BIND(set_audio_sample);BIND(set_audio_sample_batch);BIND(set_input_poll);BIND(set_input_state);
            BIND(get_system_info);BIND(get_system_av_info);BIND(set_controller_port_device);
            BIND(load_game);BIND(unload_game);BIND(run);BIND(reset);BIND(serialize_size);BIND(serialize);
            BIND(unserialize);BIND(get_memory_data);BIND(get_memory_size);
#undef BIND
            if(api_version()!=RETRO_API_VERSION)throw std::runtime_error("Versione API del core incompatibile.");
            active=this;
            set_environment(environment);set_video_refresh(video);set_audio_sample(audioSample);
            set_audio_sample_batch(audioBatch);set_input_poll(poll);set_input_state(input);
            init();initialized=true;
            retro_system_info info{};get_system_info(&info);version=info.library_version?info.library_version:"unknown";
        }catch(...){release();throw;}
    }
    Core(const Core&)=delete;
    Core& operator=(const Core&)=delete;
    ~Core(){release();}
    void release(){
        if(loaded){unload_game();loaded=false;}
        if(!contentCache.empty()){std::error_code ec;fs::remove(contentCache,ec);contentCache.clear();}
        if(initialized){deinit();initialized=false;}
        if(active==this)active=nullptr;
        if(module){
#ifdef _WIN32
            FreeLibrary(module);
#else
            dlclose(module);
#endif
            module=nullptr;
        }
    }
    void load(const fs::path& path){
        auto data=readBytes(path);
        validateROM(data,kind);
        // ParaLLEl N64 can stall on word-swapped .n64 input. Normalize every
        // cartridge dump to native big-endian (.z64) order before the core sees it.
        if(kind==Console::Nintendo64&&data.size()>=4){
            uint32_t magic=(uint32_t(data[0])<<24)|(uint32_t(data[1])<<16)|(uint32_t(data[2])<<8)|data[3];
            if(magic==0x37804012){
                for(size_t i=0;i+1<data.size();i+=2)std::swap(data[i],data[i+1]);
            }else if(magic==0x40123780){
                for(size_t i=0;i+3<data.size();i+=4){std::swap(data[i],data[i+3]);std::swap(data[i+1],data[i+2]);}
            }
        }
        if(loaded){saveRAM();unload_game();loaded=false;}
        if(!contentCache.empty()){std::error_code ec;fs::remove(contentCache,ec);contentCache.clear();}
        gamePath=fs::absolute(path);rom=std::move(data);romID=hash64(rom.data(),rom.size());
        contentPath=gamePath.u8string();contentDir=gamePath.parent_path().u8string();
        // Some cores use legacy narrow file APIs. A short local cache avoids
        // failures with protected, long or Unicode paths (e.g. Documents).
        if(kind==Console::NintendoDS||kind==Console::Nintendo64){
            const char* ext=kind==Console::NintendoDS?".nds":".z64";
            contentCache=systemRoot/("AiloEMU-"+hex64(romID)+ext);
            writeAtomic(contentCache,rom.data(),rom.size());
#ifdef _WIN32
            std::wstring wide=contentCache.wstring();std::vector<wchar_t> shortName(32768);
            DWORD count=GetShortPathNameW(wide.c_str(),shortName.data(),DWORD(shortName.size()));
            if(count&&count<shortName.size()){
                int n=WideCharToMultiByte(CP_UTF8,0,shortName.data(),int(count),nullptr,0,nullptr,nullptr);
                contentPath.assign(size_t(n),'\0');
                WideCharToMultiByte(CP_UTF8,0,shortName.data(),int(count),contentPath.data(),n,nullptr,nullptr);
            }else contentPath=contentCache.u8string();
#else
            contentPath=contentCache.u8string();
#endif
            contentDir=contentCache.parent_path().u8string();
        }
        contentName=gamePath.stem().u8string();contentExt=kind==Console::Nintendo64?"z64":gamePath.extension().u8string().substr(1);
        extended={};extended.full_path=contentPath.c_str();extended.dir=contentDir.c_str();
        extended.name=contentName.c_str();extended.ext=contentExt.c_str();extended.data=rom.data();
        extended.size=rom.size();extended.persistent_data=true;
        retro_game_info game{contentPath.c_str(),rom.data(),rom.size(),nullptr};
        pixels.clear();audio.clear();buttons={};analog={};pointerPressed=false;memoryMap.clear();shutdownRequested=false;videoFrames=0;rotation=0;
        loaded=load_game(&game);
        if(!loaded){
            rom.clear();
            if(!contentCache.empty()){std::error_code ec;fs::remove(contentCache,ec);contentCache.clear();}
            throw std::runtime_error("Il core non riesce ad avviare questa ROM: file non valido, dump danneggiato o formato non supportato.");
        }
        for(unsigned port=0;port<consolePlayers(kind);++port)set_controller_port_device(port,RETRO_DEVICE_JOYPAD);
        get_system_av_info(&av);
        if(av.timing.fps<40||av.timing.fps>75||av.timing.sample_rate<8000||av.timing.sample_rate>192000)
            throw std::runtime_error("Temporizzazione del core non valida.");
        loadMemory(RETRO_MEMORY_SAVE_RAM,".sav");
        loadMemory(RETRO_MEMORY_RTC,".rtc");
    }
    void loadMemory(unsigned id,const char* suffix){
        size_t size=get_memory_size(id);void* memory=get_memory_data(id);
        if(!size||!memory)return;
        auto p=saveFile(suffix);
        if(fs::exists(p)){
            auto b=readBytes(p);
            if(b.size()==size||(kind==Console::GBA&&id==RETRO_MEMORY_SAVE_RAM&&!b.empty()&&b.size()<size))memcpy(memory,b.data(),b.size());
            else message="Salvataggio batteria ignorato: dimensione incompatibile.";
        }
    }
    void frame(){if(loaded){audio.clear();run();}}
    void close(){
        if(loaded){saveRAM();unload_game();loaded=false;}
        if(!contentCache.empty()){std::error_code ec;fs::remove(contentCache,ec);contentCache.clear();}
        rom.clear();pixels.clear();audio.clear();buttons={};analog={};pointerPressed=false;memoryMap.clear();
    }
    // Region is applied at the next ROM load, never mid-frame.
    void setRegion(const std::string& region){
        options["fceumm_region"]=region;
        options["snes9x_region"]=region=="PAL"?"pal":region=="NTSC"?"ntsc":"auto";
    }
    // Debug-only access to directly mapped CPU RAM. Does not emulate IO reads.
    uint8_t peekMappedRAM(size_t address)const{
        if(!loaded)return 0;
        for(const auto& d:memoryMap)
            if(d.ptr&&!d.select&&(!d.addrspace||!*d.addrspace)&&address>=d.start&&address-d.start<d.len)
                return static_cast<const uint8_t*>(d.ptr)[d.offset+address-d.start];
        return 0;
    }
    void restart(){if(loaded){buttons={};analog={};pointerPressed=false;reset();audio.clear();}}
    void* ram(){return get_memory_data(RETRO_MEMORY_SAVE_RAM);}
    size_t ramSize(){return loaded?get_memory_size(RETRO_MEMORY_SAVE_RAM):0;}
    void* systemRAM(){return get_memory_data(RETRO_MEMORY_SYSTEM_RAM);}
    fs::path saveFile(const std::string& ext)const{return saveRoot/(hex64(romID)+ext);}
    void saveRAM(){
        if(!loaded)return;
        if(ramSize()&&ram())writeAtomic(saveFile(".sav"),ram(),ramSize());
        size_t n=get_memory_size(RETRO_MEMORY_RTC);void* p=get_memory_data(RETRO_MEMORY_RTC);
        if(n&&p)writeAtomic(saveFile(".rtc"),p,n);
    }
    Bytes snapshot(){
        if(!loaded)throw std::runtime_error("Apri prima una ROM.");
        Bytes b(serialize_size());
        if(b.empty()||!serialize(b.data(),b.size()))throw std::runtime_error("Salvataggio dello stato non riuscito.");
        return b;
    }
    bool restore(const Bytes& b){return loaded&&!b.empty()&&unserialize(b.data(),b.size());}
    void saveState(unsigned slot){
        Bytes state=snapshot(), file(40+state.size(),0);
        memcpy(file.data(),"NIDOST01",8);memcpy(file.data()+8,&romID,8);
        uint64_t v=hash64(version.data(),version.size());memcpy(file.data()+16,&v,8);
        uint64_t n=state.size();memcpy(file.data()+24,&n,8);
        uint32_t crc=crc32(state.data(),state.size());memcpy(file.data()+32,&crc,4);
        memcpy(file.data()+40,state.data(),state.size());
        writeAtomic(saveFile(".slot"+std::to_string(slot)+".state"),file.data(),file.size());saveRAM();
    }
    void loadState(unsigned slot){
        if(!loaded)throw std::runtime_error("Apri prima una ROM.");
        auto path=saveFile(".slot"+std::to_string(slot)+".state");
        if(!fs::exists(path))throw std::runtime_error("Nessun salvataggio presente nello slot selezionato.");
        auto b=readBytes(path);uint64_t id=0,v=0,n=0;uint32_t crc=0;
        if(b.size()<40||memcmp(b.data(),"NIDOST01",8))throw std::runtime_error("Formato del salvataggio non valido.");
        memcpy(&id,b.data()+8,8);memcpy(&v,b.data()+16,8);memcpy(&n,b.data()+24,8);memcpy(&crc,b.data()+32,4);
        if(id!=romID||v!=hash64(version.data(),version.size())||n!=b.size()-40||crc!=crc32(b.data()+40,b.size()-40))
            throw std::runtime_error("Salvataggio corrotto, di un'altra ROM o di un'altra versione del core.");
        Bytes backup=snapshot();Bytes state(b.begin()+40,b.end());
        if(!restore(state)){restore(backup);throw std::runtime_error("Stato rifiutato dal core.");}
        audio.clear();buttons={};
    }
};
