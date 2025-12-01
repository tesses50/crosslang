#include "CrossLang.hpp"



namespace Tesses::CrossLang
{
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
        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
        
        std::shared_ptr<Tesses::Framework::Streams::Stream> strm;
        
        std::string name;
        std::string version;
        std::string info;
        std::string icon="";
        TVMVersion version2;

        if(GetArgument(args,0,vfs) && GetArgument(args,1,strm) && GetArgument(args,2,name)  && GetArgument(args,4,info) && ((GetArgument(args,3,version) && TVMVersion::TryParse(version,version2)) || GetArgument(args,3,version2)))
        {
            GetArgument(args,5,icon);
            CrossArchiveCreate(vfs,strm,name,version2,info,icon);
        }
        return nullptr;
    }
    static TObject FS_ExtractArchive(GCList& ls, std::vector<TObject> args)
    {
        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
        
        std::shared_ptr<Tesses::Framework::Streams::Stream> strm;
        
     
        if(GetArgument(args,0,strm) && GetArgument(args,1,vfs))
        {
            auto res = CrossArchiveExtract(strm,vfs);

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

        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
        
        if(GetArgument(args,0,vfs) && GetArgumentAsPath(args,1,path))
        {
            return Tesses::Framework::Filesystem::Helpers::ReadAllText(vfs,path);
        }
        return "";
    }

    static TObject FS_ReadAllLines(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath path;

        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
        
        if(GetArgument(args,0,vfs) && GetArgumentAsPath(args,1,path))
        {
            std::vector<std::string> lines;

            Tesses::Framework::Filesystem::Helpers::ReadAllLines(vfs,path,lines);

            ls.GetGC()->BarrierBegin();
            auto items = TList::Create(ls);
            for(auto& l : lines) { items->Add(l);}
            ls.GetGC()->BarrierEnd();
            return items;
        }
        return nullptr;
    }

    static TObject FS_ReadAllBytes(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath path;
        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
        
        if(GetArgument(args,0,vfs) && GetArgumentAsPath(args,1,path))
        {
            auto res = TByteArray::Create(ls);
            Tesses::Framework::Filesystem::Helpers::ReadAllBytes(vfs,path,res->data);
            
            return res;
        }
        return nullptr;
    }


    static TObject FS_WriteAllLines(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath path;
        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
        

        TList* lines;
        if(GetArgument(args,0,vfs) && GetArgumentAsPath(args,1,path) && GetArgumentHeap(args,2,lines))
        {
            std::vector<std::string> content;
            ls.GetGC()->BarrierBegin();
            for(auto& item : lines->items) 
            {
                if(std::holds_alternative<std::string>(item))
                content.push_back(std::get<std::string>(item));
            }
            ls.GetGC()->BarrierEnd();
            Tesses::Framework::Filesystem::Helpers::WriteAllLines(vfs,path,content);
        }
        return nullptr;
    }
    static TObject FS_WriteAllText(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath path;
        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
        

        std::string content;
        if(GetArgument(args,0,vfs) && GetArgumentAsPath(args,1,path) && GetArgument(args,2,content))
        {
            Tesses::Framework::Filesystem::Helpers::WriteAllText(vfs,path,content);
        }
        return nullptr;
    }
    static TObject FS_WriteAllBytes(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath path;

        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;

        TByteArray* bArray;
        if(GetArgument(args,0,vfs) && GetArgumentAsPath(args,1,path) && GetArgumentHeap(args,2,bArray))
        {
            Tesses::Framework::Filesystem::Helpers::WriteAllBytes(vfs,path,bArray->data);
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
            
        
            dict->SetValue("Local", Tesses::Framework::Filesystem::LocalFS);
            dict->DeclareFunction(gc, "MakeFull", "Make absolute path from relative path",{"path"},FS_MakeFull);
            dict->DeclareFunction(gc,"getCurrentPath","Get current path",{},FS_getCurrentPath);
            dict->DeclareFunction(gc,"setCurrentPath","Set the current path",{"path"},FS_setCurrentPath);
        }

        dict->DeclareFunction(gc, "ReadAllText","Read all text from file", {"fs","filename"},FS_ReadAllText);
        dict->DeclareFunction(gc, "WriteAllText","Write all text to file", {"fs","filename","content"},FS_WriteAllText);

        dict->DeclareFunction(gc, "ReadAllLines","Read all lines from file", {"fs","filename"},FS_ReadAllLines);
        dict->DeclareFunction(gc, "WriteAllLines","Write all lines to file", {"fs","filename","lines"},FS_WriteAllLines);
    
        dict->DeclareFunction(gc, "ReadAllBytes","Read all bytes from file", {"fs","filename"},FS_ReadAllBytes);
        dict->DeclareFunction(gc, "WriteAllBytes","Write all bytes to file", {"fs","filename","content"},FS_WriteAllBytes);
    
        dict->DeclareFunction(gc, "CreateArchive", "Create a crvm archive",{"fs","strm","name","version","info"},FS_CreateArchive);
        dict->DeclareFunction(gc,"ExtractArchive", "Extract a crvm archive",{"strm","vfs"},FS_ExtractArchive);
        env->DeclareVariable("FS", dict);
        gc->BarrierEnd();
    }
}