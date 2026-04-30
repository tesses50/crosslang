#include "CrossLang.hpp"

namespace Tesses::CrossLang {

    EmbedStream::EmbedStream(std::shared_ptr<GC> gc, TFile* file, uint32_t resource)
    {
        this->offset = 0;
        this->resource = resource;
        this->file = CreateMarkedTObject(gc,file);
    }
    bool EmbedStream::CanRead()
    {
        TFile* file;
        if(GetObjectHeap(this->file->GetObject(),file))
        {
            if(this->resource >= file->resources.size()) return false;
            if(this->offset >= file->resources[this->resource].size()) return false;
            return true;
        }
        return false;
    }
    bool EmbedStream::CanSeek()
    {
        return true;
    }
    bool EmbedStream::EndOfStream()
    {
        return !CanRead();
    }
    size_t EmbedStream::Read(uint8_t* buff, size_t len)
    {
        TFile* file;
                
        if(GetObjectHeap(this->file->GetObject(),file))
        {
            if(this->resource >= file->resources.size()) return 0;
            auto flen = file->resources[this->resource].size();
            if(this->offset >= flen) return 0;
                    
            len = std::min(len, std::min(
                flen,
                flen - this->offset
            ));

            memcpy(buff,file->resources[this->resource].data() + this->offset,len);
            this->offset += len;
            return len;
        }
        return 0;
    }
    int64_t EmbedStream::GetPosition()
    {
        return (int64_t)this->offset;
    }
    int64_t EmbedStream::GetLength()
    {
        TFile* file;
                
        if(GetObjectHeap(this->file->GetObject(),file))
        {
            if(this->resource >= file->resources.size()) return 0;
            return (int64_t)file->resources[this->resource].size();
        }
        return 0;
    }
    void EmbedStream::Seek(int64_t pos, Tesses::Framework::Streams::SeekOrigin whence)
    {
        switch(whence)
        {
            case Tesses::Framework::Streams::SeekOrigin::Begin:
                this->offset = (uint32_t)pos;
            	break;
            case Tesses::Framework::Streams::SeekOrigin::Current:
            {
                int64_t cur = this->offset;
                cur += pos;
                this->offset = (uint32_t)cur;
            }
            	break;
            case Tesses::Framework::Streams::SeekOrigin::End:
            {
            	TFile* file;
                
                if(GetObjectHeap(this->file->GetObject(),file))
                {
                    if(this->resource >= file->resources.size()) return;
                    int64_t cur = (int64_t)file->resources[this->resource].size();
                    cur += pos;
                    this->offset = (uint32_t)cur;
                }
                break;
            }
        }
    }
	TObject EmbedDirectory::getEntry(Tesses::Framework::Filesystem::VFSPath path)
	{
		path = path.CollapseRelativeParents();
		auto curEntry = this->dir->GetObject();
		for(auto item : path.path)		
		{
			TDictionary* dict;
			if(GetObjectHeap(curEntry,dict) && dict->HasValue(item))
			{
				curEntry = dict->GetValue(item);
				continue; //don't want to return undefined now do we
			}
			return Undefined();
		}
		return curEntry;
	}
    
    std::shared_ptr<Tesses::Framework::Streams::Stream> EmbedDirectory::OpenFile(Tesses::Framework::Filesystem::VFSPath path, std::string mode)
	{
		if(mode != "r" && mode != "rb") return nullptr;
		
		auto ent = getEntry(path);
		TCallable* call;
		if(GetObjectHeap(ent,call))
        {
            GCList ls(this->dir->GetGC());
            auto fileO = call->Call(ls,{});
            std::shared_ptr<Tesses::Framework::Streams::Stream> strm;
            if(GetObject(fileO,strm)) return strm;
        }
        return nullptr;
	}
	
    bool EmbedDirectory::Stat(Tesses::Framework::Filesystem::VFSPath path, Tesses::Framework::Filesystem::StatData& data)
	{
		auto ent = getEntry(path);


		TDictionary* dict;
		if(GetObjectHeap(ent,dict))
        {
            data.Size = 0;
            data.Mode = Tesses::Framework::Filesystem::MODE_DIRECTORY | 0755;
            data.BlockCount = 0;
            data.BlockSize = 0;
            data.Device = 0;
            data.DeviceId = 0;
            data.GroupId = 0;
            data.HardLinks = 1;
            data.Inode = 0;
            data.LastAccess = Tesses::Framework::Date::DateTime(0);
            data.LastModified = Tesses::Framework::Date::DateTime(0);
            data.LastStatus = Tesses::Framework::Date::DateTime(0);
            data.UserId = 0;
            
            return true;
        }
        TCallable* cal;
        if(GetObjectHeap(ent, cal))
        {
            GCList ls(this->dir->GetGC());
            auto fileO= cal->Call(ls, {});
            std::shared_ptr<Tesses::Framework::Streams::Stream> strm;
            if(GetObject(fileO,strm)) {

            data.Size = (uint64_t)strm->GetLength();
            data.Mode = Tesses::Framework::Filesystem::MODE_REGULAR | 0755; 
            data.BlockSize = 512;
            data.BlockCount = data.Size / data.BlockSize;
           
            data.Device = 0;
            data.DeviceId = 0;
            data.GroupId = 0;
            data.HardLinks = 1;
            data.Inode = 0;
            data.LastAccess = Tesses::Framework::Date::DateTime(0);
            data.LastModified = Tesses::Framework::Date::DateTime(0);
            data.LastStatus = Tesses::Framework::Date::DateTime(0);
            data.UserId = 0;
            
            return true;
            }
        }
        return false;
	}

    class DICT_DIRENUM
    {
        GCList ls;
        TDictionary* dict;
        std::map<std::string, Tesses::CrossLang::TObject>::iterator current;
        bool hasStarted = false;
        public:
            std::string GetCurrent()
            {
                return this->current->first;
            }
            DICT_DIRENUM(std::shared_ptr<GC> gc, TDictionary* dict) : ls(gc), dict(dict)
            {
                ls.Add(dict);
            }
            bool MoveNext()
            {
                if(!this->hasStarted)
                {
                    this->hasStarted=true;
                    this->current = this->dict->items.begin();
                    return !this->dict->items.empty();
                }
                else
                {
                    this->current++;
                    return this->current != this->dict->items.end();
                }
            }

    };


    Tesses::Framework::Filesystem::VFSPathEnumerator EmbedDirectory::EnumeratePaths(Tesses::Framework::Filesystem::VFSPath path)
    {
        auto ent = getEntry(path);
		TDictionary* dict;
		if(GetObjectHeap(ent,dict))
        {
            DICT_DIRENUM* dir2 = new DICT_DIRENUM(this->dir->GetGC(), dict);

            Tesses::Framework::Filesystem::VFSPathEnumerator er(
                [dir2,path](Tesses::Framework::Filesystem::VFSPath& path2)->bool {
                    if(dir2->MoveNext())
                    {
                        path2 = path / dir2->GetCurrent();
                        return true;
                    }
                    return false;
                },
                [dir2]()->void {
                    delete dir2;
                }
            );

            return er;
        }
        return Tesses::Framework::Filesystem::VFSPathEnumerator();
    }

    EmbedDirectory::EmbedDirectory(std::shared_ptr<GC> gc, TDictionary* dict)
    {
        this->dir = CreateMarkedTObject(gc, dict);
    }
    
    std::string EmbedDirectory::VFSPathToSystem(Tesses::Framework::Filesystem::VFSPath path)
    {
        return path.ToString();
    }
    Tesses::Framework::Filesystem::VFSPath EmbedDirectory::SystemToVFSPath(std::string path)
    {
        return path;
    }
}