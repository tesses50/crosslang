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
    #if defined(GEKKO)
    #if defined(HW_RVL)
    static TObject OGC_WPAD_ScanPads(GCList& ls, std::vector<TObject> args)
    {
        return (int64_t)WPAD_ScanPads();
    }
    static TObject OGC_WPAD_ButtonsUp(GCList& ls, std::vector<TObject> args)
    {
        int64_t chan;
        if(GetArgument(args,0,chan))
        return (int64_t)WPAD_ButtonsUp((int)chan);
        return 0;
    }
    static TObject OGC_WPAD_ButtonsDown(GCList& ls, std::vector<TObject> args)
    {
        int64_t chan;
        if(GetArgument(args,0,chan))
        return (int64_t)WPAD_ButtonsDown((int)chan);
        return 0;
    }
    static TObject OGC_WPAD_ButtonsHeld(GCList& ls, std::vector<TObject> args)
    {
        int64_t chan;
        if(GetArgument(args,0,chan))
        return (int64_t)WPAD_ButtonsDown((int)chan);
        return 0;
    }
    static TObject OGC_WPAD_BatteryLevel(GCList& ls, std::vector<TObject> args)
    {
        int64_t chan;
        if(GetArgument(args,0,chan))
        return (int64_t)WPAD_BatteryLevel((int)chan);
        return 0;
    }
    #endif
    #endif
    void TStd::RegisterOGC(GC* gc, TRootEnvironment* env)
    {
        GCList ls(gc);
        #if defined(GEKKO)
    
        
        gc->BarrierBegin();

        TDictionary* dict_ogc_pad = TDictionary::Create(ls);
        #if defined(HW_RVL)
        TDictionary* dict_rvl_wpad = TDictionary::Create(ls);
        dict_rvl_wpad->DeclareFunction(gc, "ScanPads","Scan wiimotes",{},OGC_WPAD_ScanPads);
        dict_rvl_wpad->DeclareFunction(gc, "ButtonsDown","Is button down",{"pad"},OGC_WPAD_ButtonsDown);
        dict_rvl_wpad->SetValue("BUTTON_A",(int64_t)WPAD_BUTTON_A);
        dict_rvl_wpad->SetValue("BUTTON_B",(int64_t)WPAD_BUTTON_B);
        dict_rvl_wpad->SetValue("BUTTON_HOME",(int64_t)WPAD_BUTTON_HOME);
        env->DeclareVariable("WPAD", dict_rvl_wpad);
        
        #endif
        env->DeclareVariable("PAD", dict_ogc_pad);
        gc->BarrierEnd();
        #endif
        env->permissions.canRegisterOGC=true;
     

    
    }
}