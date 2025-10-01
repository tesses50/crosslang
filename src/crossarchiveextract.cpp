#include "CrossLang.hpp"
#include <iostream>
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Filesystem;
using namespace Tesses::Framework::Streams;

int main(int argc, char** argv)
{
    Tesses::Framework::TF_Init();
    if(argc < 3)
    {
        printf("USAGE: %s <archive.crvm> <dirasroot>\n", argv[0]);
        return 1;
    }

    auto sdfs= std::make_shared<SubdirFilesystem>(Tesses::Framework::Filesystem::LocalFS,std::string(argv[2]));

    FILE* f = fopen(argv[1],"rb");
    if(f == NULL) 
    {
        printf("ERROR: could not open %s\n", argv[1]);
        return 1;
    }

    auto strm = std::make_shared<FileStream>(f,true,"rb",true);

    auto res = CrossArchiveExtract(strm,sdfs);

    std::cout << "Crvm Name: " << res.first.first << std::endl;
    std::cout << "Crvm Version: " << res.first.second.ToString() << std::endl;
    std::cout << "Crvm Info: " << std::endl << res.second << std::endl;

    return 0;
}