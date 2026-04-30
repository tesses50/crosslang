#include "CrossLang.hpp"
#include "TessesFramework/Filesystem/VFS.hpp"
using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Filesystem;

int main(int argc, char** argv)
{
    //crosslang crossint
    //crosslang
    //crosslang ...

    std::string programName = "crosslang";

    TF_InitWithConsole();
    if(argc > 0)
    {
        TF_AllowPortable(argv[0]);
        Tesses::Framework::Filesystem::VFSPath path=(std::string)argv[0];
        path.RemoveExtension();
        programName = path.GetFileName();
    }

    std::vector<std::string> args(argc);
    for(int i = 0; i < argc; i++)
        args[i] = argv[i];
        if(programName == "crossint")
        {
            GC gc;
            gc.Start();
            int64_t myi64=0;
            {
            GCList ls(gc);
            TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
            TStd::RegisterStd(&gc,env);
            auto res= Programs::CrossLangInterperter(ls, env, args);

                GetObject(res,myi64);

            }
            return (int)myi64;
        }
        else if(programName == "crossc")
        {
            Programs::CrossLangCompiler(args);
            return 0;
        }
        else if(programName == "crossarchivecreate")
        {
            Programs::CrossArchiveCreate(args);
            return 0;
        }
        else if(programName == "crossarchiveextract")
        {
            Programs::CrossArchiveExtract(args);
            return 0;
        }
        else if(programName == "crossdump")
        {
            for(size_t i = 1; i < args.size(); i++)
            {
                VFSPath path = args[i];
                if(LocalFS->FileExists(path))
                {
                    std::cout << "File: " << path.ToString() << std::endl;
                    auto strm = LocalFS->OpenFile(path, "rb");
                    Programs::CrossLangDump(strm);
                }
                else {
                    std::cout << "File: " << path.ToString() << " does not exist." << std::endl;
                }
            }
            return 0;
        }
        else if(programName == "crossvm")
        {
            GC gc;
            gc.Start();
            int64_t myi64=0;

            {
            GCList ls(gc);
            TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
            TStd::RegisterStd(&gc,env);
            auto res= Programs::CrossLangVM(ls, env, args);
            GetObject(res,myi64);
            }
                return (int)myi64;

    }
    else if(args.size() > 1)
    {

        if(args[1] == "crossint")
        {
            int64_t myi64=0;
            args.erase(args.begin());
            GC gc;
            gc.Start();
            {
            GCList ls(gc);
            TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
            TStd::RegisterStd(&gc,env);
            auto res= Programs::CrossLangInterperter(ls, env, args);

                GetObject(res,myi64);
            }
            return (int)myi64;

        }
        else if(args[1] == "crossc")
        {
            args.erase(args.begin());
            Programs::CrossLangCompiler(args);
            return 0;
        }
        else if(args[1] == "crossarchivecreate")
        {
            args.erase(args.begin());
            Programs::CrossArchiveCreate(args);
            return 0;
        }
        else if(args[1] == "crossarchiveextract")
        {
            args.erase(args.begin());
            Programs::CrossArchiveExtract(args);
            return 0;
        }
        else if(args[1] == "crossdump")
        {
            for(size_t i = 2; i < args.size(); i++)
            {
                VFSPath path = args[i];
                if(LocalFS->FileExists(path))
                {
                    std::cout << "File: " << path.ToString() << std::endl;
                    auto strm = LocalFS->OpenFile(path, "rb");
                    Programs::CrossLangDump(strm);
                }
                else {
                    std::cout << "File: " << path.ToString() << " does not exist." << std::endl;
                }
            }
            return 0;
        }
        else if(args[1] == "crossvm")
        {
            int64_t myi64=0;

            args.erase(args.begin());
            GC gc;

            gc.Start();
            {
            GCList ls(gc);
            TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
            TStd::RegisterStd(&gc,env);
            auto res= Programs::CrossLangVM(ls, env, args);
                GetObject(res,myi64);
            }

            return (int)myi64;
        }
    }
    {
         int64_t myi64=0;
        GC gc;

        gc.Start();
        {
            GCList ls(gc);
            auto res= Programs::CrossLangShell(ls, args);

            GetObject(res,myi64);


        }
        return (int)myi64;
    }
}
