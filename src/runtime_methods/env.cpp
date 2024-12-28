#include "CrossLang.hpp"

#include "../sago/platform_folders.h"

namespace Tesses::CrossLang
{
    static TObject Env_getPlatform(GCList& ls, std::vector<TObject> args)
    {
        #if defined(__SWITCH__)
            return "Nintendo Switch";
        #endif
        #if defined(GEKKO)
            #if defined(HW_RVL)
                return "Nintendo Wii";
            #endif
            return "Nintendo Gamecube";
        #endif
        #if defined(WIN32) || defined(_WIN32)
        return "Windows";
        #endif
        #if defined(linux)
        return "Linux";
        #endif

        #ifdef __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_MAC
        return "MacOS";
    #endif
    #if TARGET_OS_IOS
        return "iOS";
    #endif
    #if TARGET_OS_TV
        return "Apple TV";
    #endif

    #if TARGET_OS_WATCH
        return "Apple Watch";
    #endif

    #if __EMSCRIPTEN__
        return "WebAssembly";
    #endif

    return "Unknown Apple Device";
#endif

        return "Unknown";
    }
    static TObject Env_GetAt(GCList& ls, std::vector<TObject> args)
    {
        std::string key;
        if(GetArgument(args,0,key))
        {
            auto res = getenv(key.c_str());
            if(res == nullptr) return nullptr;
            std::string value = res;
            return value;
        }
        return nullptr;
    }
    static TObject Env_SetAt(GCList& ls, std::vector<TObject> args)
    {
        std::string key;
        std::string value;
        if(GetArgument(args,0,key))
        {
            if(GetArgument(args,1,value))
            setenv(key.c_str(), value.c_str(),1);
            else
            unsetenv(key.c_str());
            return value;
        }
        return nullptr;
    }
    static TObject Env_getDownloads(GCList& ls, std::vector<TObject> args)
    {
        return sago::getDownloadFolder();
    }
    static TObject Env_getMusic(GCList& ls, std::vector<TObject> args)
    {
        return sago::getMusicFolder();
    }
    static TObject Env_getPictures(GCList& ls, std::vector<TObject> args)
    {
        return sago::getMusicFolder();
    }
    static TObject Env_getVideos(GCList& ls, std::vector<TObject> args)
    {
        return sago::getVideoFolder();
    }
    static TObject Env_getDocuments(GCList& ls, std::vector<TObject> args)
    {
        return sago::getDocumentsFolder();
    }
    static TObject Env_getConfig(GCList& ls, std::vector<TObject> args)
    {
        return sago::getConfigHome();
    }

    static TObject Env_getDesktop(GCList& ls, std::vector<TObject> args)
    {
        return sago::getDesktopFolder();
    }
    static TObject Env_getState(GCList& ls, std::vector<TObject> args)
    {
        return sago::getStateDir();
    }
    static TObject Env_getCache(GCList& ls, std::vector<TObject> args)
    {
        return sago::getCacheDir();
    }
     static TObject Env_getData(GCList& ls, std::vector<TObject> args)
    {
        return sago::getDataHome();
    }
    static TObject Env_getUser(GCList& ls, std::vector<TObject> args)
    {
        return sago::getHomeDir();
    }
    void TStd::RegisterEnv(GC* gc, TRootEnvironment* env)
    {

        env->permissions.canRegisterEnv=true;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc,"GetAt","Get environment variable", {"key"}, Env_GetAt);
        dict->DeclareFunction(gc,"SetAt","Set environment variable", {"key","value"}, Env_SetAt);

        dict->DeclareFunction(gc,"getDesktop","Get downloads folder",{},Env_getDownloads);
        dict->DeclareFunction(gc,"getDownloads","Get downloads folder",{},Env_getDownloads);
        dict->DeclareFunction(gc,"getDocuments","Get documents folder",{},Env_getDocuments);
        dict->DeclareFunction(gc,"getMusic","Get music folder",{},Env_getMusic);
        dict->DeclareFunction(gc,"getPictures","Get pictures folder",{},Env_getPictures);
        dict->DeclareFunction(gc,"getVideos","Get videos folder",{},Env_getVideos);
        dict->DeclareFunction(gc,"getState","Get state folder",{},Env_getState);
        dict->DeclareFunction(gc,"getCache","Get cache folder",{},Env_getCache);
        dict->DeclareFunction(gc,"getConfig","Get config folder",{},Env_getConfig);
        dict->DeclareFunction(gc,"getData","Get data folder",{},Env_getData);
        dict->DeclareFunction(gc,"getUser","Get user folder",{},Env_getUser);
        dict->DeclareFunction(gc,"getPlatform","Get platform name",{},Env_getPlatform);

        gc->BarrierBegin();
        env->SetVariable("Env", dict);
        gc->BarrierEnd();
    }
}