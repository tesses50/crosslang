#include "CrossLang.hpp"
void help(char* program)
{
    std::cout << "USAGE: " << program << " FILE.CRVM DEST.CRVM" << std::endl;
        
    exit(0);
}
int main(int argc, char** argv)
{
    using namespace Tesses::Framework;
    using namespace Tesses::Framework::Streams;
    using namespace Tesses::Framework::Filesystem;
    using namespace Tesses::CrossLang;

    if(argc < 3)
    {
        help(argv[0]);
    }

    std::string src=argv[1];
    std::string dest=argv[2];

    
    
    VFSPath srcF = src;
    VFSPath destF = dest;
    srcF.MakeAbsolute();
    destF.MakeAbsolute();


    auto sdir = std::make_shared<SubdirFilesystem>(LocalFS,srcF.GetParent());
    auto ddir = std::make_shared<SubdirFilesystem>(LocalFS,destF+"_tmp");

   
    auto outpath = Merge(sdir,"/"+srcF.GetFileName(), ddir);
    outpath.relative=true;
    outpath = (destF+"_tmp") / outpath;
    LocalFS->MoveFile(outpath,destF);
    LocalFS->DeleteDirectoryRecurse(destF+"_tmp");
    
    
    return 0;
}