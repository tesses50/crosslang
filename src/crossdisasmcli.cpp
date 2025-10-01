#include "CrossLang.hpp"

void help(char* program)
{
    std::cout << "USAGE: " << program << " [flags] FILE.CRVM" << std::endl;
    std::cout << "USAGE: " << program << " [flags] FILE.CRVM DIR" << std::endl;
    std::cout << "Flags:" << std::endl;
    std::cout << "--no-json\tNo json" << std::endl;
    std::cout << "--no-resources\nNo resources" << std::endl;
        
    exit(0);
}
int main(int argc, char** argv)
{
    
    using namespace Tesses::Framework;
    using namespace Tesses::Framework::Streams;
    using namespace Tesses::Framework::Filesystem;
    using namespace Tesses::CrossLang;
    TF_Init();

    std::string file;
    Tesses::Framework::Filesystem::VFSPath path = VFSPath::GetAbsoluteCurrentDirectory();
    int curPos = 0;
    bool json=true;
    bool resources = true;
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i],"--no-json") == 0)
        {
            json = false;
        } 
        else if(strcmp(argv[i],"--no-resources") == 0)
        {
            resources = false;
        }
        else if(strcmp(argv[i],"--help") == 0)
        {
            help(argv[0]);
        }
        else {
            if(curPos == 0)
            {
                file = argv[i];
                curPos++;
            }
            else if(curPos == 1)
            {
                path = LocalFS->SystemToVFSPath(argv[i]);
            }
        }
    }
    if(file.empty())
    {
        help(argv[0]);
    }
    auto strm = LocalFS->OpenFile(file,"rb");
    auto sdir = std::make_shared<SubdirFilesystem>(LocalFS,path);
    if(strm->CanRead())
    Disassemble(strm, sdir,json,resources);
    
    return 0;
}