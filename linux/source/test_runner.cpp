// Additional core-linking permission: ../licenses/AiloEMU-Core-Linking-Exception.txt
// GPL-2.0-or-later.
#include "diagnostics.hpp"
#include <iostream>
int main(int argc,char** argv){
    if(argc!=4){std::cerr<<"usage: test_runner core_library demo.nes output_directory\n";return 2;}
    return runDiagnostics(fs::absolute(argv[1]),fs::absolute(argv[2]),fs::absolute(argv[3]));
}
