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
    static TObject FS_CreateMemoryFilesystem(GCList& ls, std::vector<TObject> args)
    {
        return TVFSHeapObject::Create(ls, new Tesses::Framework::Filesystem::MemoryFilesystem());
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
    static TObject FS_MakeFull(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath path;
        if(GetArgumentAsPath(args,0,path))
        {
            if(path.relative)
            {
                Tesses::Framework::Filesystem::LocalFilesystem lfs;
                auto curDir = std::filesystem::current_path();
                auto myPath = lfs.SystemToVFSPath(curDir.string()) / path;
                myPath = myPath.CollapseRelativeParents();
                return myPath;
            }
            return path.CollapseRelativeParents();
        }
        return nullptr;
    }
    static TObject FS_CreateArchive(GCList& ls, std::vector<TObject> args)
    {
        TVFSHeapObject* vfs;
        TStreamHeapObject* strm;
        std::string name;
        std::string version;
        std::string info;
        std::string icon="";
        TVMVersion version2;

        if(GetArgumentHeap(args,0,vfs) && GetArgumentHeap(args,1,strm) && GetArgument(args,2,name)  && GetArgument(args,4,info) && ((GetArgument(args,3,version) && TVMVersion::TryParse(version,version2)) || GetArgument(args,3,version2)))
        {
            GetArgument(args,5,icon);
            CrossArchiveCreate(vfs->vfs,strm->stream,name,version2,info,icon);
        }
        return nullptr;
    }
    static TObject FS_ExtractArchive(GCList& ls, std::vector<TObject> args)
    {
        TVFSHeapObject* vfs;
        TStreamHeapObject* strm;
     
        if(GetArgumentHeap(args,0,strm) && GetArgumentHeap(args,1,vfs))
        {
            auto res = CrossArchiveExtract(strm->stream,vfs->vfs);

            TDictionary* dict = TDictionary::Create(ls);
            ls.GetGC()->BarrierBegin();
            dict->SetValue("Name",res.first.first);
            dict->SetValue("Version",res.first.second.ToString());
            dict->SetValue("Info",res.second);
            ls.GetGC()->BarrierEnd();
            return dict;
        }
        return nullptr;
    }

    static TObject FS_ReadAllText(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath path;

        TVFSHeapObject* vfs;
        if(GetArgumentHeap(args,0,vfs) && GetArgumentAsPath(args,1,path))
        {
            auto txtFile = vfs->vfs->OpenFile(path,"rb");
            if(txtFile == nullptr) return "";
            Tesses::Framework::TextStreams::StreamReader reader(txtFile,true);
            return reader.ReadToEnd();
        }
        return "";
    }

    static TObject FS_WriteAllText(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath path;

        TVFSHeapObject* vfs;

        std::string content;
        if(GetArgumentHeap(args,0,vfs) && GetArgumentAsPath(args,1,path) && GetArgument(args,2,content))
        {
            auto txtFile = vfs->vfs->OpenFile(path,"wb");
            if(txtFile == nullptr) return nullptr;
            Tesses::Framework::TextStreams::StreamWriter writer(txtFile,true);
            writer.Write(content);
        }
        return nullptr;
    }

    static TObject FS_getCurrentPath(GCList& ls, std::vector<TObject> args)
    {
        return Tesses::Framework::Filesystem::VFSPath::GetAbsoluteCurrentDirectory();
    }
    static TObject FS_setCurrentPath(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath path;
        if(GetArgumentAsPath(args,0,path))
        Tesses::Framework::Filesystem::VFSPath::SetAbsoluteCurrentDirectory(path);
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
            dict->DeclareFunction(gc, "MakeFull", "Make absolute path from relative path",{"path"},FS_MakeFull);
            dict->DeclareFunction(gc,"getCurrentPath","Get current path",{},FS_getCurrentPath);
            dict->DeclareFunction(gc,"setCurrentPath","Set the current path",{"path"},FS_setCurrentPath);
        }

        dict->DeclareFunction(gc, "ReadAllText","Read all text from file", {"fs","filename"},FS_ReadAllText);
        dict->DeclareFunction(gc, "WriteAllText","Write all text to file", {"fs","filename","content"},FS_WriteAllText);
    
        dict->DeclareFunction(gc, "MountableFilesystem","Create a mountable filesystem",{"root"}, FS_MountableFilesystem);
        dict->DeclareFunction(gc, "SubdirFilesystem","Create a subdir filesystem",{"fs","subdir"}, FS_SubdirFilesystem);
        dict->DeclareFunction(gc, "MemoryStream","Create a memory stream",{"writable"}, FS_MemoryStream);
        dict->DeclareFunction(gc, "CreateStream","Create stream", {"strm"},FS_CreateStream);
        dict->DeclareFunction(gc, "CreateFilesystem","Create filesystem", {"fs"},FS_CreateFilesystem);

        dict->DeclareFunction(gc, "CreateMemoryFilesystem","Create in memory filesystem", {},FS_CreateMemoryFilesystem);
        dict->DeclareFunction(gc, "CreateArchive", "Create a crvm archive",{"fs","strm","name","version","info"},FS_CreateArchive);
        dict->DeclareFunction(gc,"ExtractArchive", "Extract a crvm archive",{"strm","vfs"},FS_ExtractArchive);
        env->DeclareVariable("FS", dict);
        gc->BarrierEnd();
    }
}