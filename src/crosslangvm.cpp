#include "CrossLang.hpp"

#include <mbedtls/error.h>
using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
int main(int argc, char** argv)
{
    TF_Init();
    if(argc < 2)
    {
        printf("USAGE: %s <filename.crvm> <args...>\n",argv[0]);
        return 1;
    }
    
    GC gc;
    gc.Start();
    GCList ls(gc);
    TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
    Tesses::Framework::Filesystem::LocalFilesystem fs;
    TStd::RegisterStd(&gc,env);
    env->LoadFileWithDependencies(&gc, &fs, fs.SystemToVFSPath(argv[1]));
    
    TList* args = TList::Create(ls);
    for(int arg=1;arg<argc;arg++)
        args->Add(std::string(argv[arg]));
   
    auto res = env->CallFunction(ls,"main",{args});
    int64_t iresult;
    if(GetObject(res,iresult))
        return (int)iresult;
    return 0;
}