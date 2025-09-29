#include "CrossLang.hpp"
using namespace Tesses::Framework::Serialization::Json;
namespace Tesses::CrossLang {
    static void LoadDependency(std::shared_ptr<Tesses::Framework::Filesystem::VFS> srcFS, Tesses::Framework::Filesystem::VFSPath sourceDir, std::pair<std::string,TVMVersion> dep, std::vector<std::pair<std::string, TVMVersion>>& files, std::vector<std::pair<std::string, TVMVersion>>& tools);
    static void LoadDependencies(std::shared_ptr<Tesses::Framework::Filesystem::VFS> srcFS, Tesses::Framework::Filesystem::VFSPath sourceDir,TFile* file, std::vector<std::pair<std::string, TVMVersion>>& files, std::vector<std::pair<std::string, TVMVersion>>& tools);
    static void LoadDependency(std::shared_ptr<Tesses::Framework::Filesystem::VFS> srcFS, Tesses::Framework::Filesystem::VFSPath sourceDir, std::pair<std::string,TVMVersion> dep, std::vector<std::pair<std::string, TVMVersion>>& files, std::vector<std::pair<std::string, TVMVersion>>& tools)
    {
        for(auto index = files.begin(); index != files.end(); index++)
        {
            if(index->first == dep.first)
            {
                if(index->second.CompareTo(dep.second) >= 0) return;
                files.erase(index);
                break;
            }
        }
        std::string name = {};
        name.append(dep.first);
        name.push_back('-');
        name.append(dep.second.ToString());
        name.append(".crvm");
        auto filename= sourceDir / name;

        if(srcFS->RegularFileExists(filename))
        {
            auto file = srcFS->OpenFile(filename,"rb");
            
            TFile f;
            f.Load(nullptr, file);
            
            LoadDependencies(srcFS,sourceDir,&f,files,tools);
        }
        else throw VMException("Could not open file: \"" + name + "\".");
        
    }
    static void LoadDependencies(std::shared_ptr<Tesses::Framework::Filesystem::VFS> srcFS, Tesses::Framework::Filesystem::VFSPath sourceDir,TFile* file, std::vector<std::pair<std::string, TVMVersion>>& files, std::vector<std::pair<std::string, TVMVersion>>& tools)
    {
        files.push_back(std::pair<std::string,TVMVersion>(file->name,file->version));
        for(auto item : file->tools)
        {
            bool exists = false;
            for(auto& itm2 : tools)
            {
                if(itm2.first == item.first)
                {
                    exists=true;
                    if(itm2.second.CompareTo(item.second) < 0) {
                        itm2.second = item.second;
                    }
                    break;
                }
            }
            if(!exists)
                tools.push_back(item);
        }
        for(auto item : file->dependencies)
        {
            LoadDependency(srcFS,sourceDir,item,files,tools);
        }
    }
    static void EnumerateCRVM(std::shared_ptr<Tesses::Framework::Filesystem::VFS> srcFS, Tesses::Framework::Filesystem::VFSPath sourceDir,std::string filename, std::vector<std::pair<std::string, TVMVersion>>& files, std::shared_ptr<Tesses::Framework::Filesystem::VFS> destFS)
    {
        

        TFile file;
        auto strm = srcFS->OpenFile(sourceDir / filename,"rb");
        if(strm->EndOfStream()) {


            throw std::runtime_error("File does not exist: " + (sourceDir / filename).ToString() );
        }
        file.Load(nullptr,strm);

      

        std::vector<std::pair<std::string, TVMVersion>> tools;
        LoadDependencies(srcFS,sourceDir,&file,files,tools);


        

       
        JObject json_data {
                JOItem{"name", file.name},
                JOItem{"version",file.version.ToString()}
                
         };
            if(!file.info.empty())
            {
                json_data.SetValue("info",Json::Decode(file.info));
                
            }

            if(file.icon > -1 && file.icon < file.resources.size())
            {
                json_data.SetValue("icon",file.name + "-" + file.version.ToString()+"_"+ std::to_string(file.icon) + ".bin");
            }
            if(!file.tools.empty())
            {
                JArray array;
                for(auto& item : file.tools)
                {
                    array.Add(JObject {
                        JOItem {
                            "name",
                            item.first
                        },
                        JOItem {
                            "version",
                            item.second.ToString()
                        }
                    });
                }
                json_data.SetValue("tools", array);
            }
            if(!file.vms.empty())
            {
                JArray array;
                for(auto& item : file.vms)
                {
                    array.Add(JObject {
                        JOItem {
                            "name",
                            item.first
                        },
                        JOItem {
                            "how_to_get",
                            item.second
                        }
                    });
                }
                json_data.SetValue("vms", array);
            }


        Tesses::Framework::TextStreams::StreamWriter json_writer(destFS->OpenFile(Tesses::Framework::Filesystem::VFSPath() / "crossapp.json","wb" )); 
        json_writer.WriteLine(Json::Encode(json_data,true));
    }
    Tesses::Framework::Filesystem::VFSPath Merge(std::shared_ptr<Tesses::Framework::Filesystem::VFS> srcFS, Tesses::Framework::Filesystem::VFSPath sourcePath, std::shared_ptr<Tesses::Framework::Filesystem::VFS> destFS)
    {
        std::vector<std::pair<std::string, TVMVersion>> files;

        for(auto ent : destFS->EnumeratePaths(Tesses::Framework::Filesystem::VFSPath()))
        {
            if(destFS->DirectoryExists(ent))
            {
                destFS->DeleteDirectoryRecurse(ent);
            }
            else {
                destFS->DeleteFile(ent);
            }
        }

        
        EnumerateCRVM(srcFS,sourcePath.GetParent(), sourcePath.GetFileName(),files,destFS);

        for(auto item : files)
        {
            auto filePath = sourcePath.GetParent() / item.first + "-" + item.second.ToString() + ".crvm";
            if(srcFS->RegularFileExists(filePath))
            {
                auto strm = srcFS->OpenFile(filePath,"rb");
                Disassemble(strm,destFS,false);
            }
        }

        return Assemble(destFS);
    }
}