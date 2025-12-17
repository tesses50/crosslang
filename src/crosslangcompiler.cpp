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
    printf("  -I:  Set icon resource name (in the resource folder), should be a 128x128 png\n");
    printf("  -v:  Set version (1.0.0.0-prod defaults to 1.0.0.0-dev)\n");
    printf("  -d:  Add dependency (DependencyName-1.0.0.0-prod)\n");
    printf("  -D: enable debug)\n");
    printf("  -t:  Declare a tool (ToolName-1.0.0.0-prod)\n");
    printf("  -n:  Set name (MyAppOrLibName defaults to out)\n");
    printf("  -r:  Set resource directory (RESDIR defaults to res)\n");
    printf("  -h, --help:  Prints help\n");
    printf("  -e:  Set comptime permissions defaults to none, none for no support, \"secure\" for sane without file access, \"secure_file\" for sane with file access to current directory and sub directories, \"full\" has full runtime.\n");
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
   TF_InitWithConsole();
   std::filesystem::path outputDir = std::filesystem::current_path() / "bin";
   std::vector<std::filesystem::path> source;
   std::filesystem::path resourceDir = std::filesystem::current_path() / "res";
   std::vector<std::pair<std::string, TVMVersion>> dependencies;
   std::vector<std::pair<std::string, TVMVersion>> tools;
   std::string name="out";
   std::string info="{}";
   std::string icon="";
   std::string comptime="none";
   TVMVersion version;
   bool debug=false;

   

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
        else if(strcmp(argv[i], "-e") == 0)
        {
            i++;
            if(i < argc)
            {
                comptime = argv[i];
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
        else if(strcmp(argv[i],"-I") == 0)
        {
            i++;
            if(i < argc)
            {
                icon = argv[i];
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
        else if(strcmp(argv[i], "-t") == 0)
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
                    tools.push_back(std::pair<std::string,TVMVersion>(str1,v2));
                    
                }
                else
                {
                    printf("ERROR: Tool must have version\n");
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
        else if(strcmp(argv[i],"-D") == 0)
        {
            debug = true;
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
        int res = Lex(std::filesystem::absolute(src).string(),strm,tokens);
        
    }

    GC* gc=nullptr;
    GCList* ls=nullptr;
    TRootEnvironment* env=nullptr;
    if(comptime != "none")
    {
        gc = new GC();
        gc->Start();
        ls = new GCList(gc);
        env = TRootEnvironment::Create(ls,TDictionary::Create(ls));

        if(comptime == "secure")
        {
            TStd::RegisterConsole(gc,env);
            TStd::RegisterClass(gc,env);
            TStd::RegisterCrypto(gc,env);
            TStd::RegisterDictionary(gc,env);
            TStd::RegisterJson(gc,env);
            TStd::RegisterRoot(gc,env);
            TStd::RegisterIO(gc,env,false);
            env->permissions.locked=true;
        }
        else if(comptime == "secure_file")
        {
            TStd::RegisterConsole(gc,env);
            TStd::RegisterClass(gc,env);
            TStd::RegisterCrypto(gc,env);
            TStd::RegisterDictionary(gc,env);
            TStd::RegisterJson(gc,env);
            TStd::RegisterRoot(gc,env);
            TStd::RegisterIO(gc,env,false);
            env->permissions.locked=true;
            auto fs = env->EnsureDictionary(gc,"FS");
            fs->SetValue("Local", std::make_shared<SubdirFilesystem>(LocalFS,Tesses::Framework::Filesystem::VFSPath::GetAbsoluteCurrentDirectory()));
        }
        else if(comptime == "full")
        {
            TStd::RegisterStd(gc,env);
            env->permissions.locked=true;
        }
    }
    
    Parser parser(tokens,gc,env);
    parser.debug = debug;
    CodeGen gen;

    auto sfs = std::make_shared<SubdirFilesystem>(LocalFS,LocalFS->SystemToVFSPath(resourceDir.string()));
    gen.embedFS = sfs;

    gen.GenRoot(parser.ParseRoot());
    gen.name = name;
    gen.version = version;
    gen.info = info;
    gen.icon = icon;
    for(auto deps : dependencies)
    {
        gen.dependencies.push_back(deps);
    }
    for(auto tool : tools)
    {
        gen.tools.push_back(tool);
    }
    
    std::filesystem::create_directory(outputDir);

    {
        auto strm = std::make_shared<Tesses::Framework::Streams::FileStream>(outputDir / (name + "-" + version.ToString() + ".crvm"),"wb");
    
        gen.Save(strm);
    }
    if(gc != nullptr)
    {
        delete ls;
        delete gc;
    }
    return 0;
}