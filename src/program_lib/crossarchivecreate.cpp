#include "CrossLang.hpp"
#include <iostream>
namespace Tesses::CrossLang::Programs {
using namespace Tesses::Framework::Filesystem;
using namespace Tesses::Framework::Streams;
static void Help(std::string& filename)
{
    std::cout << "USAGE: " << filename << " [OPTIONS] <dirasroot> <archive.crvm>" << std::endl;
    printf("OPTIONS:\n");
    printf("  -i:  Set info (ex {\"maintainer\": \"Mike Nolan\", \"repo\": \"https://example.com/\", \"homepage\": \"https://example.com/\",\"license\":\"MIT\"})\n");
    printf("  -I:  Set icon name (relative to dirasroot), should be a 128x128 png\n");
    printf("  -v:  Set version (1.0.0.0-prod defaults to 1.0.0.0-dev)\n");
    printf("  -n:  Set name (MyAppOrLibName defaults to out)\n");
    printf("  -h, --help:  Prints help\n");
    printf("Options except for help have flag with arg like this: -F ARG\n");
    exit(1);
}

int64_t CrossArchiveCreate(std::vector<std::string>& argv)
{
    Tesses::Framework::TF_Init();
    std::string name="out";
   std::string info="{}";
   TVMVersion version;
   std::string icon="";
   std::vector<std::string> args;
    for(int i = 1; i < argv.size(); i++)
   {
        if(argv[i] == "--help" || argv[i] == "-h") 
        {
            Help(argv[0]);
        }
        else if(argv[i] == "-i")
        {
            i++;
            if(i < argv.size())
            {
                info = argv[i];
            }
        }
        else if(argv[i] == "-I")
        {
            i++;
            if(i < argv.size())
            {
                icon = argv[i];
            }
        }
        else if(argv[i] == "-n")
        {
            i++;
            if(i < argv.size())
            {
                name = argv[i];
            }
        }
        else if(argv[i] == "-v")
        {
            i++;
            if(i < argv.size())
            {
                
                if(!TVMVersion::TryParse(argv[i],version))
                {
                    printf("ERROR: Invalid syntax for version\n");
                    printf("Expected MAJOR[.MINOR[.PATCH[.BUILD[-dev,-alpha,-beta,-prod]]]]\n");
                    exit(1);
                }
            }
        }
        else {
            args.push_back(argv[i]);
        }
   }

   if(args.size() < 2) Help(argv[0]);
 
    
    auto path = Tesses::Framework::Filesystem::LocalFS->SystemToVFSPath(args[0]);
    Tesses::Framework::Filesystem::LocalFS->CreateDirectory(path);
    auto sdfs = std::make_shared<SubdirFilesystem>(Tesses::Framework::Filesystem::LocalFS,path);

    FILE* f = fopen(args[1].c_str(),"wb");
    if(f == NULL) 
    {
        printf("ERROR: could not open %s\n", args[1].c_str());
        return 1;
    }

    auto strm = std::make_shared<FileStream>(f,true,"wb",true);
    CrossArchiveCreate(sdfs,strm,name,version,info,icon);

    return 0;
}
}