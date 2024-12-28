#include "CrossLang.hpp"
#if defined(GEKKO)
#include <ogcsys.h>
#include <gccore.h>
#include <ogc/pad.h>
#if defined(HW_RVL)
#include <wiiuse/wpad.h>
#endif
#endif
namespace Tesses::CrossLang
{
    void TStd::RegisterOGC(GC* gc, TRootEnvironment* env)
    {

        env->permissions.canRegisterOGC=true;
    }
}