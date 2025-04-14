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
#define TVM_MAJOR 1
#define TVM_MINOR 0
#define TVM_PATCH 0
#define TVM_BUILD 0
#define TVM_VERSIONSTAGE DevVersion

namespace Tesses::CrossLang {
    
    std::string EscapeString(std::string text,bool quote);

    Tesses::Framework::Filesystem::VFSPath GetRealExecutablePath(Tesses::Framework::Filesystem::VFSPath realPath);
    Tesses::Framework::Filesystem::VFSPath GetCrossLangConfigDir();

    enum TVMVersionStage : uint8_t
    {
        DevVersion=0,
        AlphaVersion=1,
        BetaVersion=2,
        ProductionVersion=3
    };
    class TVMVersion
    {
        uint8_t major;
        uint8_t minor;
        uint8_t patch;
        uint16_t build;
        public:
            uint8_t Major()
            {
                return major;
            }
            uint8_t Minor()
            {
                return minor;
            }
            uint8_t Patch()
            {
                return patch;
            }
            uint16_t Build()
            {
                return build >> 2;
            }

            TVMVersionStage VersionStage()
            {
                return (TVMVersionStage)(build & 3);
            }

            void SetMajor(uint8_t major)
            {
                this->major=major;
            }
            void SetMinor(uint8_t minor)
            {
                this->minor=minor;
            }
            void SetPatch(uint8_t patch)
            {
                this->patch=patch;
            }
            void SetBuild(uint16_t build)
            {
                this->build = build;
            }

            void SetBuild(uint16_t build,TVMVersionStage version)
            {
                this->build = (build << 2) | version;
            }

            void SetVersion(uint8_t major, uint8_t minor, uint8_t patch, uint16_t build, TVMVersionStage ver)
            {
                this->SetMajor(major);
                this->SetMinor(minor);
                this->SetPatch(patch);
                this->SetBuild(build,ver);
            }

            void SetVersion(uint8_t major, uint8_t minor, uint8_t patch, uint16_t build)
            {
                this->SetMajor(major);
                this->SetMinor(minor);
                this->SetPatch(patch);
                this->SetBuild(build);
            }
            void SetVersion(uint8_t major, uint8_t minor, uint8_t patch, uint8_t buildHigh, uint8_t buildLow)
            {
                this->SetVersion(major, minor, patch, buildHigh << 8 | buildLow);
            }

            void SetVersion(uint8_t* versionData)
            {
                this->SetVersion(versionData[0],versionData[1],versionData[2],versionData[3],versionData[4]);
            }
            void ToArray(uint8_t* versionData)
            {
                versionData[0] = major;
                versionData[1] = minor;
                versionData[2] = patch;
                versionData[3] = build >> 8;
                versionData[4] = build & 0xFF;
            }
            TVMVersion(uint8_t major, uint8_t minor, uint8_t patch, uint16_t build, TVMVersionStage version)
            {
                this->SetVersion(major,minor,patch,build,version);
            }
            TVMVersion(uint8_t* versionData)
            {
                this->SetVersion(versionData);
            }
            TVMVersion()
            {
                this->major=1;
                this->minor=0;
                this->patch=0;
                this->SetBuild(0, TVMVersionStage::DevVersion);
            }
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

            uint64_t AsLong()
            {
                uint64_t v = (uint64_t)major << 32;
                v |= (uint64_t)minor << 24;
                v |= (uint64_t)patch << 16;
                v |= (uint64_t)build;
                return v;
            }
            int CompareToRuntime()
            {
                TVMVersion version(TVM_MAJOR,TVM_MINOR,TVM_PATCH,TVM_BUILD,TVM_VERSIONSTAGE);
                return CompareTo(version);
            }
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


    void CrossArchiveCreate(Tesses::Framework::Filesystem::VFS* vfs,Tesses::Framework::Streams::Stream* strm,std::string name,TVMVersion version,std::string info, std::string icon="");
    std::pair<std::pair<std::string,TVMVersion>,std::string> CrossArchiveExtract(Tesses::Framework::Streams::Stream* strm,Tesses::Framework::Filesystem::VFS* vfs);




typedef enum {
        Identifier,
        Symbol,
        String,
        Char,
        Documentation
} LexTokenType;

    class LexTokenLineInfo
    {
        public:
            int line;
            int column;
            int offset;
            std::string filename;
            void Add(int c);
            void Subtract(size_t c);
    };


    class LexToken {
        public:
            LexTokenLineInfo lineInfo;
            LexTokenType type;
            std::string text;
    };
    int Lex(std::string filename, std::istream& strm, std::vector<LexToken>& tokens);


class Undefined
{

};

class AdvancedSyntaxNode;

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
    PUSHRELATIVEPATH
} Instruction;

class ByteCodeInstruction {
    public:
        virtual size_t Size()=0;
        virtual void Write(std::vector<uint8_t>& data)=0;
        virtual ~ByteCodeInstruction()
        {}
};

class SimpleInstruction : public ByteCodeInstruction {
    public:
        Instruction instruction;
        SimpleInstruction(Instruction instr);
        size_t Size();
        void Write(std::vector<uint8_t>& data);
};

class BitConverter {
    public:
        static double ToDoubleBits(uint64_t v);
        static uint64_t ToUintBits(double v);
        static double ToDoubleBE(uint8_t& b);
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

using SyntaxNode = std::variant<int64_t, double, std::string, char, bool, std::nullptr_t, Undefined, AdvancedSyntaxNode>;

class ObjectEntry {
    public:
        std::vector<std::string> name;
        std::string doc;
        std::vector<std::pair<std::string,SyntaxNode>> variables;
        std::vector<std::pair<std::pair<std::string,std::string>,uint32_t>> funcs;
        std::vector<std::pair<std::pair<std::string,std::string>,uint32_t>> static_funcs;

};

class CodeGen {
    uint32_t id;
    uint32_t NewId();
    
    void GetFunctionArgs(std::vector<uint32_t>& args,SyntaxNode n);
    void GetFunctionName(std::vector<uint32_t>& name,SyntaxNode n);
    void GetFunctionArgs(std::vector<SyntaxNode>& args,SyntaxNode n);

    uint32_t GetString(std::string str);
    uint32_t GetResource(std::string res);
    std::vector<std::string> strs;
    std::vector<std::string> res;
    
    std::vector<std::pair<std::vector<uint32_t>, std::vector<ByteCodeInstruction*>>> chunks;
    std::vector<std::pair<std::vector<uint32_t>,uint32_t>> funcs;
    std::vector<ObjectEntry> objectEntries;
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


constexpr std::string_view EmbedExpression = "embedExpression";
constexpr std::string_view NegativeExpression = "negativeExpression";
constexpr std::string_view NotExpression = "notExpression";
constexpr std::string_view BitwiseNotExpression = "bitwiseNotExpression";
constexpr std::string_view PrefixIncrementExpression = "prefixIncrementExpression";
constexpr std::string_view PrefixDecrementExpression = "prefixDecrementExpression";
constexpr std::string_view PostfixIncrementExpression = "postfixIncrementExpression";
constexpr std::string_view PostfixDecrementExpression = "postfixDecrementExpression";
constexpr std::string_view GetVariableExpression = "getVariableExpression";
constexpr std::string_view GetFieldExpression = "getFieldExpression";
constexpr std::string_view GetArrayExpression = "getArrayExpression";
constexpr std::string_view FunctionCallExpression = "functionCallExpression";
constexpr std::string_view CompoundAssignExpression = "compoundAssignExpression";
constexpr std::string_view AssignExpression = "assignExpression";
constexpr std::string_view CharExpression = "charExpression";
constexpr std::string_view UndefinedExpression = "undefinedExpression";
constexpr std::string_view ScopeNode = "scopeNode";
constexpr std::string_view NodeList = "nodeList";

constexpr std::string_view ArrayExpression = "arrayExpression";
constexpr std::string_view DictionaryExpression = "dictionaryExpression";
constexpr std::string_view DeclareExpression = "declareExpression";

constexpr std::string_view ClosureExpression = "closureExpression";
constexpr std::string_view ScopelessClosureExpression = "scopelessClosureExpression";

constexpr std::string_view TernaryExpression = "ternaryExpression";
constexpr std::string_view IfStatement = "ifStatement";
constexpr std::string_view ForStatement = "forStatement";
constexpr std::string_view WhileStatement = "whileStatement";
constexpr std::string_view DoStatement = "doStatement";
constexpr std::string_view EachStatement = "eachStatement";
constexpr std::string_view ReturnStatement = "returnStatement";
constexpr std::string_view BreakStatement = "breakStatement";
constexpr std::string_view ContinueStatement = "continueStatement";
constexpr std::string_view FunctionStatement = "functionStatement";
constexpr std::string_view DocumentationStatement = "documentationStatement";
constexpr std::string_view CommaExpression = "commaExpression";

constexpr std::string_view AddExpression = "addExpression";
constexpr std::string_view SubExpression = "subtractExpression";
constexpr std::string_view TimesExpression = "timesExpression";
constexpr std::string_view DivideExpression = "divideExpression";
constexpr std::string_view ModExpression = "modExpression";
constexpr std::string_view LeftShiftExpression = "leftShiftExpression";
constexpr std::string_view RightShiftExpression = "rightShiftExpression";
constexpr std::string_view BitwiseOrExpression = "bitwiseOrExpression";
constexpr std::string_view BitwiseAndExpression = "bitwiseAndExpression";

constexpr std::string_view XOrExpression = "xOrExpression";
constexpr std::string_view LogicalOrExpression = "logicalOrExpression";
constexpr std::string_view LogicalAndExpression = "logicalAndExpression";

constexpr std::string_view EqualsExpression = "equalsExpression";
constexpr std::string_view NotEqualsExpression = "notEqualsExpression";
constexpr std::string_view LessThanExpression = "lessThanExpression";
constexpr std::string_view GreaterThanExpression = "greaterThanExpression";
constexpr std::string_view LessThanEqualsExpression = "lessThanEqualsExpression";
constexpr std::string_view GreaterThanEqualsExpression = "greaterThanEqualsExpression";
constexpr std::string_view ParenthesesExpression = "parenthesesExpression";
constexpr std::string_view ThrowStatement = "throwStatement";
constexpr std::string_view TryStatement = "tryStatement";
constexpr std::string_view DeferStatement = "deferStatement";
constexpr std::string_view YieldStatement = "yieldStatement";
constexpr std::string_view EnumerableStatement = "enumerableStatement";
constexpr std::string_view SwitchStatement = "switchStatement";
constexpr std::string_view CaseStatement = "caseStatement";
constexpr std::string_view DefaultStatement = "defaultStatement";
constexpr std::string_view ObjectStatement = "objectStatement";
constexpr std::string_view StaticStatement = "staticStatement";
constexpr std::string_view MethodDeclaration = "methodDeclaration";
constexpr std::string_view RootPathExpression = "rootPathExpression";
constexpr std::string_view RelativePathExpression = "relativePathExpression";

class AdvancedSyntaxNode {
    public:
        std::string nodeName;
        bool isExpression;
        std::vector<SyntaxNode> nodes;
        static AdvancedSyntaxNode Create(std::string_view nodeName,bool isExpression, std::vector<SyntaxNode> n);
};



SyntaxNode Deserialize(std::string astData);

std::string Serialize(SyntaxNode node);

class Parser {
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
    public:
        Parser(std::vector<LexToken> tokens);
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
using TObject = std::variant<int64_t,double,char,bool,std::string,std::regex,Tesses::Framework::Filesystem::VFSPath,std::nullptr_t,Undefined,MethodInvoker,THeapObjectHolder,TVMVersion>;
class TRootEnvironment;
class GC;
class GC {
    Tesses::Framework::Threading::Thread* thrd;
    Tesses::Framework::Threading::Mutex* mtx;
    volatile std::atomic<bool> running;
    std::vector<THeapObject*> roots;
    std::vector<THeapObject*> objects;
    std::vector<std::function<void(GC* gc, TRootEnvironment* env)>> register_everything;
    public:
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

    class TFile : public THeapObject
    {
        public:
            static TFile* Create(GCList* gc);
            static TFile* Create(GCList& gc);
            std::vector<TFileChunk*> chunks;
            
            std::vector<std::string> strings;
            std::vector<std::pair<std::vector<std::string>, uint32_t>> functions;
            std::vector<std::pair<std::string,TVMVersion>> dependencies;
            std::vector<std::pair<std::string,TVMVersion>> tools;
            std::vector<std::vector<uint8_t>> resources;
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
            virtual TObject GetVariable(std::string key)=0;
            virtual void SetVariable(std::string key, TObject value)=0;
            virtual void DeclareVariable(std::string key, TObject value)=0;
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
            bool canRegisterTime;
            bool sqlite3Scoped;
            bool locked;
    };

    class TRootEnvironment : public TEnvironment
    {
        TDictionary* dict;
        
        void LoadDependency(GC* gc,Tesses::Framework::Filesystem::VFS* vfs, std::pair<std::string,TVMVersion> dep);
        public:
            EnvironmentPermissions permissions;

            std::vector<std::pair<std::string, TVMVersion>> dependencies;
            
            void LoadFileWithDependencies(GC* gc,Tesses::Framework::Filesystem::VFS* vfs, TFile* f);
            void LoadFileWithDependencies(GC* gc,Tesses::Framework::Filesystem::VFS* vfs, Tesses::Framework::Filesystem::VFSPath path);
    
            TDictionary* GetDictionary();
            static TRootEnvironment* Create(GCList* gc,TDictionary* dict);
            static TRootEnvironment* Create(GCList& gc,TDictionary* dict);
            TRootEnvironment(TDictionary* dict);
            bool HasVariable(std::string key);
            bool HasVariableRecurse(std::string key);
            TObject GetVariable(std::string key);
            void SetVariable(std::string key, TObject value);
            void DeclareVariable(std::string key, TObject value);
            TRootEnvironment* GetRootEnvironment();
            TEnvironment* GetParentEnvironment();
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
            static void RegisterTime(GC* gc, TRootEnvironment* env);

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
            TObject GetVariable(std::string key);
            void SetVariable(std::string key, TObject value);
            void DeclareVariable(std::string key, TObject value);
            TRootEnvironment* GetRootEnvironment();
            TEnvironment* GetParentEnvironment();
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
            void GetDate(Tesses::Framework::Filesystem::VFSPath path, time_t& lastWrite, time_t& lastAccess);
            void SetDate(Tesses::Framework::Filesystem::VFSPath path, time_t lastWrite, time_t lastAccess);
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

            void SetAt(GCList& ls, int64_t index, TObject val);

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

            void SetField(GCList& ls, std::string key, TObject value);

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
};
