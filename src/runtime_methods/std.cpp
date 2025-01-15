#include "CrossLang.hpp"

namespace Tesses::CrossLang
{
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
    static TObject TypeOf(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() < 1) return "Undefined";
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
        this->canRegisterSDL2=false;
        this->canRegisterSqlite=false;
        this->canRegisterVM = false;
        this->locked=false;
        this->sqlite3Scoped=false;
    
    }

    void TStd::RegisterRoot(GC* gc, TRootEnvironment* env)
    {

        env->permissions.canRegisterRoot=true;
        env->DeclareFunction(gc, "ParseLong","Parse Long from String",{"arg","$base"},ParseLong);
        env->DeclareFunction(gc, "ParseDouble","Parse Double from String",{"arg"},ParseDouble);
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
        
        
        
        env->DeclareFunction(gc, "Thread","Create thread",{"callback"},[](GCList& ls, std::vector<TObject> args)-> TObject
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
        env->DeclareFunction(gc,"ByteArray","Create bytearray, with optional either size (to size it) or string argument (to fill byte array)",{"$data"},ByteArray);
        gc->BarrierBegin();
        env->DeclareVariable("InvokeMethod",MethodInvoker());
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
        RegisterSDL2(gc, env);
        RegisterOGC(gc, env);
        RegisterProcess(gc,env);

        gc->RegisterEverything(env);

        GCList ls(gc);

        TDictionary* dict = TDictionary::Create(ls);
        gc->BarrierBegin();
        env->SetVariable("Reflection",dict);
        gc->BarrierEnd();
        env->permissions.locked=true;
    }
}