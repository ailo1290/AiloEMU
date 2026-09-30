// Additional core-linking permission: ../licenses/AiloEMU-Core-Linking-Exception.txt
// Integration runner for separately obtained public NES hardware test ROMs.
// GPL-2.0-or-later. ROMs are not distributed with this application.
#include "core.hpp"
#include <iostream>
int main(int argc,char** argv){
    if(argc!=4){std::cerr<<"usage: blargg_runner core_library test.nes output_dir\n";return 2;}
    fs::path output=fs::absolute(argv[3]);fs::create_directories(output);
    std::ofstream report(output/"result.txt");
    try{
        Core c(fs::absolute(argv[1]),output/"saves",output/"system");c.load(fs::absolute(argv[2]));
        bool started=false;int resetAt=-1;
        for(int i=0;i<36000;i++){
            c.frame();
            if(c.peekMappedRAM(0x6001)==0xDE&&c.peekMappedRAM(0x6002)==0xB0&&c.peekMappedRAM(0x6003)==0x61){
                int status=c.peekMappedRAM(0x6000);
                if(status==0x80)started=true;
                if(status==0x81&&resetAt<0)resetAt=i+10;
                if(resetAt==i){c.restart();resetAt=-1;}
                if(started&&status<0x80){
                    report<<argv[2]<<"\nframes="<<i+1<<"\nstatus="<<status<<"\n";
                    for(size_t a=0x6004;a<0x6800;a++){char ch=char(c.peekMappedRAM(a));if(!ch)break;report<<ch;}
                    report<<"\n";return status?1:0;
                }
            }
        }
        report<<"TIMEOUT (no completed $6000 protocol result within 36000 frames)\n";
        return 2;
    }catch(const std::exception& e){report<<"ERROR "<<e.what()<<"\n";return 3;}
}
