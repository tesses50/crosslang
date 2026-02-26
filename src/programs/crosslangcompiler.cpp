#include "CrossLang.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Filesystem;
int main(int argc, char** argv)
{
    TF_InitWithConsole();
    if(argc > 0)
        TF_AllowPortable(argv[0]);
   
    

    std::vector<std::string> args(argc);
    for(int i = 0; i < argc; i++)
        args[i] = argv[i];
    Programs::CrossLangCompiler(args);

    return 0;
}