// AiloEMU Nintendo DS integration smoke test. GPL-2.0-or-later.
#include "core.hpp"
#include <iostream>

static void put32(Bytes& rom,size_t offset,uint32_t value){
    rom[offset]=uint8_t(value);rom[offset+1]=uint8_t(value>>8);
    rom[offset+2]=uint8_t(value>>16);rom[offset+3]=uint8_t(value>>24);
}

int main(int argc,char** argv){
    if(argc!=3){std::cerr<<"usage: nds_smoke_runner CORE OUTPUT_DIR\n";return 2;}
    try{
        fs::path root=fs::absolute(argv[2]);
        fs::create_directories(root/"ROM con spazio e caratteri");
        Bytes rom(32768,0);
        memcpy(rom.data(),"AILOEMU NDS ",12);memcpy(rom.data()+0x0C,"AILE",4);
        memcpy(rom.data()+0x10,"00",2);rom[0x12]=0;rom[0x14]=0;
        put32(rom,0x20,0x200);put32(rom,0x24,0x02000000);put32(rom,0x28,0x02000000);put32(rom,0x2C,4);
        put32(rom,0x30,0x204);put32(rom,0x34,0x03800000);put32(rom,0x38,0x03800000);put32(rom,0x3C,4);
        put32(rom,0x80,uint32_t(rom.size()));put32(rom,0x84,0x200);
        put32(rom,0x200,0xEAFFFFFE);put32(rom,0x204,0xEAFFFFFE);
        fs::path game=root/"ROM con spazio e caratteri"/u8"prova-è.nds";
        writeAtomic(game,rom.data(),rom.size());
        fs::path system=root/"system",saves=root/"saves";
        {
            Core core(fs::absolute(argv[1]),saves,system,Console::NintendoDS);
            core.load(game);core.frame();
            if(!core.loaded||core.videoFrames==0)throw std::runtime_error("Il core NDS non ha prodotto un fotogramma.");
            core.close();
        }
        for(const auto& entry:fs::directory_iterator(system))
            if(entry.path().extension()==".nds")throw std::runtime_error("La cache NDS non e' stata rimossa.");
        std::cout<<"RESULT: Nintendo DS smoke test passed\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
