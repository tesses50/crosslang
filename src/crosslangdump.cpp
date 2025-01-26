#include "CrossLang.hpp"
#include <iostream>
using namespace Tesses::CrossLang;
void Ensure(Tesses::Framework::Streams::Stream& strm,uint8_t* buffer, size_t len)
{
    if(strm.ReadBlock(buffer,len) != len)
    {
        throw VMException("Could not read " + std::to_string(len) + " byte(s).");
    }
}
uint32_t EnsureInt(Tesses::Framework::Streams::Stream& strm)
{
    uint8_t buff[4];
    Ensure(strm,buff,sizeof(buff));
    return BitConverter::ToUint32BE(buff[0]);
}
std::string EnsureString(Tesses::Framework::Streams::Stream& strm)
{
    size_t len = (size_t)EnsureInt(strm);
    std::string myStr={};
    myStr.resize(len);
    Ensure(strm,(uint8_t*)myStr.data(), len);
    return myStr;
}
void DumpFile(std::filesystem::path p)
{
    if(std::filesystem::is_regular_file(p))
    {
        try
        {
            std::cout << "File: " << p.string() << std::endl;
            Tesses::Framework::Streams::FileStream strm(p,"rb");
            uint8_t main_header[18];
            Ensure(strm,main_header,sizeof(main_header));
            if(strncmp((const char*)main_header,"TCROSSVM",8) != 0) throw VMException("Invalid TCrossVM image.");
            TVMVersion version(main_header+8);
            if(version.CompareToRuntime() == 1)
            {
                throw VMException("Runtime is too old.");
            }
            TVMVersion v2(main_header+13);
            std::cout << "Version: " << v2.ToString() << std::endl;

            size_t _len = (size_t)EnsureInt(strm);

            std::cout << "SectionCount: " << _len << std::endl;

            std::vector<std::string> strs;

            std::unordered_map<uint32_t, std::vector<std::string>> funs;
            std::vector<std::vector<std::string>> closures;

            

            char table_name[4];

            for(size_t i = 0; i < _len; i++)
            {
                Ensure(strm,(uint8_t*)table_name,sizeof(table_name));
                size_t tableLen = (size_t)EnsureInt(strm);
                std::string tableName(table_name,4);
                if(tableName == "STRS")
                {
                    size_t strsLen = (size_t)EnsureInt(strm);
                    for(size_t j = 0;j < strsLen;j++)
                    {
                        strs.push_back(EnsureString(strm));
                    }
                }
                else if(tableName == "DEPS")
                {

                    std::string name = strs.at((size_t)EnsureInt(strm));
                    
                    uint8_t version_bytes[5];
                    Ensure(strm,version_bytes,sizeof(version_bytes));
                    TVMVersion depVersion(version_bytes);
                    std::cout << "Dependency: " << name << "-" << depVersion.ToString() << std::endl;
                }
                else if(tableName == "NAME")
                {
                    std::cout << "Name: " << strs.at((size_t)EnsureInt(strm)) << std::endl;
                }
                else if(tableName == "CHKS")
                {
                    size_t chunkCount = (size_t)EnsureInt(strm);
                    
                    for(size_t j = 0; j < chunkCount; j++)
                    {
                        std::vector<std::string> args;
                        size_t argCount = (size_t)EnsureInt(strm);
                        for(size_t k = 0; k < argCount; k++)
                        {
                            args.push_back(strs.at(EnsureInt(strm)));
                        }
                        auto len = EnsureInt(strm);

                        strm.Seek(len,Tesses::Framework::Streams::SeekOrigin::Current);

                        closures.push_back(args);
                    }
                }
                else if(tableName == "FUNS")
                {
                    size_t funLength = (size_t)EnsureInt(strm);

                for(size_t j = 0; j < funLength;j++)
                {
                    std::vector<std::string> fnParts;
                    uint32_t fnPartsC = EnsureInt(strm);
                    for(uint32_t k = 0; k < fnPartsC; k++)
                    {
                        fnParts.push_back(strs.at(EnsureInt(strm)));
                    }

                    uint32_t fnNumber = EnsureInt(strm);
                    funs[fnNumber] = fnParts;
       
                    }   
                }
                else if(tableName == "INFO")
                {

                    std::cout << "Info: " << strs.at((size_t)EnsureInt(strm)) << std::endl;
                    
                }
                else 
                {
                    strm.Seek((int64_t)tableLen,Tesses::Framework::Streams::SeekOrigin::Current);
                }
            }

            for(size_t i = 1; i < closures.size(); i++)
            {
                if(funs.count((uint32_t)i) > 0)
                {
                    std::cout << "Func: ";
                    auto res = funs[(uint32_t)i];
                    if(!res.empty()) {
                        std::cout << "/^" << res[0] << "^/ ";
                    }
                    for(size_t i = 1; i < res.size(); i++)
                    {
                        if(i > 1) std::cout << ".";
                        std::cout << res[i];
                    }
                }
                else
                {
                    std::cout << "Closure: ";
                }
                std::cout << "(";      
                bool first=true;
                for(auto arg : closures[i])
                {
                    if(!first) std::cout << ", ";
                    std::cout << arg;

                    if(first) first=false;
                }
                std::cout << ")" << std::endl;
            }
            std::cout << std::endl;
            std::cout << "String Table:" << std::endl;

            for(auto str : strs) {
                std::cout << EscapeString(str, true) << std::endl;
            }
        } 
        catch(std::exception& ex)
        {
            std::cout << "Error when reading file \"" << p.string() << "\" " << ex.what() << std::endl;
        }
    }
    else
    {
        std::cout << "CrossVM file \"" << p.string() << "\" does not exist." << std::endl;
    }
}
int main(int argc, char** argv)
{
    for(int i = 1; i < argc; i++)
    {
        DumpFile(argv[i]);
    }
}
