#include "CrossLang.hpp"

namespace Tesses::CrossLang
{
    TObject TypeOf(GCList& ls, std::vector<TObject> args)
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
            auto list = dynamic_cast<TList*>(obj);
            auto argWrapper = dynamic_cast<TArgWrapper*>(obj);
            auto closure = dynamic_cast<TClosure*>(obj);
            auto externalMethod = dynamic_cast<TExternalMethod*>(obj);
            auto byteArray = dynamic_cast<TByteArray*>(obj);
            auto native = dynamic_cast<TNative*>(obj);
            auto vfs = dynamic_cast<TVFSHeapObject*>(obj);
            auto strm = dynamic_cast<TStreamHeapObject*>(obj);
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

    

    static TObject Throw(GCList& ls, std::vector<TObject> args)
    {
        if(!args.empty())
        {
            VMByteCodeException bce(ls.GetGC(),args[0]);
            throw bce;
        }
        return nullptr;
    }
    static TObject Try(GCList& ls, std::vector<TObject> args)
    {
        TCallable* tryPart;
        TCallable* catchPart;
        if(GetArgumentHeap(args,0,tryPart) && GetArgumentHeap(args,1,catchPart))
        {
            try {
                tryPart->Call(ls,{});
            }   
            catch(VMByteCodeException& ex)
            {
                catchPart->Call(ls,{ex.exception});
            }
            catch(std::exception& ex)
            {
                TDictionary* dict = TDictionary::Create(ls);
                auto gc = ls.GetGC();
                gc->BarrierBegin();
                dict->SetValue("Type","NativeException");
                dict->SetValue("Text",ex.what());
                gc->BarrierEnd();
                catchPart->Call(ls,{dict});
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
        env->DeclareFunction(gc, "Throw", "Throw an exception",{"object"},Throw);
        env->DeclareFunction(gc, "Try", "Handle exceptions",{"callbackToTry","callbackToCatch"},Try);
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