#include "CrossLang.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Filesystem;
static GC gc;
int main(int argc, char** argv)
{
    TF_InitWithConsole();
    if(argc > 0)
        TF_AllowPortable(argv[0]);
    gc.Start();
    GCList ls(gc);
    TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
    TStd::RegisterStd(&gc,env);

    std::vector<std::string> args(argc);
    for(int i = 0; i < argc; i++)
        args[i] = argv[i];
    auto res = Programs::CrossLangInterperter(ls, env, args);

    int64_t myi64;
    if(GetObject(res,myi64))
        return (int)myi64;
    return 0;
}
