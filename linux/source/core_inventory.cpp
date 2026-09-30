// Loads each libretro module without game content and verifies its public ABI.
#include "libretro.h"
#include <dlfcn.h>
#include <iostream>
#include <stdexcept>
#include <string>

template<class T> T symbol(void* module,const char* name){
    auto result=reinterpret_cast<T>(dlsym(module,name));
    if(!result)throw std::runtime_error(std::string("simbolo mancante: ")+name);
    return result;
}
static bool environment(unsigned command,void* data){
    if(command==RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION){*static_cast<unsigned*>(data)=2;return true;}
    if(command==RETRO_ENVIRONMENT_GET_CAN_DUPE){*static_cast<bool*>(data)=true;return true;}
    if(command==RETRO_ENVIRONMENT_GET_LANGUAGE){*static_cast<unsigned*>(data)=RETRO_LANGUAGE_ENGLISH;return true;}
    return false;
}
int main(int argc,char** argv){
    if(argc!=2){std::cerr<<"uso: core_inventory core.so\n";return 2;}
    void* module=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);
    if(!module){std::cerr<<dlerror()<<"\n";return 1;}
    try{
        auto setEnvironment=symbol<decltype(&retro_set_environment)>(module,"retro_set_environment");
        auto init=symbol<decltype(&retro_init)>(module,"retro_init");
        auto deinit=symbol<decltype(&retro_deinit)>(module,"retro_deinit");
        auto api=symbol<decltype(&retro_api_version)>(module,"retro_api_version");
        auto info=symbol<decltype(&retro_get_system_info)>(module,"retro_get_system_info");
        symbol<decltype(&retro_load_game)>(module,"retro_load_game");
        symbol<decltype(&retro_run)>(module,"retro_run");
        symbol<decltype(&retro_serialize)>(module,"retro_serialize");
        if(api()!=RETRO_API_VERSION)throw std::runtime_error("API libretro incompatibile");
        setEnvironment(environment);init();
        retro_system_info details{};info(&details);
        if(!details.library_name||!details.valid_extensions)throw std::runtime_error("metadati core incompleti");
        std::cout<<details.library_name<<" | "<<(details.library_version?details.library_version:"?")
                 <<" | "<<details.valid_extensions<<"\n";
        deinit();dlclose(module);return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<"\n";dlclose(module);return 1;}
}
