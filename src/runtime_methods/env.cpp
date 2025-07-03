#include "CrossLang.hpp"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace Tesses::CrossLang
{
    using namespace Tesses::Framework::Platform::Environment;
    #if defined(_WIN32)
    static char EnvPathSeperator=';';
    #else
    static char EnvPathSeperator=':';
    #endif

    
    Tesses::Framework::Filesystem::VFSPath GetCrossLangConfigDir()
    {
        
        return  SpecialFolders::GetConfig() / "Tesses" / "CrossLang";
    }

    
    
    static TObject Env_getCrossLangConfig(GCList& ls, std::vector<TObject> args)
    {
        return GetCrossLangConfigDir();
    }
    static TObject Env_getPlatform(GCList& ls, std::vector<TObject> args)
    {
        return Tesses::Framework::Platform::Environment::GetPlatform();
    }
    static TObject Env_GetAt(GCList& ls, std::vector<TObject> args)
    {
        std::string key;
        if(GetArgument(args,0,key))
        {
            auto v = GetVariable(key);
            if(v) return v.value();

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
            {
                SetVariable(key,value);

                return value;
            }
            else 
            {
                SetVariable(key,std::nullopt);
            }
        }
        return nullptr;
    }
    static TObject Env_getDownloads(GCList& ls, std::vector<TObject> args)
    {
        return SpecialFolders::GetDownloads();
    }
    static TObject Env_getMusic(GCList& ls, std::vector<TObject> args)
    {
        return SpecialFolders::GetMusic();
    }
    static TObject Env_getPictures(GCList& ls, std::vector<TObject> args)
    {
        return SpecialFolders::GetPictures();
    }
    static TObject Env_getVideos(GCList& ls, std::vector<TObject> args)
    {
        return SpecialFolders::GetVideos();
    }
    static TObject Env_getDocuments(GCList& ls, std::vector<TObject> args)
    {
        return SpecialFolders::GetDocuments();
    }
    static TObject Env_getConfig(GCList& ls, std::vector<TObject> args)
    {
        return SpecialFolders::GetConfig();
    }

    static TObject Env_getDesktop(GCList& ls, std::vector<TObject> args)
    {
        return SpecialFolders::GetDesktop();
    }
    static TObject Env_getState(GCList& ls, std::vector<TObject> args)
    {
        return SpecialFolders::GetState();
    }
    static TObject Env_getCache(GCList& ls, std::vector<TObject> args)
    {
       return SpecialFolders::GetCache();
    }
     static TObject Env_getData(GCList& ls, std::vector<TObject> args)
    {
        return SpecialFolders::GetData();
    }
    static TObject Env_getUser(GCList& ls, std::vector<TObject> args)
    {
        return SpecialFolders::GetHomeFolder();
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
