#include <CrossLang.hpp>
using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
int main(int argc, char** argv)
{
    std::string name = argv[0];
    Tesses::Framework::Filesystem::LocalFilesystem fs;
    Tesses::Framework::Filesystem::VFSPath exePath=fs.SystemToVFSPath(name);
    exePath.MakeAbsolute();
    exePath.ChangeExtension(".crvm");


    TF_InitWithConsole();
        
    GC gc;
    gc.Start();
    GCList ls(gc);
    TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
    
    TStd::RegisterStd(&gc,env);
    env->LoadFileWithDependencies(&gc, &fs, exePath);
    
   

    if(env->HasVariable("WebAppMain"))
    {
        Args args(argc, argv);
        int port = 4206;
        for(auto& item : args.options)
        {
            if(item.first == "port")
            {
                port = std::stoi(item.second);
            }
        }
        TList* args2 = TList::Create(ls);
        args2->Add(exePath.ToString());
        for(auto& item : args.positional)
        {
            args2->Add(item);
        }

        auto res = env->CallFunction(ls, "WebAppMain", {args2});
        TObjectHttpServer http(&gc, res);
        Tesses::Framework::Http::HttpServer svr(port,http,false);
        svr.StartAccepting();
        TF_RunEventLoop();
        TF_Quit();
    }
    else {
         TList* args = TList::Create(ls);
         args->Add(exePath.ToString());
        for(int arg=1;arg<argc;arg++)
            args->Add(argv[arg]);
        auto res = env->CallFunction(ls,"main",{args});
        int64_t iresult;
        if(GetObject(res,iresult))
            return (int)iresult;
    }
    return 0;
}