#include "CrossLang.hpp"

namespace Tesses::CrossLang {

    TObjectVFS::TObjectVFS(std::shared_ptr<GC> gc, TObject obj)
    {
        this->ls = new GCList(gc);
        this->ls->Add(obj);
        this->obj = obj;
        
        
    }
    Tesses::Framework::Filesystem::FIFOCreationResult TObjectVFS::CreateFIFO(Tesses::Framework::Filesystem::VFSPath path)
    {
        TDictionary* dict;
        
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "CreateFIFO",{path});
            int64_t n=0;
            if(GetObject(res, n)) return (Tesses::Framework::Filesystem::FIFOCreationResult)n;
        }
        return Tesses::Framework::Filesystem::FIFOCreationResult::UnknownError;
    }
            
    std::shared_ptr<Tesses::Framework::Streams::Stream> TObjectVFS::OpenFile(Tesses::Framework::Filesystem::VFSPath path, std::string mode)
    {
        TDictionary* dict;
        
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "OpenFile",{path,mode});
            std::shared_ptr<Tesses::Framework::Streams::Stream> strm;
            if(GetObject(res,strm))
            {
                return strm;
            }
        }
        return nullptr;
    }
    void TObjectVFS::CreateDirectory(Tesses::Framework::Filesystem::VFSPath path)
    {
        TDictionary* dict;
       
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "CreateDirectory",{path});
            
        }
    
    }
    void TObjectVFS::DeleteDirectory(Tesses::Framework::Filesystem::VFSPath path)
    {
        TDictionary* dict;
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "DeleteDirectory",{path});
            
        }
    }
    void TObjectVFS::Lock(Tesses::Framework::Filesystem::VFSPath path)
    {
        TDictionary* dict;
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "Lock",{path});
            
        }
    }
    void TObjectVFS::Unlock(Tesses::Framework::Filesystem::VFSPath path)
    {
        TDictionary* dict;
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "Unlock",{path});
            
        }
    }
  void TObjectVFS::CreateSymlink(Tesses::Framework::Filesystem::VFSPath existingFile, Tesses::Framework::Filesystem::VFSPath symlinkFile)
    {
        TDictionary* dict;
        
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "CreateSymlink",{existingFile,symlinkFile});
            
        }
    }
    void TObjectVFS::CreateHardlink(Tesses::Framework::Filesystem::VFSPath existingFile, Tesses::Framework::Filesystem::VFSPath newName)
    {
        TDictionary* dict;
       
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "CreateHardlink",{existingFile,newName});
            
        }
    }
   void TObjectVFS::DeleteFile(Tesses::Framework::Filesystem::VFSPath path)
    {
        TDictionary* dict;
        
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "DeleteFile",{path});
            
        }
    }
    void TObjectVFS::DeleteDirectoryRecurse(Tesses::Framework::Filesystem::VFSPath path)
    {
        TDictionary* dict;
        
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "DeleteDirectoryRecurse",{path});
            
        }
    }
    Tesses::Framework::Filesystem::VFSPathEnumerator TObjectVFS::EnumeratePaths(Tesses::Framework::Filesystem::VFSPath path)
    {
        TDictionary* dict;
        
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
        return Tesses::Framework::Filesystem::VFSPathEnumerator();
    }
    void TObjectVFS::MoveFile(Tesses::Framework::Filesystem::VFSPath src, Tesses::Framework::Filesystem::VFSPath dest)
    {

        TDictionary* dict;
        
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "MoveFile",{src,dest});
            
        }
    }
    void TObjectVFS::MoveDirectory(Tesses::Framework::Filesystem::VFSPath src, Tesses::Framework::Filesystem::VFSPath dest)
    {

        TDictionary* dict;
       
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "MoveDirectory",{src,dest});
            
        }
    }
    Tesses::Framework::Filesystem::VFSPath TObjectVFS::ReadLink(Tesses::Framework::Filesystem::VFSPath path)
    {

        TDictionary* dict;
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

        TDictionary* dict;
        
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
        TDictionary* dict;
       
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
    void TObjectVFS::SetDate(Tesses::Framework::Filesystem::VFSPath path, Tesses::Framework::Date::DateTime lastWrite, Tesses::Framework::Date::DateTime lastAccess)
    {

        TDictionary* dict;
        
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "SetDate",{path,std::make_shared<Tesses::Framework::Date::DateTime>(lastWrite),std::make_shared<Tesses::Framework::Date::DateTime>(lastAccess)});
            
        }
    }
    void TObjectVFS::Chmod(Tesses::Framework::Filesystem::VFSPath path, uint32_t mode)
    {
        TDictionary* dict;
        
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "Chmod",{path,(int64_t)mode});
            
        }
    }
    void TObjectVFS::Chown(Tesses::Framework::Filesystem::VFSPath path, uint32_t uid, uint32_t gid)
    {
        TDictionary* dict;
        
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls, "Chown",{path,(int64_t)uid, (int64_t)gid});
            
        }
    }
    bool TObjectVFS::Stat(Tesses::Framework::Filesystem::VFSPath path, Tesses::Framework::Filesystem::StatData& data)
    {
        TDictionary* dict;
        
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "Stat",{path});
            int64_t _num;
            TDictionary* _dict;
            TObject _o;
            if(GetObjectHeap(res,_dict))
            {
                this->ls->GetGC()->BarrierBegin();
                _o = _dict->GetValue("BlockSize");
                if(GetObject(_o,_num)) data.BlockSize = (uint64_t)_num;

                _o = _dict->GetValue("BlockCount");
                if(GetObject(_o,_num)) data.BlockCount = (uint64_t)_num;
                
                _o = _dict->GetValue("Device");
                if(GetObject(_o,_num)) data.Device = (uint64_t)_num;
                
                _o = _dict->GetValue("DeviceId");
                if(GetObject(_o,_num)) data.DeviceId = (uint64_t)_num;
                
                _o = _dict->GetValue("GroupId");
                if(GetObject(_o,_num)) data.GroupId = (uint32_t)_num;
                
                _o = _dict->GetValue("HardLinks");
                if(GetObject(_o,_num)) data.HardLinks = (uint64_t)_num;
                std::shared_ptr<Tesses::Framework::Date::DateTime> dt;
                _o = _dict->GetValue("LastAccess");
                if(GetObject(_o,dt)) data.LastAccess = dt ? *dt : Tesses::Framework::Date::DateTime(0);
                

                _o = _dict->GetValue("LastModified");
                if(GetObject(_o,dt)) data.LastModified = dt ? *dt : Tesses::Framework::Date::DateTime(0);
                
                

                _o = _dict->GetValue("LastStatus");
                if(GetObject(_o,dt)) data.LastStatus = dt ? *dt : Tesses::Framework::Date::DateTime(0);
                


                _o = _dict->GetValue("Mode");
                if(GetObject(_o,_num)) data.Mode = (uint32_t)_num;
                
                _o = _dict->GetValue("Size");
                if(GetObject(_o,_num)) data.Size = (uint64_t)_num;
                
                _o = _dict->GetValue("UserId");
                if(GetObject(_o,_num)) data.UserId = (uint32_t)_num;


                this->ls->GetGC()->BarrierEnd();
                return true;
            }
        }

        return false;
    }
    bool TObjectVFS::StatVFS(Tesses::Framework::Filesystem::VFSPath path, Tesses::Framework::Filesystem::StatVFSData& data)
    {
        TDictionary* dict;
        
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            auto res = dict->CallMethod(ls, "StatVFS",{path});
            int64_t _num;
            TDictionary* _dict;
            TObject _o;
            if(GetObjectHeap(res,_dict))
            {
                this->ls->GetGC()->BarrierBegin();
                _o = _dict->GetValue("BlockSize");
                if(GetObject(_o,_num)) data.BlockSize = (uint64_t)_num;

                _o = _dict->GetValue("FragmentSize");
                if(GetObject(_o,_num)) data.FragmentSize = (uint64_t)_num;
                _o = _dict->GetValue("Blocks");
                if(GetObject(_o,_num)) data.Blocks = (uint64_t)_num;
                _o = _dict->GetValue("BlocksFree");
                if(GetObject(_o,_num)) data.BlocksFree = (uint64_t)_num;
                _o = _dict->GetValue("BlocksAvailable");
                if(GetObject(_o,_num)) data.BlocksAvailable = (uint64_t)_num;
                _o = _dict->GetValue("TotalInodes");
                if(GetObject(_o,_num)) data.TotalInodes = (uint64_t)_num;
                _o = _dict->GetValue("FreeInodes");
                if(GetObject(_o,_num)) data.FreeInodes = (uint64_t)_num;
                _o = _dict->GetValue("AvailableInodes");
                if(GetObject(_o,_num)) data.AvailableInodes = (uint64_t)_num;
                _o = _dict->GetValue("Id");
                if(GetObject(_o,_num)) data.Id = (uint64_t)_num;
                _o = _dict->GetValue("Flags");
                if(GetObject(_o,_num)) data.Flags = (uint64_t)_num;
                _o = _dict->GetValue("MaxNameLength");
                if(GetObject(_o,_num)) data.MaxNameLength = (uint64_t)_num;
                
                
                
                
                

                this->ls->GetGC()->BarrierEnd();
                return true;
            }
        }

        return false;
    }
    void TObjectVFS::Close()
    {
        TDictionary* dict;
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls,"Dispose",{});
        }
    }
    TObjectVFS::~TObjectVFS()
    {
        TDictionary* dict;
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls(this->ls->GetGC());
            dict->CallMethod(ls,"Dispose",{});
        }
        delete this->ls;
    }
    
}