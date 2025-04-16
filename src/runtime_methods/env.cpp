#include "CrossLang.hpp"
#if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
#include "../sago/platform_folders.h"
#endif
#if defined(_WIN32)
#include <windows.h>
#endif

namespace Tesses::CrossLang
{
    #if defined(_WIN32)
    static char EnvPathSeperator=';';
    #else
    static char EnvPathSeperator=':';
    #endif
    static std::string GetHomeFolder()
    {
        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        return sago::getHomeDir();
        #elif defined(__EMSCRIPTEN__)
        return "/home/web_user";
        #else
        return "/CrossLangProfile";
        #endif
    }
    Tesses::Framework::Filesystem::VFSPath GetCrossLangConfigDir()
    {
        #if defined(CROSSLANG_ENABLE_CONFIG_ENVVAR)
        char* conf = std::getenv("CROSSLANG_CONFIG");
        if(conf != NULL)
        {
            return std::string(conf);
        }
        #endif
        Tesses::Framework::Filesystem::VFSPath p;
        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        p=sago::getConfigHome();
        #else
        p = GetHomeFolder() + "/Config";
        #endif
        return p / "Tesses" / "CrossLang";
    }

    Tesses::Framework::Filesystem::VFSPath GetRealExecutablePath(Tesses::Framework::Filesystem::VFSPath realPath)
    {
        using namespace Tesses::Framework::Filesystem;
        using namespace Tesses::Framework::Http;
        LocalFilesystem lfs;
        if(!realPath.relative) return realPath;
        const char* path = std::getenv("PATH");
        #if defined(_WIN32)
        const char* pathext = std::getenv("PATHEXT");
        auto pext = HttpUtils::SplitString(pathext,";");
        pext.push_back({});
        auto pathParts = HttpUtils::SplitString(path,";");
        for(auto item : pathParts)
        {
            for(auto item2 : pext)
            {
                auto newPath = (lfs.SystemToVFSPath(item) / realPath) + item2;
                if(lfs.FileExists(newPath)) return newPath;
            }
        }
        return realPath.RelativeCurrentDirectory();
        #else
        
        auto pathParts = HttpUtils::SplitString(path,":");
        for(auto item : pathParts)
        {
            auto newPath = lfs.SystemToVFSPath(item) / realPath;
            if(lfs.FileExists(newPath)) return newPath;
        }
        return realPath.RelativeCurrentDirectory();
        #endif
    }

    
    static TObject Env_getCrossLangConfig(GCList& ls, std::vector<TObject> args)
    {
        return GetCrossLangConfigDir();
    }
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
            auto res = std::getenv(key.c_str());
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
            #if defined(_WIN32)
                SetEnvironmentVariable(key.c_str(),value.c_str());
            #else
                setenv(key.c_str(), value.c_str(),1);
            #endif
            else
            #if defined(_WIN32)
            {

            }
            #else
            unsetenv(key.c_str());
            #endif
            return value;
        }
        return nullptr;
    }
    static TObject Env_getDownloads(GCList& ls, std::vector<TObject> args)
    {
        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        return sago::getDownloadFolder();
        #else
        return GetHomeFolder() + "/Downloads";
        #endif
    }
    static TObject Env_getMusic(GCList& ls, std::vector<TObject> args)
    {

        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        return sago::getMusicFolder();
        #else
        return GetHomeFolder() + "/Music";
        #endif
    }
    static TObject Env_getPictures(GCList& ls, std::vector<TObject> args)
    {
        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        return sago::getPicturesFolder();
        #else
        return GetHomeFolder() + "/Pictures";
        #endif
    }
    static TObject Env_getVideos(GCList& ls, std::vector<TObject> args)
    {
        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        return sago::getVideoFolder();
        #else
        return GetHomeFolder() + "/Videos";
        #endif
    }
    static TObject Env_getDocuments(GCList& ls, std::vector<TObject> args)
    {
        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        return sago::getDocumentsFolder();
        #else
        return GetHomeFolder() + "/Documents";
        #endif
    }
    static TObject Env_getConfig(GCList& ls, std::vector<TObject> args)
    {
        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        return sago::getConfigHome();
        #else
        return GetHomeFolder() + "/Config";
        #endif
    }

    static TObject Env_getDesktop(GCList& ls, std::vector<TObject> args)
    {
        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        return sago::getDesktopFolder();
        #else
        return GetHomeFolder() + "/Desktop";
        #endif
    }
    static TObject Env_getState(GCList& ls, std::vector<TObject> args)
    {
        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        return sago::getStateDir();
        #else
        return GetHomeFolder() + "/State";
        #endif
    }
    static TObject Env_getCache(GCList& ls, std::vector<TObject> args)
    {
        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        return sago::getCacheDir();
        #else
        return GetHomeFolder() + "/Cache";
        #endif
    }
     static TObject Env_getData(GCList& ls, std::vector<TObject> args)
    {
        #if defined(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
        return sago::getDataHome();
        #else
        return GetHomeFolder() + "/Data";
        #endif
    }
    static TObject Env_getUser(GCList& ls, std::vector<TObject> args)
    {
        return GetHomeFolder();
    }
    static TObject Env_GetRealExecutablePath(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath p;
        if(GetArgumentAsPath(args,0,p))
        {
            return GetRealExecutablePath(p);
        }
        return Tesses::Framework::Filesystem::VFSPath();
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
        dict->DeclareFunction(gc,"getCrossLangConfig","Get crosslang configuration folder",{}, Env_getCrossLangConfig);
        dict->DeclareFunction(gc,"getData","Get data folder",{},Env_getData);
        dict->DeclareFunction(gc,"getUser","Get user folder",{},Env_getUser);
        dict->DeclareFunction(gc,"getPlatform","Get platform name",{},Env_getPlatform);
        dict->DeclareFunction(gc,"GetRealExecutablePath", "Get the absolute path for executable", {"path"},Env_GetRealExecutablePath);
        gc->BarrierBegin();
        dict->SetValue("EnvPathSeperator",EnvPathSeperator);
        env->SetVariable("Env", dict);
        gc->BarrierEnd();
    }
}
