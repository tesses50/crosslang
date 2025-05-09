#include "CrossLang.hpp"
#if defined(CROSSLANG_ENABLE_SHARED)
#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#endif
namespace Tesses::CrossLang
{
    #if defined(CROSSLANG_ENABLE_SHARED)
    class DL {
        void* handle;
        public:
            DL(Tesses::Framework::Filesystem::VFSPath p)
            {
                Tesses::Framework::Filesystem::LocalFilesystem lfs;
                std::string str = lfs.VFSPathToSystem(p);
                #if defined(_WIN32)
                handle = LoadLibraryExA(str.c_str() , NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
                #else
                handle = dlopen(str.c_str(), RTLD_LAZY);
                #endif
            }
            template<typename T>
            T Resolve(std::string name)
            {
                #if defined(_WIN32)
                return (T)GetProcAddress((HMODULE)handle,name.c_str());
                #else
                return (T)dlsym(handle,name.c_str());
                
                #endif
            }
            ~DL()
            {
                #if defined(_WIN32)
                FreeLibrary((HMODULE)handle);
                #else
                dlclose(handle);
                #endif
            }
    };
    #endif
    
    void LoadPlugin(GC* gc, TRootEnvironment* env, Tesses::Framework::Filesystem::VFSPath sharedObjectPath)
    {
        #if defined(CROSSLANG_ENABLE_SHARED)
        auto ptr = std::make_shared<DL>(sharedObjectPath);
        auto cb = ptr->Resolve<PluginFunction>("CrossLangPluginInit");
        if(cb == nullptr) return;
        gc->RegisterEverythingCallback([ptr,cb](GC* gc, TRootEnvironment* env)-> void{
            cb(gc,env);
        });
        cb(gc,env);
        #endif
    }

    static TObject TypeIsDefined(GCList& ls,std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return (!std::holds_alternative<std::nullptr_t>(args[0]) && !std::holds_alternative<Undefined>(args[0]));
    }
    static TObject TypeIsHeap(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return std::holds_alternative<THeapObjectHolder>(args[0]);
    }
    static TObject TypeIsNumber(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return std::holds_alternative<int64_t>(args[0]) || std::holds_alternative<double>(args[0]);
    }
    static TObject TypeIsLong(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return std::holds_alternative<int64_t>(args[0]);
    }
    static TObject TypeIsDouble(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return std::holds_alternative<double>(args[0]);
    }
    static TObject TypeIsString(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return std::holds_alternative<std::string>(args[0]);
    }
    static TObject TypeIsCallable(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        TCallable* call;
        return GetArgumentHeap(args,0,call);
    }
    static TObject TypeIsDictionary(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        TDictionary* dict;
        TDynamicDictionary* dynDict;
        return GetArgumentHeap(args,0,dict) || GetArgumentHeap(args,0,dynDict);
    }
    static TObject TypeIsList(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        TList* list;
        TDynamicList* dynList;
        return GetArgumentHeap(args,0,list) || GetArgumentHeap(args,0, dynList);
    }
    static TObject TypeIsStream(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        TStreamHeapObject* strm;
        return GetArgumentHeap(args,0,strm);
    }
    static TObject TypeIsVFS(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        TVFSHeapObject* vfs;
        return GetArgumentHeap(args,0,vfs);
    }
    static TObject New_SubdirFilesystem(GCList& ls, std::vector<TObject> args)
    {
        TVFSHeapObject* vfsho;

        Tesses::Framework::Filesystem::VFSPath path;

        if(GetArgumentHeap(args,0,vfsho) && GetArgumentAsPath(args,1,path))
        {
            return TVFSHeapObject::Create(ls,new Tesses::Framework::Filesystem::SubdirFilesystem(new TObjectVFS(ls.GetGC(),vfsho),path,true));
        }
        return nullptr;
    }
    static TObject New_MountableFilesystem(GCList& ls, std::vector<TObject> args)
    {
        TVFSHeapObject* vfsho;

        if(GetArgumentHeap(args,0,vfsho))
        {
            return TVFSHeapObject::Create(ls,new Tesses::Framework::Filesystem::MountableFilesystem(new TObjectVFS(ls.GetGC(),vfsho),true));
        }
        return nullptr;
    }
    static TObject New_MemoryStream(GCList& ls, std::vector<TObject> args)
    {
        bool writable;
        if(GetArgument(args,0,writable))
        {
            return TStreamHeapObject::Create(ls,new Tesses::Framework::Streams::MemoryStream(writable));
        }
        return nullptr;
    }
    static TObject New_MemoryFilesystem(GCList& ls, std::vector<TObject> args)
    {
        return TVFSHeapObject::Create(ls, new Tesses::Framework::Filesystem::MemoryFilesystem());
    }

    static TObject New_Filesystem(GCList& ls, std::vector<TObject> args)
    {
        TDictionary* dict;
        if(GetArgumentHeap(args,0,dict))
        {
            return TVFSHeapObject::Create(ls, new TObjectVFS(ls.GetGC(),dict));
        }
        return nullptr;
    }
    static TObject New_Stream(GCList& ls, std::vector<TObject> args)
    {
        TDictionary* dict;
        if(GetArgumentHeap(args,0,dict))
        {
            return TStreamHeapObject::Create(ls, new TObjectStream(ls.GetGC(),dict));
        }
        return nullptr;
    }
  
    static TObject TypeOf(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() < 1) return "Undefined";
        if(std::holds_alternative<std::regex>(args[0])) return "Regex";
        if(std::holds_alternative<Undefined>(args[0])) return "Undefined";
        if(std::holds_alternative<std::nullptr_t>(args[0])) return "Null";
        if(std::holds_alternative<bool>(args[0])) return "Boolean";
        if(std::holds_alternative<int64_t>(args[0])) return "Long";
        if(std::holds_alternative<double>(args[0])) return "Double";
        if(std::holds_alternative<char>(args[0])) return "Char";
        if(std::holds_alternative<MethodInvoker>(args[0])) return "MethodInvoker";
        if(std::holds_alternative<std::string>(args[0])) return "String";
        
        if(std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(args[0])) return "Path";
        if(std::holds_alternative<THeapObjectHolder>(args[0]))
        {
            auto obj = std::get<THeapObjectHolder>(args[0]).obj;
            auto dict = dynamic_cast<TDictionary*>(obj);
            auto dynDict = dynamic_cast<TDynamicDictionary*>(obj);
            
            auto list = dynamic_cast<TList*>(obj);
            auto dynList = dynamic_cast<TDynamicList*>(obj);
            auto argWrapper = dynamic_cast<TArgWrapper*>(obj);
            auto closure = dynamic_cast<TClosure*>(obj);
            auto externalMethod = dynamic_cast<TExternalMethod*>(obj);
            auto byteArray = dynamic_cast<TByteArray*>(obj);
            auto native = dynamic_cast<TNative*>(obj);
            auto vfs = dynamic_cast<TVFSHeapObject*>(obj);
            auto strm = dynamic_cast<TStreamHeapObject*>(obj);
            auto svr = dynamic_cast<TServerHeapObject*>(obj);
            auto cse = dynamic_cast<CallStackEntry*>(obj);
            auto rootEnv = dynamic_cast<TRootEnvironment*>(obj);
            auto subEnv = dynamic_cast<TSubEnvironment*>(obj);
            auto env = dynamic_cast<TEnvironment*>(obj);

            if(rootEnv != nullptr) return "RootEnvironment";
            if(subEnv != nullptr) return "SubEnvironment";
            if(env != nullptr) return "Environment";

            if(cse != nullptr) return "YieldedClosure";
            if(dynDict != nullptr) return "DynamicDictionary";
            if(dynList != nullptr) return "DynamicList";
            if(strm != nullptr)
            {
                auto netStrm = dynamic_cast<Tesses::Framework::Streams::NetworkStream*>(strm->stream);
                if(netStrm != nullptr)
                {
                    return "NetworkStream";
                }
                

                return "Stream";
            }
            if(svr != nullptr)
            {
                auto fileServer = dynamic_cast<Tesses::Framework::Http::FileServer*>(svr->server);
                auto mountableServer = dynamic_cast<Tesses::Framework::Http::MountableServer*>(svr->server);
                if(fileServer != nullptr)
                {
                    return "FileServer";
                }
                if(mountableServer != nullptr)
                {
                    return "MountableServer";
                }
                return "HttpServer";
            }
            if(vfs != nullptr)
            {
                auto localVFS = dynamic_cast<Tesses::Framework::Filesystem::LocalFilesystem*>(vfs->vfs);

                auto mountableVFS = dynamic_cast<Tesses::Framework::Filesystem::MountableFilesystem*>(vfs->vfs);


                auto subFS = dynamic_cast<Tesses::Framework::Filesystem::SubdirFilesystem*>(vfs->vfs);

                if(localVFS != nullptr) return "LocalFilesystem";
                if(subFS != nullptr) return "SubdirFilesystem";
                if(mountableVFS != nullptr) return "MountableFilesystem";
                
                return "VFS";
            }
            if(dict != nullptr) return "Dictionary";
            if(list != nullptr) return "List";
            if(argWrapper != nullptr) return "ArgWrapper";
            if(closure != nullptr) return "Closure";
            if(externalMethod != nullptr) return "ExternalMethod";
            if(byteArray != nullptr) return "ByteArray";
            if(native != nullptr) return "Native";
            

            return "HeapObject";
        }

        return "Object";
    }
    
     TObject ByteArray(GCList& ls, std::vector<TObject> args)
    {
        auto res= TByteArray::Create(ls);
        if(args.size() == 1){
                
            if(std::holds_alternative<int64_t>(args[0]))
            res->data.resize((size_t)std::get<int64_t>(args[0]));
            else if(std::holds_alternative<std::string>(args[0]))
            {
                std::string& txt = std::get<std::string>(args[0]);
                res->data.insert(res->data.end(), txt.begin(),txt.end());
            }
                
        }
        return res;

    }
    TObject ParseLong(GCList& ls, std::vector<TObject> args)
    {
        
        if(args.size() >= 1)
        {
            int base = 10;
            
            if(args.size() == 2 && std::holds_alternative<int64_t>(args[1]))
            {
                base = (int)std::get<int64_t>(args[1]);
            }

            size_t pos;


            std::string str = ToString(ls.GetGC(),args[0]);
            
            try {
                int64_t v =  std::stoll(str,&pos,base);
                if(pos < str.size()) return nullptr;
                return v;
            }catch(std::exception& ex)
            {
                return nullptr;
            }

        }
        return nullptr;

    }
 TObject ParseDouble(GCList& ls, std::vector<TObject> args)
    {
        
        if(args.size() == 1)
        {
            

            size_t pos;


            std::string str = ToString(ls.GetGC(),args[0]);
            
            try {
                double v =  std::stod(str,&pos);
                if(pos < str.size()) return nullptr;
                return v;
            }catch(std::exception& ex)
            {
                return nullptr;
            }

        }
        return nullptr;

    }

    

    EnvironmentPermissions::EnvironmentPermissions()
    {
        this->canRegisterConsole=false;
        this->canRegisterCrypto=false;
        this->canRegisterDictionary=false;
        this->canRegisterEnv=false;
        this->canRegisterIO=false;
        this->canRegisterJSON=false;
        this->canRegisterLocalFS=false;
        this->canRegisterNet=false;
        this->canRegisterOGC=false;
        this->canRegisterPath=false;
        this->canRegisterProcess=false;
        this->canRegisterRoot=false;
        this->canRegisterSqlite=false;
        this->canRegisterVM = false;
        this->locked=false;
        this->sqlite3Scoped=false;
    
    }

    static TObject YieldEnumerableFunc(GCList& ls, std::vector<TObject> args)
    {
        TClosure* closure;
        if(GetArgumentHeap(args,0,closure))
        {
            TDictionary* enumerableItem = TDictionary::Create(ls);
            ls.GetGC()->BarrierBegin();

            auto fn = TExternalMethod::Create(ls,"Get Enumerator for yield",{},[closure](GCList& ls2, std::vector<TObject> args)->TObject {
                return TYieldEnumerator::Create(ls2,closure);
            });
            fn->watch.push_back(closure);
                
            enumerableItem->SetValue("GetEnumerator", fn);
                
            ls.GetGC()->BarrierEnd();

            return enumerableItem;
            
        }

        return Undefined();
    }

    void TStd::RegisterRoot(GC* gc, TRootEnvironment* env)
    {
        GCList ls(gc);        
        
        env->permissions.canRegisterRoot=true;
        TDictionary* newTypes = TDictionary::Create(ls);

        newTypes->DeclareFunction(gc, "MountableFilesystem","Create a mountable filesystem",{"root"}, New_MountableFilesystem);
        newTypes->DeclareFunction(gc, "SubdirFilesystem","Create a subdir filesystem",{"fs","subdir"}, New_SubdirFilesystem);
        newTypes->DeclareFunction(gc, "MemoryStream","Create a memory stream",{"writable"}, New_MemoryStream);
        newTypes->DeclareFunction(gc, "Stream","Create stream", {"strm"},New_Stream);
        newTypes->DeclareFunction(gc, "Filesystem","Create filesystem", {"fs"},New_Filesystem);

        newTypes->DeclareFunction(gc, "MemoryFilesystem","Create in memory filesystem", {},New_MemoryFilesystem);

    
        newTypes->DeclareFunction(gc,"Version","Create a version object",{"$major","$minor","$patch","$build","$stage"},[](GCList& ls, std::vector<TObject> args)->TObject{
            int64_t major=1;
            int64_t minor=0;
            int64_t patch=0;
            int64_t build=0;
            std::string stageS="prod";
            TVMVersionStage stage=TVMVersionStage::ProductionVersion;
            GetArgument(args, 0,major);
            GetArgument(args, 1,minor);
            GetArgument(args, 2,patch);
            GetArgument(args, 3,build);
            GetArgument(args, 4,stageS);
            if(stageS == "dev")
                stage = TVMVersionStage::DevVersion;
            else if(stageS == "alpha")
                stage = TVMVersionStage::AlphaVersion;
            else if(stageS == "beta")
                stage = TVMVersionStage::BetaVersion;

            return TVMVersion((uint8_t)major,(uint8_t)minor,(uint8_t)patch,(uint16_t)build,stage);
        });
        
       
        env->DeclareFunction(gc, "ParseLong","Parse Long from String",{"arg","$base"},ParseLong);
        env->DeclareFunction(gc, "ParseDouble","Parse Double from String",{"arg"},ParseDouble);
        env->DeclareFunction(gc, "YieldEmumerable","Turn yield in function into enumerable",{"closure"},YieldEnumerableFunc);
        env->DeclareFunction(gc, "TypeOf","Get type of object",{"object"},TypeOf);
        env->DeclareFunction(gc, "TypeIsDefined","Get whether object is not null or undefined",{"object"},TypeIsDefined);
        env->DeclareFunction(gc, "TypeIsHeap","Get whether object is susceptible to garbage collection",{"object"},TypeIsHeap);
        env->DeclareFunction(gc, "TypeIsNumber","Get whether object is a number",{"object"},TypeIsNumber);
        env->DeclareFunction(gc, "TypeIsLong","Get whether object is a long (not a double)",{"object"},TypeIsLong);
        env->DeclareFunction(gc, "TypeIsDouble","Get whether object is a double (not a long)",{"object"},TypeIsDouble);
        env->DeclareFunction(gc, "TypeIsString","Get whether object is a string",{"object"},TypeIsString);
        env->DeclareFunction(gc, "TypeIsCallable","Get whether object is callable",{"object"},TypeIsCallable);
        env->DeclareFunction(gc, "TypeIsDictionary","Get whether object is a dictionary or dynamic dictionary",{"object"},TypeIsDictionary);
        env->DeclareFunction(gc, "TypeIsList","Get whether object is a list or dynamic list",{"object"},TypeIsList);
        env->DeclareFunction(gc, "TypeIsStream","Get whether object is a stream",{"object"},TypeIsStream);
        env->DeclareFunction(gc, "TypeIsVFS","Get whether object is a virtual filesystem",{"object"},TypeIsVFS);
        
        
        newTypes->DeclareFunction(gc, "Regex", "Create regex object",{"regex"},[](GCList& ls,std::vector<TObject> args)->TObject {
            std::string str;
            if(GetArgument(args,0,str))
            {
                std::regex regex(str);
                return regex;
            }
            return nullptr;
        });
        newTypes->DeclareFunction(gc, "Mutex", "Create mutex",{}, [](GCList& ls,std::vector<TObject> args)->TObject {
            ls.GetGC()->BarrierBegin();
            auto mtx = TDictionary::Create(ls);
            auto native = TNative::Create(ls, new Tesses::Framework::Threading::Mutex(),[](void* ptr)->void{
                delete static_cast<Tesses::Framework::Threading::Mutex*>(ptr);
            });
            auto lock = TExternalMethod::Create(ls,"Lock the mutex",{},[native](GCList& ls, std::vector<TObject> args)->TObject {
                if(native->GetDestroyed()) return nullptr;
                auto r = static_cast<Tesses::Framework::Threading::Mutex*>(native->GetPointer());
                r->Lock();
                return nullptr;
            });
            lock->watch.push_back(native);
            mtx->SetValue("Lock",lock);

            auto unlock = TExternalMethod::Create(ls,"Unlock the mutex",{},[native](GCList& ls, std::vector<TObject> args)->TObject {
                if(native->GetDestroyed()) return nullptr;
                auto r = static_cast<Tesses::Framework::Threading::Mutex*>(native->GetPointer());
                r->Unlock();
                return nullptr;
            });
            unlock->watch.push_back(native);
            mtx->SetValue("Unlock",unlock);


            auto trylock = TExternalMethod::Create(ls,"Try to lock the mutex, returns true if we aquire the lock, false if we can't due to another thread owning it",{},[native](GCList& ls, std::vector<TObject> args)->TObject {
                if(native->GetDestroyed()) return true;
                auto r = static_cast<Tesses::Framework::Threading::Mutex*>(native->GetPointer());
                return r->TryLock();
            });
            trylock->watch.push_back(native);
            mtx->SetValue("TryLock",trylock);
            ls.GetGC()->BarrierEnd();


            auto close = TExternalMethod::Create(ls,"Try to lock the mutex, returns true if we aquire the lock, false if we can't due to another thread owning it",{},[native](GCList& ls, std::vector<TObject> args)->TObject {
                native->Destroy();
                return nullptr;
            });
            close->watch.push_back(native);
            mtx->SetValue("Close",close);
            ls.GetGC()->BarrierEnd();
            return mtx;
        });
        newTypes->DeclareFunction(gc, "Thread","Create thread",{"callback"},[](GCList& ls, std::vector<TObject> args)-> TObject
        {
            if(args.size() == 1 && std::holds_alternative<THeapObjectHolder>(args[0]))
            {
                auto cb = dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(args[0]).obj);
                if(cb != nullptr)
                {
                    return CreateThread(ls,cb,false);
                }
            }
            return Undefined();
        });
        newTypes->DeclareFunction(gc,"ByteArray","Create bytearray, with optional either size (to size it) or string argument (to fill byte array)",{"$data"},ByteArray);
        gc->BarrierBegin();
        env->DeclareVariable("Version", TDictionary::Create(ls,{
            TDItem("Parse",TExternalMethod::Create(ls,"Parse version from string",{"versionStr"},[](GCList& ls, std::vector<TObject> args)->TObject{
                std::string str;
                if(GetArgument(args, 0, str))
                {
                    TVMVersion version;
                    if(TVMVersion::TryParse(str,version))
                    {
                        return version;
                    }
                }
                return nullptr;
            })),
            TDItem("FromByteArray",TExternalMethod::Create(ls,"Create from ByteArray",{"byteArray","$offset"},[](GCList& ls,std::vector<TObject> args)->TObject {
                TByteArray* ba;
                if(GetArgumentHeap(args,0, ba))
                {
                    int64_t offset=0;
                    GetArgument(args, 1, offset);
                    if(ba->data.size() < 5) throw VMException("ByteArray too small");
                    size_t o = (size_t)offset;
                    if((o + 5) > ba->data.size() || (o + 5) < 5) throw VMException("ByteArray too small");
                    
                    return TVMVersion(ba->data.data()+o);
                }
                return nullptr;
            })),
            TDItem("FromLong", TExternalMethod::Create(ls, "Create from long",{"longBasedVersion"},[](GCList& ls, std::vector<TObject> args)->TObject {
                int64_t num;
                if(GetArgument(args,0,num))
                {
                    return TVMVersion((uint64_t)num);
                }
                return nullptr;
            }))
        }));
        env->DeclareVariable("InvokeMethod",MethodInvoker());
        env->DeclareVariable("New", newTypes);
        gc->BarrierEnd();
    }
    void TStd::RegisterStd(GC* gc, TRootEnvironment* env)
    {
        env->permissions.canRegisterEverything=true;
        RegisterEnv(gc, env);
        RegisterRoot(gc,env);
        RegisterPath(gc,env);
        RegisterConsole(gc, env);
        RegisterIO(gc, env);
        RegisterNet(gc, env);
        RegisterSqlite(gc, env);
        RegisterVM(gc, env);
        RegisterJson(gc, env);
        RegisterDictionary(gc, env);
        RegisterCrypto(gc,env);
        RegisterOGC(gc, env);
        RegisterProcess(gc,env);
        RegisterTime(gc, env);

        gc->RegisterEverything(env);

        GCList ls(gc);

        TDictionary* dict = TDictionary::Create(ls);
         TDictionary* gc_dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc,"LoadNativePlugin","Load a native plugin, requires a dynamic linker and shared build of libcrosslang",{"path"},[gc,env](GCList& ls, std::vector<TObject> args)->TObject {
            Tesses::Framework::Filesystem::VFSPath path;
            if(GetArgumentAsPath(args,0,path))
            {
                LoadPlugin(gc,env,path);
            }
            return nullptr;
        });
        gc_dict->DeclareFunction(gc,"Collect","Collect garbage",{},[](GCList& ls, std::vector<TObject> args)->TObject{
            ls.GetGC()->Collect();
            return nullptr;
        });
        gc_dict->DeclareFunction(gc,"BarrierBegin","Lock globally",{},[](GCList& ls,std::vector<TObject> args)->TObject {
            ls.GetGC()->BarrierBegin();
            return nullptr;
        });
        gc_dict->DeclareFunction(gc, "BarrierEnd","Unlock globally",{},[](GCList& ls, std::vector<TObject> args)->TObject {
            ls.GetGC()->BarrierEnd();
            return  nullptr;
        });
        gc->BarrierBegin();
        env->SetVariable("Reflection",dict);
        env->SetVariable("GC", gc_dict);
        gc->BarrierEnd();
        env->permissions.locked=true;
    }
}
