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

    for(int i = 1; i < argc; i++)
    {
        VFSPath path(argv[i]);
        if(LocalFS->FileExists(path))
        {
            std::cout << "File: " << path.ToString() << std::endl;
            auto strm = LocalFS->OpenFile(path, "rb");
            Programs::CrossLangDump(strm);
        }
        else {
            std::cout << "File: " << path.ToString() << " does not exist." << std::endl;
        }
    }
}