#include "CrossLang.hpp"
#include <TessesFramework/Common.hpp>
#include <TessesFramework/Filesystem/LocalFS.hpp>
#include <TessesFramework/Filesystem/VFS.hpp>
#include <TessesFramework/Streams/Stream.hpp>
#include <ios>
#include <iostream>
#include <fstream>
int main(int argc,char** argv)
{
    std::string p = argv[0];
    auto emptyThumb =Tesses::CrossLang::GetRealExecutablePath(p).GetParent().GetParent() / "share" / "icons" / "crosslang.png";
    
    if(argc < 3)
    {
        std::cout << "USAGE: " << argv[0] << " CRVMFILE NEWPNG" << std::endl;
        return 1;
    }
    std::string crvm = argv[1];
    std::string png = argv[2];

    Tesses::Framework::Filesystem::LocalFilesystem lfs;
    
    if(lfs.FileExists(crvm))
    {
        
            Tesses::CrossLang::TFile file;
            auto f = lfs.OpenFile(crvm, "rb");
    
            file.Load(nullptr,f);

            delete f;

            if(file.icon >= 0 && file.icon < file.resources.size())
            {
                auto f2 = lfs.OpenFile(png, "wb");
                if(f2 != nullptr)
                {
                    auto& icon = file.resources[file.icon];
                    f2->WriteBlock(icon.data(),icon.size());
                    delete f2;
                }
                return 0;
            
        }


    }
    if(lfs.FileExists(emptyThumb))
    {
        auto src = lfs.OpenFile(emptyThumb,"rb");
        auto dest = lfs.OpenFile(png,"wb");
        if(src != nullptr && dest != nullptr)
        {
            src->CopyTo(dest);
        }
        delete src;
        delete dest;
    }
    return 0;
}