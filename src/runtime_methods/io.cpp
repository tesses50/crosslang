#include "CrossLang.hpp"



namespace Tesses::CrossLang
{
    static TObject FS_SubdirFilesystem(GCList& ls, std::vector<TObject> args)
    {
        TVFSHeapObject* vfsho;

        Tesses::Framework::Filesystem::VFSPath path;

        if(GetArgumentHeap(args,0,vfsho) && GetArgumentAsPath(args,1,path))
        {
            return TVFSHeapObject::Create(ls,new Tesses::Framework::Filesystem::SubdirFilesystem(new TObjectVFS(ls.GetGC(),vfsho),path,true));
        }
        return nullptr;
    }
    static TObject FS_MountableFilesystem(GCList& ls, std::vector<TObject> args)
    {
        TVFSHeapObject* vfsho;

        if(GetArgumentHeap(args,0,vfsho))
        {
            return TVFSHeapObject::Create(ls,new Tesses::Framework::Filesystem::MountableFilesystem(new TObjectVFS(ls.GetGC(),vfsho),true));
        }
        return nullptr;
    }
    static TObject FS_MemoryStream(GCList& ls, std::vector<TObject> args)
    {
        bool writable;
        if(GetArgument(args,0,writable))
        {
            return TStreamHeapObject::Create(ls,new Tesses::Framework::Streams::MemoryStream(writable));
        }
        return nullptr;
    }

    static TObject FS_CreateFilesystem(GCList& ls, std::vector<TObject> args)
    {
        TDictionary* dict;
        if(GetArgumentHeap(args,0,dict))
        {
            return TVFSHeapObject::Create(ls, new TObjectVFS(ls.GetGC(),dict));
        }
        return nullptr;
    }
    static TObject FS_CreateStream(GCList& ls, std::vector<TObject> args)
    {
        TDictionary* dict;
        if(GetArgumentHeap(args,0,dict))
        {
            return TStreamHeapObject::Create(ls, new TObjectStream(ls.GetGC(),dict));
        }
        return nullptr;
    }
    void TStd::RegisterIO(GC* gc,TRootEnvironment* env,bool enableLocalFilesystem)
    {

        env->permissions.canRegisterIO=true;
        env->permissions.canRegisterLocalFS = enableLocalFilesystem;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        
        gc->BarrierBegin();
        if(enableLocalFilesystem)
        {
            TVFSHeapObject* vfs = TVFSHeapObject::Create(ls, new Tesses::Framework::Filesystem::LocalFilesystem());
        
            dict->SetValue("Local", vfs);
        }
    
        dict->DeclareFunction(gc, "MountableFilesystem","Create a mountable filesystem",{"root"}, FS_MountableFilesystem);
        dict->DeclareFunction(gc, "SubdirFilesystem","Create a subdir filesystem",{"fs","subdir"}, FS_SubdirFilesystem);
        dict->DeclareFunction(gc, "MemoryStream","Create a memory stream",{"writable"}, FS_MemoryStream);
        dict->DeclareFunction(gc, "CreateStream","Create stream", {"strm"},FS_CreateStream);
        dict->DeclareFunction(gc, "CreateFilesystem","Create filesystem", {"fs"},FS_CreateFilesystem);
        env->DeclareVariable("FS", dict);
        gc->BarrierEnd();
    }
}