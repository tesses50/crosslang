#include <CrossLang.hpp>
#include <iostream>
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Filesystem;
using namespace Tesses::Framework::Streams;
void Help(const char* filename)
{
    printf("USAGE: %s [OPTIONS] <dirasroot> <archive.crvm>\n", filename);
    printf("OPTIONS:\n");
    printf("  -i:  Set info (ex {\"maintainer\": \"Mike Nolan\", \"repo\": \"https://example.com/\", \"homepage\": \"https://example.com/\",\"license\":\"MIT\"})\n");
    printf("  -I:  Set icon name (relative to dirasroot), should be a 128x128 png\n");
    printf("  -v:  Set version (1.0.0.0-prod defaults to 1.0.0.0-dev)\n");
    printf("  -n:  Set name (MyAppOrLibName defaults to out)\n");
    printf("  -h, --help:  Prints help\n");
    printf("Options except for help have flag with arg like this: -F ARG\n");
    exit(1);
}
int main(int argc, char** argv)
{
    std::string name="out";
   std::string info="{}";
   TVMVersion version;
   std::string icon="";
   std::vector<std::string> args;
    for(int i = 1; i < argc; i++)
   {
        if(strcmp(argv[i],"--help") == 0 || strcmp(argv[i],"-h")==0) 
        {
            Help(argv[0]);
        }
        else if(strcmp(argv[i], "-i") == 0)
        {
            i++;
            if(i < argc)
            {
                info = argv[i];
            }
        }
        else if(strcmp(argv[i], "-I") == 0)
        {
            i++;
            if(i < argc)
            {
                icon = argv[i];
            }
        }
        else if(strcmp(argv[i], "-n") == 0)
        {
            i++;
            if(i < argc)
            {
                name = argv[i];
            }
        }
        else if(strcmp(argv[i], "-v") == 0)
        {
            i++;
            if(i < argc)
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
 
    
    LocalFilesystem fs;
    auto path = fs.SystemToVFSPath(args[0]);
    fs.CreateDirectory(path);
    SubdirFilesystem sdfs(&fs,path,false);

    FILE* f = fopen(args[1].c_str(),"wb");
    if(f == NULL) 
    {
        printf("ERROR: could not open %s\n", args[1].c_str());
        return 1;
    }

    FileStream strm(f,true,"wb",true);
    CrossArchiveCreate(&sdfs,&strm,name,version,info,icon);

    return 0;
}