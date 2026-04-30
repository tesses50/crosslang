#include "CrossLang.hpp"

namespace Tesses::CrossLang
{
    RelativeFilesystem::RelativeFilesystem(std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs, Tesses::Framework::Filesystem::VFSPath working)
    {
        this->vfs = vfs;
        this->path = working;
    }
    std::shared_ptr<Tesses::Framework::Streams::Stream> RelativeFilesystem::OpenFile(Tesses::Framework::Filesystem::VFSPath path, std::string mode)
    {
        if(path.relative)
        {
            return this->vfs->OpenFile(path.MakeAbsolute(GetWorking()), mode);
        }
        else
        {
            return this->vfs->OpenFile(path,mode);
        }
    }

    void RelativeFilesystem::CreateDirectory(Tesses::Framework::Filesystem::VFSPath path)
    {
        if(path.relative)
        {
            this->vfs->CreateDirectory(path.MakeAbsolute(GetWorking()));
        }
        else
        {
            this->vfs->CreateDirectory(path);
        }
    }
    void RelativeFilesystem::DeleteDirectory(Tesses::Framework::Filesystem::VFSPath path)
    {
        if(path.relative)
        {
            this->vfs->DeleteDirectory(path.MakeAbsolute(GetWorking()));
        }
        else
        {
            this->vfs->DeleteDirectory(path);
        }
    }


    void RelativeFilesystem::DeleteFile(Tesses::Framework::Filesystem::VFSPath path)
    {
        if(path.relative)
        {
            this->vfs->DeleteFile(path.MakeAbsolute(GetWorking()));
        }
        else
        {
            this->vfs->DeleteFile(path);
        }
    }
    void RelativeFilesystem::CreateSymlink(Tesses::Framework::Filesystem::VFSPath existingFile, Tesses::Framework::Filesystem::VFSPath symlinkFile)
    {
        if(symlinkFile.relative)
        {
            this->vfs->CreateSymlink(existingFile,symlinkFile.MakeAbsolute(GetWorking()));
        }
        else
        {
            this->vfs->CreateSymlink(existingFile, symlinkFile);
        }
    }
    Tesses::Framework::Filesystem::VFSPathEnumerator RelativeFilesystem::EnumeratePaths(Tesses::Framework::Filesystem::VFSPath path)
    {
        if(path.relative)
        {
            return this->vfs->EnumeratePaths(path.MakeAbsolute(GetWorking()));
        }
        else
        {
            return this->vfs->EnumeratePaths(path);
        }
    }
    void RelativeFilesystem::CreateHardlink(Tesses::Framework::Filesystem::VFSPath existingFile, Tesses::Framework::Filesystem::VFSPath newName)
    {
        auto working = GetWorking();
        if(existingFile.relative)
        {
            existingFile = existingFile.MakeAbsolute(working);
            existingFile = existingFile.CollapseRelativeParents();
        }

        if(newName.relative)
        {
            newName = newName.MakeAbsolute(working);
            newName = newName.CollapseRelativeParents();
        }

        this->vfs->CreateHardlink(existingFile, newName);
    }

    void RelativeFilesystem::MoveFile(Tesses::Framework::Filesystem::VFSPath src, Tesses::Framework::Filesystem::VFSPath dest)
    {
        auto working = GetWorking();
        if(src.relative)
        {
            src = src.MakeAbsolute(working);
            src = src.CollapseRelativeParents();
        }

        if(dest.relative)
        {
            dest = dest.MakeAbsolute(working);
            dest = dest.CollapseRelativeParents();
        }

        this->vfs->MoveFile(src, dest);
    }

    void RelativeFilesystem::MoveDirectory(Tesses::Framework::Filesystem::VFSPath src, Tesses::Framework::Filesystem::VFSPath dest)
    {

        auto working = GetWorking();
        if(src.relative)
        {
            src = src.MakeAbsolute(working);
            src = src.CollapseRelativeParents();
        }

        if(dest.relative)
        {
            dest = dest.MakeAbsolute(working);
            dest = dest.CollapseRelativeParents();
        }

        this->vfs->MoveFile(src, dest);
    }
    Tesses::Framework::Filesystem::VFSPath RelativeFilesystem::ReadLink(Tesses::Framework::Filesystem::VFSPath path)
    {
         if(path.relative)
        {
            return this->vfs->ReadLink(path.MakeAbsolute(GetWorking()));
        }
        else
        {
            return this->vfs->ReadLink(path);
        }
    }
    std::string RelativeFilesystem::VFSPathToSystem(Tesses::Framework::Filesystem::VFSPath path)
    {
        return this->vfs->VFSPathToSystem(path);
    }
    Tesses::Framework::Filesystem::VFSPath RelativeFilesystem::SystemToVFSPath(std::string path)
    {
        return this->vfs->SystemToVFSPath(path);
    }

    void RelativeFilesystem::SetDate(Tesses::Framework::Filesystem::VFSPath path, Tesses::Framework::Date::DateTime lastWrite, Tesses::Framework::Date::DateTime lastAccess)
    {
        if(path.relative)
        {
            this->vfs->SetDate(path.MakeAbsolute(GetWorking()), lastWrite,lastAccess);
        }
        else
        {
            this->vfs->SetDate(path,lastWrite,lastAccess);
        }
    }
    bool RelativeFilesystem::Stat(Tesses::Framework::Filesystem::VFSPath path, Tesses::Framework::Filesystem::StatData& stat)
    {
        if(path.relative)
        {
            return this->vfs->Stat(path.MakeAbsolute(GetWorking()), stat);
        }
        else
        {
            return this->vfs->Stat(path,stat);
        }
    }
    bool RelativeFilesystem::StatVFS(Tesses::Framework::Filesystem::VFSPath path, Tesses::Framework::Filesystem::StatVFSData& vfsData)
    {
        if(path.relative)
        {
            return this->vfs->StatVFS(path.MakeAbsolute(GetWorking()), vfsData);
        }
        else
        {
            return this->vfs->StatVFS(path,vfsData);
        }
    }

    void RelativeFilesystem::Chmod(Tesses::Framework::Filesystem::VFSPath path, uint32_t mode)
    {
        if(path.relative)
        {
            this->vfs->Chmod(path.MakeAbsolute(GetWorking()), mode);
        }
        else
        {
            this->vfs->Chmod(path,mode);
        }
    }
    void RelativeFilesystem::Chown(Tesses::Framework::Filesystem::VFSPath path, uint32_t uid, uint32_t gid)
    {
        if(path.relative)
        {
            this->vfs->Chown(path.MakeAbsolute(GetWorking()), uid, gid);
        }
        else
        {
            this->vfs->Chown(path,uid,gid);
        }
    }

    void RelativeFilesystem::Lock(Tesses::Framework::Filesystem::VFSPath path)
    {
        if(path.relative)
        {
            this->vfs->Lock(path.MakeAbsolute(GetWorking()));
        }
        else
        {
            this->vfs->Lock(path);
        }
    }
    void RelativeFilesystem::Unlock(Tesses::Framework::Filesystem::VFSPath path)
    {
        if(path.relative)
        {
            this->vfs->Unlock(path.MakeAbsolute(GetWorking()));
        }
        else
        {
            this->vfs->Unlock(path);
        }
    }

    Tesses::Framework::Filesystem::FIFOCreationResult RelativeFilesystem::CreateFIFO(Tesses::Framework::Filesystem::VFSPath path, uint32_t mod)
    {
        if(path.relative)
        {
            return this->vfs->CreateFIFO(path.MakeAbsolute(GetWorking()), mod);
        }
        else
        {
            return this->vfs->CreateFIFO(path, mod);
        }
    }
    Tesses::Framework::Filesystem::VFSPath RelativeFilesystem::GetWorking()
    {
        mtx.Lock();
        auto path = this->path;
        mtx.Unlock();   
        return path;
    }
    void RelativeFilesystem::SetWorking(Tesses::Framework::Filesystem::VFSPath working)
    {
        mtx.Lock();
        this->path = working;
        mtx.Unlock();
    }
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> RelativeFilesystem::GetVFS()
    {
        return this->vfs;
    }

            
    std::shared_ptr<Tesses::Framework::Filesystem::FSWatcher> RelativeFilesystem::CreateWatcher(std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs, Tesses::Framework::Filesystem::VFSPath path)
    {
        if(path.relative)
        {
            return Tesses::Framework::Filesystem::FSWatcher::Create(vfs, path.MakeAbsolute(GetWorking()));
        }
        else
        {
            return Tesses::Framework::Filesystem::FSWatcher::Create(vfs,path);
        }
    }
            
}