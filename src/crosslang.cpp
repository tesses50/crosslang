#include "CrossLang.hpp"
#include <iostream>

#include <string>
using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Http;

bool Download(Tesses::Framework::Filesystem::VFSPath filename,std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs)
{
    while(true)
    {
        std::cout << "File " << filename.ToString() << " not found, do you want to download the installer from: https://downloads.tesses.net/ShellPackage.crvm (this may install other stuff as well) (Y/n)? ";
        std::string line;
        std::getline(std::cin,line);
        if(line == "Y" || line == "y")
        {
            HttpRequest req;
            req.url = "https://downloads.tesses.net/ShellPackage.crvm";
            req.method = "GET";
            HttpResponse resp(req);
            if(resp.statusCode == StatusCode::OK)
            {
                auto strm = resp.ReadAsStream();
                CrossArchiveExtract(strm, vfs);
               
                return true;
            }
            else
            {
                std::cout << "Error when fetching the script error: " << std::to_string(resp.statusCode) << " " << HttpUtils::StatusCodeString(resp.statusCode) << std::endl;
                return false;
            }
        }
        else if(line == "N" || line == "n")
        {
            std::cout << "Looks like you will need to install manually" << std::endl;
            return false;
        }
        else 
        {
            std::cout << "Please use Y or N (case insensitive)" << std::endl;
            
        }
    }
    return false;
}

int main(int argc, char** argv)
{
    TF_InitWithConsole();

    
    
    Tesses::Framework::Filesystem::VFSPath dir = GetCrossLangConfigDir();

    Tesses::Framework::Filesystem::VFSPath filename = dir / "Shell" / "Shell.crvm";
    

    auto p = Tesses::Framework::Platform::Environment::GetRealExecutablePath(Tesses::Framework::Filesystem::LocalFS->SystemToVFSPath(argv[0])).GetParent().GetParent() / "share" / "Tesses" / "CrossLang" / "Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm";

    if(argc > 1 && strcmp(argv[1],"update-shell") == 0)
    {

        auto subdir = std::make_shared<Tesses::Framework::Filesystem::SubdirFilesystem>(Tesses::Framework::Filesystem::LocalFS,dir);
        HttpRequest req;
            req.url = "https://downloads.tesses.net/ShellPackage.crvm";
            req.method = "GET";
            HttpResponse resp(req);
            if(resp.statusCode == StatusCode::OK)
            {
                auto strm = resp.ReadAsStream();
                CrossArchiveExtract(strm, subdir);
                
                return 0;
            }
            else
            {
                std::cout << "Error when fetching the script error: " << std::to_string(resp.statusCode) << " " << HttpUtils::StatusCodeString(resp.statusCode) << std::endl;
                return 1;
            }
        return 0;
    }

    if(!Tesses::Framework::Filesystem::LocalFS->RegularFileExists(filename))
    {
        auto subdir = std::make_shared<Tesses::Framework::Filesystem::SubdirFilesystem>(Tesses::Framework::Filesystem::LocalFS,dir);
        if(Tesses::Framework::Filesystem::LocalFS->RegularFileExists(p))
        {
            std::cout << "Installing " << p.ToString() << " -> " << dir.ToString() << std::endl;
            auto strm = Tesses::Framework::Filesystem::LocalFS->OpenFile(p,"rb");
            if(strm != nullptr)
            {
                CrossArchiveExtract(strm, subdir);
                
            }
            else
            {
                return 1;
            }
        }
        else
        {
            if(!Download(filename,subdir)) return 1;
            return 0;
        }
    }
    

    GC gc;
    gc.Start();

    GCList ls(gc);
    TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
    

    TStd::RegisterStd(&gc,env);
    
    
    env->LoadFileWithDependencies(&gc, Tesses::Framework::Filesystem::LocalFS, filename);
    

    TList* args = TList::Create(ls);

    args->Add(filename.ToString());
    
    for(int arg=1;arg<argc;arg++)
        args->Add(std::string(argv[arg]));
   
    auto res = env->CallFunctionWithFatalError(ls,"main",{args});
    int64_t iresult;
    if(GetObject(res,iresult))
        return (int)iresult;
    return 0;
}
