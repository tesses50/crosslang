#include "CrossLang.hpp"

#if defined(GEKKO)
#undef CROSSLANG_ENABLE_PROCESS
#endif

//#if defined(CROSSLANG_ENABLE_PROCESS)
#include "subprocess.h"
//#endif

namespace Tesses::CrossLang
{
    //#if defined(CROSSLANG_ENABLE_PROCESS)
    static TObject Process_Start(GCList& ls, std::vector<TObject> args)
    {
        
        //Process.Start({
        // FileName = "git",
        // Arguments = ["clone","https://gitea.site.tesses.net/tesses50/crosslang.git"],
        // Environment = []
        //})

        
    }
    //#endif
}