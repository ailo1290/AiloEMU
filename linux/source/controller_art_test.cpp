#include "controller_art.hpp"
#include <fstream>
#include <iostream>
int main(int argc,char** argv){
    if(argc!=2)return 2;
    fs::create_directories(argv[1]);unsigned count=0;
    struct Expected{Console c;int action;const wchar_t* label;};
    for(auto e:{Expected{Console::Genesis,7,L"A"},Expected{Console::Genesis,4,L"C"},
        Expected{Console::WonderSwan,11,L"Y1"},Expected{Console::VirtualBoy,12,L"R↓"},
        Expected{Console::Nintendo64,5,L"A"},Expected{Console::Nintendo64,4,L"C↓"},
        Expected{Console::PokemonMini,9,L"C"}}){
        if(controller_art::make(e.c,0).buttons[e.action].label!=e.label)return 1;
    }
    for(unsigned ci=unsigned(Console::NES);ci<=unsigned(Console::PokemonMini);++ci){
        auto c=Console(ci);auto s=controller_art::make(c,0x101715);
        for(size_t a=0;a<s.buttons.size();++a){
            auto r=s.buttons[a].bounds;if(!r.w)continue;
            if(r.x<0||r.y<0||r.w<=0||r.h<=0||r.x+r.w>controller_art::W||r.y+r.h>controller_art::H){
                std::cerr<<consoleName(c)<<": invalid hit region "<<a<<"\n";return 1;
            }
            for(size_t b=a+1;b<s.buttons.size();++b){
                auto q=s.buttons[b].bounds;if(q.w&&std::max(r.x,q.x)<std::min(r.x+r.w,q.x+q.w)&&std::max(r.y,q.y)<std::min(r.y+r.h,q.y+q.h)){
                    std::cerr<<consoleName(c)<<": overlapping keys "<<a<<","<<b<<"\n";return 1;
                }
            }
            ++count;
        }
        std::ofstream f(fs::path(argv[1])/(std::to_string(ci)+".ppm"),std::ios::binary);
        f<<"P6\n"<<controller_art::W<<" "<<controller_art::H<<"\n255\n";
        for(auto p:s.pixels){char bytes[]={char(p>>16),char(p>>8),char(p)};f.write(bytes,3);}
        if(!f)return 1;
    }
    std::cout<<"PASS: 21 sprites, "<<count<<" hit regions in bounds and non-overlapping.\n";
}
