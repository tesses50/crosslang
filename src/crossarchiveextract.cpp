#include <CrossLang.hpp>
#include <iostream>
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Filesystem;
using namespace Tesses::Framework::Streams;

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        printf("USAGE: %s <archive.crvm> <dirasroot>\n", argv[0]);
        return 1;
    }

    LocalFilesystem fs;
    SubdirFilesystem sdfs(&fs,std::string(argv[2]),false);

    FILE* f = fopen(argv[1],"rb");
    if(f == NULL) 
    {
        printf("ERROR: could not open %s\n", argv[1]);
        return 1;
    }

    FileStream strm(f,true,"rb",true);

    auto res = CrossArchiveExtract(&strm,&sdfs);

    std::cout << "Crvm Name: " << res.first.first << std::endl;
    std::cout << "Crvm Version: " << res.first.second.ToString() << std::endl;
    std::cout << "Crvm Info: " << std::endl << res.second << std::endl;

    return 0;
}