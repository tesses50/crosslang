#pragma once
#include <cstddef>
#include <initializer_list>
#include <vector>
#include <cstdint>
#include <string>
#include <cstdint>
#include <string>
#include <functional>
#include <variant>
#include <cstdio>
#include <atomic>
#include <filesystem>
#include <memory>
#include <cstring>
#include <TessesFramework/TessesFramework.hpp>
#include <regex>
#include <time.h>
#include <any>

/**
 * @brief CrossLang Runtime Major version
 * 
 */
#define TVM_MAJOR 1
/**
 * @brief CrossLang Runtime Minor version
 * 
 */
#define TVM_MINOR 0
/**
 * @brief CrossLang Runtime Patch version
 * 
 */
#define TVM_PATCH 0
/**
 * @brief CrossLang Runtime Build version
 * 
 */
#define TVM_BUILD 0
/**
 * @brief CrossLang Runtime Stage version
 * 
 */
#define TVM_VERSIONSTAGE DevVersion

#if defined(_WIN32)
#undef Yield
#endif
/**
 * @namespace Tesses::CrossLang
 * @brief The CrossLang namespace
 * 
 */
namespace Tesses::CrossLang {
    constexpr std::string_view VMName = "CrossLangVM";
    constexpr std::string_view VMHowToGet = "https://crosslang.tesseslanguage.com/";
    /**
     * @brief Escape a crosslang string (for generating source code)
     * 
     * @param text text to escape
     * @param quote whether to surround with double quotes
     * @return std::string the escaped string
     */
    std::string EscapeString(std::string text,bool quote);

    /**
     * @brief Get the config folder used by crosslang
     * 
     * @return Tesses::Framework::Filesystem::VFSPath the config folder for crosslang
     */
    Tesses::Framework::Filesystem::VFSPath GetCrossLangConfigDir();



    /**
     * @brief Version stage
     * 
     */
    enum TVMVersionStage : uint8_t
    {
        DevVersion=0,
        AlphaVersion=1,
        BetaVersion=2,
        ProductionVersion=3
    };
    /**
     * @brief CrossLang version
     * 
     */
    class TVMVersion
    {
        uint8_t major;
        uint8_t minor;
        uint8_t patch;
        uint16_t build;
        public:
            /**
             * @brief Major
             * 
             * @return uint8_t The major
             */
            uint8_t Major()
            {
                return major;
            }
            /**
             * @brief Minor
             * 
             * @return uint8_t The minor
             */
            uint8_t Minor()
            {
                return minor;
            }
            /**
             * @brief Patch
             * 
             * @return uint8_t The patch
             */
            uint8_t Patch()
            {
                return patch;
            }
            /**
             * @brief Build
             * 
             * @return uint16_t The build
             */
            uint16_t Build()
            {
                return build >> 2;
            }
            /**
             * @brief Stage (dev, alpha, beta or prod)
             * 
             * @return TVMVersionStage The stage
             */
            TVMVersionStage VersionStage()
            {
                return (TVMVersionStage)(build & 3);
            }
            /**
             * @brief Set the Major
             * 
             * @param major The major
             */
            void SetMajor(uint8_t major)
            {
                this->major=major;
            }
             /**
             * @brief Set the Minor
             * 
             * @param minor The minor
             */
            void SetMinor(uint8_t minor)
            {
                this->minor=minor;
            }
             /**
             * @brief Set the Patch
             * 
             * @param patch The patch
             */
            void SetPatch(uint8_t patch)
            {
                this->patch=patch;
            }
             /**
             * @brief Set the Build
             * 
             * @param build The build << 2 | (stage & 3)
             */
            void SetBuild(uint16_t build)
            {
                this->build = build;
            }
            /**
             * @brief Set the Build (and stage)
             * 
             * @param build 
             * @param version 
             */
            void SetBuild(uint16_t build,TVMVersionStage version)
            {
                this->build = (build << 2) | version;
            }
            /**
             * @brief Set the Version
             * 
             * @param major The major
             * @param minor The minor
             * @param patch The patch
             * @param build The build
             * @param ver The stage
             */
            void SetVersion(uint8_t major, uint8_t minor, uint8_t patch, uint16_t build, TVMVersionStage ver)
            {
                this->SetMajor(major);
                this->SetMinor(minor);
                this->SetPatch(patch);
                this->SetBuild(build,ver);
            }
            /**
             * @brief Set the Version
             * 
             * @param major The major
             * @param minor The minor
             * @param patch The patch
             * @param build The build << 2 | (stage & 3)
             */
            void SetVersion(uint8_t major, uint8_t minor, uint8_t patch, uint16_t build)
            {
                this->SetMajor(major);
                this->SetMinor(minor);
                this->SetPatch(patch);
                this->SetBuild(build);
            }
            /**
             * @brief Set the Version
             * 
             * @param major The major
             * @param minor The minor
             * @param patch The patch
             * @param buildHigh The build (high 8 bits)
             * @param buildLow The build (low 6 bits) << 2 | (stage & 3)
             */
            void SetVersion(uint8_t major, uint8_t minor, uint8_t patch, uint8_t buildHigh, uint8_t buildLow)
            {
                this->SetVersion(major, minor, patch, buildHigh << 8 | buildLow);
            }
            /**
             * @brief Set the Version
             * 
             * @param versionData same order as SetVersion(uint8_t major, uint8_t minor, uint8_t patch, uint8_t buildHigh, uint8_t buildLow) but is a pointer to uint8_t version[5]
             */
            void SetVersion(uint8_t* versionData)
            {
                this->SetVersion(versionData[0],versionData[1],versionData[2],versionData[3],versionData[4]);
            }
            /**
             * @brief Set the version from long
             * 
             * @param v version serialized as long
             */
            void SetFromLong(uint64_t v)
            {

                this->major = (uint8_t)((v >> 32) & 0xFF);
                this->minor = (uint8_t)(v >> 24) & 0xFF;
                this->patch = (uint8_t)(v >> 16) & 0xFF;
                this->build = (uint16_t)(v & 0xFFFF);
            }
            /**
             * @brief Encodes the version to bytes to be decoded by SetVersion(uint8_t*) or the constructor
             * 
             * @param versionData an array that is 5 bytes long
             */
            void ToArray(uint8_t* versionData)
            {
                versionData[0] = major;
                versionData[1] = minor;
                versionData[2] = patch;
                versionData[3] = build >> 8;
                versionData[4] = build & 0xFF;
            }
            /**
             * @brief Construct a new Version object
             * 
             * @param major Major
             * @param minor Minor
             * @param patch Patch
             * @param build Build
             * @param version Stage (dev, alpha, beta, prod) see TVMVersionStage for actual names 
             */
            TVMVersion(uint8_t major, uint8_t minor, uint8_t patch, uint16_t build, TVMVersionStage version)
            {
                this->SetVersion(major,minor,patch,build,version);
            }
            /**
             * @brief Construct a new Version object
             * 
             * @param versionData same order as SetVersion(uint8_t major, uint8_t minor, uint8_t patch, uint8_t buildHigh, uint8_t buildLow) but is a pointer to uint8_t version[5]
             */
            TVMVersion(uint8_t* versionData)
            {
                this->SetVersion(versionData);
            }
            /**
             * @brief Construct a new 1.0.0.0-dev Version object
             * 
             */
            TVMVersion()
            {
                this->major=1;
                this->minor=0;
                this->patch=0;
                this->SetBuild(0, TVMVersionStage::DevVersion);
            }
            /**
             * @brief Construct a new Version object from a long
             * 
             * @param v version serialized as long
             */
            TVMVersion(uint64_t v)
            {
                this->SetFromLong(v);
            }
            /**
             * @brief Compares to another version
             * 
             * @param version another version object
             * @return int returns 1 if this is newer than other version, 0 if same, -1 if this is older than other version
             */
            int CompareTo(TVMVersion& version)
            {
                if(this->major > version.major) return 1;
                if(this->major < version.major) return -1;
                if(this->minor > version.minor) return 1;
                if(this->minor < version.minor) return -1;
                if(this->patch > version.patch) return 1;
                if(this->patch < version.patch) return -1;
                if(this->build > version.build) return 1;
                if(this->build < version.build) return -1;
                return 0;
            }
            /**
             * @brief Serializes to a long
             * 
             * @return uint64_t serialized as a long
             */
            uint64_t AsLong()
            {
                uint64_t v = (uint64_t)major << 32;
                v |= (uint64_t)minor << 24;
                v |= (uint64_t)patch << 16;
                v |= (uint64_t)build;
                return v;
            }
            /**
             * @brief Compares this to the runtime version
             * 
             * @return int CompareTo(RuntimeVersion) where RuntimeVersion is the runtime version
             */
            int CompareToRuntime()
            {
                TVMVersion version(TVM_MAJOR,TVM_MINOR,TVM_PATCH,TVM_BUILD,TVM_VERSIONSTAGE);
                return CompareTo(version);
            }
            /**
             * @brief Try to parse the version from a string
             * 
             * @param versionStr the version string like 1.0.0.0-prod (or dev, alpha, beta)
             * @param version a reference to a version
             * @return true the parsing succeeded
             * @return false the parsing failed
             */
            static bool TryParse(std::string versionStr, TVMVersion& version)
            {
                if(versionStr.empty()) return false;
                size_t sep=versionStr.find_last_of('-');

                std::string left = versionStr;
                std::string right = "prod";

                if(sep != std::string::npos)
                {
                    left = versionStr.substr(0, sep);
                    right = versionStr.substr(sep+1);
                }
                
                if(left.empty()) return false;

                

                TVMVersionStage stage;
                if(right == "dev")
                    stage = TVMVersionStage::DevVersion;
                else if(right == "alpha")
                    stage = TVMVersionStage::AlphaVersion;
                else if(right == "beta")
                    stage = TVMVersionStage::BetaVersion;
                else if(right == "prod")
                    stage = TVMVersionStage::ProductionVersion;
                else
                    return false;
                

                //1 0.0.0
                sep=left.find_first_of('.');

                if(sep != std::string::npos)
                {
                    auto lStr = left.substr(0, sep);
                    
                    right = left.substr(sep+1);
                    left = lStr;
                }
                else
                {
                    version.SetMajor((uint8_t)std::stoul(left));
                    version.SetBuild(0,stage);
                    return true;
                }
                version.SetMajor((uint8_t)std::stoul(left));
                left = right;
                sep=left.find_first_of('.');

                if(sep != std::string::npos)
                {
                    auto lStr = left.substr(0, sep);
                    
                    right = left.substr(sep+1);
                    left = lStr;
                }
                else
                {
                    version.SetMinor((uint8_t)std::stoul(left));
                    version.SetBuild(0,stage);
                    return true;
                }
                version.SetMinor((uint8_t)std::stoul(left));
                left = right;
                 sep=left.find_first_of('.');

                if(sep != std::string::npos)
                {
                    auto lStr = left.substr(0, sep);
                    
                    right = left.substr(sep+1);
                    left = lStr;
                }
                else
                {
                    version.SetPatch((uint8_t)std::stoul(left));
                    version.SetBuild(0,stage);
                    return true;
                }
                version.SetPatch((uint8_t)std::stoul(left));

                version.SetBuild((uint16_t)std::stoul(right),stage);
                return true;

            }
            /**
             * @brief Converts version to string
             * 
             * @return std::string the version string like 1.0.0.0-prod (or dev, alpha, beta)
             */
            std::string ToString()
            {
                std::string str={};
                str.append(std::to_string((int)this->Major()));
                str.push_back('.');
                str.append(std::to_string((int)this->Minor()));
                str.push_back('.');
                str.append(std::to_string((int)this->Patch()));
                str.push_back('.');
                str.append(std::to_string((int)this->Build()));
                if(this->VersionStage() == TVMVersionStage::DevVersion)
                {
                    str.append("-dev");
                }
                else if(this->VersionStage() == TVMVersionStage::AlphaVersion)
                {
                    str.append("-alpha");
                }
                else if(this->VersionStage() == TVMVersionStage::BetaVersion)
                {
                    str.append("-beta");
                }
                else if(this->VersionStage() == TVMVersionStage::ProductionVersion)
                {
                    str.append("-prod");
                }
                return str;
            }
    };

    /**
     * @brief Create a crvm archive file
     * 
     * @param vfs the filesystem used as root of the archive
     * @param strm the output stream
     * @param name the crvm name
     * @param version the crvm version
     * @param info the crvm info
     * @param icon the crvm icon
     */
    void CrossArchiveCreate(Tesses::Framework::Filesystem::VFS* vfs,Tesses::Framework::Streams::Stream* strm,std::string name,TVMVersion version,std::string info, std::string icon="");
    /**
     * @brief 
     * 
     * @param strm the input stream
     * @param vfs vfs to extract to (as root)
     * @return std::pair<std::pair<std::string,TVMVersion>,std::string> ((name, version),info)
     */
    std::pair<std::pair<std::string,TVMVersion>,std::string> CrossArchiveExtract(Tesses::Framework::Streams::Stream* strm,Tesses::Framework::Filesystem::VFS* vfs);



/**
 * @brief The token type for lexer
 * 
 */
typedef enum {
        Identifier,
        Symbol,
        String,
        Char,
        Documentation
} LexTokenType;
    /**
     * @brief The line info for lextoken
     * 
     */
    class LexTokenLineInfo
    {
        public:
            /**
             * @brief line number
             * 
             */
            int line;
            /**
             * @brief column number
             * 
             */
            int column;
            /**
             * @brief byte offset in file
             * 
             */
            int offset;
            /**
             * @brief filename
             * 
             */
            std::string filename;
            /**
             * @brief Add a char
             * 
             * @param c a char to add to line, column, offset
             */
            void Add(int c);
            /**
             * @brief Subtract from column and offset
             * 
             * @param c the number of chars
             */
            void Subtract(size_t c);
    };

    /**
     * @brief The lextoken for lexer
     * 
     */
    class LexToken {
        public:
            /**
             * @brief line information
             * 
             */
            LexTokenLineInfo lineInfo;
            /**
             * @brief the token type
             * 
             */
            LexTokenType type;
            /**
             * @brief the token text
             * 
             */
            std::string text;
            /**
             * @brief any white space characters before this token
             * 
             */
            std::string whiteSpaceCharsBefore;
    };
    /**
     * @brief Start lexing the file
     * 
     * @param filename the filename used for exception purposes
     * @param strm the character stream
     * @param tokens the tokens
     * @return int 0 on success, line number on failure
     */
    int Lex(std::string filename, std::istream& strm, std::vector<LexToken>& tokens);

/**
 * @brief Undefined type
 * 
 */
class Undefined
{

};
class AdvancedSyntaxNode;
/**
 * @brief Bytecode instruction enumeration
 * 
 */
typedef enum {
    ADD,
    SUB,
    TIMES,
    DIVIDE,
    MODULO,
    LEFTSHIFT,
    RIGHTSHIFT,
    BITWISEOR,
    BITWISEAND,
    BITWISENOT,
    LESSTHAN,
    GREATERTHAN,
    LESSTHANEQ,
    GREATERTHANEQ,
    EQ,
    NEQ,
    NOT,
    NEGATIVE,
    XOR,
    POP,
    DUP,
    NOP,
    PUSHCLOSURE,
    CREATEDICTIONARY,
    CREATEARRAY,
    APPENDLIST,
    APPENDDICT,
    PUSHRESOURCE,
    PUSHLONG,
    PUSHCHAR,
    PUSHDOUBLE,
    PUSHSTRING,
    PUSHNULL,
    PUSHUNDEFINED,
    SCOPEBEGIN,
    SCOPEEND,
    SCOPEENDTIMES,
    PUSHFALSE,
    PUSHTRUE,
    SETVARIABLE,
    GETVARIABLE,
    DECLAREVARIABLE,
    SETFIELD,
    GETFIELD,
    CALLFUNCTION,
    CALLMETHOD,
    RET,
    JMPC,
    JMP,
    JMPUNDEFINED,
    DEFER,
    TRYCATCH,
    THROW,
    PUSHSCOPELESSCLOSURE,
    YIELD,
    PUSHROOTPATH,
    PUSHRELATIVEPATH,
    BREAKPOINT,
    PUSHBREAK,
    PUSHCONTINUE,
    JMPIFBREAK,
    JMPIFCONTINUE,
    JMPIFDEFINED
} Instruction;
/**
 * @brief Base type for bytecode instruction
 * 
 */
class ByteCodeInstruction {
    public:
        /**
         * @brief Size of instruction
         * 
         * @return size_t the length of the instruction
         */
        virtual size_t Size()=0;
        /**
         * @brief Write the instruction to vector
         * 
         * @param data the vector of uint8_t
         */
        virtual void Write(std::vector<uint8_t>& data)=0;
        virtual ~ByteCodeInstruction()
        {}
};
/**
 * @brief Basic bytecode instruction
 * 
 */
class SimpleInstruction : public ByteCodeInstruction {
    public:
        /**
         * @brief The instruction
         * 
         */
        Instruction instruction;
        /**
         * @brief Construct a new Simple Instruction object
         * 
         * @param instr the instruction
         */
        SimpleInstruction(Instruction instr);
        /**
         * @brief Size of instruction
         * 
         * @return size_t 1, because simple instructions are just an instruction
         */
        size_t Size();
        /**
         * @brief Write the opcode to the vector
         * 
         * @param data the vector of uint8_t
         */
        void Write(std::vector<uint8_t>& data);
};

/**
 * @brief A bit converter
 * 
 */
class BitConverter {
    public:
        /**
         * @brief Get the bits of a double from a int64_t
         * 
         * @param v a int64_t with double's bits
         * @return double the double
         */
        static double ToDoubleBits(uint64_t v);
        /**
         * @brief Get the bits of a int64_t from a double
         * 
         * @param v a double with int64_t's bits
         * @return uint64_t the int64_t
         */
        static uint64_t ToUintBits(double v);
        /**
         * @brief Get big endian double from uint8_t reference of first element of 8 byte array
         * 
         * @param b a reference to the first byte of an array
         * @return double the double
         */
        static double ToDoubleBE(uint8_t& b);
        /**
         * @brief Get big endian uint64_t from uint8_t reference of first element of 8 byte array
         * 
         * @param b a reference to the first byte of an array
         * @return uint64_t the uint64_t
         */
        static uint64_t ToUint64BE(uint8_t& b);
        static uint32_t ToUint32BE(uint8_t& b);
        static uint16_t ToUint16BE(uint8_t& b);
        static void FromDoubleBE(uint8_t& b, double v);
        static void FromUint64BE(uint8_t& b, uint64_t v);
        static void FromUint32BE(uint8_t& b, uint32_t v);
        static void FromUint16BE(uint8_t& b, uint16_t v);
};

class StringInstruction : public ByteCodeInstruction {
    public:
        uint32_t n;
        StringInstruction(uint32_t n);
        size_t Size();
        void Write(std::vector<uint8_t>& data);
};
class ScopeEndTimesInstruction : public ByteCodeInstruction {
    public:
        uint32_t n;
        ScopeEndTimesInstruction(uint32_t n);
        size_t Size();
        void Write(std::vector<uint8_t>& data);
};

class LabelInstruction : public ByteCodeInstruction {
    public:
        std::string label;
        LabelInstruction(std::string lbl);
        size_t Size();
        void Write(std::vector<uint8_t>& data);
};
class LabelableInstruction : public ByteCodeInstruction {
    public:
        uint32_t n;
        std::string label;
        virtual size_t Size();
        virtual void Write(std::vector<uint8_t>& data);
};

class JumpStyleInstruction : public LabelableInstruction {
    public:
        Instruction type;
       
        JumpStyleInstruction(Instruction conditional,std::string label);
        size_t Size();
        void Write(std::vector<uint8_t>& data);
};

class EmbedInstruction : public ByteCodeInstruction {
    public:
        uint32_t n;
        EmbedInstruction(uint32_t n);
        size_t Size();
        void Write(std::vector<uint8_t>& data);
};

class ClosureInstruction : public ByteCodeInstruction {
    public:
        bool hasScope;
        uint32_t n;
        ClosureInstruction(uint32_t n,bool hasScope=true);
        size_t Size();
        void Write(std::vector<uint8_t>& data);
};

class DoubleInstruction : public ByteCodeInstruction {
    public:
        double n;
        DoubleInstruction(double n);
        size_t Size();
        void Write(std::vector<uint8_t>& data);
};

class LongInstruction : public ByteCodeInstruction {
    public:
        int64_t n;
        LongInstruction(int64_t n);
        size_t Size();
        void Write(std::vector<uint8_t>& data);
};

class CharInstruction : public ByteCodeInstruction {
    public:
        char n;
        CharInstruction(char n);
        size_t Size();
        void Write(std::vector<uint8_t>& data);
};

using SyntaxNode = std::variant<int64_t, double, std::string, char, bool, std::nullptr_t, Undefined,std::vector<uint8_t>, AdvancedSyntaxNode>;



struct CodeGenClassEntry {
    uint8_t type;
    uint32_t documentation;
    uint32_t name;
    std::vector<uint32_t> arguments;
    uint32_t closure;
};

struct CodeGenClass {
    uint32_t documentation;
    std::vector<uint32_t> name;
    std::vector<uint32_t> inherits;
    std::vector<CodeGenClassEntry> entries;
};

class ResourceBase {
    public:
        virtual uint32_t GetLength(Tesses::Framework::Filesystem::VFS* embedFS)=0;
        virtual void Write(Tesses::Framework::Streams::Stream* output)=0;
        virtual ~ResourceBase();
        virtual bool IsEqual(ResourceBase* base);
};

class ResourceFile : public ResourceBase 
{
    Tesses::Framework::Streams::Stream* strm=nullptr;
    public:
        ResourceFile();
        ResourceFile(std::string file);
        std::string file;
        uint32_t GetLength(Tesses::Framework::Filesystem::VFS* embedFS);
        void Write(Tesses::Framework::Streams::Stream* output);
        bool IsEqual(ResourceBase* base);
        ~ResourceFile();
};

class ResourceByteArray : public ResourceBase
{
    public:
        std::vector<uint8_t> data;
        uint32_t GetLength(Tesses::Framework::Filesystem::VFS* embedFS);
        void Write(Tesses::Framework::Streams::Stream* output);
};


class CodeGen {
    uint32_t id;
    uint32_t NewId();
    
    void GetFunctionArgs(std::vector<uint32_t>& args,SyntaxNode n);
    void GetFunctionName(std::vector<uint32_t>& name,SyntaxNode n);
    void GetFunctionArgs(std::vector<SyntaxNode>& args,SyntaxNode n);
    SyntaxNode StringifyListOfVars(SyntaxNode n);

    uint32_t GetString(std::string str);
    uint32_t GetResource(std::shared_ptr<ResourceBase> resource);
    std::vector<std::string> strs;
    std::vector<std::shared_ptr<ResourceBase>> res;
    
    std::vector<std::pair<std::vector<uint32_t>, std::vector<ByteCodeInstruction*>>> chunks;
    std::vector<std::pair<std::vector<uint32_t>,uint32_t>> funcs;
    std::vector<CodeGenClass> classes;
   

    void GenNode(std::vector<ByteCodeInstruction*>& instructions, SyntaxNode n,int32_t scope, int32_t contscope, int32_t brkscope, int32_t contI, int32_t brkI);
    void GenPop(std::vector<ByteCodeInstruction*>& instrs,SyntaxNode n);
    public:
        std::vector<std::pair<std::string, TVMVersion>> dependencies;
        std::vector<std::pair<std::string, TVMVersion>> tools;
        TVMVersion version;
        std::string name;
        std::string info;
        std::string icon;
        void GenRoot(SyntaxNode n);
        void Save(Tesses::Framework::Filesystem::VFS* embedFS,Tesses::Framework::Streams::Stream* output);
        ~CodeGen();
};
/**
 * @brief A html expression
 * 
 */
constexpr std::string_view HtmlRootExpression = "htmlRootExpression";
/**
 * @brief an intrinsic function that embeads a resource from the filename based on the constant string argument
 * 
 */
constexpr std::string_view EmbedExpression = "embedExpression";
/**
 * @brief Negative operator -EXPR
 * 
 */
constexpr std::string_view NegativeExpression = "negativeExpression";
/**
 * @brief Not operator !EXPR
 * 
 */
constexpr std::string_view NotExpression = "notExpression";
/**
 * @brief Bitwise not operator ~EXPR
 * 
 */
constexpr std::string_view BitwiseNotExpression = "bitwiseNotExpression";
/**
 * @brief ++i
 * 
 */
constexpr std::string_view PrefixIncrementExpression = "prefixIncrementExpression";
/**
 * @brief --i
 * 
 */
constexpr std::string_view PrefixDecrementExpression = "prefixDecrementExpression";
/**
 * @brief i++
 * 
 */
constexpr std::string_view PostfixIncrementExpression = "postfixIncrementExpression";
/**
 * @brief i--
 * 
 */
constexpr std::string_view PostfixDecrementExpression = "postfixDecrementExpression";
/**
 * @brief Gets variable value
 * 
 */
constexpr std::string_view GetVariableExpression = "getVariableExpression";
/**
 * @brief Get field or get property (EXPR.getNAME()) (unless it is wrapped in AssignExpression) which then it sets the field or calls EXPR.setNAME(VALUE)
 * 
 */
constexpr std::string_view GetFieldExpression = "getFieldExpression";
/**
 * @brief Syntax sugar for EXPR.GetAt(index) (unless it is wrapped in AssignExpresion) which then it is EXPR.SetAt(index, VALUE)
 * 
 */
constexpr std::string_view GetArrayExpression = "getArrayExpression";
/**
 * @brief Function Call myFn() or MyDict.Fn()
 * 
 */
constexpr std::string_view FunctionCallExpression = "functionCallExpression";
/**
 * @brief Used for compound assignment like VAR += EXPR
 * 
 */
constexpr std::string_view CompoundAssignExpression = "compoundAssignExpression";
/**
 * @brief Assign something using =
 * 
 */
constexpr std::string_view AssignExpression = "assignExpression";
/**
 * @brief A char litteral
 * 
 */
constexpr std::string_view CharExpression = "charExpression";
/**
 * @brief The undefined literal
 * 
 */
constexpr std::string_view UndefinedExpression = "undefinedExpression";
/**
 * @brief A list of nodes that does create a new scope
 * 
 */
constexpr std::string_view ScopeNode = "scopeNode";
/**
 * @brief A list of nodes that doesn't create a new scope
 * 
 */
constexpr std::string_view NodeList = "nodeList";
/**
 * @brief Array expresion [1,2,10]
 * 
 */
constexpr std::string_view ArrayExpression = "arrayExpression";
/**
 * @brief Dictionary expression  { a = 42, b = false}
 * 
 */
constexpr std::string_view DictionaryExpression = "dictionaryExpression";
/**
 * @brief var v = 59;
 * 
 */
constexpr std::string_view DeclareExpression = "declareExpression";
/**
 * @brief Closure expression (a,b)=> a * b
 * 
 */
constexpr std::string_view ClosureExpression = "closureExpression";
/**
 * @brief Just like normal closures but doesn't create its own scope
 * 
 */
constexpr std::string_view ScopelessClosureExpression = "scopelessClosureExpression";
/**
 * @brief Ternary operator COND ? EXPR : EXPR
 * 
 */
constexpr std::string_view TernaryExpression = "ternaryExpression";
/**
 * @brief If statement if(COND) {} else if(COND) {} else {}
 * 
 */
constexpr std::string_view IfStatement = "ifStatement";
/**
 * @brief For statement for(var i = 0; i < 42; i++)
 * 
 */
constexpr std::string_view ForStatement = "forStatement";
/**
 * @brief While statement while(COND)
 * 
 */
constexpr std::string_view WhileStatement = "whileStatement";
/**
 * @brief Do statement do(COND)
 * 
 */
constexpr std::string_view DoStatement = "doStatement";
/**
 * @brief Each statement each(var item : EXPR)
 * 
 */
constexpr std::string_view EachStatement = "eachStatement";
/**
 * @brief Return statement return EXPR;
 * 
 */
constexpr std::string_view ReturnStatement = "returnStatement";
/**
 * @brief Break statement break;
 * 
 */
constexpr std::string_view BreakStatement = "breakStatement";
/**
 * @brief Continue statement continue;
 * 
 */
constexpr std::string_view ContinueStatement = "continueStatement";
/**
 * @brief Function statement func main(args)
 * 
 */
constexpr std::string_view FunctionStatement = "functionStatement";
/**
 * @brief The documentation statement /^ Documentation ^/ func main(args) {}
 * 
 */
constexpr std::string_view DocumentationStatement = "documentationStatement";
/**
 * @brief The comma expression EXPR, EXPR
 * 
 */
constexpr std::string_view CommaExpression = "commaExpression";
/**
 * @brief The addition operator EXPR + EXPR
 * 
 */
constexpr std::string_view AddExpression = "addExpression";
/**
 * @brief The subtraction operator EXPR - EXPR
 * 
 */
constexpr std::string_view SubExpression = "subtractExpression";
/**
 * @brief The times operator EXPR * EXPR
 * 
 */
constexpr std::string_view TimesExpression = "timesExpression";
/**
 * @brief The divsion operator EXPR / EXPR
 * 
 */
constexpr std::string_view DivideExpression = "divideExpression";
/**
 * @brief The modulo operator (get remainder) EXPR % EXPR
 * 
 */
constexpr std::string_view ModExpression = "modExpression";
/**
 * @brief The left shift operator EXPR << EXPR
 * 
 */
constexpr std::string_view LeftShiftExpression = "leftShiftExpression";
/**
 * @brief The right shift operator EXPR >> EXPR
 * 
 */
constexpr std::string_view RightShiftExpression = "rightShiftExpression";
/**
 * @brief The bitwise or expression EXPR | EXPR
 * 
 */
constexpr std::string_view BitwiseOrExpression = "bitwiseOrExpression";
/**
 * @brief The bitwise and expression EXPR & EXPR
 * 
 */
constexpr std::string_view BitwiseAndExpression = "bitwiseAndExpression";
/**
 * @brief The xor expression EXPR ^ EXPR
 * 
 */
constexpr std::string_view XOrExpression = "xOrExpression";
/**
 * @brief The logical or expression EXPR || EXPR
 * 
 */
constexpr std::string_view LogicalOrExpression = "logicalOrExpression";
/**
 * @brief The logical and expression EXPR && EXPR
 * 
 */
constexpr std::string_view LogicalAndExpression = "logicalAndExpression";
/**
 * @brief The equals expression EXPR == EXPR
 * 
 */
constexpr std::string_view EqualsExpression = "equalsExpression";
/**
 * @brief The not equals expression EXPR != EXPR
 * 
 */
constexpr std::string_view NotEqualsExpression = "notEqualsExpression";
/**
 * @brief The less than expression EXPR < EXPR
 * 
 */
constexpr std::string_view LessThanExpression = "lessThanExpression";
/**
 * @brief The greater than expression EXPR > EXPR
 * 
 */
constexpr std::string_view GreaterThanExpression = "greaterThanExpression";
/**
 * @brief The less than equals expression EXPR <= EXPR
 * 
 */
constexpr std::string_view LessThanEqualsExpression = "lessThanEqualsExpression";
/**
 * @brief The greater than equals expression EXPR >= EXPR
 * 
 */
constexpr std::string_view GreaterThanEqualsExpression = "greaterThanEqualsExpression";
/**
 * @brief The parentheses expression (expression)
 * 
 */
constexpr std::string_view ParenthesesExpression = "parenthesesExpression";
/**
 * @brief The throw statement
 * 
 */
constexpr std::string_view ThrowStatement = "throwStatement";
/**
 * @brief The breakpoint statement
 */
constexpr std::string_view BreakpointStatement = "breakpointStatement";
/**
 * @brief The try statement
 * 
 */
constexpr std::string_view TryStatement = "tryStatement";
/**
 * @brief The defer statement
 * 
 */
constexpr std::string_view DeferStatement = "deferStatement";
/**
 * @brief The defer statement
 * 
 */
constexpr std::string_view YieldStatement = "yieldStatement";
/**
 * @brief The enumerable statement
 * 
 */
constexpr std::string_view EnumerableStatement = "enumerableStatement";
/**
 * @brief The switch statement
 * 
 */
constexpr std::string_view SwitchStatement = "switchStatement";
/**
 * @brief Case clause of switch statement
 * 
 */
constexpr std::string_view CaseStatement = "caseStatement";
/**
 * @brief Default clause of switch statement
 * 
 */
constexpr std::string_view DefaultStatement = "defaultStatement";
/**
 * @brief Class statement (not implemented)
 * 
 */
constexpr std::string_view ClassStatement = "classStatement";
/**
 * @brief Method statement (in class statement)
 *
 */
constexpr std::string_view MethodStatement = "methodStatement";
/**
 * @brief Abstract method statement (in class statement)
 *
 */
constexpr std::string_view AbstractMethodStatement = "abstractMethodStatement";
/**
 * @brief Field statement (in class statement)
 *
 */
constexpr std::string_view FieldStatement = "fieldStatement";
/**
 * @brief Root path expression / "path" / "to" / "file" (fullfils the first /)
 * 
 */
constexpr std::string_view RootPathExpression = "rootPathExpression";
/**
 * @brief The relative path expression ./ "path" / "to" / "subdir" (fullfills the ./)
 * 
 */
constexpr std::string_view RelativePathExpression = "relativePathExpression";
/**
 * @brief await expression for async/await
 */
constexpr std::string_view AwaitExpression = "awaitExpression";
/**
 * @brief ?? operator
 */
constexpr std::string_view NullCoalescingExpression = "nullCoalescingExpression";
/**
 * @brief Advanced AST node
 * 
 */
class AdvancedSyntaxNode {
    public:
        /**
         * @brief The name of the advanced AST node
         * 
         */
        std::string nodeName;
        /**
         * @brief The advanced syntax node is an expression
         * 
         */
        bool isExpression;
        /**
         * @brief The subnodes of the abstract syntax node
         * 
         */
        std::vector<SyntaxNode> nodes;
        /**
         * @brief Create abstract syntax node
         * 
         * @param nodeName The name of the advanced AST node
         * @param isExpression The advanced syntax node is an expression
         * @param n The subnodes of the abstract syntax node
         * @return AdvancedSyntaxNode The AST node
         */
        static AdvancedSyntaxNode Create(std::string_view nodeName,bool isExpression, std::vector<SyntaxNode> n);
};



SyntaxNode Deserialize(std::string astData);

std::string Serialize(SyntaxNode node);
class GC;
class TRootEnvironment;
/**
 * @brief Token Parser
 * 
 */
class Parser {
    uint32_t id;
    uint32_t NewId();
    uint64_t i;
    std::vector<LexToken> tokens;
        
    LexToken tkn;
    void EnsureSymbol(std::string txt);
    bool IsIdentifier(std::string txt,bool pop=true);
    bool IsAnyIdentifier(std::initializer_list<std::string> idents, bool pop=true);
    bool IsSymbol(std::string txt,bool pop=true);
    bool IsAnySymbol(std::initializer_list<std::string> idents, bool pop=true);
    
    SyntaxNode ParseExpression();
    SyntaxNode ParseTernary();
    SyntaxNode ParseNode(bool isRoot=false);
    SyntaxNode ParseAssignment();
    SyntaxNode ParseXOr();
    SyntaxNode ParseBOr();
    SyntaxNode ParseLOr();
    SyntaxNode ParseLAnd();
    SyntaxNode ParseBAnd();
    SyntaxNode ParseEq();
    SyntaxNode ParseRel();
    SyntaxNode ParseShift();
    SyntaxNode ParseSum();
    SyntaxNode ParseFactor();
    SyntaxNode ParseValue();
    SyntaxNode ParseUnary();
    SyntaxNode ParseNullCoalescing();
    void ParseHtml(std::vector<SyntaxNode>& nodes,std::string var);
    GC* gc;
    TRootEnvironment* env;
    public:
        
        /**
         * @brief Construct a new Parser object
         * 
         * @param tokens the tokens from lexer
         */
        Parser(std::vector<LexToken> tokens);
        Parser(std::vector<LexToken> tokens, GC* gc, TRootEnvironment* env);
        /**
         * @brief Turn tokens into abstract syntax tree
         * 
         * @return SyntaxNode the abstract syntax tree
         */
        SyntaxNode ParseRoot()
        {
            return ParseNode(true);
        }
};
   class THeapObject;
    class CallStackEntry;
    class InterperterThread;
        class THeapObject {
        public:
            bool marked;
        virtual void Mark()
        {
            marked = true;
        }
          
        virtual ~THeapObject()
        {

        }

        
    };
    
    
        class THeapObjectHolder
    {
        public:
            THeapObject* obj;
            THeapObjectHolder(THeapObject* obj)
            {
                this->obj = obj;
            }
            THeapObjectHolder()
            {
                
            }
    };



    //this is a dummy type with
    class MethodInvoker {

    };
    

    class TDateTime {
        Tesses::Framework::Date::DateTime* dt;
        public:
            TDateTime();
            TDateTime(Tesses::Framework::Date::DateTime t);
            TDateTime(const TDateTime& dt);
            Tesses::Framework::Date::DateTime& GetDate();
            ~TDateTime();
    };
class TBreak {

};
class TContinue {

};
    /**
     * @brief A script object
     * 
     */

using TObject = std::variant<int64_t,double,char,bool,std::string,std::regex,Tesses::Framework::Filesystem::VFSPath,std::nullptr_t,Undefined,MethodInvoker,THeapObjectHolder,TVMVersion,TDateTime,TBreak,TContinue>;

class TRootEnvironment;
class GC;
class GC {
    Tesses::Framework::Threading::Thread* thrd;
    Tesses::Framework::Threading::Mutex* mtx;
    volatile std::atomic<bool> running;
    std::vector<THeapObject*> roots;
    std::vector<THeapObject*> objects;

    Tesses::Framework::Lazy<Tesses::Framework::Threading::ThreadPool*>* tpool;
    std::vector<std::function<void(GC* gc, TRootEnvironment* env)>> register_everything;
    public:
        Tesses::Framework::Threading::ThreadPool* GetPool();
        bool UsingNullThreads();
        GC();
        void Start();
        bool IsRunning();
        void BarrierBegin();
        void BarrierEnd();
        void Collect();
        void Watch(TObject obj);
        void Unwatch(TObject obj);
        void SetRoot(TObject obj);
        void UnsetRoot(TObject obj);
        static void Mark(TObject obj);
        void RegisterEverythingCallback(std::function<void(GC* gc, TRootEnvironment* env)> cb);
        void RegisterEverything(TRootEnvironment* env);
        ~GC();
};


    std::string ToString(GC* gc, TObject obj);

    class GCList : public THeapObject
    {   
        std::vector<THeapObject*> items;
        
        GC* gc;
        public:
            GCList(GC* gc);
            GCList(GC& gc);
            GC* GetGC();
            void Add(TObject v);
            void Remove(TObject v);
            void Mark();

            
            
            ~GCList();

    };

  


    
    class TFile;
    
    class TFileChunk : public THeapObject
    {
        public:
            static TFileChunk* Create(GCList* gc);
            static TFileChunk* Create(GCList& gc);
            TFile* file;
            std::vector<uint8_t> code;
            std::vector<std::string> args;
            void Mark();
    };
    
  

    
    class TByteArray : public THeapObject
    {
        public:
            std::vector<uint8_t> data;
            static TByteArray* Create(GCList* gc);
            static TByteArray* Create(GCList& gc);
    };
    enum class TClassModifier {
        Private,
        Protected,
        Public,
        Static
    };
    class TClassEntry {
        public: 

            TClassModifier modifier;
            bool isFunction;
            bool isAbstract;
            std::vector<std::string> args;
            std::string documentation;
            std::string name;

            uint32_t chunkId;
    };
    class TClass {
        public:
            std::string documentation;
            std::vector<std::string> name;
            std::vector<std::string> inherits;
            std::vector<TClassEntry> entry;
            
    };
    class TClassObjectEntry {
        public:
            TClassModifier modifier;
            bool canSet;
            std::string name;
            std::string owner;
            TObject value;

    };

    class TFile : public THeapObject
    {
        public:
            
            static TFile* Create(GCList* gc);
            static TFile* Create(GCList& gc);
            std::vector<TFileChunk*> chunks;
            
            std::vector<std::string> strings;
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
        
            void Load(GC* gc, Tesses::Framework::Streams::Stream* strm);
            void Skip(Tesses::Framework::Streams::Stream* strm,size_t len);
            void Ensure(Tesses::Framework::Streams::Stream* strm,uint8_t* buffer, size_t len);
            uint32_t EnsureInt(Tesses::Framework::Streams::Stream* strm);
            std::string EnsureString(Tesses::Framework::Streams::Stream* strm);

            std::string GetString(Tesses::Framework::Streams::Stream* strm);
            void Mark();

            void EnsureCanRunInCrossLang();
    };
    class TAssociativeArray : public THeapObject
    {
        public:
            std::vector<std::pair<TObject,TObject>> items;
            static TAssociativeArray* Create(GCList& ls);
            static TAssociativeArray* Create(GCList* ls);
            void Set(GC* gc, TObject key, TObject value);
            TObject Get(GC* gc, TObject key);
            TObject GetKey(int64_t index);
            TObject GetValue(int64_t index);
            void SetKey(int64_t index, TObject key);
            void SetValue(int64_t index,TObject value);
            int64_t Count();
            void Mark();
    };
 

    class TList : public THeapObject
    {
        public:
            std::vector<TObject> items;
            static TList* Create(GCList* gc);
            static TList* Create(GCList& gc);
            template<typename Itterator>
            static TList* Create(GCList* gc, Itterator begin, Itterator end)
            {
                auto list = Create(gc);
                gc->GetGC()->BarrierBegin();
                for(Itterator i = begin; i != end; ++i)
                {
                    TObject item = *i;
                    list->Add(item);
                }
                gc->GetGC()->BarrierEnd();
                return list;
            }
            template<typename Itterator>
            static TList* Create(GCList& gc, Itterator begin, Itterator end)
            {
                return Create(&gc,begin,end);
            }
            static TList* Create(GCList* gc, std::initializer_list<TObject> il)
            {
                return Create(gc,il.begin(),il.end());
            }
            static TList* Create(GCList& gc, std::initializer_list<TObject> il)
            {
                return Create(gc,il.begin(),il.end());
            }
            virtual int64_t Count();
            virtual TObject Get(int64_t index);
            virtual void Set(int64_t index, TObject value);
            virtual void Insert(int64_t index,TObject value);
            virtual void Add(TObject value);
            virtual void RemoveAt(int64_t index);
            virtual void Clear();
            virtual void Mark();

    };
    using TDItem = std::pair<std::string, TObject>;
    class TDictionary : public THeapObject
    {
        public:
            std::map<std::string,TObject> items;
            static TDictionary* Create(GCList* gc);
            static TDictionary* Create(GCList& gc);
            template<typename Itterator>
            static TDictionary* Create(GCList* gc, Itterator begin, Itterator end)
            {
                auto dict = Create(gc);
                gc->GetGC()->BarrierBegin();
                for(Itterator i = begin; i != end; ++i)
                {
                    TDItem item = *i;
                    dict->SetValue(item.first, item.second);
                }
                gc->GetGC()->BarrierEnd();
                return dict;
            }
            template<typename Itterator>
            static TDictionary* Create(GCList& gc, Itterator begin, Itterator end)
            {
                return Create(&gc,begin,end);
            }

            static TDictionary* Create(GCList* gc, std::initializer_list<TDItem> il)
            {
                return Create(gc,il.begin(),il.end());
            }
            static TDictionary* Create(GCList& gc, std::initializer_list<TDItem> il)
            {
                return Create(gc, il.begin(),il.end());
            }
            

          
            virtual bool HasValue(std::string key);
            virtual bool MethodExists(GCList& ls,std::string key);
            virtual TObject GetValue(std::string key);
            virtual void SetValue(std::string key, TObject value);
            virtual void Mark();
            void DeclareFunction(GC* gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb);
            void DeclareFunction(GC& gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb);
            void DeclareFunction(GC* gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb,std::function<void()> destroy);
            void DeclareFunction(GC& gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb,std::function<void()> destroy);
            TObject CallMethod(GCList& ls, std::string key, std::vector<TObject> args);
    };

   

    class TRootEnvironment;
    class TSubEnvironment;

    class TCallable : public THeapObject
    {
        public:
            TObject tag;
            std::string documentation;
            virtual TObject Call(GCList& ls,std::vector<TObject> args)=0;
            virtual void Mark();
    };

    

    class TEnvironment : public THeapObject {
        
        public:
            std::vector<TCallable*> defers;
            TObject LoadFile(GC* gc, TFile* f);
            TObject Eval(GCList& ls,std::string code);
            virtual bool HasVariable(std::string key)=0;
            virtual bool HasVariableRecurse(std::string key)=0;
            

            virtual bool HasVariableOrFieldRecurse(std::string key,bool setting=false)=0;
            virtual TObject GetVariable(std::string key)=0;
            
            virtual TObject GetVariable(GCList& ls, std::string key)=0;
            virtual TObject SetVariable(GCList& ls, std::string key, TObject v)=0;
            
            virtual void SetVariable(std::string key, TObject value)=0;
            TDictionary* EnsureDictionary(GC* gc, std::string key);
            virtual void DeclareVariable(std::string key, TObject value)=0;
            void DeclareVariable(GC* gc,std::vector<std::string> key, TObject value);
            virtual TRootEnvironment* GetRootEnvironment()=0;
            virtual TEnvironment* GetParentEnvironment()=0;
            virtual TSubEnvironment* GetSubEnvironment(TDictionary* dict);
            TSubEnvironment* GetSubEnvironment(GCList* gc);
            TSubEnvironment* GetSubEnvironment(GCList& gc);

            void DeclareFunction(GC* gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb);
            void DeclareFunction(GC& gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb);
            void DeclareFunction(GC* gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb,std::function<void()> destroy);
            void DeclareFunction(GC& gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb,std::function<void()> destroy);
            TObject CallFunction(GCList& ls, std::string key, std::vector<TObject> args);
    
    };
    class TClassEnvironment;
    class TClassObject : public THeapObject 
    {
        TClassObjectEntry* GetEntry(std::string classN, std::string key);
        public:
            TFile* file;
            uint32_t classIndex;
            TEnvironment* ogEnv;
            TClassEnvironment* env;
            std::string name;
            std::vector<std::string> inherit_tree;
            std::vector<TClassObjectEntry> entries;
            
            static TClassObject* Create(GCList& ls, TFile* f, uint32_t classIndex, TEnvironment* env, std::vector<TObject> args);
            static TClassObject* Create(GCList* ls, TFile* f, uint32_t classIndex, TEnvironment* env, std::vector<TObject> args);

            TObject GetValue(std::string className, std::string name);
            void SetValue(std::string className, std::string name,TObject value);
            bool HasValue(std::string className,std::string name);
            bool HasField(std::string className,std::string name);
            bool HasMethod(std::string className,std::string name);
            std::string TypeName();
            void Mark();
    };
    class TClassEnvironment : public TEnvironment
    {
        TEnvironment* env;
        TClassObject* clsObj;
        public:
            static TClassEnvironment* Create(GCList* gc,TEnvironment* env,TClassObject* obj);
            static TClassEnvironment* Create(GCList& gc,TEnvironment* env,TClassObject* obj);
            TClassEnvironment(TEnvironment* env,TClassObject* obj);
            bool HasVariable(std::string key);
            bool HasVariableRecurse(std::string key);
            bool HasVariableOrFieldRecurse(std::string key, bool setting=false);
            
            TObject GetVariable(std::string key);
            void SetVariable(std::string key, TObject value);
            TObject GetVariable(GCList& ls, std::string key);
            TObject SetVariable(GCList& ls, std::string key, TObject v);
            
            void DeclareVariable(std::string key, TObject value);
            TRootEnvironment* GetRootEnvironment();
            TEnvironment* GetParentEnvironment();
            
            void Mark();
    };
   
   
    class EnvironmentPermissions {
        public:
            EnvironmentPermissions();
            Tesses::Framework::Filesystem::VFSPath sqliteOffsetPath;
            bool canRegisterEverything;
            bool canRegisterConsole;
            bool canRegisterIO;
            bool canRegisterLocalFS;
            bool canRegisterNet;
            bool canRegisterSqlite;
            bool canRegisterVM;
            bool canRegisterDictionary;
            bool canRegisterJSON;
            bool canRegisterCrypto;
            bool canRegisterRoot;
            bool canRegisterProcess;
            bool canRegisterPath;
            bool canRegisterOGC;
            bool canRegisterEnv;
            bool canRegisterClass;
            bool sqlite3Scoped;
            bool locked;
            TDictionary* customConsole =nullptr;
    };

    class TRootEnvironment : public TEnvironment
    {
        TDictionary* dict;
        TCallable* error=nullptr;
        
        void LoadDependency(GC* gc,Tesses::Framework::Filesystem::VFS* vfs, std::pair<std::string,TVMVersion> dep);
        public:
            EnvironmentPermissions permissions;

            std::vector<std::pair<std::string, TVMVersion>> dependencies;
            std::vector<std::pair<TFile*, uint32_t>> classes;

            bool TryFindClass(std::vector<std::string>& name, size_t& index);
            
            void LoadFileWithDependencies(GC* gc,Tesses::Framework::Filesystem::VFS* vfs, TFile* f);
            void LoadFileWithDependencies(GC* gc,Tesses::Framework::Filesystem::VFS* vfs, Tesses::Framework::Filesystem::VFSPath path);
    
            TDictionary* GetDictionary();
            static TRootEnvironment* Create(GCList* gc,TDictionary* dict);
            static TRootEnvironment* Create(GCList& gc,TDictionary* dict);
            TRootEnvironment(TDictionary* dict);
            bool HasVariable(std::string key);
            bool HasVariableRecurse(std::string key);
            bool HasVariableOrFieldRecurse(std::string key,bool setting=false);
            
            TObject GetVariable(std::string key);
            void SetVariable(std::string key, TObject value);
            
            TObject GetVariable(GCList& ls, std::string key);
            TObject SetVariable(GCList& ls, std::string key, TObject v);
            void DeclareVariable(std::string key, TObject value);
            TRootEnvironment* GetRootEnvironment();
            TEnvironment* GetParentEnvironment();
            void RegisterOnError(TCallable* callable);
            bool HandleException(GC* gc,TEnvironment* env, TObject err);
            bool HandleBreakpoint(GC* gc,TEnvironment* env, TObject err);
            void Mark();
    };
    class TStd {
        public:
            static void RegisterStd(GC* gc, TRootEnvironment* env);
            static void RegisterConsole(GC* gc,TRootEnvironment* env);
            static void RegisterIO(GC* gc,TRootEnvironment* env, bool enableLocalFilesystem=true);
            static void RegisterNet(GC* gc, TRootEnvironment* env);
            static void RegisterSqlite(GC* gc,TRootEnvironment* env);
            static void RegisterVM(GC* gc,TRootEnvironment* env);
            static void RegisterDictionary(GC* gc, TRootEnvironment* env);
            static void RegisterJson(GC* gc, TRootEnvironment* env);
            static void RegisterCrypto(GC* gc,TRootEnvironment* env);
            static void RegisterRoot(GC* gc, TRootEnvironment* env);
            static void RegisterPath(GC* gc, TRootEnvironment* env);
            static void RegisterOGC(GC* gc, TRootEnvironment* env);
            static void RegisterEnv(GC* gc, TRootEnvironment* env);
            static void RegisterProcess(GC* gc, TRootEnvironment* env);
            static void RegisterClass(GC* gc, TRootEnvironment* env);
    };

    class TSubEnvironment : public TEnvironment
    {
        TEnvironment* env;
        TDictionary* dict;
        public:
            static TSubEnvironment* Create(GCList* gc,TEnvironment* env,TDictionary* dict);
            static TSubEnvironment* Create(GCList& gc,TEnvironment* env,TDictionary* dict);
            TSubEnvironment(TEnvironment* env,TDictionary* dict);
            bool HasVariable(std::string key);
            bool HasVariableRecurse(std::string key);
            bool HasVariableOrFieldRecurse(std::string key, bool setting=false);
            
            TObject GetVariable(std::string key);
            void SetVariable(std::string key, TObject value);
            TObject GetVariable(GCList& ls, std::string key);
            TObject SetVariable(GCList& ls, std::string key, TObject v);
            
            void DeclareVariable(std::string key, TObject value);
            TRootEnvironment* GetRootEnvironment();
            TEnvironment* GetParentEnvironment();
            TDictionary* GetDictionary();
            void Mark();
    };

    
    TDictionary* CreateThread(GCList& ls, TCallable* callable,bool detached);

    class TArgWrapper : public TCallable
    {
        public:
            TCallable* callable;
            static TArgWrapper* Create(GCList& ls, TCallable* callable);
            static TArgWrapper* Create(GCList* ls, TCallable* callable);
            TObject Call(GCList& ls,std::vector<TObject> args);
            void Mark();
    };

    class TExternalMethod : public TCallable
    {
        std::function<TObject(GCList& ls, std::vector<TObject> args)> cb;
        std::function<void()> destroy;
    
        public: 
            std::vector<std::string> args;
            std::vector<TObject> watch;
            TExternalMethod(std::function<TObject(GCList& ls, std::vector<TObject> args)> cb,std::string documentation, std::vector<std::string> argNames,std::function<void()> destroy);
            static TExternalMethod* Create(GCList& ls,std::string documentation, std::vector<std::string> argNames,std::function<TObject(GCList& ls, std::vector<TObject> args)> cb,std::function<void()> destroy);
            static TExternalMethod* Create(GCList* ls,std::string documentation, std::vector<std::string> argNames,std::function<TObject(GCList& ls, std::vector<TObject> args)> cb,std::function<void()> destroy);

            static TExternalMethod* Create(GCList& ls,std::string documentation, std::vector<std::string> argNames,std::function<TObject(GCList& ls, std::vector<TObject> args)> cb);
            static TExternalMethod* Create(GCList* ls,std::string documentation, std::vector<std::string> argNames,std::function<TObject(GCList& ls, std::vector<TObject> args)> cb);
            TObject Call(GCList& ls, std::vector<TObject> args);

            ~TExternalMethod();
            void Mark()
            {
                if(this->marked) return;
                this->marked=true;
                for(auto item : watch)
                    GC::Mark(item);
                    GC::Mark(this->tag);
            }
            
    };
    
    class TEnumerator : public THeapObject
    {
        public:
            virtual bool MoveNext(GC* ls)=0;
            virtual TObject GetCurrent(GCList& ls)=0;
            static TEnumerator* CreateFromObject(GCList& ls, TObject obj);
    };
    class TAssociativeArrayEnumerator : public TEnumerator 
    {
        int64_t index;
        TAssociativeArray* ls;
        public:
            static TAssociativeArrayEnumerator* Create(GCList& ls, TAssociativeArray* list);
            static TAssociativeArrayEnumerator* Create(GCList* ls, TAssociativeArray* list);
            bool MoveNext(GC* ls);
            TObject GetCurrent(GCList& ls);
            void Mark();
    };

    class TCustomEnumerator : public TEnumerator {
        public:
            TDictionary* dict;
            bool MoveNext(GC* ls);
            TObject GetCurrent(GCList& ls);
            void Mark();
            static TCustomEnumerator* Create(GCList& ls,TDictionary* dict);
            static TCustomEnumerator* Create(GCList* ls,TDictionary* dict);
            
    };

    class TYieldEnumerator : public TEnumerator
    {
        bool hasStarted;
        TObject enumerator;
        TObject current;
        public:
            bool MoveNext(GC* ls);
            TObject GetCurrent(GCList& ls);
            void Mark();
            static TYieldEnumerator* Create(GCList& ls,TObject v);
            static TYieldEnumerator* Create(GCList* ls,TObject v);
            
    };
    
    class TStringEnumerator : public TEnumerator 
    {
        bool hasStarted;
        size_t index;
        std::string str;
        public:
            static TStringEnumerator* Create(GCList& ls,std::string str);
            static TStringEnumerator* Create(GCList* ls,std::string str);
            bool MoveNext(GC* ls);
            TObject GetCurrent(GCList& ls);
    };
   
    class TListEnumerator : public TEnumerator 
    {
        int64_t index;
        TList* ls;
        public:
            static TListEnumerator* Create(GCList& ls, TList* list);
            static TListEnumerator* Create(GCList* ls, TList* list);
            bool MoveNext(GC* ls);
            TObject GetCurrent(GCList& ls);
            void Mark();
    };
    class TDynamicList;
    class TDynamicDictionary;
    class TDynamicListEnumerator : public TEnumerator 
    {
        int64_t index;
        TDynamicList* ls;
        public:
            static TDynamicListEnumerator* Create(GCList& ls, TDynamicList* list);
            static TDynamicListEnumerator* Create(GCList* ls, TDynamicList* list);
            bool MoveNext(GC* ls);
            TObject GetCurrent(GCList& ls);
            void Mark();
    };
    class TVFSPathEnumerator : public TEnumerator
    {
        Tesses::Framework::Filesystem::VFSPathEnumerator enumerator;
        public:
            static TVFSPathEnumerator* Create(GCList& ls, Tesses::Framework::Filesystem::VFSPathEnumerator list);
            static TVFSPathEnumerator* Create(GCList* ls, Tesses::Framework::Filesystem::VFSPathEnumerator list);
            bool MoveNext(GC* ls);
            TObject GetCurrent(GCList& ls);
            
    };


    class TStreamHeapObject : public THeapObject
    {
        public:
            Tesses::Framework::Streams::Stream* stream;

            static TStreamHeapObject* Create(GCList& ls, Tesses::Framework::Streams::Stream* strm);
            static TStreamHeapObject* Create(GCList* ls, Tesses::Framework::Streams::Stream* strm);
            ~TStreamHeapObject();
            void Close();

    };

    class TVFSHeapObject : public THeapObject
    {
        public:
            Tesses::Framework::Filesystem::VFS* vfs;
            static TVFSHeapObject* Create(GCList& ls, Tesses::Framework::Filesystem::VFS* vfs);
            static TVFSHeapObject* Create(GCList* ls, Tesses::Framework::Filesystem::VFS* vfs);
            ~TVFSHeapObject();
            void Close();
    };

    class TServerHeapObject : public THeapObject
    {
        public:
            Tesses::Framework::Http::IHttpServer* server;
            static TServerHeapObject* Create(GCList& ls, Tesses::Framework::Http::IHttpServer* vfs);
            static TServerHeapObject* Create(GCList* ls, Tesses::Framework::Http::IHttpServer* vfs);
            ~TServerHeapObject();
            void Close();
    };
    class TObjectVFS : public Tesses::Framework::Filesystem::VFS
    {
        public:
            TObject obj;
            GCList* ls;
            TObjectVFS(GC* gc, TObject obj);
            Tesses::Framework::Streams::Stream* OpenFile(Tesses::Framework::Filesystem::VFSPath path, std::string mode);
            void CreateDirectory(Tesses::Framework::Filesystem::VFSPath path);
            void DeleteDirectory(Tesses::Framework::Filesystem::VFSPath path);
            bool RegularFileExists(Tesses::Framework::Filesystem::VFSPath path);
            bool SymlinkExists(Tesses::Framework::Filesystem::VFSPath path);
            bool CharacterDeviceExists(Tesses::Framework::Filesystem::VFSPath path);
            bool BlockDeviceExists(Tesses::Framework::Filesystem::VFSPath path);
            bool SocketFileExists(Tesses::Framework::Filesystem::VFSPath path);
            bool FIFOFileExists(Tesses::Framework::Filesystem::VFSPath path);
            bool FileExists(Tesses::Framework::Filesystem::VFSPath path);
            bool SpecialFileExists(Tesses::Framework::Filesystem::VFSPath path);
            void CreateSymlink(Tesses::Framework::Filesystem::VFSPath existingFile, Tesses::Framework::Filesystem::VFSPath symlinkFile);
            void CreateHardlink(Tesses::Framework::Filesystem::VFSPath existingFile, Tesses::Framework::Filesystem::VFSPath newName);
            bool DirectoryExists(Tesses::Framework::Filesystem::VFSPath path);
            void DeleteFile(Tesses::Framework::Filesystem::VFSPath path);
            void DeleteDirectoryRecurse(Tesses::Framework::Filesystem::VFSPath path);
            Tesses::Framework::Filesystem::VFSPathEnumerator EnumeratePaths(Tesses::Framework::Filesystem::VFSPath path);
            void MoveFile(Tesses::Framework::Filesystem::VFSPath src, Tesses::Framework::Filesystem::VFSPath dest);
            void MoveDirectory(Tesses::Framework::Filesystem::VFSPath src, Tesses::Framework::Filesystem::VFSPath dest);
            Tesses::Framework::Filesystem::VFSPath ReadLink(Tesses::Framework::Filesystem::VFSPath path);
            std::string VFSPathToSystem(Tesses::Framework::Filesystem::VFSPath path);
            Tesses::Framework::Filesystem::VFSPath SystemToVFSPath(std::string path);
            void GetDate(Tesses::Framework::Filesystem::VFSPath path, Tesses::Framework::Date::DateTime& lastWrite, Tesses::Framework::Date::DateTime& lastAccess);
            void SetDate(Tesses::Framework::Filesystem::VFSPath path, Tesses::Framework::Date::DateTime lastWrite, Tesses::Framework::Date::DateTime lastAccess);
            ~TObjectVFS();
    };

    class TObjectStream : public Tesses::Framework::Streams::Stream
    {
        int64_t _GetPosInternal();
        public:
            TObject obj;
            GCList* ls;
            TObjectStream(GC* gc, TObject obj);
            bool EndOfStream();
            size_t Read(uint8_t* buff, size_t sz);
            size_t Write(const uint8_t* buff, size_t sz);
            bool CanRead();
            bool CanWrite();
            bool CanSeek();
            int64_t GetPosition();
            int64_t GetLength();
            void Flush();
            void Seek(int64_t pos, Tesses::Framework::Streams::SeekOrigin whence);
            ~TObjectStream();
    };

    class TObjectHttpServer : public Tesses::Framework::Http::IHttpServer
    {
        public:
            TObject obj;
            GCList* ls;
            TObjectHttpServer(GC* gc,TObject obj);
            bool Handle(Tesses::Framework::Http::ServerContext& ctx);
            ~TObjectHttpServer();
    };
    

    class TDictionaryEnumerator : public TEnumerator 
    {
        bool hasStarted;
        std::map<std::string, Tesses::CrossLang::TObject>::iterator ittr;
        TDictionary* dict;
        public:
            static TDictionaryEnumerator* Create(GCList& ls, TDictionary* dict);
            static TDictionaryEnumerator* Create(GCList* ls, TDictionary* dict);
            bool MoveNext(GC* ls);
            TObject GetCurrent(GCList& ls);
            void Mark();
    };

    class TClosure : public TCallable
    {
        public:
            static TClosure* Create(GCList& ls,TEnvironment* env,TFile* file,uint32_t chunkId,bool ownScope=true);
            static TClosure* Create(GCList* ls,TEnvironment* env,TFile* file,uint32_t chunkId,bool ownScope=true);
            bool ownScope;
            TFileChunk* closure;
            TFile* file;
            uint32_t chunkId;
            TEnvironment* env;
            std::string className;
            TObject Call(GCList& ls,std::vector<TObject> args);
            void Mark();
    };
    class TDynamicList : public THeapObject
    {
        public:
            TCallable* cb;
            static TDynamicList* Create(GCList& ls,TCallable* callable);
            static TDynamicList* Create(GCList* ls,TCallable* callable);

            void Mark();

            int64_t Count(GCList& ls);

            TObject GetAt(GCList& ls, int64_t index);

            TObject SetAt(GCList& ls, int64_t index, TObject val);

            TObject Add(GCList& ls, TObject v);
            TObject Insert(GCList& ls, int64_t index, TObject v);
            TObject RemoveAllEqual(GCList& ls, TObject v);
            TObject Remove(GCList& ls, TObject v);
            TObject RemoveAt(GCList& ls, int64_t v);
            TObject Clear(GCList& ls);
            TObject ToString(GCList& ls);

            ~TDynamicList();
    };
    class TDynamicDictionary : public THeapObject
    {
        public:
            TCallable* cb;

            static TDynamicDictionary* Create(GCList& ls,TCallable* callable);
            static TDynamicDictionary* Create(GCList* ls,TCallable* callable);

            void Mark();

            TObject GetField(GCList& ls, std::string key);

            TObject SetField(GCList& ls, std::string key, TObject value);

            TObject CallMethod(GCList& ls, std::string name, std::vector<TObject> args);      

            bool MethodExists(GCList& ls, std::string name);

            TEnumerator* GetEnumerator(GCList& ls);


            ~TDynamicDictionary();
    };

    
     class CallStackEntry : public THeapObject
    {
        public:
            static CallStackEntry* Create(GCList* ls);
            static CallStackEntry* Create(GCList& ls);
            std::vector<TObject> stack;
            TEnvironment* env;
            TClosure* callable;
            uint32_t ip;
            uint32_t scopes; 
            bool mustReturn;
            
            void Mark();
            void Push(GC* gc,TObject v);
            TObject Pop(GCList& gcl);

            TObject Resume(GCList& ls);
    };
    class TTask : public THeapObject
    {
        TCallable* cont=nullptr;
        std::exception_ptr ex=nullptr;
        GC* gc;
        TObject obj=Undefined();
        bool isCompleted=false;
        TTask(GC* gc);
        public:
            static TTask* Create(GCList& ls);
            bool IsCompleted();

            void ContinueWith(TCallable* callable);
            TTask* ContinueWith(GCList& ls,TCallable* callable);


            void SetFailed(std::exception_ptr ex);
            void SetSucceeded(TObject v);

            TObject Wait();
            void Mark();

            static TTask* FromClosure(GCList& ls, TClosure* closure);

            static TTask* FromCallStackEntry(GCList& ls, CallStackEntry* ent);

            static TTask* Run(GCList& ls, TCallable* callable);

            static TTask* FromResult(GCList& ls, TObject v);
    };
    extern thread_local CallStackEntry* current_function;
    
    class InterperterThread : public THeapObject
    {
        private:
            bool InvokeTwo(GCList& ls,TObject fn, TObject left, TObject right);
            bool InvokeOne(GCList& ls,TObject fn, TObject arg);
            bool InvokeMethod(GCList& ls, TObject fn, TObject instance, std::vector<TObject> args);
            bool ExecuteMethod2(GC* gc, TObject instance, std::string key, std::vector<TObject> args);
        protected:
            static void* ThreadCallback(void* ptr);
            bool JumpIfDefined(GC* gc);
            bool Add(GC* gc);
            bool Sub(GC* gc);
            bool Times(GC* gc);
            bool Divide(GC* gc);
            bool Mod(GC* gc);
            bool Neg(GC* gc);
            bool Lt(GC* gc);
            bool Gt(GC* gc);
            bool Lte(GC* gc);
            bool Gte(GC* gc);
            bool Eq(GC* gc);
            bool NEq(GC* gc);
            bool LShift(GC* gc);
            bool RShift(GC* gc);
            bool BOr(GC* gc);
            bool XOr(GC* gc);
            bool LNot(GC* gc);
            bool BNot(GC* gc);
            bool BAnd(GC* gc);
            bool ExecuteFunction(GC* gc);
            bool ExecuteMethod(GC* gc);
            bool GetVariable(GC* gc);
            bool SetVariable(GC* gc);
            bool GetField(GC* gc);
            bool SetField(GC* gc);
            bool GetArray(GC* gc);
            bool SetArray(GC* gc);
            bool DeclareVariable(GC* gc);
            bool PushLong(GC* gc);
            bool PushDouble(GC* gc);
            bool PushChar(GC* gc);
            bool PushString(GC* gc);
            bool PushClosure(GC* gc);
            bool PushScopelessClosure(GC* gc);
            bool PushResource(GC* gc);
            bool Illegal(GC* gc);
            bool Throw(GC* gc);
            bool Yield(GC* gc);
            bool Jump(GC* gc);
            bool JumpConditional(GC* gc);
            bool JumpUndefined(GC* gc);
            bool Defer(GC* gc);
            bool TryCatch(GC* gc);
            bool Return(GC* gc);
            bool ScopeBegin(GC* gc);
            bool ScopeEnd(GC* gc);
            bool ScopeEndTimes(GC* gc);
            bool PushTrue(GC* gc);
            bool PushFalse(GC* gc);
            bool PushNull(GC* gc);
            bool PushUndefined(GC* gc);
            bool CreateDictionary(GC* gc);
            bool CreateArray(GC* gc);
            bool AppendList(GC* gc);
            bool AppendDictionary(GC* gc);
            bool PushRootPath(GC* gc);
            bool PushRelativePath(GC* gc);
            bool Pop(GC* gc);
            bool Dup(GC* gc);
            bool Nop(GC* gc);
            bool Breakpoint(GC* gc);
            bool PushBreak(GC* gc);
            bool PushContinue(GC* gc);
            bool JumpIfBreak(GC* gc);
            bool JumpIfContinue(GC* gc);
        public:
            static InterperterThread* Create(GCList* ls);
            static InterperterThread* Create(GCList& ls);
            std::vector<CallStackEntry*> call_stack_entries;
            virtual void Execute(GC* gc);
            void AddCallStackEntry(GCList& ls,TClosure* closure, std::vector<TObject> args);
            void Mark();
    };

    


    class VMException : public std::exception {
        
        std::string error_message;
        public:
           
            VMException(std::string ex)
            {
                error_message = "VMException: ";
                error_message.append(ex);
            }

           
            
        const char * what() const noexcept override
        {
            return error_message.c_str();
        }
    };
    class TAny : public THeapObject {
        public:
            std::any any;
            TObject other;    

            static TAny* Create(GCList& ls);
            static TAny* Create(GCList* ls);
            void Mark();
    };
    class TNativeObject : public THeapObject 
    {
        public:
        template<typename T,typename... TArgs>
        static T* Create(GCList& ls,TArgs... args)
        {
            T* obj = new T(args...);
            GC* gc = ls.GetGC();
            ls.Add(obj);
            gc->Watch(obj);
            return obj;
        }
        template<typename T,typename... TArgs>
        static T* Create(GCList* ls,TArgs... args)
        {
            T* obj = new T(args...);
            GC* gc = ls->GetGC();
            ls->Add(obj);
            gc->Watch(obj);
            return obj;
        }

        virtual TObject CallMethod(GCList& ls,std::string name, std::vector<TObject> args)=0;
        virtual std::string TypeName()=0;
        virtual bool ToBool();
        virtual bool Equals(GC* gc, TObject right);
        virtual ~TNativeObject();
    };
    class TNative : public THeapObject
    {
        std::atomic<bool> destroyed;
        void* ptr;
        std::function<void(void*)> destroy;
        public:
            TObject other;    
            TNative(void* ptr,std::function<void(void*)> destroy);
            bool GetDestroyed();
            void* GetPointer();
            void Destroy();
            void Mark();
            static TNative* Create(GCList& ls, void* ptr,std::function<void(void*)> destroy);
            static TNative* Create(GCList* ls, void* ptr,std::function<void(void*)> destroy);
            ~TNative();
    };

   

    class ThreadHandle : public THeapObject {
        public:
            Tesses::Framework::Threading::Thread* thrd;
            std::atomic<bool> hasInit;
            std::atomic<bool> hasReturned;
            std::atomic<bool> detached;
            TCallable* callable;
            TObject returnValue;
            GC* gc;
            

            void Mark()
            {
                if(this->marked) return;
                this->marked = true;
                callable->Mark();
                GC::Mark(returnValue);
            }
            ~ThreadHandle()
            {
                if(this->detached)
                {
                    this->thrd->Join();
                    delete this->thrd;
                }
                
            }
    };

   
    TObject ExecuteFunction(GCList& ls,TCallable* callable,std::vector<TObject> args);
    class VMByteCodeException : public std::exception
    {
        std::string lastErrorText;
        std::shared_ptr<GCList> gcList;
        
        public:

            TObject exception;
            
            VMByteCodeException()
            {
            }

            VMByteCodeException(GC* gc,TObject obj)
            {
                gcList = std::make_shared<GCList>(gc);
                gcList.get()->Add(obj);
                this->exception = obj;   
                UpdateError();
            }

            void UpdateError()
            {
                lastErrorText = ToString(gcList.get()->GetGC(),exception);

            }

            const char * what() const noexcept override
            {
                return lastErrorText.c_str();
            }

    };

    class SyntaxException : public std::exception
    {
        std::string msg={};
        public:
            SyntaxException(LexTokenLineInfo lineInfo, std::string message)
            {
                msg.append("in file: ");
                msg.append(lineInfo.filename);
                msg.append(":");
                msg.append(std::to_string(lineInfo.line));
                msg.append(":");
                msg.append(std::to_string(lineInfo.column));
                msg.append(":");
                msg.append(std::to_string(lineInfo.offset));
                msg.append(": ");
                msg.append(message);
            }
            const char * what() const noexcept override
            {
                return msg.c_str();
            }
    };

    
    template<typename T>
    bool GetObject(TObject& obj, T& res)
    {
        if(!std::holds_alternative<T>(obj)) return false;
        res = std::get<T>(obj);
        return true;
    }
    template<typename T>
    bool GetArgument(std::vector<TObject>& args, size_t index, T& obj)
    {
        if( index >= args.size()) return false;
        return GetObject(args[index], obj);
    }

    template<typename T>
    bool GetObjectHeap(TObject& obj, T& res)
    {
        THeapObjectHolder h;
        if(!GetObject<THeapObjectHolder>(obj,h)) return false;
        auto v = dynamic_cast<T>(h.obj);
        if(v == nullptr) return false;
        res = v;
        return true;
    }

    template<typename T>
    bool GetArgumentHeap(std::vector<TObject>& args, size_t index, T& obj)
    {
        if( index >= args.size()) return false;
        return GetObjectHeap(args[index], obj);
    }
    bool GetObjectAsPath(TObject& obj, Tesses::Framework::Filesystem::VFSPath& path, bool allowString=true);
    bool GetArgumentAsPath(std::vector<TObject>& args, size_t index, Tesses::Framework::Filesystem::VFSPath& path,bool allowString=true);
    bool ToBool(TObject obj);
    bool Equals(GC* gc, TObject left, TObject right);
    typedef void (*PluginFunction)(GC* gc,TRootEnvironment* env);
    #if !defined(_WIN32)
    #define DLLEXPORT
    #else
    #define DLLEXPORT __declspec(dllexport)
    #endif
    #define CROSSLANG_PLUGIN(plugin) DLLEXPORT extern "C" void CrossLangPluginInit(GC* gc, TRootEnvironment* env) { plugin(gc,env); }

    void LoadPlugin(GC* gc, TRootEnvironment* env, Tesses::Framework::Filesystem::VFSPath sharedObjectPath);
    std::string Json_Encode(TObject o,bool indent=false);
    TObject Json_Decode(GCList ls,std::string str);
    //DO NOT USE DIRECTLY
    class SharedPtrTObject {
        GCList* ls;
        TObject o;
        public:
            SharedPtrTObject(GC* gc, TObject o);
            TObject& GetObject();
            GC* GetGC();
            ~SharedPtrTObject();
    };
    using MarkedTObject = std::shared_ptr<SharedPtrTObject>;

    MarkedTObject CreateMarkedTObject(GC* gc, TObject o);
    MarkedTObject CreateMarkedTObject(GC& gc, TObject o);
    MarkedTObject CreateMarkedTObject(GCList* gc, TObject o);
    MarkedTObject CreateMarkedTObject(GCList& gc, TObject o);
    std::string JoinPeriod(std::vector<std::string>& p);
    TObject GetClassInfo(GCList& ls,TFile* f, uint32_t index);

    extern Tesses::Framework::Filesystem::VFSPath CrossLangConfigPath;

    Tesses::Framework::Filesystem::VFSPath Assemble(Tesses::Framework::Filesystem::VFS* vfs);
    void Disassemble(Tesses::Framework::Streams::Stream* src,Tesses::Framework::Filesystem::VFS* vfs, bool generateJSON=true,bool extractResources=true);
    Tesses::Framework::Filesystem::VFSPath Merge(Tesses::Framework::Filesystem::VFS* srcFS, Tesses::Framework::Filesystem::VFSPath sourcePath, Tesses::Framework::Filesystem::VFS* destFS);
};
