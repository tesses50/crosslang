#include "CrossLang.hpp"



namespace Tesses::CrossLang
{
    bool GetObjectAsPath(TObject& obj, Tesses::Framework::Filesystem::VFSPath& path, bool allowString)
    {
        if(GetObject(obj,path)) return true;
        std::string str;
        if(allowString && GetObject<std::string>(obj,str))
        {
            path = Tesses::Framework::Filesystem::VFSPath(str);
            return true;
        }
        return false;
    }
    bool GetArgumentAsPath(std::vector<TObject>& args, size_t index, Tesses::Framework::Filesystem::VFSPath& path,bool allowString)
    {
        if(GetArgument(args,index,path)) return true;
        std::string str;
        if(allowString && GetArgument<std::string>(args,index,str))
        {
            path = Tesses::Framework::Filesystem::VFSPath(str);
            return true;
        }
        return false;
    }
    
    static TObject Path_Root(GCList& ls, std::vector<TObject> args)
    {
        auto res = Tesses::Framework::Filesystem::VFSPath();\
        res.relative=false;
        return res;
    }
    static TObject Path_FromString(GCList& ls, std::vector<TObject> args)
    {
        std::string str;
        if(GetArgument(args,0,str))
        {
            return Tesses::Framework::Filesystem::VFSPath(str);
        }
        return nullptr;
    }
    static TObject Path_Create(GCList& ls, std::vector<TObject> args)
    {
        TList* myls;
        bool relative;
        if(GetArgument(args,0,relative) && GetArgumentHeap(args,1,myls))
        {
            std::vector<std::string> items;
            for(int64_t i = 0; i < myls->Count(); i++)
            {
                std::string str;
                TObject o =myls->Get(i);
                if(GetObject<std::string>(o,str))
                {
                    items.push_back(str);
                }
            }
            auto res = Tesses::Framework::Filesystem::VFSPath(items);
            res.relative = relative;
            return res;
        }

        return nullptr;
    }
    void TStd::RegisterPath(std::shared_ptr<GC> gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterPath=true;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        
        gc->BarrierBegin();
        dict->DeclareFunction(gc,"FromString","Create a Path from string",{"path"},Path_FromString);
        
        dict->DeclareFunction(gc,"Create","Create a Path from parts",{"relative","parts"},Path_Create);
        dict->DeclareFunction(gc,"Root","Create Absolute Root Path",{}, Path_Root);
        env->DeclareVariable("Path", dict);
        gc->BarrierEnd();
    }
}