#include "CrossLang.hpp"
using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
int main(int argc, char** argv)
{
    TF_InitWithConsole();
    
    if(argc < 2)
    {
        printf("USAGE: %s <filename.crvm> <args...>\n",argv[0]);
        return 1;
    }
    if(argc > 0)
        TF_AllowPortable(argv[0]);
    
    GC gc;
    gc.Start();
    GCList ls(gc);
    TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
    
    TStd::RegisterStd(&gc,env);
    env->LoadFileWithDependencies(&gc, Tesses::Framework::Filesystem::LocalFS, Tesses::Framework::Filesystem::LocalFS->SystemToVFSPath(argv[1]));
    
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

        env->EnsureDictionary(&gc,"Net")->SetValue("WebServerPort", (int64_t)port);
        TList* args2 = TList::Create(ls);
        for(auto& item : args.positional)
        {
            args2->Add(item);
        }

        auto res = env->CallFunctionWithFatalError(ls, "WebAppMain", {args2});
        auto svr2 = Tesses::CrossLang::ToHttpServer(&gc,res);
        if(svr2 == nullptr) return 1;
        Tesses::Framework::Http::HttpServer svr(port,svr2);
        svr.StartAccepting();
        TF_RunEventLoop();
        TDictionary* _dict;
        TClassObject* _co;
        if(GetObjectHeap(res,_dict))
        {
            _dict->CallMethod(ls,"Close",{});
        }
        if(GetObjectHeap(res,_co))
        {
            _co->CallMethod(ls,"","Close",{});
        }
        TF_Quit();
    }
    else {
    TList* args = TList::Create(ls);
    for(int arg=1;arg<argc;arg++)
        args->Add(std::string(argv[arg]));
   
    auto res = env->CallFunctionWithFatalError(ls,"main",{args});
    int64_t iresult;
    if(GetObject(res,iresult))
        return (int)iresult;
    }
    return 0;
}