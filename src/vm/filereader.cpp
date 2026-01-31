#include "CrossLang.hpp"


#include <cstring>
#include <filesystem>
#include <iostream>
namespace Tesses::CrossLang
{

    TFile* TFile::Create(GCList& ls)
    {
        TFile* f = new TFile();
        f->icon = -1;
        GC* _gc = ls.GetGC();
        ls.Add(f);
        _gc->Watch(f);
        return f;
    }
    TFile* TFile::Create(GCList* ls)
    {
        TFile* f = new TFile();
        f->icon=-1;
        GC* _gc = ls->GetGC();
        ls->Add(f);
        _gc->Watch(f);
        return f;
    }
    void TFileChunk::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        this->file->Mark();
    }
   
    TFileChunk* TFileChunk::Create(GCList& ls)
    {
        TFileChunk* chk = new TFileChunk();
        GC* _gc = ls.GetGC();
        ls.Add(chk);
        _gc->Watch(chk);
        return chk;
    }
    TFileChunk* TFileChunk::Create(GCList* ls)
    {
        TFileChunk* chk = new TFileChunk();
        GC* _gc = ls->GetGC();
        ls->Add(chk);
        _gc->Watch(chk);
        return chk;
    }
    void TFile::Mark()
    {
        if(this->marked) return;
        this->marked=true;
    
        for(auto item : this->chunks)
            item->Mark();
        
    }
    void TFile::Skip(std::shared_ptr<Tesses::Framework::Streams::Stream> stream,size_t len)
    {
        if(stream->CanSeek())
        {
            stream->Seek((int64_t)len,Tesses::Framework::Streams::SeekOrigin::Current);
        }
        else{
        uint8_t* buffer=new uint8_t[len];
        Ensure(stream,buffer,len);
        delete[] buffer;
        }
    }
    void TFile::Ensure(std::shared_ptr<Tesses::Framework::Streams::Stream> stream, uint8_t* buffer,size_t len)
    {
        size_t read = stream->ReadBlock(buffer, len);
        if(read < len) throw VMException("End of file, could not read " + std::to_string((int64_t)len) + " byte(s)., offset=" + std::to_string(stream->GetLength()));
    }
    std::string TFile::EnsureString(std::shared_ptr<Tesses::Framework::Streams::Stream> stream)
    {
        auto len = EnsureInt(stream);
        if(len == 0) return {};
        std::string str={};
        str.resize((size_t)len);
        Ensure(stream,(uint8_t*)str.data(),str.size());
        return str;
    }
    
    uint32_t TFile::EnsureInt(std::shared_ptr<Tesses::Framework::Streams::Stream> stream)
    {
        uint8_t buffer[4];
        Ensure(stream,buffer,4);
        return BitConverter::ToUint32BE(buffer[0]);
    }
    std::string TFile::GetString(std::shared_ptr<Tesses::Framework::Streams::Stream> stream)
    {
        uint32_t index=EnsureInt(stream);
        if(index >= this->strings.size()) throw VMException("String does not exist in TCrossVM file, expected string index: " + std::to_string(index) + ", total strings: " + std::to_string(this->strings.size()));
        return this->strings[index];
    }
    void TFile::EnsureCanRunInCrossLang()
    {
        if(this->vms.empty()) return;
        
        for(auto item : this->vms)
        {
            if(item.first == VMName)
            {
                return;
            }
        }


        std::string errorMessage="The virtual machines supported are:\n";

        for(auto item : this->vms)
        {
            errorMessage += item.first + "\n\t" + item.second + "\n";
        }
        throw VMException(errorMessage);
    }

    TDictionary* TFile::MetadataDecode(GCList& ls, size_t midx)
    {
        if(midx >= this->metadata.size()) return nullptr;
        if(this->metadata[midx].second.empty()) return nullptr;
        if(this->metadata[midx].second[0] != 8) return nullptr;
        size_t index = 0;
        auto& bytes = this->metadata[midx].second;
        
        std::function<TObject()> parseEnt;
        parseEnt = [&]()->TObject {
            if(index >= bytes.size()) throw std::out_of_range("Abrupt end of metadata");
            switch(bytes[index++])
            {
                case 0:
                    return false;
                case 1:
                    return true;
                case 2:
                    return nullptr;
                case 3:
                {
                    if(index + 8 <= bytes.size())
                    {
                        auto val= BitConverter::ToUint64BE(bytes[index]);
                        index+=8;
                        int64_t val2;
                        memcpy(&val2,&val,sizeof(val));
                        return val2;
                    }
                    else throw std::out_of_range("Abrupt end of metadata");
                }
                    break;
                case 4:
                {
                    if(index + 8 <= bytes.size())
                    {
                        auto val= BitConverter::ToDoubleBE(bytes[index]);
                        index+=8;
                        return val;
                    }
                    else throw std::out_of_range("Abrupt end of metadata");
                }
                    break;
                case 5:

                    if(index + 1 <= bytes.size())
                    {
                        return (char)bytes[index++];
                    }
                    else throw std::out_of_range("Abrupt end of metadata");
                    break;
                case 6:
                {
                    if(index + 4 <= bytes.size())
                    {
                        auto val= BitConverter::ToUint32BE(bytes[index]);
                        index+=4;
                        return this->strings.at((size_t)val);
                    } else throw std::out_of_range("Abrupt end of metadata");
                   
                }
                    break;
                case 7:
                {
                    if(index + 4 <= bytes.size())
                    {
                        auto val= BitConverter::ToUint32BE(bytes[index]);
                        index+=4;
                        std::vector<TObject> items;
                        for(uint32_t i = 0; i < val; i++)
                        {
                            items.push_back(parseEnt());
                        }

                        return TList::Create(ls,items.begin(),items.end());
                    } else throw std::out_of_range("Abrupt end of metadata");
                }
                    break;
                case 8:
                    if(index + 4 <= bytes.size())
                    {
                        auto val= BitConverter::ToUint32BE(bytes[index]);
                        index+=4;
                        std::vector<TDItem> items;

                        for(uint32_t i = 0; i < val; i++)
                        {
                            if(index + 4 <= bytes.size())
                            {
                                auto val2= BitConverter::ToUint32BE(bytes[index]);
                                index+=4;
                                std::string& text=this->strings.at((size_t)val2);
                                items.emplace_back(text,parseEnt());
                            } else throw std::out_of_range("Abrupt end of metadata");

                        }

                        return TDictionary::Create(ls, items.begin(),items.end());
                    }else throw std::out_of_range("Abrupt end of metadata");
                    break;
                case 9:
                {
                    if(index + 4 <= bytes.size())
                    {
                        auto val= BitConverter::ToUint32BE(bytes[index]);
                        index+=4;
                        auto ba = TByteArray::Create(ls);
                        ba->data = this->resources.at((size_t)val);
                        return ba;
                    } else throw std::out_of_range("Abrupt end of metadata");
                }
                    break;
                case 10:
                {
                    if(index + 4 <= bytes.size())
                    {
                        auto val= BitConverter::ToUint32BE(bytes[index]);
                        index+=4;
                        return std::make_shared<EmbedStream>(ls.GetGC(),this,val);
                    } else throw std::out_of_range("Abrupt end of metadata");
                }
                    break;
                case 11:
                {
                    
                    auto data = parseEnt();
                    TDictionary* dict;
                    if(GetObjectHeap(data,dict))
                    {
                        return std::make_shared<EmbedDirectory>(ls.GetGC(),dict);
                    }
                    else return Undefined();
                }
                    break;
                case 12:
                {
                    if(index + 4 <= bytes.size())
                    {
                        auto val= BitConverter::ToUint32BE(bytes[index]);
                        index+=4;
                        ls.GetGC()->BarrierBegin();
                        auto em = TExternalMethod::Create(ls,"",{},[val,this](GCList& ls, std::vector<TObject> args)->TObject {
                            return std::make_shared<EmbedStream>(ls.GetGC(),this,val);
                        });
                        em->watch.push_back(this);
                        ls.GetGC()->BarrierEnd();

                        return em;
                    } else throw std::out_of_range("Abrupt end of metadata");
                }
                    break;
                default:
                    throw std::runtime_error("Invalid metadata opcode");
            }
        };

        TDictionary* dict_res;
        TObject ent = parseEnt();
        if(GetObjectHeap(ent, dict_res)) return dict_res;
        return nullptr;
    }

    void TFile::Load(GC* gc, std::shared_ptr<Tesses::Framework::Streams::Stream> stream)
    {
        
        uint8_t main_header[18];
        Ensure(stream,main_header,sizeof(main_header));
        if(strncmp((const char*)main_header,"TCROSSVM",8) != 0) throw VMException("Invalid TCrossVM image.");
        TVMVersion version(main_header+8);
        if(version.CompareToRuntime() == 1)
        {
            throw VMException("Runtime is too old.");
        }
        TVMVersion v2(main_header+13);
        this->version = v2;

        size_t _len = (size_t)EnsureInt(stream);

        char table_name[4];

        for(size_t i = 0;i < _len; i++)
        {
            Ensure(stream,(uint8_t*)table_name,sizeof(table_name));
            size_t tableLen = (size_t)EnsureInt(stream);
            if(strncmp(table_name,"NAME",4) == 0) 
            {
                this->name = GetString(stream);
            }
            else if(strncmp(table_name,"INFO",4) == 0) 
            {
                this->info = GetString(stream);
            }
            else if(strncmp(table_name,"DEPS",4) == 0) //dependencies
            {
                std::string name = GetString(stream);
                uint8_t version_bytes[5];
                Ensure(stream,version_bytes,sizeof(version_bytes));
                TVMVersion depVersion(version_bytes);
                this->dependencies.push_back(std::pair<std::string,TVMVersion>(name, depVersion));
            }
            else if(strncmp(table_name,"TOOL",4) == 0) //compile tools (for package manager)
            {
                std::string name = GetString(stream);
                uint8_t version_bytes[5];
                Ensure(stream,version_bytes,sizeof(version_bytes));
                TVMVersion depVersion(version_bytes);
                this->tools.push_back(std::pair<std::string,TVMVersion>(name, depVersion));
            }
            else if(strncmp(table_name,"RESO",4) == 0) //resources (using embed)
            {
                auto& data = this->resources.emplace_back(tableLen);
                Ensure(stream,data.data(), data.size());
            }
            else if(strncmp(table_name,"CHKS",4) == 0 && gc != nullptr) //chunks
            {
                GCList ls(gc);
                size_t chunkCount = (size_t)EnsureInt(stream);
                for(size_t j = 0; j < chunkCount; j++)
                {
                    
                    auto chunk = TFileChunk::Create(ls);
                    chunk->file = this;
                    size_t argCount = (size_t)EnsureInt(stream);
                    for(size_t k = 0; k < argCount; k++)
                    {
                        chunk->args.push_back(GetString(stream));
                    }
                    size_t len = (size_t)EnsureInt(stream);
                    chunk->code.resize(len);
                    Ensure(stream,chunk->code.data(),len);
                    //reader.ReadIntoBuffer(chunk->code);

                    this->chunks.push_back(chunk);
                }
                
            }
            else if(strncmp(table_name,"FUNS",4) == 0) //functions
            {
                size_t funLength = (size_t)EnsureInt(stream);

                for(size_t j = 0; j < funLength;j++)
                {
                    std::vector<std::string> fnParts;
                    uint32_t fnPartsC = EnsureInt(stream);
                    for(uint32_t k = 0; k < fnPartsC; k++)
                    {
                        fnParts.push_back(GetString(stream));
                    }

                    uint32_t fnNumber = EnsureInt(stream);
                    this->functions.push_back(std::pair<std::vector<std::string>,uint32_t>(fnParts,fnNumber));
       
                }
                
            }
            else if(strncmp(table_name,"STRS",4) == 0) //strings
            {
                size_t strsLen = (size_t)EnsureInt(stream);
                for(size_t j = 0;j < strsLen;j++)
                {
                    this->strings.push_back(EnsureString(stream));
                }
            }
            else if(strncmp(table_name,"ICON",4) == 0) //icon
            {
                this->icon = (int32_t)EnsureInt(stream);
            }
            else if(strncmp(table_name,"MACH",4) == 0) //machine
            {
                std::string name = GetString(stream);
                std::string howToGet = GetString(stream);
                this->vms.push_back(std::pair<std::string,std::string>(name,howToGet));
            }
            else if(strncmp(table_name,"CLSS",4) == 0) //classes
            {
                uint32_t clsCnt = EnsureInt(stream);
                for(uint32_t j = 0; j < clsCnt; j++)
                {
                    TClass cls;
                    cls.documentation= GetString(stream);
                    uint32_t name_cnt = EnsureInt(stream);
                    for(uint32_t k = 0; k < name_cnt; k++)
                    {
                        cls.name.push_back(GetString(stream));
                    }
                    name_cnt = EnsureInt(stream);
                    for(uint32_t k = 0; k < name_cnt; k++)
                    {
                        cls.inherits.push_back(GetString(stream));
                    }
                    name_cnt = EnsureInt(stream);
                    for(uint32_t k = 0; k < name_cnt; k++)
                    {
                        TClassEntry ent;
                        Ensure(stream,main_header,1);
                        uint8_t sig = main_header[0];
                        ent.isAbstract = (sig & 0b00001000) != 0;
                        ent.isFunction = (sig & 0b00000100) == 0;
                        ent.modifier = (TClassModifier)(sig & 3);
                        ent.documentation = GetString(stream);
                        ent.name = GetString(stream);
                        uint32_t arglen = EnsureInt(stream);
                        for(uint32_t l = 0; l < arglen; l++)
                            ent.args.push_back(GetString(stream));
                        ent.chunkId = EnsureInt(stream);
                        cls.entry.push_back(ent);
                    }
                    this->classes.push_back(cls);
                }
            }
            else if(strncmp(table_name,"META",4) == 0) //structured metadata
            {
                if(tableLen > 4)
                {
                    auto name = this->GetString(stream);
                    auto& data = this->metadata.emplace_back(name, std::vector<uint8_t>(tableLen-4));
                    Ensure(stream,data.second.data(), tableLen-4);
                }
                else throw VMException("meta tag is not valid");

            }
            else
            {
                auto& data = this->sections.emplace_back(std::string(table_name, 4), std::vector<uint8_t>(tableLen));
                Ensure(stream,data.second.data(), tableLen);
                
            }
        }
        
    }

    
}
