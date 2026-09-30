// Additional core-linking permission: ../licenses/AiloEMU-Core-Linking-Exception.txt
// AiloEMU integration tests for SNES, GB, GBC and GBA. GPL-2.0-or-later.
#pragma once
#include "core.hpp"
#include "resampler.hpp"
#include <set>
#include <cmath>
inline int runExtraTests(const fs::path& snes,const fs::path& gba,const fs::path& roms,const fs::path& output){
    fs::create_directories(output);std::ofstream report(output/"multisystem.txt");
    int passed=0,failed=0;
    auto check=[&](bool condition,const std::string& label){report<<(condition?"PASS ":"FAIL ")<<label<<"\n";report.flush();condition?++passed:++failed;};
    auto ppm=[&](const Core& c,const fs::path& file){std::ofstream f(file,std::ios::binary);f<<"P6\n"<<c.width<<" "<<c.height<<"\n255\n";for(auto p:c.pixels){char b[]={char(p>>16),char(p>>8),char(p)};f.write(b,3);}};
    auto demo=[&](Console type){
        if(type==Console::SNES)return roms/"Ailo-SNES-Demo.sfc";
        if(type==Console::GB)return roms/"Ailo-GB-Demo.gb";
        if(type==Console::GBC)return roms/"Ailo-GBC-Demo.gbc";
        return roms/"Ailo-GBA-Demo.gba";
    };
    try{
        for(Console type:{Console::SNES,Console::GB,Console::GBC,Console::GBA}){
            std::string name=consoleName(type);
            Core c(type==Console::SNES?snes:gba,output/name/"saves",output/"system",type);
            auto rom=demo(type);
            c.load(rom);for(int f=0;f<120;f++)c.frame();
            report<<"Core "<<name<<" version: "<<c.version<<"; FPS "<<c.av.timing.fps<<"; audio "<<c.av.timing.sample_rate<<"\n";
            check(c.loaded&&c.videoFrames>=120,name+" loads ROM and delivers video");
            check(type==Console::SNES?(c.width==256&&c.height==224):type==Console::GBA?(c.width==240&&c.height==160):(c.width==160&&c.height==144),name+" native geometry");
            check(std::set<uint32_t>(c.pixels.begin(),c.pixels.end()).size()>=2,name+" visible graphics");
            check(c.av.timing.fps>59&&c.av.timing.fps<61&&!c.audio.empty(),name+" frame timing and audio callbacks");
            auto* ram=static_cast<uint8_t*>(c.systemRAM());
            if(!ram)throw std::runtime_error("System RAM not exposed");
            auto original=c.pixels;
            c.buttons[0]=(1u<<RETRO_DEVICE_ID_JOYPAD_A)|(1u<<RETRO_DEVICE_ID_JOYPAD_L)|(1u<<RETRO_DEVICE_ID_JOYPAD_R);
            int peak=0;size_t soundFrames=0;
            for(int f=0;f<60;f++){c.frame();soundFrames+=c.audio.size()/2;for(auto sample:c.audio)peak=std::max(peak,std::abs(int(sample)));}
            unsigned pad=unsigned(ram[0])|(unsigned(ram[1])<<8);
            unsigned expected=type==Console::SNES?0x00B0:type==Console::GBA?0x0301:0x0001;
            check((pad&expected)==expected,name+" A, L and R input reach emulated hardware");
            check(original!=c.pixels,name+" input changes display");
            check(soundFrames>size_t(c.av.timing.sample_rate*0.97)&&soundFrames<size_t(c.av.timing.sample_rate*1.03),name+" one-second audio sample count");
            if(type==Console::GBA)check(peak>100,name+" synthesized tone is non-silent");
            c.buttons={};for(int f=0;f<3;f++)c.frame();
            if(type==Console::SNES){
                c.buttons[0]=(1u<<RETRO_DEVICE_ID_JOYPAD_X)|(1u<<RETRO_DEVICE_ID_JOYPAD_Y);
                for(int f=0;f<3;f++)c.frame();
                pad=unsigned(ram[0])|(unsigned(ram[1])<<8);
                check((pad&0x4040)==0x4040,"SNES X and Y inputs");c.buttons={};
            }
            c.saveState(2);auto state=c.snapshot();Bytes memory(ram,ram+16);
            for(int f=0;f<20;f++)c.frame();
            check(Bytes(ram,ram+16)!=memory,name+" CPU advances before restore");
            c.loadState(2);check(Bytes(ram,ram+16)==memory,name+" disk save-state restores CPU RAM");
            for(int f=0;f<10;f++)c.frame();
            auto pixels=c.pixels;Bytes replay(ram,ram+16);
            c.restore(state);for(int f=0;f<10;f++)c.frame();
            check(pixels==c.pixels&&replay==Bytes(ram,ram+16),name+" deterministic video/RAM replay");
            check(c.ramSize()>0&&c.ram(),name+" battery memory available");
            auto* battery=static_cast<uint8_t*>(c.ram());uint8_t value=battery?battery[0]:0;
            c.saveRAM();c.load(rom);for(int f=0;f<5;f++)c.frame();
            check(c.ram()&&static_cast<uint8_t*>(c.ram())[0]==value,name+" battery save survives ROM reload");
            c.restart();for(int f=0;f<120;f++)c.frame();
            check(c.loaded&&std::set<uint32_t>(c.pixels.begin(),c.pixels.end()).size()>=2,name+" reset returns to demo");
            ppm(c,output/(name+".ppm"));
            for(int f=0;f<1800;f++)c.frame();
            check(c.loaded,name+" 30-second emulated soak run");
            if(type==Console::SNES){c.setRegion("PAL");c.load(rom);check(c.av.timing.fps>49&&c.av.timing.fps<51,"SNES manual PAL mode");}
        }
        // Every core has been deinitialized before the next one is loaded.
        for(Console type:{Console::GBA,Console::GB,Console::SNES,Console::GBC,Console::GBA}){
            Core c(type==Console::SNES?snes:gba,output/"switch-saves",output/"system",type);
            c.load(demo(type));
            for(int f=0;f<5;f++)c.frame();
            check(!c.pixels.empty(),std::string("Core switch to ")+consoleName(type));
        }
        for(double sourceRate:{32040.0,65536.0,48000.0}){
            Resampler r;size_t total=0;bool dcCorrect=true;
            for(int batch=0;batch<100;batch++){
                std::vector<int16_t> input(1000*2,1234);auto out=r.convert(input,sourceRate);total+=out.size()/2;
                dcCorrect &= std::all_of(out.begin(),out.end(),[](int16_t v){return v==1234;});
            }
            check(dcCorrect,"Resampler preserves DC across 100 batches at "+std::to_string(sourceRate));
            check(std::abs(double(total)-100000.0*48000/sourceRate)<3,"Resampler output rate "+std::to_string(sourceRate));
            r.clear();check(!r.convert({-1234,-1234,-1234,-1234},sourceRate).empty(),"Resampler reset");
        }
        check(detectConsole("game.SFC")==Console::SNES&&detectConsole("game.SMC")==Console::SNES&&
              detectConsole("game.GB")==Console::GB&&detectConsole("game.GBC")==Console::GBC&&detectConsole("game.GBA")==Console::GBA&&
              detectConsole("game.SMD")==Console::Genesis&&detectConsole("game.SMS")==Console::SMS&&detectConsole("game.GG")==Console::GameGear&&
              detectConsole("game.SG1000")==Console::SG1000&&detectConsole("game.32X")==Console::Sega32X&&detectConsole("game.A26")==Console::Atari2600&&
              detectConsole("game.A78")==Console::Atari7800&&detectConsole("game.NGP")==Console::NGP&&detectConsole("game.NGPC")==Console::NGPC&&
              detectConsole("game.WS")==Console::WonderSwan&&detectConsole("game.WSC")==Console::WonderSwanColor&&detectConsole("game.PCE")==Console::PCEngine&&
              detectConsole("game.NDS")==Console::NintendoDS&&detectConsole("game.Z64")==Console::Nintendo64&&detectConsole("game.VBOY")==Console::VirtualBoy&&
              detectConsole("game.MIN")==Console::PokemonMini,"Case-insensitive ROM dispatch for every system");
    }catch(const std::exception& e){report<<"FAIL exception: "<<e.what()<<"\n";++failed;}
    report<<"RESULT: "<<passed<<" passed, "<<failed<<" failed\n";
    report<<"Does not test Windows GUI, physical speakers/controllers or commercial games.\n";
    return failed?1:0;
}
