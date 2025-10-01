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
    Tesses::Framework::TF_Init();
    std::string p = argv[0];
    auto emptyThumb =Tesses::Framework::Platform::Environment::GetRealExecutablePath(p).GetParent().GetParent() / "share" / "icons" / "crosslang.png";
    
    if(argc < 3)
    {
        std::cout << "USAGE: " << argv[0] << " CRVMFILE NEWPNG" << std::endl;
        return 1;
    }
    std::string crvm = argv[1];
    std::string png = argv[2];

    
    if(Tesses::Framework::Filesystem::LocalFS->FileExists(crvm))
    {
        
            Tesses::CrossLang::TFile file;
            auto f = Tesses::Framework::Filesystem::LocalFS->OpenFile(crvm, "rb");
    
            file.Load(nullptr,f);


            if(file.icon >= 0 && file.icon < file.resources.size())
            {
                auto f2 = Tesses::Framework::Filesystem::LocalFS->OpenFile(png, "wb");
                if(f2 != nullptr)
                {
                    auto& icon = file.resources[file.icon];
                    f2->WriteBlock(icon.data(),icon.size());
                    
                }
                return 0;
            
        }


    }
    if(Tesses::Framework::Filesystem::LocalFS->FileExists(emptyThumb))
    {
        auto src = Tesses::Framework::Filesystem::LocalFS->OpenFile(emptyThumb,"rb");
        auto dest = Tesses::Framework::Filesystem::LocalFS->OpenFile(png,"wb");
        if(src != nullptr && dest != nullptr)
        {
            src->CopyTo(dest);
        }
        
    }
    return 0;
}