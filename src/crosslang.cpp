#include "CrossLang.hpp"
#include <iostream>
#include "sago/platform_folders.h"
#include <mbedtls/error.h>
#include <string>
using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Http;

bool Download(Tesses::Framework::Filesystem::VFSPath filename,Tesses::Framework::Filesystem::VFS* vfs)
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
                delete strm;
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
    TF_Init();
    
    Tesses::Framework::Filesystem::VFSPath dir = sago::getConfigHome();
    dir = dir / "Tesses" / "CrossLang";

    Tesses::Framework::Filesystem::VFSPath filename = dir / "Shell" / "Shell.crvm";
    
    Tesses::Framework::Filesystem::LocalFilesystem fs;

    auto p = GetRealExecutablePath(fs.SystemToVFSPath(argv[0])).GetParent().GetParent() / "share" / "Tesses" / "CrossLang" / "Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm";

    if(!fs.RegularFileExists(filename))
    {
        Tesses::Framework::Filesystem::SubdirFilesystem subdir(&fs,dir,false);
        if(fs.RegularFileExists(p))
        {
            std::cout << "Installing " << p.ToString() << " -> " << dir.ToString() << std::endl;
            auto strm = fs.OpenFile(p,"rb");
            if(strm != nullptr)
            {
                CrossArchiveExtract(strm, &subdir);
                delete strm;
            }
            else
            {
                return 1;
            }
        }
        else
        {
            if(!Download(filename,&subdir)) return 1;
        }
    }
    

    GC gc;
    gc.Start();

    GCList ls(gc);
    TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
    

    TStd::RegisterStd(&gc,env);
    
    
    env->LoadFileWithDependencies(&gc, &fs, filename);
    

    TList* args = TList::Create(ls);

    args->Add(filename.ToString());
    
    for(int arg=1;arg<argc;arg++)
        args->Add(std::string(argv[arg]));
   
    auto res = env->CallFunction(ls,"main",{args});
    int64_t iresult;
    if(GetObject(res,iresult))
        return (int)iresult;
    return 0;
}
