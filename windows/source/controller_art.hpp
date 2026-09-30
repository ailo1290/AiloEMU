// Original code-native pixel artwork, GPL-2.0-or-later. No external image assets.
// Additional core-linking permission: ../licenses/AiloEMU-Core-Linking-Exception.txt
// Geometry is shared by rendering and hit testing. One pixel = 3 screen pixels.
#pragma once
#include "core.hpp"
#include <cmath>
#include <initializer_list>
namespace controller_art {
constexpr int W=240,H=112;
struct Point{int x,y;};
struct Rect{int x=0,y=0,w=0,h=0;};
struct Button{Rect bounds;std::wstring label;};
inline uint32_t shade(uint32_t c,int d){
    auto s=[&](int shift){return uint32_t(std::clamp(int((c>>shift)&255)+d,0,255));};
    return (s(16)<<16)|(s(8)<<8)|s(0);
}
struct Sprite {
    std::array<uint32_t,W*H> pixels{};
    std::array<Button,16> buttons{};
    void put(int x,int y,uint32_t c){if(x>=0&&x<W&&y>=0&&y<H)pixels[size_t(y)*W+x]=c;}
    void rect(int x,int y,int w,int h,uint32_t c){for(int j=y;j<y+h;++j)for(int i=x;i<x+w;++i)put(i,j,c);}
    template<class Mask> void relief(int x,int y,int w,int h,uint32_t c,Mask mask){
        for(int j=0;j<h;++j)for(int i=0;i<w;++i){
            if(!mask(i,j))continue;
            uint32_t v=c;
            if(!mask(i-1,j)||!mask(i+1,j)||!mask(i,j-1)||!mask(i,j+1))v=0x15151d;
            else if(!mask(i-3,j)||!mask(i,j-3))v=shade(c,47);
            else if(!mask(i+4,j)||!mask(i,j+4))v=shade(c,-38);
            else if(!mask(i-5,j)||!mask(i,j-5))v=shade(c,18);
            put(x+i,y+j,v);
        }
    }
    void rounded(int x,int y,int w,int h,uint32_t c,int corner=5){
        relief(x,y,w,h,c,[=](int i,int j){if(i<0||j<0||i>=w||j>=h)return false;int dx=std::min(i,w-1-i),dy=std::min(j,h-1-j);return dx+dy>=corner;});
    }
    void oval(int x,int y,int w,int h,uint32_t c){
        relief(x,y,w,h,c,[=](int i,int j){double dx=(i+0.5-w/2.0)/(w/2.0),dy=(j+0.5-h/2.0)/(h/2.0);return dx*dx+dy*dy<=1;});
    }
    void poly(std::initializer_list<Point> points,uint32_t c){
        std::vector<Point> p(points);
        relief(0,0,W,H,c,[&](int x,int y){bool inside=false;for(size_t i=0,k=p.size()-1;i<p.size();k=i++){
            if((p[i].y>y)!=(p[k].y>y)&&x<(p[k].x-p[i].x)*double(y-p[i].y)/double(p[k].y-p[i].y)+p[i].x)inside=!inside;
        }return inside;});
    }
    void screw(int x,int y){oval(x,y,4,4,0x686873);rect(x+1,y+1,2,1,0x25252c);}
    void speaker(int x,int y,int n){for(int i=0;i<n;++i){rect(x+i*4,y,2,10,0x30313a);rect(x+i*4,y+10,2,1,0xc1bdce);}}
    void screen(int x,int y,int w,int h,uint32_t lcd){
        rounded(x,y,w,h,0x343540,3);rect(x+4,y+4,w-8,h-8,lcd);rect(x+4,y+4,w-8,2,shade(lcd,-30));
        rect(x+4,y+6,2,h-10,shade(lcd,-20));rect(x+w-6,y+6,2,h-12,shade(lcd,14));
        // Two sparse pixel reflections, not a smooth gradient.
        for(int j=0;j<h-14;++j){int x0=x+10+j; if(x0+5<x+w-5)rect(x0,y+8+j,5,1,shade(lcd,9));}
    }
    void key(int a,const wchar_t* label,int x,int y,int w,int h,uint32_t c,bool circle=true){
        if(circle)oval(x,y,w,h,c);else rounded(x,y,w,h,c,3);
        buttons[size_t(a)]={{x+3,y+3,w-6,h-6},label};
    }
    void cross(int x,int y,int size=42){
        int arm=size/3;
        relief(x,y,size,size,0x292a31,[=](int i,int j){return i>=0&&j>=0&&i<size&&j<size&&((i>=arm&&i<2*arm)||(j>=arm&&j<2*arm));});
        rect(x+arm+4,y+arm+4,std::max(2,arm-8),std::max(2,arm-8),0x373942);
        buttons[0]={{x+arm,y+1,arm,arm-1},L"↑"};buttons[1]={{x+arm,y+2*arm,arm,arm-1},L"↓"};
        buttons[2]={{x+1,y+arm,arm-1,arm},L"←"};buttons[3]={{x+2*arm,y+arm,arm-1,arm},L"→"};
    }
    void startSelect(int x,int y){key(15,L"SELECT",x,y,25,13,0x55545f,false);key(14,L"START",x+31,y,25,13,0x55545f,false);}
};
inline Sprite make(Console c,uint32_t background){
    Sprite s;s.pixels.fill(background);
    const uint32_t red=0xd93249,grey=0xa3a0ae,purple=0x8872ad;
    auto cable=[&](){s.rect(119,0,3,23,0x171921);s.rect(122,0,1,23,0x696672);};
    auto shoulder=[&](int x,int y,int w){s.key(8,L"L",x,y,w,14,0x777782,false);s.key(9,L"R",240-x-w,y,w,14,0x777782,false);};
    switch(c){
    case Console::NES:
        cable();s.rounded(13,19,214,83,0xc8c7c7,3);s.rect(17,24,206,72,0x33353b);
        for(int y:{26,36,46,84})s.rect(90,y,58,6,0x82858b);
        for(int x=19;x<220;x+=5){s.rect(x,25,2,2,0x45484d);s.rect(x,92,2,2,0x45484d);}
        s.rounded(89,57,61,22,0xa4a7ad,2);s.cross(27,42,45);s.startSelect(93,61);
        s.rounded(159,55,28,30,0xbcbcc2,1);s.rounded(191,55,28,30,0xbcbcc2,1);
        s.key(5,L"B",162,58,22,24,red);s.key(4,L"A",194,58,22,24,red);s.screw(18,20);s.screw(218,96);break;
    case Console::SNES:
        cable();shoulder(37,15,47);
        s.poly({{38,24},{65,20},{175,20},{200,25},{221,42},{229,64},{227,83},{215,98},{194,107},{174,105},{151,94},{89,94},{68,105},{45,107},{23,99},{12,82},{10,60},{19,39}},grey);
        s.oval(20,33,66,63,0x7b7b78);s.oval(155,29,63,65,0x34313e);s.cross(31,44,45);
        s.key(7,L"Y",158,52,23,23,0x42af4c);s.key(6,L"X",180,32,23,23,0x3986da);
        s.key(5,L"B",180,73,23,23,0xd6c834);s.key(4,L"A",202,52,23,23,0xe73b42);s.startSelect(92,61);break;
    case Console::GB:case Console::GBC:case Console::PokemonMini:{
        bool mini=c==Console::PokemonMini;uint32_t body=c==Console::GB?0xc0c0ac:c==Console::GBC?0x8160b1:0x70a86c;
        s.poly({{78,2},{163,2},{168,7},{168,94},{153,111},{77,111},{72,106},{72,8}},body);
        s.rect(78,10,84,1,shade(body,-26));s.screen(83,15,73,44,c==Console::GB?0x8c9b64:0x9aadb3);
        s.rect(77,25,2,2,0xe54b43);s.cross(79,65,30);s.key(4,L"A",143,65,19,19,mini?0x293c33:0x9b345e);
        s.key(5,L"B",123,76,19,19,mini?0x293c33:0x9b345e);
        if(mini){s.key(15,L"POWER",81,96,26,12,0x405841,false);s.key(9,L"C",158,60,17,18,0x405841,false);s.key(8,L"SHAKE",112,97,30,12,0x405841,false);}
        else s.startSelect(90,96);
        s.speaker(149,93,3);break;}
    case Console::GBA:
        shoulder(30,14,40);
        s.poly({{48,18},{92,13},{148,13},{192,18},{211,30},{221,50},{221,74},{211,91},{187,101},{52,101},{30,94},{19,78},{18,48},{28,29}},purple);
        s.screen(78,23,86,65,0x98b1bd);s.cross(28,39,39);s.key(4,L"A",193,40,20,21,0xbda9d0);s.key(5,L"B",173,56,20,21,0xbda9d0);
        s.key(14,L"START",169,83,24,13,0x5d4779,false);s.key(15,L"SELECT",141,88,24,13,0x5d4779,false);s.speaker(196,78,4);s.rect(208,32,2,3,0x70e296);break;
    case Console::Genesis:case Console::Sega32X:
        cable();s.poly({{47,20},{91,17},{154,17},{194,25},{217,42},{225,68},{219,88},{205,104},{188,108},{165,94},{148,87},{86,87},{62,103},{46,108},{26,99},{17,82},{15,64},{24,39}},0x37363f);
        s.oval(22,30,68,66,0x55535c);s.cross(34,42,45);s.rect(97,30,46,2,0x9b3643);s.key(14,L"START",105,51,29,16,0x8c859a,false);
        for(int i=0;i<3;i++){const int lower[]={7,5,4},upper[]={8,6,9};s.key(lower[i],i==0?L"A":i==1?L"B":L"C",145+24*i,64-i*5,23,23,0x58515f);s.key(upper[i],i==0?L"X":i==1?L"Y":L"Z",146+24*i,37-i*3,20,20,c==Console::Sega32X?0x70727d:0x55515d);}
        s.key(15,L"MODE",170,17,28,12,0x59555f,false);break;
    case Console::SMS:case Console::SG1000:
        cable();s.rounded(23,22,194,76,c==Console::SMS?0x35353b:0xd3d1c8,3);
        s.rect(28,27,184,2,0x707079);s.rect(28,31,184,1,0x181923);
        s.rounded(33,39,59,49,c==Console::SMS?0x585860:0x313849,3);s.cross(41,43,42);
        s.key(5,L"1",145,55,25,25,c==Console::SMS?0xc34649:0x5988b4);s.key(4,L"2",179,55,25,25,c==Console::SMS?0xc34649:0x5988b4);
        s.rect(109,43,89,3,0x7d777b);s.screw(27,90);s.screw(208,90);break;
    case Console::GameGear:
        s.poly({{30,20},{206,20},{218,29},{225,48},{225,80},{212,94},{28,94},{15,80},{15,44},{22,28}},0x383c47);
        s.screen(76,29,87,57,0x889eaa);s.cross(26,38,42);s.key(4,L"2",195,41,23,23,0x5578ab);s.key(5,L"1",173,58,23,23,0x5578ab);s.key(14,L"START",183,24,27,13,0x474953,false);s.speaker(179,82,7);s.rect(70,33,2,3,0xe5535c);break;
    case Console::NGP:case Console::NGPC:
        s.poly({{22,18},{214,18},{223,27},{228,84},{218,100},{25,100},{15,89},{12,35}},c==Console::NGP?0x999d9a:0x638abc);
        s.screen(77,29,87,56,c==Console::NGP?0xa7b294:0x9eafba);s.oval(22,38,50,50,0x424651);s.cross(28,44,39);
        s.key(4,L"A",194,40,23,23,0x3f4350);s.key(5,L"B",174,61,23,23,0x3f4350);s.key(14,L"OPTION",173,23,33,13,0x454654,false);s.speaker(178,87,7);break;
    case Console::WonderSwan:case Console::WonderSwanColor:
        s.rounded(13,6,214,101,c==Console::WonderSwan?0xaaa9a0:0x668abd,9);s.screen(87,22,92,67,0x9eaeb0);
        s.cross(23,63,36);s.key(11,L"Y1",35,12,17,16,0x414451);s.key(9,L"Y2",51,29,17,16,0x414451);
        s.key(10,L"Y3",35,45,17,16,0x414451);s.key(8,L"Y4",19,29,17,16,0x414451);
        s.key(4,L"A",202,48,20,20,0x454652);s.key(5,L"B",181,66,20,20,0x454652);s.key(14,L"START",120,92,28,13,0x454652,false);s.speaker(194,85,5);break;
    case Console::PCEngine:
        cable();s.rounded(22,24,195,73,0xd7d6ca,4);s.rect(29,30,181,2,0x85828b);s.cross(34,40,42);
        s.startSelect(86,64);s.key(5,L"II",153,54,24,24,0x923248);s.key(4,L"I",187,54,24,24,0x923248);s.rounded(155,39,20,10,0x55515b,1);s.rounded(188,39,20,10,0x55515b,1);break;
    case Console::Atari2600:
        cable();s.poly({{55,49},{178,49},{207,78},{194,106},{44,106},{33,79}},0x36333a);
        s.rect(54,87,131,2,0x75604b);s.rect(54,92,131,2,0x75604b);s.oval(80,50,70,47,0x50515a);
        for(int i=0;i<3;i++)s.oval(88+i*4,56+i*3,54-i*8,34-i*6,0x30323b);
        s.rounded(103,13,25,62,0x42434a,3);s.oval(99,6,33,21,0x33353c);s.key(4,L"FIRE",49,54,25,25,red);
        s.buttons[0]={{105,8,21,13},L"↑"};s.buttons[1]={{105,75,21,13},L"↓"};s.buttons[2]={{81,54,19,15},L"←"};s.buttons[3]={{131,54,19,15},L"→"};break;
    case Console::Atari7800:
        cable();s.poly({{66,17},{173,17},{187,29},{194,92},{180,104},{60,104},{46,92},{53,29}},0x56545c);
        s.rect(63,28,112,4,0xa8414d);s.cross(89,41,48);s.key(4,L"FIRE 1",54,50,26,28,red);s.key(5,L"FIRE 2",162,50,26,28,red);break;
    case Console::NintendoDS:
        s.rounded(28,2,209,52,0x8f939e,5);s.screen(82,7,77,41,0x9fb8c6);s.speaker(49,20,3);s.speaker(199,20,3);
        s.rounded(28,57,209,53,0x9296a2,4);s.rect(38,53,190,4,0xb9bcc6);s.screen(83,62,75,42,0x9fb8c6);s.cross(41,67,33);
        s.key(4,L"A",184,77,16,16,0x51596b);s.key(5,L"B",169,91,16,16,0x51596b);s.key(6,L"X",169,62,16,16,0x51596b);s.key(7,L"Y",154,77,16,16,0x51596b);
        s.key(8,L"L",40,52,27,12,0x686e7a,false);s.key(9,L"R",174,52,27,12,0x686e7a,false);s.key(14,L"START",206,69,31,13,0x686e7a,false);s.key(15,L"SELECT",206,87,31,13,0x686e7a,false);break;
    case Console::Nintendo64:
        cable();s.key(15,L"L",25,10,50,14,0x777782,false);s.key(11,L"R",165,10,50,14,0x777782,false);s.poly({{31,23},{74,19},{105,29},{135,29},{164,19},{206,23},{223,44},{216,77},{202,108},{182,108},{169,68},{145,70},{137,110},{105,110},{96,72},{73,68},{58,108},{37,108},{19,74},{17,43}},0x9c9da3);
        s.cross(29,29,36);s.key(14,L"START",105,33,25,18,red,false);s.oval(98,60,45,43,0xc3c4c9);s.oval(107,67,27,26,0x797c88);
        s.key(7,L"B",149,54,23,23,0x42ad73);s.key(5,L"A",166,75,23,23,0x4c77d1);
        s.key(6,L"C↑",185,26,17,17,0xe1bd3d);s.key(8,L"C←",167,43,17,17,0xe1bd3d);s.key(9,L"C→",203,43,17,17,0xe1bd3d);s.key(4,L"C↓",185,59,17,17,0xe1bd3d);s.key(10,L"Z (retro)",105,13,38,14,0x656977,false);break;
    case Console::VirtualBoy:
        cable();shoulder(25,15,47);s.poly({{38,21},{83,25},{102,39},{139,39},{158,25},{204,21},{225,42},{221,92},{203,109},{180,99},{164,71},{80,71},{62,100},{36,109},{17,92},{15,43}},0x53545d);
        s.cross(29,36,36);s.key(10,L"R↑",179,29,20,17,0x31343b,false);s.key(12,L"R↓",179,63,20,17,0x31343b,false);s.key(11,L"R←",160,46,20,17,0x31343b,false);s.key(13,L"R→",198,46,20,17,0x31343b,false);
        s.key(4,L"A",142,52,19,19,red);s.key(5,L"B",123,70,19,19,red);s.startSelect(72,57);break;
    default:break;
    }
    return s;
}
} // namespace controller_art
