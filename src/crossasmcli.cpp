#include "CrossLang.hpp"

int main(int argc, char** argv)
{
    using namespace Tesses::Framework;
    using namespace Tesses::Framework::Streams;
    using namespace Tesses::Framework::Filesystem;
    using namespace Tesses::CrossLang;
    TF_Init();
    if(argc > 1 && strcmp(argv[1],"--help"))
    {
        std::cout << "Run this command in directory you want to assemble (with the crossasm.json)" << std::endl;
        return 0;
    }
    auto curdir = VFSPath::GetAbsoluteCurrentDirectory();
    auto sdfs=std::make_shared<SubdirFilesystem>(LocalFS,curdir);
    auto path = Assemble(sdfs);
    path.relative = true;
    std::cout << "Output: " << (curdir / path).ToString() << std::endl;

    return 0;
}