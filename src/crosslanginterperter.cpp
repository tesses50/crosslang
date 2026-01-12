#include "CrossLang.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Filesystem;
int main(int argc, char** argv)
{
    TF_InitWithConsole();
    if(argc > 0)
        TF_AllowPortable(argv[0]);
    GC gc;
    gc.Start();
    GCList ls(gc);
    TRootEnvironment* env = TRootEnvironment::Create(ls, TDictionary::Create(ls));
    TStd::RegisterStd(&gc,env);
    
    if(argc > 1)
    {
        std::ifstream strm(argv[1],std::ios_base::in|std::ios_base::binary);
        std::vector<LexToken> tokens;
        Lex(argv[1],strm,tokens);

        Parser parser(tokens);

        CodeGen gen;

        auto sfs = std::make_shared<SubdirFilesystem>(LocalFS,VFSPath("."));
        gen.embedFS = sfs;
        gen.GenRoot(parser.ParseRoot());
        std::vector<uint8_t> data;
        {
        auto strm2 = std::make_shared<Tesses::Framework::Streams::MemoryStream>(true);
        gen.Save(strm2);

        
        {
            TFile* file = TFile::Create(ls);

            strm2->Seek(0,Tesses::Framework::Streams::SeekOrigin::Begin);
            file->Load(&gc,strm2);
    
            env->LoadFile(&gc, file);

            
        }
    }

         TList* args = TList::Create(ls);
    for(int arg=1;arg<argc;arg++)
        args->Add(std::string(argv[arg]));
   
    auto res = env->CallFunctionWithFatalError(ls,"main",{args});
    int64_t iresult;
    if(GetObject(res,iresult))
        return (int)iresult;
   
    }
    else
    {
    
   
        while(true)
        {
            std::cout << "> ";
            std::string source;
            std::getline(std::cin,source);

            auto strm2 = std::make_shared<Tesses::Framework::Streams::MemoryStream>(true);

            if(source.find("loadfile ") == 0)
            {
                std::string filename = source.substr(9);

                std::ifstream strm(filename,std::ios_base::in|std::ios_base::binary);
                std::vector<LexToken> tokens;
                Lex(filename,strm,tokens);

                Parser parser(tokens);

                CodeGen gen;

                auto sfs = std::make_shared<SubdirFilesystem>(LocalFS,VFSPath("."));
                gen.embedFS = sfs;
                gen.GenRoot(parser.ParseRoot());
                
                gen.Save(strm2);

            }
            else if(source == "exit")
            {
                return 0;
            }
            else
            {
                std::vector<LexToken> tokens;
                std::stringstream strm(source,std::ios_base::in | std::ios_base::binary);
                Lex("lexed.tcross",strm,tokens);

                Parser parser(tokens);

                CodeGen gen;

                auto sfs = std::make_shared<SubdirFilesystem>(LocalFS,VFSPath("."));
                gen.embedFS = sfs;

                gen.GenRoot(parser.ParseRoot());
                
                gen.Save(strm2);
            }

            {
                
            TFile* file = TFile::Create(ls);

            strm2->Seek(0,Tesses::Framework::Streams::SeekOrigin::Begin);
            file->Load(&gc,strm2);
    
            env->LoadFile(&gc, file);

                
            }   

    
        }
    }
}
