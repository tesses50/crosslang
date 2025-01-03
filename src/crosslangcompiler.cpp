#include "CrossLang.hpp"
#include <iostream>
#include <fstream>
using namespace Tesses::CrossLang;
using namespace Tesses::Framework;
using namespace Tesses::Framework::Filesystem;
void Help(const char* filename)
{
    printf("USAGE: %s [OPTIONS] source_file1 source_file2 source_file_n \n", filename);
    printf("OPTIONS:\n");
    printf("  -o:  Output directory (OUTDIR, defaults to ./bin)\n");
    printf("  -i:  Set info (ex {\"maintainer\": \"Mike Nolan\", \"repo\": \"https://example.com/\", \"homepage\": \"https://example.com/\",\"license\":\"MIT\"})\n");
    printf("  -v:  Set version (1.0.0.0-prod defaults to 1.0.0.0-dev)\n");
    printf("  -d:  Add dependency (DependencyName-1.0.0.0-prod)\n");
    printf("  -n:  Set name (MyAppOrLibName defaults to out)\n");
    printf("  -r:  Set resource directory (RESDIR defaults to res)\n");
    printf("  -h, --help:  Prints help\n");
    printf("Options except for help have flag with arg like this: -F ARG\n");
    exit(1);
}
int main(int argc, char** argv)
{
    /*std::ifstream strm(argv[1],std::ios_base::in|std::ios_base::binary);
    std::vector<LexToken> tokens;
    Lex(argv[1],strm,tokens);

    Parser parser(tokens);

    CodeGen gen;
    gen.GenRoot(parser.ParseRoot());
    std::vector<uint8_t> data;
    ByteCodeVectorWriter w(data);
    gen.Save(std::filesystem::current_path(),w);
    */
   TF_Init();
   std::filesystem::path outputDir = std::filesystem::current_path() / "bin";
   std::vector<std::filesystem::path> source;
   std::filesystem::path resourceDir = std::filesystem::current_path() / "res";
   std::vector<std::pair<std::string, TVMVersion>> dependencies;
   std::string name="out";
   std::string info="{}";
   TVMVersion version;
   

   for(int i = 1; i < argc; i++)
   {
        if(strcmp(argv[i],"--help") == 0 || strcmp(argv[i],"-h")==0) 
        {
            Help(argv[0]);
        }
        else if(strcmp(argv[i], "-o") == 0)
        {
            i++;
            if(i < argc)
            {
                outputDir = argv[i];
            }
        }
        else if(strcmp(argv[i], "-r") == 0)
        {
            i++;
            if(i < argc)
            {
                resourceDir = argv[i];
            }
        }
        else if(strcmp(argv[i], "-i") == 0)
        {
            i++;
            if(i < argc)
            {
                info = argv[i];
            }
        }
        else if(strcmp(argv[i], "-d") == 0)
        {
            i++;
            if(i < argc)
            {
                std::string str = argv[i];
                auto lastDash = str.find_last_of('-');
                if(lastDash < str.size())
                {
                    std::string str2 = str.substr(lastDash+1);
                    if(str2 == "dev" || str2 == "alpha" || str2 == "beta" || str2 == "prod")
                    {
                        lastDash = str.find_last_of('-',lastDash-1);
                    }
                    std::string str1 = str.substr(0,lastDash);
                    str2 = str.substr(lastDash+1);
                    
                    TVMVersion v2;
                    if(!TVMVersion::TryParse(str2,v2))
                    {
                        printf("ERROR: Invalid syntax for version\n");
                        printf("Expected MAJOR[.MINOR[.PATCH[.BUILD[-dev,-alpha,-beta,-prod]]]]\n");
                        exit(1);
                    }
                    dependencies.push_back(std::pair<std::string,TVMVersion>(str1,v2));
                    
                }
                else
                {
                    printf("ERROR: Dependency must have version\n");
                    exit(1);
                }
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
            source.push_back(argv[i]);
        }
   }
    if(source.empty())
   {
        Help(argv[0]);
   }
  

    std::vector<LexToken> tokens;

    for(auto src : source)
    {
        std::ifstream strm(src,std::ios_base::in|std::ios_base::binary);
        int res = Lex(argv[1],strm,tokens);
        
    }
    
    Parser parser(tokens);

    CodeGen gen;
    gen.GenRoot(parser.ParseRoot());
    gen.name = name;
    gen.version = version;
    gen.info = info;
    for(auto deps : dependencies)
    {
        gen.dependencies.push_back(deps);
    }
    std::filesystem::create_directory(outputDir);

    {
        Tesses::Framework::Streams::FileStream strm(outputDir / (name + "-" + version.ToString() + ".crvm"),"wb");
    
        LocalFilesystem fs;
        SubdirFilesystem sfs(&fs,fs.SystemToVFSPath(resourceDir.string()),false);

        gen.Save(&sfs,&strm);
    }
    return 0;
}