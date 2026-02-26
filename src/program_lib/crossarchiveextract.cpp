#include "CrossLang.hpp"
#include <iostream>

namespace Tesses::CrossLang::Programs {
using namespace Tesses::Framework::Filesystem;
using namespace Tesses::Framework::Streams;

int64_t CrossArchiveExtract(std::vector<std::string>& argv)
{
    Tesses::Framework::TF_Init();
    if(argv.size() < 3)
    {
        std::cout << "USAGE: " << argv[0] << " <archive.crvm> <dirasroot>" << std::endl;
        return 1;
    }

    auto sdfs= std::make_shared<SubdirFilesystem>(Tesses::Framework::Filesystem::LocalFS,std::string(argv[2]));
    auto strm= LocalFS->OpenFile(argv[1], "rb");
    if(strm->CanRead()) 
    {
        std::cout << "ERROR: could not open " << argv[1] << std::endl;
        return 1;
    }


    auto res = Tesses::CrossLang::CrossArchiveExtract(strm,sdfs);

    std::cout << "Crvm Name: " << res.first.first << std::endl;
    std::cout << "Crvm Version: " << res.first.second.ToString() << std::endl;
    std::cout << "Crvm Info: " << std::endl << res.second << std::endl;

    return 0;
}
}