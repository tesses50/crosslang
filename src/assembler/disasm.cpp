#include "CrossLang.hpp"

namespace Tesses::CrossLang {
    class CrossLangFileReader {
        Tesses::Framework::Streams::Stream* strm;

        void Ensure(uint8_t* buffer, size_t len)
        {
            auto read = this->strm->ReadBlock(buffer, len);
            if(read < len) throw VMException("End of file, could not read " + std::to_string((int64_t)len) + " byte(s)., offset=" + std::to_string(strm->GetLength()));
        }

        uint32_t ReadInt()
        {
            uint8_t buffer[4];
            Ensure(buffer,4);
            return BitConverter::ToUint32BE(buffer[0]);
        }
        std::string ReadString()
        {
            auto len = ReadInt();
            if(len == 0) return {};
            std::string str={};
            str.resize((size_t)len);
            Ensure((uint8_t*)str.data(),str.size());
            return str;
        }

        std::string GetString()
        {
            uint32_t index=ReadInt();
            if(index >= this->strings.size()) throw VMException("String does not exist in TCrossVM file, expected string index: " + std::to_string(index) + ", total strings: " + std::to_string(this->strings.size()));
            return this->strings[index];
        }

        public:
        CrossLangFileReader(Tesses::Framework::Streams::Stream* strm)
        {
            this->strm = strm;

            
        
        uint8_t main_header[18];
        Ensure(main_header,sizeof(main_header));
        if(strncmp((const char*)main_header,"TCROSSVM",8) != 0) throw VMException("Invalid TCrossVM image.");
        TVMVersion version(main_header+8);
        if(version.CompareToRuntime() == 1)
        {
            throw VMException("Runtime is too old.");
        }
        TVMVersion v2(main_header+13);
        this->version = v2;

        size_t _len = (size_t)ReadInt();

        char table_name[4];

        for(size_t i = 0;i < _len; i++)
        {
            Ensure((uint8_t*)table_name,sizeof(table_name));
            size_t tableLen = (size_t)ReadInt();
            if(strncmp(table_name,"NAME",4) == 0) 
            {
                this->name = GetString();
            }
            else if(strncmp(table_name,"INFO",4) == 0) 
            {
                this->info = GetString();
            }
            else if(strncmp(table_name,"DEPS",4) == 0) //dependencies
            {
                std::string name = GetString();
                uint8_t version_bytes[5];
                Ensure(version_bytes,sizeof(version_bytes));
                TVMVersion depVersion(version_bytes);
                this->dependencies.push_back(std::pair<std::string,TVMVersion>(name, depVersion));
            }
            else if(strncmp(table_name,"TOOL",4) == 0) //compile tools (for package manager)
            {
                std::string name = GetString();
                uint8_t version_bytes[5];
                Ensure(version_bytes,sizeof(version_bytes));
                TVMVersion depVersion(version_bytes);
                this->tools.push_back(std::pair<std::string,TVMVersion>(name, depVersion));
            }
            else if(strncmp(table_name,"RESO",4) == 0) //resources (using embed)
            {
                std::vector<uint8_t> data;
                data.resize(tableLen);
                Ensure(data.data(), tableLen);
                this->resources.push_back(data);
            }
            else if(strncmp(table_name,"CHKS",4) == 0) //chunks
            {
                size_t chunkCount = (size_t)ReadInt();
                for(size_t j = 0; j < chunkCount; j++)
                {
                    std::vector<std::string> args;
                    
                   
                    size_t argCount = (size_t)ReadInt();
                    for(size_t k = 0; k < argCount; k++)
                    {
                        args.push_back(GetString());
                    }
                    std::vector<uint8_t> code;
                    size_t len = (size_t)ReadInt();
                    code.resize(len);
                    Ensure(code.data(),len);
                    //reader.ReadIntoBuffer(chunk->code);

                    this->chunks.emplace(this->chunks.end(),args,code);
                }
                
            }
            else if(strncmp(table_name,"FUNS",4) == 0) //functions
            {
                size_t funLength = (size_t)ReadInt();

                for(size_t j = 0; j < funLength;j++)
                {
                    std::vector<std::string> fnParts;
                    uint32_t fnPartsC = ReadInt();
                    for(uint32_t k = 0; k < fnPartsC; k++)
                    {
                        fnParts.push_back(GetString());
                    }

                    uint32_t fnNumber = ReadInt();
                    this->functions.push_back(std::pair<std::vector<std::string>,uint32_t>(fnParts,fnNumber));
       
                }
                
            }
            else if(strncmp(table_name,"STRS",4) == 0) //strings
            {
                size_t strsLen = (size_t)ReadInt();
                for(size_t j = 0;j < strsLen;j++)
                {
                    this->strings.push_back(ReadString());
                }
            }
            else if(strncmp(table_name,"ICON",4) == 0) //icon
            {
                this->icon = (int32_t)ReadInt();
            }
            else if(strncmp(table_name,"MACH",4) == 0) //machine
            {
                std::string name = GetString();
                std::string howToGet = GetString();
                this->vms.push_back(std::pair<std::string,std::string>(name,howToGet));
            }
            else if(strncmp(table_name,"CLSS",4) == 0) //classes
            {
                uint32_t clsCnt = ReadInt();
                for(uint32_t j = 0; j < clsCnt; j++)
                {
                    TClass cls;
                    cls.documentation= GetString();
                    uint32_t name_cnt = ReadInt();
                    for(uint32_t k = 0; k < name_cnt; k++)
                    {
                        cls.name.push_back(GetString());
                    }
                    name_cnt = ReadInt();
                    for(uint32_t k = 0; k < name_cnt; k++)
                    {
                        cls.inherits.push_back(GetString());
                    }
                    name_cnt = ReadInt();
                    for(uint32_t k = 0; k < name_cnt; k++)
                    {
                        TClassEntry ent;
                        Ensure(main_header,1);
                        uint8_t sig = main_header[0];
                        ent.isAbstract = (sig & 0b00001000) != 0;
                        ent.isFunction = (sig & 0b00000100) == 0;
                        ent.modifier = (TClassModifier)(sig & 3);
                        ent.documentation = GetString();
                        ent.name = GetString();
                        uint32_t arglen = ReadInt();
                        for(uint32_t l = 0; l < arglen; l++)
                            ent.args.push_back(GetString());
                        ent.chunkId = ReadInt();
                        cls.entry.push_back(ent);
                    }
                    this->classes.push_back(cls);
                }
            }
            else
            {
                std::vector<uint8_t> data;
                data.resize(tableLen);
                Ensure(data.data(), tableLen);
                std::string key(std::string(table_name), 4);
                this->sections.push_back(std::pair<std::string,std::vector<uint8_t>>(key,data));
            }
        }
        }
        std::vector<std::string> strings;
        std::vector<std::pair<std::vector<std::string>,std::vector<uint8_t>>> chunks;
        std::vector<std::pair<std::string,std::string>> vms;
        std::vector<std::pair<std::vector<std::string>, uint32_t>> functions;
        std::vector<std::pair<std::string,TVMVersion>> dependencies;
        std::vector<std::pair<std::string,TVMVersion>> tools;
        std::vector<std::pair<std::string,std::vector<uint8_t>>> sections;
        std::vector<std::vector<uint8_t>> resources;
        std::vector<TClass> classes;
        std::string name;
        TVMVersion version;
        std::string info;
        int32_t icon;
        
        std::string Chunk2String(size_t chunkId,size_t tab)
        {
            if(chunkId >= this->chunks.size()) return {};
            auto& code = this->chunks[chunkId].second;

            std::string buffer = {};
            std::vector<uint32_t> labels;
            size_t i = 0;
            while(i < code.size())
            {
                switch(code[i++])
                {
                    case PUSHRESOURCE:
                    case PUSHSTRING:
                    case SCOPEENDTIMES:
                    case PUSHCLOSURE:
                    case PUSHSCOPELESSCLOSURE:
                        i+=4;
                        break;
                    case PUSHLONG:
                    case PUSHDOUBLE:
                        i+=8;
                        break;
                    case PUSHCHAR:
                        i+=1;
                        break;
                    case JMP:
                    case JMPC:
                    case JMPIFBREAK:
                    case JMPIFCONTINUE:
                    case JMPIFDEFINED:
                    case JMPUNDEFINED:
                    {   
                        bool already = false;
                        for(auto item : labels)
                        {
                            if(item == i) {
                                already=true;
                                break;
                            }
                        }
                        if(!already)
                        labels.push_back(BitConverter::ToUint32BE(code[i]));
                        i+=4;
                    }
                        break;
                    
                    default:
                        break;
                }
            }
            i=0;

            while(i < code.size())
            {
                for(size_t j = 0; j < labels.size(); j++)
                {
                    if(labels[j] == i)
                    {
                        buffer.append(tab,'\t');
                        buffer.append("lbl l" + std::to_string((uint32_t)j) + "\n");
                        break;
                    }
                }

                buffer.append(tab,'\t');
                

                switch(code[i++])
                {
                    case ADD:
                        buffer.append("add");
                        break;
                    case SUB:
                        buffer.append("sub");
                        break;
                    case TIMES:
                        buffer.append("mul");
                        break;
                    case DIVIDE:
                        buffer.append("div");
                        break;
                    case MODULO:
                        buffer.append("mod");
                        break;
                    case LEFTSHIFT:
                        buffer.append("lsh");
                        break;
                    case RIGHTSHIFT:
                        buffer.append("rsh");
                        break;
                    case BITWISEOR:
                        buffer.append("bor");
                        break;
                    case BITWISEAND:
                        buffer.append("band");
                        break;
                    case BITWISENOT:
                        buffer.append("bnot");
                        break;
                    case LESSTHAN:
                        buffer.append("lt");
                        break;
                    case GREATERTHAN:
                        buffer.append("gt");
                        break;
                    case LESSTHANEQ:
                        buffer.append("lte");
                        break;
                    case GREATERTHANEQ:
                        buffer.append("gte");
                        break;
                    case EQ:
                        buffer.append("eq");
                        break;
                    case NEQ:
                        buffer.append("neq");
                        break;
                    case NOT:
                        buffer.append("not");
                        break;
                    case NEGATIVE:
                        buffer.append("neg");
                        break;
                    case XOR:
                        buffer.append("xor");
                        break;
                    case POP:
                        buffer.append("pop");
                        break;
                    case DUP:
                        buffer.append("dup");
                        break;
                    case PUSHCLOSURE:
                    {
                        uint32_t clId = (uint32_t)code[i++] << 24;
                        clId |= (uint32_t)code[i++] << 16;
                        clId |= (uint32_t)code[i++] << 8;
                        clId |= (uint32_t)code[i++];
                        buffer.append("pushclosure (");
                        bool first=true;

                        for(auto item : this->chunks.at(clId).first)
                        {
                            if(!first)
                                buffer.append(", ");
                            buffer.append(item);
                            first=false;
                        }

                        

                        buffer.append(") {\n");
                        buffer.append(Chunk2String((size_t)clId,tab+1));
                        buffer.append(tab,'\t');
                        buffer.append("}");
                    }
                    break;
                    case CREATEDICTIONARY:
                        buffer.append("createdict");
                        break;
                    case CREATEARRAY:
                        buffer.append("createarray");
                        break;
                    case APPENDLIST:
                        buffer.append("appendlist");
                        break;
                    case APPENDDICT:
                        buffer.append("appenddict");
                        break;
                    case PUSHRESOURCE:
                    {
                        uint32_t clId = (uint32_t)code[i++] << 24;
                        clId |= (uint32_t)code[i++] << 16;
                        clId |= (uint32_t)code[i++] << 8;
                        clId |= (uint32_t)code[i++];
                        buffer.append("embed ");
                        buffer.append(EscapeString(name + "-" + version.ToString()+"_"+ std::to_string(clId) + ".bin",true));
                    }
                        break;
                    case PUSHLONG:
                    {
                        uint64_t number = (uint64_t)code[i++] << 56;
                        number |= (uint64_t)code[i++] << 48;
                        number |= (uint64_t)code[i++] << 40;
                        number |= (uint64_t)code[i++] << 32;
                        number |= (uint64_t)code[i++] << 24;
                        number |= (uint64_t)code[i++] << 16;
                        number |= (uint64_t)code[i++] << 8;
                        number |= (uint64_t)code[i++];
                        buffer.append("push " + std::to_string(number));

                    }
                        break;
                    case PUSHCHAR:
                    {
                        buffer.append("push '" + EscapeString(std::string({(char)code[i++]}),false) + "'");
                    }
                        break;
                    case PUSHDOUBLE:
                    {
                        
                        auto res = BitConverter::ToDoubleBE(code[i]);
                        i+=8;
                        buffer.append("push " + std::to_string(res));
                    }
                        break;
                    case PUSHSTRING:
                    {
                        auto res = BitConverter::ToUint32BE(code[i]);
                        i+=4;
                        buffer.append("push " + EscapeString(strings[res],true));
                    }
                        break;
                    case PUSHNULL:
                        buffer.append("push null");
                        break;
                    case PUSHUNDEFINED:
                        buffer.append("push undefined");
                        break;
                    case SCOPEBEGIN:
                        buffer.append("scopebegin");
                        break;
                    case SCOPEEND:
                        buffer.append("scopeend");
                        break;
                    case SCOPEENDTIMES:
                    {
                        auto res = BitConverter::ToUint32BE(code[i]);
                        i+=4;
                        buffer.append("scopeendtimes " + std::to_string(res));
                    }
                        break;
                    case PUSHFALSE:
                        buffer.append("push false");
                        break;
                    case PUSHTRUE:
                        buffer.append("push true");
                        break;
                    case SETVARIABLE:
                        buffer.append("setvariable");
                        break;
                    case GETVARIABLE:
                        buffer.append("getvariable");
                        break;
                    case DECLAREVARIABLE:
                        buffer.append("declarevariable");
                        break;
                    case SETFIELD:
                        buffer.append("setfield");
                        break;
                    case GETFIELD:
                        buffer.append("getfield");
                        break;
                    case CALLFUNCTION:
                        buffer.append("callfunction");
                        break;
                    case CALLMETHOD:
                        buffer.append("callmethod");
                        break;
                    case RET:
                        buffer.append("ret");
                        break;
                    case JMPC:
                    {
                        auto res = BitConverter::ToUint32BE(code[i]);
                        i+=4;
                        buffer.append("jmpc l");
                        for(size_t j = 0; j < labels.size(); j++)
                        {
                            if(labels[j] == res)
                            {
                                buffer.append(std::to_string((uint32_t)j));
                                break;
                            }
                        }
                        
                    }
                        break;
                    case JMP:
                    {
                        auto res = BitConverter::ToUint32BE(code[i]);
                        i+=4;
                        buffer.append("jmp l");
                        for(size_t j = 0; j < labels.size(); j++)
                        {
                            if(labels[j] == res)
                            {
                                buffer.append(std::to_string((uint32_t)j));
                                break;
                            }
                        }
                        
                    }
                        break;
                    case JMPUNDEFINED:
                    {
                        auto res = BitConverter::ToUint32BE(code[i]);
                        i+=4;
                        buffer.append("jmpundefined l");
                        for(size_t j = 0; j < labels.size(); j++)
                        {
                            if(labels[j] == res)
                            {
                                buffer.append(std::to_string((uint32_t)j));
                                break;
                            }
                        }
                        
                    }
                        break;  
                    case DEFER:
                        buffer.append("defer");
                        break;
                    case TRYCATCH:
                        buffer.append("trycatch");
                        break;
                    case THROW:
                        buffer.append("throw");
                        break;
                    case PUSHSCOPELESSCLOSURE:
                    {
                        uint32_t clId = (uint32_t)code[i++] << 24;
                        clId |= (uint32_t)code[i++] << 16;
                        clId |= (uint32_t)code[i++] << 8;
                        clId |= (uint32_t)code[i++];
                        buffer.append("pushscopelessclosure (");
                        bool first=true;

                        for(auto item : this->chunks.at(clId).first)
                        {
                            if(!first)
                                buffer.append(", ");
                            buffer.append(item);
                            first=false;
                        }

                        

                        buffer.append(") {\n");
                        buffer.append(Chunk2String((size_t)clId,tab+1));
                        buffer.append(tab,'\t');
                        buffer.append("}");
                    }
                    break;
                    case YIELD:
                        buffer.append("yield");
                        break;
                    case PUSHROOTPATH:
                        buffer.append("push /");
                        break;
                    case PUSHRELATIVEPATH:
                        buffer.append("push .");
                        break;
                    case BREAKPOINT:
                        buffer.append("breakpoint");
                        break;
                    case PUSHBREAK:
                        buffer.append("push break");
                        break;
                    case PUSHCONTINUE:
                        buffer.append("push continue");
                        break;
                    case JMPIFBREAK:
                    {
                        auto res = BitConverter::ToUint32BE(code[i]);
                        i+=4;
                        buffer.append("jmpifbreak l");
                        for(size_t j = 0; j < labels.size(); j++)
                        {
                            if(labels[j] == res)
                            {
                                buffer.append(std::to_string((uint32_t)j));
                                break;
                            }
                        }
                        
                    }
                        break;
                    case JMPIFCONTINUE:
                    {
                        auto res = BitConverter::ToUint32BE(code[i]);
                        i+=4;
                        buffer.append("jmpifcontinue l");
                        for(size_t j = 0; j < labels.size(); j++)
                        {
                            if(labels[j] == res)
                            {
                                buffer.append(std::to_string((uint32_t)j));
                                break;
                            }
                        }
                        
                    }
                        break;
                    case JMPIFDEFINED:
                    {
                        auto res = BitConverter::ToUint32BE(code[i]);
                        i+=4;
                        buffer.append("jmpifdefined l");
                        for(size_t j = 0; j < labels.size(); j++)
                        {
                            if(labels[j] == res)
                            {
                                buffer.append(std::to_string((uint32_t)j));
                                break;
                            }
                        }
                        
                    }
                        break;
                    default:
                        printf("ILL: %i\n", (int)code[i-1]);
                        break;
                }

                buffer.push_back('\n');
            }

            if(code.empty()) buffer.push_back('\n');
            
            return buffer;
        }
    };

    


    void Disassemble(Tesses::Framework::Streams::Stream* src,Tesses::Framework::Filesystem::VFS* vfs, bool generateJSON,bool extractResources)
    {
        using namespace Tesses::Framework::Filesystem;
        CrossLangFileReader file(src);
        if(extractResources)
        {
            std::string resdir = "res";
            VFSPath path=resdir;
            vfs->CreateDirectory(path);
            for(size_t i = 0; i < file.resources.size(); i++)
            {
                auto path2 = path / file.name + "-" + file.version.ToString()+"_"+ std::to_string((uint32_t)i) + ".bin";
                auto strm = vfs->OpenFile(path2,"wb");
                strm->WriteBlock(file.resources[i].data(),file.resources[i].size());
                delete strm;
            }

            std::string secdir = "sections";
            VFSPath secpath=secdir;
            vfs->CreateDirectory(secpath);

            for(size_t i = 0; i < file.sections.size(); i++)
            {
                auto path2 = secpath / file.name + "-" + file.version.ToString()+"_"+ std::to_string((uint32_t)i) + ".tsec";
               
                if(file.sections[i].first.size() != 4) {
                   throw std::runtime_error("Chunk name is not 4 bytes");
                }
                auto strm = vfs->OpenFile(path2,"wb");
                strm->WriteBlock((const uint8_t*)file.sections[i].first.data(), 4);
                strm->WriteBlock(file.sections[i].second.data(),file.sections[i].second.size());
                delete strm;
            }
        }

        std::string srcFile = {};

        srcFile.append("root {\n");
        srcFile.append(file.Chunk2String(0,1));
        srcFile.append("}\n");

        for(auto& item : file.functions)
        {
            if(!item.first[0].empty())
            {
                srcFile.append("/^" + Tesses::Framework::Http::HttpUtils::Replace(item.first[0],"^","^^") +"^/\n");
            }
            srcFile.append("func ");
            for(size_t i = 1; i < item.first.size(); i++)
            {
                if(i > 1)
                {
                    srcFile.push_back('.');
                }
                srcFile.append(item.first[i]);
            }
            srcFile.append("(");
            auto& chunk = file.chunks.at(item.second);
            for(size_t i = 0; i < chunk.first.size(); i++)
            {
                if(i > 0)
                {
                    srcFile.append(", ");
                }
                srcFile.append(chunk.first[i]);
            }
            srcFile.append(") {\n");
            srcFile.append(file.Chunk2String((size_t)item.second,1));
            srcFile.append("}\n");
        }
        for(auto& cls : file.classes)
        {
            if(!cls.documentation.empty())
            {
                srcFile.append("/^" + Tesses::Framework::Http::HttpUtils::Replace(cls.documentation,"^","^^") +"^/\n");
            }
            srcFile.append("class ");
            for(size_t i = 0; i < cls.name.size(); i++)
            {
                if(i > 0)
                {
                    srcFile.push_back('.');
                }
                srcFile.append(cls.name[i]);
            }   

            if(!(cls.inherits.size() == 1 && cls.inherits[0] == "ClassObject"))
            {
                srcFile.append(" : ");
                for(size_t i = 0; i < cls.inherits.size(); i++)
                {
                    if(i > 0)
                    {
                        srcFile.push_back('.');
                    }
                    srcFile.append(cls.inherits[i]);
                }   
            }
            srcFile.append(" {\n");
            for(auto& item : cls.entry)
            {
                if(!item.documentation.empty())
                    srcFile.append("\t/^"+Tesses::Framework::Http::HttpUtils::Replace(item.documentation,"^","^^")+"^/\n");

                switch(item.modifier)
                {
                    case TClassModifier::Private:
                        srcFile.append("\tprivate ");
                        break;
                    case TClassModifier::Protected:
                        srcFile.append("\tprotected ");
                        break;
                    case TClassModifier::Public:
                        srcFile.append("\tpublic ");
                        break;
                    case TClassModifier::Static:
                        srcFile.append("\tstatic ");
                    default:
                        break;
                }
                
                if(item.isFunction)
                {
                    
                    if(item.isAbstract)
                    {
                        srcFile.append("abstract " + item.name);
                        srcFile.append("(");
                        for(size_t i = 0; i < item.args.size(); i++)
                        {
                            if(i > 0)
                            {
                                srcFile.append(", ");
                            }
                            srcFile.append(item.args[i]);
                        }           
                        srcFile.append(");\n");
                    }
                    else {
                        srcFile.append(item.name);
                        srcFile.append("(");
                        for(size_t i = 0; i < item.args.size(); i++)
                        {
                            if(i > 0)
                            {
                                srcFile.append(", ");
                            }
                            srcFile.append(item.args[i]);
                        }           
                        srcFile.append(") {\n");
                        srcFile.append(file.Chunk2String(item.chunkId,2));
                        srcFile.append("\t}\n");
                    }
                }
                else {
                    srcFile.append(item.name);
                    if(item.isAbstract)
                    {

                        srcFile.append(";\n");
                    }
                    else {
                        srcFile.append(" {\n");
                        srcFile.append(file.Chunk2String(item.chunkId,2));
                        srcFile.append("\t}\n");
                    }
                }
            }
            srcFile.append("}\n");
        }
        std::string srcdirs = "src";
        VFSPath srcdir=srcdirs;
        vfs->CreateDirectory(srcdir);
        Tesses::Framework::TextStreams::StreamWriter writer(vfs->OpenFile(srcdir / file.name + "-" + file.version.ToString() + ".tcasm","wb"),true);
        writer.Write(srcFile);

        if(generateJSON)
        {
            using namespace Tesses::Framework::Serialization::Json;

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
            if(!file.dependencies.empty())
            {
                JArray array;
                for(auto& item : file.dependencies)
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
                json_data.SetValue("dependencies", array);
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


            Tesses::Framework::TextStreams::StreamWriter json_writer(vfs->OpenFile(VFSPath() / "crossapp.json","wb" ),true); 
            json_writer.WriteLine(Json::Encode(json_data,true));

        }
    }
}