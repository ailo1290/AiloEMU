// AiloEMU Nintendo 64 load smoke test. GPL-2.0-or-later.
// Additional core-linking permission: ../licenses/AiloEMU-Core-Linking-Exception.txt
#include "core.hpp"
#include <iostream>
int main(int argc,char** argv){
    if(argc!=4)return 2;
    try{
        fs::path root=fs::absolute(argv[2]);fs::create_directories(root/u8"Documenti/ROM con accento è");
        Bytes rom=readBytes(fs::absolute(argv[3]));
        fs::path game=root/u8"Documenti/ROM con accento è/prova.z64";
        writeAtomic(game,rom.data(),rom.size());
        {
            Core core(fs::absolute(argv[1]),root/"saves",root/"system",Console::Nintendo64);
            core.load(game);
            if(!core.loaded)throw std::runtime_error("Core N64 non caricato.");
            core.frame();
            core.close();
        }
        for(const auto& entry:fs::directory_iterator(root/"system"))
            if(entry.path().extension()==".z64")throw std::runtime_error("Cache N64 non rimossa.");
        std::cout<<"RESULT: Nintendo 64 load and Unicode-path cache passed\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
