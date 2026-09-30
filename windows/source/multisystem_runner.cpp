// Additional core-linking permission: ../licenses/AiloEMU-Core-Linking-Exception.txt
#include "multisystem_tests.hpp"
int main(int argc,char** argv){if(argc!=5)return 2;return runExtraTests(fs::absolute(argv[1]),fs::absolute(argv[2]),fs::absolute(argv[3]),fs::absolute(argv[4]));}
