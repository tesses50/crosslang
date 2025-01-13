#include "CrossLang.hpp"

#if defined(GEKKO)
#undef CROSSLANG_ENABLE_PROCESS
#endif

#if defined(CROSSLANG_ENABLE_PROCESS)
#include "subprocess.h"
#endif

namespace Tesses::CrossLang
{
    #if defined(CROSSLANG_ENABLE_PROCESS)
    static TObject Process_Start(GCList& ls, std::vector<TObject> args)
    {
        
        //Process.Start({
        // FileName = "git",
        // Arguments = ["clone","https://gitea.site.tesses.net/tesses50/crosslang.git"],
        // Environment = []
        //})

        TDictionary* dict;

        if(GetArgumentHeap(args, 0, dict))
        {
            auto gc = ls.GetGC();
            gc->BarrierBegin();

            auto fobj = dict->GetValue("FileName");
            auto myargs = dict->GetValue("Arguments");
            auto env = dict->GetValue("Environment");
            auto opt = dict->GetValue("Options");
            //for any C# version
            //RedirectStandardIn
            //RedirectStandardOut
            //RedirectStandardError

            std::string filename;
            TList* _args;
            TList* env0;
            std::vector<std::string> _args2;
            std::vector<std::string> _env;
            int64_t options;

            bool hasEnv=false;

            if(!GetObject(fobj,filename))
            {
                gc->BarrierEnd();
                return nullptr;
            }
            _args2.push_back(filename);
            if(GetObjectHeap(myargs,_args))
            {
                for(auto a : _args->items)
                {
                    std::string a2;
                    if(GetObject(a,a2))
                    {
                        _args2.push_back(a2);
                    }
                }
            }
            if(GetObjectHeap(env,env0))
            {
                hasEnv=true;
                for(auto a : env0->items)
                {
                    std::string a2;
                    if(GetObject(a,a2))
                    {
                        _env.push_back(a2);
                    }
                }
            }

            const char** args3 = new const char*[sizeof(char*) * (_args2.size()+1)];
            const char** env3 = hasEnv ? new const char*[sizeof(char*) * (_env.size()+1)] : nullptr;

            for(size_t i = 0; i < _args2.size();i++)
                args3[i] = _args2[i].c_str();
            args3[_args2.size()]=NULL;

            if(hasEnv)
            {
                for(size_t i = 0; i < _env.size();i++)
                    env3[i] = _env[i].c_str();
                env3[_env.size()]=NULL;
            }


            if(!GetObject(opt,options))options=hasEnv ? 0 : subprocess_option_inherit_environment;

            gc->BarrierEnd();
            struct subprocess_s* subprocess=new struct subprocess_s();
            int r = subprocess_create_ex(args3,(int)options,env3,subprocess);
            if(r != 0)
            {
                printf("Here2 %i\n",r);
                delete[] args3;
                delete[] env3;
                delete subprocess;
                return nullptr;
            }
            else
            {
                delete[] args3;
                delete[] env3;
                
                TDictionary* dict=TDictionary::Create(ls);
                auto r = TNative::Create(ls,subprocess,[](void* v)->void{
                    delete (struct subprocess_s*)v;
                });
                gc->BarrierBegin();
                dict->SetValue("_native",r);
                gc->BarrierEnd();
                dict->DeclareFunction(gc,"getHasExited","Gets whether process has stopped",{},[r](GCList& ls, std::vector<TObject> args)->TObject{
                    if(r->GetDestroyed()) return true;
                    return subprocess_alive((struct subprocess_s*)r->GetPointer()) == 0;
                });
                dict->DeclareFunction(gc,"Join","Wait till process exits and get its return code",{},[r](GCList& ls, std::vector<TObject> args)->TObject{
                    if(r->GetDestroyed()) return nullptr;
                    int returnCode;
                    if(subprocess_join((struct subprocess_s*)r->GetPointer(),&returnCode) == 0)
                    {
                        return (int64_t)returnCode;
                    }
                    return nullptr;
                });

                 dict->DeclareFunction(gc,"Terminate","Terminate the process",{},[r](GCList& ls, std::vector<TObject> args)->TObject{
                    if(r->GetDestroyed()) return nullptr;
                    subprocess_terminate((struct subprocess_s*)r->GetPointer());
                    return nullptr;
                });
                 dict->DeclareFunction(gc,"Close","Wait till process exits and get its return code",{},[r](GCList& ls, std::vector<TObject> args)->TObject{
                    if(r->GetDestroyed()) return nullptr;
                    r->Destroy();
                    return nullptr;
                });

                dict->DeclareFunction(gc,"getStandardInput","Gets the standard input stream",{},[r](GCList& ls,std::vector<TObject> args)->TObject {
                    if(r->GetDestroyed()) return nullptr;
                    return TStreamHeapObject::Create(ls,new Tesses::Framework::Streams::FileStream(subprocess_stdin((struct subprocess_s*)r->GetPointer()),false,"w",false));
                });
                dict->DeclareFunction(gc,"getStandardOutput","Gets the standard output stream",{},[r](GCList& ls,std::vector<TObject> args)->TObject {
                    if(r->GetDestroyed()) return nullptr;
                    return TStreamHeapObject::Create(ls,new Tesses::Framework::Streams::FileStream(subprocess_stdout((struct subprocess_s*)r->GetPointer()),false,"r",false));
                });
                dict->DeclareFunction(gc,"getStandardError","Gets the standard error stream",{},[r](GCList& ls,std::vector<TObject> args)->TObject {
                    if(r->GetDestroyed()) return nullptr;
                    return TStreamHeapObject::Create(ls,new Tesses::Framework::Streams::FileStream(subprocess_stderr((struct subprocess_s*)r->GetPointer()),false,"r",false));
                });
                return dict;
            }
        }
        return nullptr;
    }
    #endif

    void TStd::RegisterProcess(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterProcess=true;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        #if defined(CROSSLANG_ENABLE_PROCESS)
        dict->DeclareFunction(gc,"Start","Start a process",{"process_object"},Process_Start);
        #endif
        gc->BarrierBegin();
        env->SetVariable("Process",dict);
        gc->BarrierEnd();
    }
}