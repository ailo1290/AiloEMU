// Additional core-linking permission: ../licenses/AiloEMU-Core-Linking-Exception.txt
// Integration tests shared by the native test runner and the Windows executable.
// GPL-2.0-or-later.
#pragma once
#include "core.hpp"
#include <set>
#include <sstream>

inline int runDiagnostics(const fs::path& dll,const fs::path& demo,const fs::path& output){
    fs::create_directories(output);
    std::ofstream report(output/"report.txt",std::ios::trunc);
    int passed=0,failed=0;
    auto check=[&](bool b,const char* name){report<<(b?"PASS ":"FAIL ")<<name<<"\n";report.flush();if(b)passed++;else failed++;};
    try{
        Core c(dll,output/"saves",output/"system");c.load(demo);
        report<<"Core: FCEUmm "<<c.version<<"\n";
#ifdef _WIN32
        report<<"Platform: Windows executable\n";
#else
        report<<"Platform: native Linux test build (not Windows GUI)\n";
#endif
        for(int i=0;i<120;i++)c.frame();
        check(c.loaded&&c.videoFrames==120,"ROM loaded and 120 video callbacks delivered");
        check(c.width==256&&c.height==240,"Native framebuffer is 256 x 240");
        std::set<uint32_t> colors(c.pixels.begin(),c.pixels.end());
        check(colors.size()>=3,"PPU produces multiple visible colors");
        check(c.av.timing.fps>59&&c.av.timing.fps<61,"NTSC timing");
        check(c.av.timing.sample_rate==48000&&!c.audio.empty(),"48 kHz stereo sample generation");
        auto* ram=static_cast<uint8_t*>(c.systemRAM());
        if(!ram)throw std::runtime_error("Missing 6502 system RAM");
        check(ram[0]==120&&ram[1]==120,"CPU startup / zero-page / initial sprite coordinates");
        auto before=c.pixels;
        c.buttons[0]=1u<<RETRO_DEVICE_ID_JOYPAD_RIGHT;
        for(int i=0;i<12;i++)c.frame();
        check(ram[0]==132,"Controller 1 shifts right input through $4016");
        check(c.pixels!=before,"Sprite movement changes rendered image");
        c.buttons={};c.buttons[1]=1u<<RETRO_DEVICE_ID_JOYPAD_A;c.frame();
        check(ram[3]==0x80,"Controller 2 shifts A input through $4017");
        c.buttons={};c.frame();
        auto* sram=static_cast<uint8_t*>(c.ram());
        check(sram&&c.ramSize()==8192,"Battery-backed PRG RAM exposed");
        uint8_t saved=sram?sram[0]:0;
        c.buttons[0]=1u<<RETRO_DEVICE_ID_JOYPAD_A;
        int peak=0;uint64_t samples=0;
        for(int i=0;i<60;i++){c.frame();for(int16_t s:c.audio)peak=std::max(peak,std::abs(int(s)));samples+=c.audio.size()/2;}
        check(peak>500&&samples>47000&&samples<49000,"APU pulse signal amplitude and 1-second sample count");
        check(sram&&sram[0]==uint8_t(saved+1),"6502 stores A press to battery-backed SRAM");
        c.buttons={};c.frame();
        c.saveState(1);uint8_t savedX=ram[0];
        c.buttons[0]=1u<<RETRO_DEVICE_ID_JOYPAD_LEFT;for(int i=0;i<30;i++)c.frame();
        check(ram[0]==uint8_t(savedX-30),"Game advances before state restoration");
        c.loadState(1);check(ram[0]==savedX,"Save-state restores CPU / RAM");
        auto snapshot=c.snapshot();
        c.buttons[0]=1u<<RETRO_DEVICE_ID_JOYPAD_DOWN;
        for(int i=0;i<15;i++)c.frame();
        auto imageA=c.pixels;auto memoryA=Bytes(ram,ram+2048);
        check(c.restore(snapshot),"Core accepts serialized state");
        for(int i=0;i<15;i++)c.frame();
        check(imageA==c.pixels&&memoryA==Bytes(ram,ram+2048),"Deterministic replay: video and RAM after restoring state");
        auto path=c.saveFile(".slot1.state");auto valid=readBytes(path);auto corrupt=valid;corrupt.back()^=1;
        writeAtomic(path,corrupt.data(),corrupt.size());bool rejected=false;
        try{c.loadState(1);}catch(...){rejected=true;}
        check(rejected,"Corrupt save-state rejected by checksum");
        writeAtomic(path,valid.data(),valid.size());c.loadState(1);
        c.saveRAM();saved=static_cast<uint8_t*>(c.ram())[0];
        c.load(demo);check(static_cast<uint8_t*>(c.ram())[0]==saved,"SRAM persists after ROM unload / reload");
        for(int i=0;i<120;i++)c.frame();
        c.buttons[0]=1u<<RETRO_DEVICE_ID_JOYPAD_START;c.frame();c.buttons={};
        for(int i=0;i<2;i++)c.frame();
        check(static_cast<uint8_t*>(c.systemRAM())[0]==120,"Start button resets demo position");
        std::ofstream ppm(output/"demo.ppm",std::ios::binary);ppm<<"P6\n"<<c.width<<" "<<c.height<<"\n255\n";
        for(auto p:c.pixels){char rgb[]={char(p>>16),char(p>>8),char(p)};ppm.write(rgb,3);}
        // Keep a valid game running when input validation rejects a truncated file.
        auto badROM=output/"truncated.nes";Bytes bad(16,0);memcpy(bad.data(),"NES\x1a",4);bad[4]=1;
        writeAtomic(badROM,bad.data(),bad.size());rejected=false;
        try{c.load(badROM);}catch(...){rejected=true;}
        check(rejected&&c.loaded,"Truncated iNES rejected without closing current game");fs::remove(badROM);
        badROM=output/"invalid.nes";bad.assign(64,0);writeAtomic(badROM,bad.data(),bad.size());rejected=false;
        try{c.load(badROM);}catch(...){rejected=true;}
        check(rejected&&c.loaded,"Invalid file signature rejected without closing current game");fs::remove(badROM);
        c.restart();for(int i=0;i<120;i++)c.frame();
        check(static_cast<uint8_t*>(c.systemRAM())[0]==120,"Console reset reboots CPU and cartridge");
        for(int i=0;i<3600;i++)c.frame();
        check(c.loaded&&!c.pixels.empty(),"60-second emulated soak run completes");
        // NTSC/PAL detection: make a PAL-marked copy of our own cartridge.
        auto pal=readBytes(demo);pal[7]=8;pal[10]=0x70;pal[12]=1;auto palPath=output/"Ailo-Demo-PAL.nes";
        writeAtomic(palPath,pal.data(),pal.size());c.load(palPath);for(int i=0;i<120;i++)c.frame();
        check(c.av.timing.fps>49&&c.av.timing.fps<51,"PAL header selects 50 Hz timing");
        auto wrongPath=c.saveFile(".slot1.state");writeAtomic(wrongPath,valid.data(),valid.size());rejected=false;
        try{c.loadState(1);}catch(...){rejected=true;}
        check(rejected,"State from another ROM is rejected");fs::remove(palPath);
    }catch(const std::exception& e){failed++;report<<"FAIL exception: "<<e.what()<<"\n";}
    report<<"\nRESULT: "<<passed<<" passed, "<<failed<<" failed\n";
    report<<"These checks do not verify speakers, physical controllers, or Windows GUI rendering.\n";
    return failed?1:0;
}
