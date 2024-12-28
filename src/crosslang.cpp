#include "CrossLang.hpp"
#include <iostream>
#include "sago/platform_folders.h"
#include <mbedtls/error.h>
#include <string>
using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Http;

int main(int argc, char** argv)
{
    TF_Init();
    
    Tesses::Framework::Filesystem::VFSPath filename = sago::getConfigHome();
    filename = filename / "Tesses" / "CrossLang" / "Shell" / "Shell.crvm";
    
    Tesses::Framework::Filesystem::LocalFilesystem fs;
    GC gc;
    gc.Start();
    GCList ls(gc);
    tryAgain:
    TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
    

    TStd::RegisterStd(&gc,env);
    
    if(fs.RegularFileExists(filename))
    env->LoadFileWithDependencies(&gc, &fs, filename);
    else
    {
        tryAgainFast:
        std::cout << "File " << filename.ToString() << " not found, do you want to download the installer from: https://crosslang.tesseslanguage.com/crosslang-shell-install.tcross (this will install other stuff as well) (Y/n)? ";
        std::string line;
        std::getline(std::cin,line);
        if(line == "Y" || line == "y")
        {
            HttpRequest req;
            req.url = "https://crosslang.tesseslanguage.com/crosslang-shell-install.tcross";
            req.method = "GET";
            HttpResponse resp(req);
            if(resp.statusCode == StatusCode::OK)
            {
                std::string str = resp.ReadAsString();
                env->Eval(ls, str);
                goto tryAgain;
            }
            else
            {
                std::cout << "Error when fetching the script error: " << std::to_string(resp.statusCode) << " " << HttpUtils::StatusCodeString(resp.statusCode) << std::endl;
                return 1;
            }
        }
        else if(line == "N" || line == "n")
        {
            std::cout << "Looks like you will need to install manually" << std::endl;
            return 0;
        }
        else 
        {
            std::cout << "Please use Y or N (case insensitive)" << std::endl;
            goto tryAgainFast;
        }
    }

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