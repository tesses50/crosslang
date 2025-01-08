#include "CrossLang.hpp"

#if defined(GEKKO)
#undef CROSSLANG_ENABLE_PROCESS
#endif

//#if defined(CROSSLANG_ENABLE_PROCESS)
#include "subprocess.h"
//#endif

namespace Tesses::CrossLang
{
    //#if defined(CROSSLANG_ENABLE_PROCESS)
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

            std::string filename;
            TList* _args;
            TList* env;
            std::vector<std::string> _args2;
            std::vector<std::string> _env;

            if(GetObject(fobj,filename))
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



            gc->BarrierEnd();
            subprocess_create_ex()
        }
        return nullptr;
    }
    //#endif
}