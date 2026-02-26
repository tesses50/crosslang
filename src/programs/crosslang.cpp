#include "CrossLang.hpp"

using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Filesystem;
int main(int argc, char** argv)
{
    TF_InitWithConsole();
    if(argc > 0)
        TF_AllowPortable(argv[0]);
    GC gc;
    gc.Start();
    GCList ls(gc);
    

    std::vector<std::string> args(argc);
    for(int i = 0; i < argc; i++)
        args[i] = argv[i];
    auto res = Programs::CrossLangShell(ls, args);

    int64_t myi64;
    if(GetObject(res,myi64))
        return (int)myi64;
    return 0;
}