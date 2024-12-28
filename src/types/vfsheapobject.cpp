#include "CrossLang.hpp"

namespace Tesses::CrossLang {

    TObjectVFS::TObjectVFS(GC* gc, TObject obj)
    {
        this->ls = new GCList(gc);
        this->ls->Add(obj);
        this->obj = obj;
        TDictionary* dict;
        if(GetObjectHeap(obj,dict))
        {
            gc->BarrierBegin();
            if(!dict->HasValue("OpenFile"))
            {
                dict->DeclareFunction(gc,"OpenFile","Open a file",{"path","mode"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    return nullptr;
                });
            }
            if(!dict->HasValue("EnumeratePaths"))
            {
                dict->DeclareFunction(gc,"EnumeratePaths","Enumerate paths",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    return TVFSPathEnumerator::Create(ls,Tesses::Framework::Filesystem::VFSPathEnumerator());
                });
            }
            if(!dict->HasValue("ReadLink"))
            {
                dict->DeclareFunction(gc,"ReadLink","Read a symlink",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    return Tesses::Framework::Filesystem::VFSPath();
                });
            }

            if(!dict->HasValue("VFSPathToSystem"))
            {
                dict->DeclareFunction(gc,"VFSPathToSystem","Convert path to system",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    Tesses::Framework::Filesystem::VFSPath path;
                    if(GetArgumentAsPath(args,0,path))
                    {
                        return path.ToString();
                    }
                    return "/";
                });
            }
            if(!dict->HasValue("SystemToVFSPath"))
            {
                dict->DeclareFunction(gc,"SystemToVFSPath","Convert system to path",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    std::string p;
                    if(GetArgument(args,0,p))
                    {
                        return Tesses::Framework::Filesystem::VFSPath(p);
                    }
                    return Tesses::Framework::Filesystem::VFSPath();
                });
            }
            if(!dict->HasValue("GetDate"))
            {
                dict->DeclareFunction(gc,"GetDate","Get date from file",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    auto dict = TDictionary::Create(ls);
                    ls.GetGC()->BarrierBegin();
                    dict->SetValue("LastWrite", (int64_t)time(NULL));
                    dict->SetValue("LastAccess", (int64_t)time(NULL));
                    ls.GetGC()->BarrierEnd();
                    return dict;    
                });
            }
            if(!dict->HasValue("RegularFileExists"))
            {
                dict->DeclareFunction(gc,"RegularFileExists","Regular file exists",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    
                    return false;
                });
            }
            if(!dict->HasValue("SymlinkExists"))
            {
                dict->DeclareFunction(gc,"SymlinkExists","Symlink exists",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    
                    return false;
                });
            }
            if(!dict->HasValue("CharacterDeviceExists"))
            {
                dict->DeclareFunction(gc,"CharacterDeviceExists","Character file exists",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    
                    return false;
                });
            }
            if(!dict->HasValue("BlockDeviceExists"))
            {
                dict->DeclareFunction(gc,"BlockDeviceExists","Block file exists",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    
                    return false;
                });
            }
            if(!dict->HasValue("SocketFileExists"))
            {
                dict->DeclareFunction(gc,"SocketFileExists","Socket file exists",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    
                    return false;
                });
            }

            if(!dict->HasValue("FIFOFileExists"))
            {
                dict->DeclareFunction(gc,"FIFOFileExists","FIFO file exists",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    
                    return false;
                });
            }
            if(!dict->HasValue("FileExists"))
            {
                dict->DeclareFunction(gc,"FileExists","File exists",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    
                    return false;
                });
            }
            if(!dict->HasValue("SpecialFileExists"))
            {
                dict->DeclareFunction(gc,"SpecialFileExists","Special file exists",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    
                    return false;
                });
            }
            if(!dict->HasValue("DirectoryExists"))
            {
                dict->DeclareFunction(gc,"DirectoryExists","Directory exists",{"path"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    
                    return false;
                });
            }
            gc->BarrierEnd();
        }
        
    }
    Tesses::Framework::Streams::Stream* TObjectVFS::OpenFile(Tesses::Framework::Filesystem::VFSPath path, std::string mode)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->OpenFile(path,mode);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "OpenFile",{path,mode});
            TStreamHeapObject* strm;
            if(GetObjectHeap(res,strm))
            {
                return new TObjectStream(this->ls->GetGC(), strm);
            }
        }
        return nullptr;
    }
    void TObjectVFS::CreateDirectory(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            vfs->vfs->CreateDirectory(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "CreateDirectory",{path});
            
        }
    
    }
    void TObjectVFS::DeleteDirectory(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            vfs->vfs->DeleteDirectory(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "DeleteDirectory",{path});
            
        }
    }
    bool TObjectVFS::RegularFileExists(Tesses::Framework::Filesystem::VFSPath path)
    {

        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->RegularFileExists(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "RegularFileExists",{path});
            bool out;
            if(GetObject(res,out))
            {
                return out;
            }
        }
    }
    bool TObjectVFS::SymlinkExists(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->SymlinkExists(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "SymlinkExists",{path});
            bool out;
            if(GetObject(res,out))
            {
                return out;
            }
        }
    }
    bool TObjectVFS::CharacterDeviceExists(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->CharacterDeviceExists(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "CharacterDeviceExists",{path});
            bool out;
            if(GetObject(res,out))
            {
                return out;
            }
        }
    }
    bool TObjectVFS::BlockDeviceExists(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->BlockDeviceExists(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "BlockDeviceExists",{path});
            bool out;
            if(GetObject(res,out))
            {
                return out;
            }
        }
    }
    bool TObjectVFS::SocketFileExists(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->SocketFileExists(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "SocketFileExists",{path});
            bool out;
            if(GetObject(res,out))
            {
                return out;
            }
        }
    }
    bool TObjectVFS::FIFOFileExists(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->FIFOFileExists(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "FIFOFileExists",{path});
            bool out;
            if(GetObject(res,out))
            {
                return out;
            }
        }
    }
    bool TObjectVFS::FileExists(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->FileExists(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "FileExists",{path});
            bool out;
            if(GetObject(res,out))
            {
                return out;
            }
        }
    }
    bool TObjectVFS::SpecialFileExists(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->SpecialFileExists(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "SpecialFileExists",{path});
            bool out;
            if(GetObject(res,out))
            {
                return out;
            }
        }
    }
    void TObjectVFS::CreateSymlink(Tesses::Framework::Filesystem::VFSPath existingFile, Tesses::Framework::Filesystem::VFSPath symlinkFile)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            vfs->vfs->CreateSymlink(existingFile,symlinkFile);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "CreateSymlink",{existingFile,symlinkFile});
            
        }
    }
    void TObjectVFS::CreateHardlink(Tesses::Framework::Filesystem::VFSPath existingFile, Tesses::Framework::Filesystem::VFSPath newName)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            vfs->vfs->CreateHardlink(existingFile,newName);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "CreateHardlink",{existingFile,newName});
            
        }
    }
    bool TObjectVFS::DirectoryExists(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->DirectoryExists(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "DirectoryExists",{path});
            bool out;
            if(GetObject(res,out))
            {
                return out;
            }
        }
    }
    void TObjectVFS::DeleteFile(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            vfs->vfs->DeleteFile(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "DeleteFile",{path});
            
        }
    }
    void TObjectVFS::DeleteDirectoryRecurse(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            vfs->vfs->DeleteDirectoryRecurse(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "DeleteDirectoryRecurse",{path});
            
        }
    }
    Tesses::Framework::Filesystem::VFSPathEnumerator TObjectVFS::EnumeratePaths(Tesses::Framework::Filesystem::VFSPath path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->EnumeratePaths(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList* ls=new GCList(this->ls->GetGC());
            auto res = dict->CallMethod(*ls, "EnumeratePaths",{path});
            auto enumerator = TEnumerator::CreateFromObject(*ls,res);
            return Tesses::Framework::Filesystem::VFSPathEnumerator([path,enumerator,ls](Tesses::Framework::Filesystem::VFSPath& path0)->bool{
                if(enumerator == nullptr) return false;
                while(enumerator->MoveNext(ls->GetGC()))
                {
                    auto res = enumerator->GetCurrent(*ls);
                    std::string name;
                    Tesses::Framework::Filesystem::VFSPath path1;
                    if(GetObject(res,path1))
                    {
                        path0 = path1;
                        return true;
                    }
                    else if(GetObject(res,name))
                    {
                        path0 = path / name;
                        return true;
                    }
                }
                return false;
            },[ls]()->void{
                delete ls;
            });
        }
    }
    void TObjectVFS::MoveFile(Tesses::Framework::Filesystem::VFSPath src, Tesses::Framework::Filesystem::VFSPath dest)
    {

        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            vfs->vfs->MoveFile(src,dest);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "MoveFile",{src,dest});
            
        }
    }
    void TObjectVFS::MoveDirectory(Tesses::Framework::Filesystem::VFSPath src, Tesses::Framework::Filesystem::VFSPath dest)
    {

        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            vfs->vfs->MoveDirectory(src,dest);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "MoveDirectory",{src,dest});
            
        }
    }
    Tesses::Framework::Filesystem::VFSPath TObjectVFS::ReadLink(Tesses::Framework::Filesystem::VFSPath path)
    {

        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->ReadLink(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "ReadLink",{path});
            Tesses::Framework::Filesystem::VFSPath myPath;
            if(GetObject(res,myPath))
            {
                return myPath;
            }
        }
        return Tesses::Framework::Filesystem::VFSPath();
    }
    std::string TObjectVFS::VFSPathToSystem(Tesses::Framework::Filesystem::VFSPath path)
    {

        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->VFSPathToSystem(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "VFSPathToSystem",{path});
            std::string myPath;
            if(GetObject(res,myPath))
            {
                return myPath;
            }
        }
        return "/";
    }
    Tesses::Framework::Filesystem::VFSPath TObjectVFS::SystemToVFSPath(std::string path)
    {
        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            return vfs->vfs->SystemToVFSPath(path);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "SystemToVFSPath",{path});
            Tesses::Framework::Filesystem::VFSPath myPath;
            if(GetObject(res,myPath))
            {
                return myPath;
            }
        }
        return Tesses::Framework::Filesystem::VFSPath();
    }
    void TObjectVFS::GetDate(Tesses::Framework::Filesystem::VFSPath path, time_t& lastWrite, time_t& lastAccess)
    {

        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            vfs->vfs->GetDate(path,lastWrite,lastAccess);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "GetDate",{path});
            if(GetObjectHeap(res,dict))
            {
                this->ls->GetGC()->BarrierBegin();
                res = dict->GetValue("LastWrite");
                int64_t v;
                if(GetObject(res,v)) lastWrite=(time_t)v;

                res = dict->GetValue("LastAccess");
                if(GetObject(res,v)) lastAccess=(time_t)v;

                this->ls->GetGC()->BarrierEnd();
            }
        }
    }
    void TObjectVFS::SetDate(Tesses::Framework::Filesystem::VFSPath path, time_t lastWrite, time_t lastAccess)
    {

        TVFSHeapObject* vfs;
        TDictionary* dict;
        if(GetObjectHeap(this->obj, vfs))
        {
            vfs->vfs->SetDate(path,lastWrite,lastAccess);
        }
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "SetDate",{path,(int64_t)lastWrite,(int64_t)lastAccess});
            
        }
    }
    TObjectVFS::~TObjectVFS()
    {
        delete this->ls;
    }
    TVFSHeapObject* TVFSHeapObject::Create(GCList& ls, Tesses::Framework::Filesystem::VFS* vfs)
    {
        TVFSHeapObject* heapObj = new TVFSHeapObject();
        GC* _gc = ls.GetGC();
        ls.Add(heapObj);
        _gc->Watch(heapObj);
        heapObj->vfs = vfs;
        return heapObj;
    }
    TVFSHeapObject* TVFSHeapObject::Create(GCList* ls, Tesses::Framework::Filesystem::VFS* vfs)
    {
        TVFSHeapObject* heapObj = new TVFSHeapObject();
        GC* _gc = ls->GetGC();
        ls->Add(heapObj);
        _gc->Watch(heapObj);
        heapObj->vfs = vfs;
        return heapObj;
    }
    TVFSHeapObject::~TVFSHeapObject()
    {
        if(this->vfs != nullptr)
        delete this->vfs;
    }
    void TVFSHeapObject::Close()
    {
        if(this->vfs != nullptr)
        {
            delete this->vfs;
            this->vfs = nullptr;
        }
    }
}