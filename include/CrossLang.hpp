#pragma once
#include <TessesFramework/TessesFramework.hpp>
#include <any>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <time.h>
#include <unordered_set>
#include <variant>
#include <vector>

#include "CrossLangVersion.h"
#include "TessesFramework/Streams/ByteReader.hpp"

#define CROSSLANG_BYTECODE_MAJOR 1
#define CROSSLANG_BYTECODE_MINOR 0
#define CROSSLANG_BYTECODE_PATCH 0
#define CROSSLANG_BYTECODE_BUILD 0 // SHOULD ALWAYS BE 0
#define CROSSLANG_BYTECODE_VERSIONSTAGE DevVersion

#if defined(_WIN32)
#undef Yield
#endif
/**
 * @namespace Tesses::CrossLang
 * @brief The CrossLang namespace
 *
 */
namespace Tesses::CrossLang {
constexpr uint32_t ALLOC_THRESHOLD = 10000;
using BitConverter = Tesses::Framework::Serialization::BitConverter;
constexpr std::string_view VMName = "CrossLangVM";
constexpr std::string_view VMHowToGet = "https://crosslang.tesseslanguage.com/";

/**
 * @brief Escape a crosslang string (for generating source code)
 *
 * @param text text to escape
 * @param quote whether to surround with double quotes
 * @return std::string the escaped string
 */
std::string EscapeString(std::string text, bool quote);

/**
 * @brief Get the config folder used by crosslang
 *
 * @return Tesses::Framework::Filesystem::VFSPath the config folder for
 * crosslang
 */
Tesses::Framework::Filesystem::VFSPath GetCrossLangConfigDir();

/**
 * @brief Version stage
 *
 */
enum TVMVersionStage : uint8_t {
    DevVersion = 0,
    AlphaVersion = 1,
    BetaVersion = 2,
    ProductionVersion = 3
};
/**
 * @brief CrossLang version
 *
 */
class TVMVersion {
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
    uint8_t Major() { return major; }
    /**
     * @brief Minor
     *
     * @return uint8_t The minor
     */
    uint8_t Minor() { return minor; }
    /**
     * @brief Patch
     *
     * @return uint8_t The patch
     */
    uint8_t Patch() { return patch; }
    /**
     * @brief Build
     *
     * @return uint16_t The build
     */
    uint16_t Build() { return build >> 2; }
    /**
     * @brief Stage (dev, alpha, beta or prod)
     *
     * @return TVMVersionStage The stage
     */
    TVMVersionStage VersionStage() { return (TVMVersionStage)(build & 3); }
    /**
     * @brief Set the Major
     *
     * @param major The major
     */
    void SetMajor(uint8_t major) { this->major = major; }
    /**
     * @brief Set the Minor
     *
     * @param minor The minor
     */
    void SetMinor(uint8_t minor) { this->minor = minor; }
    /**
     * @brief Set the Patch
     *
     * @param patch The patch
     */
    void SetPatch(uint8_t patch) { this->patch = patch; }
    /**
     * @brief Set the Build
     *
     * @param build The build << 2 | (stage & 3)
     */
    void SetBuild(uint16_t build) { this->build = build; }
    /**
     * @brief Set the Build (and stage)
     *
     * @param build
     * @param version
     */
    void SetBuild(uint16_t build, TVMVersionStage version) {
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
    void SetVersion(uint8_t major, uint8_t minor, uint8_t patch, uint16_t build,
                    TVMVersionStage ver) {
        this->SetMajor(major);
        this->SetMinor(minor);
        this->SetPatch(patch);
        this->SetBuild(build, ver);
    }
    /**
     * @brief Set the Version
     *
     * @param major The major
     * @param minor The minor
     * @param patch The patch
     * @param build The build << 2 | (stage & 3)
     */
    void SetVersion(uint8_t major, uint8_t minor, uint8_t patch,
                    uint16_t build) {
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
    void SetVersion(uint8_t major, uint8_t minor, uint8_t patch,
                    uint8_t buildHigh, uint8_t buildLow) {
        this->SetVersion(major, minor, patch, buildHigh << 8 | buildLow);
    }
    /**
     * @brief Set the Version
     *
     * @param versionData same order as SetVersion(uint8_t major, uint8_t minor,
     * uint8_t patch, uint8_t buildHigh, uint8_t buildLow) but is a pointer to
     * uint8_t version[5]
     */
    void SetVersion(uint8_t *versionData) {
        this->SetVersion(versionData[0], versionData[1], versionData[2],
                         versionData[3], versionData[4]);
    }
    /**
     * @brief Set the version from long
     *
     * @param v version serialized as long
     */
    void SetFromLong(uint64_t v) {

        this->major = (uint8_t)((v >> 32) & 0xFF);
        this->minor = (uint8_t)(v >> 24) & 0xFF;
        this->patch = (uint8_t)(v >> 16) & 0xFF;
        this->build = (uint16_t)(v & 0xFFFF);
    }
    /**
     * @brief Encodes the version to bytes to be decoded by SetVersion(uint8_t*)
     * or the constructor
     *
     * @param versionData an array that is 5 bytes long
     */
    void ToArray(uint8_t *versionData) {
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
     * @param version Stage (dev, alpha, beta, prod) see TVMVersionStage for
     * actual names
     */
    TVMVersion(uint8_t major, uint8_t minor, uint8_t patch, uint16_t build,
               TVMVersionStage version) {
        this->SetVersion(major, minor, patch, build, version);
    }
    /**
     * @brief Construct a new Version object
     *
     * @param versionData same order as SetVersion(uint8_t major, uint8_t minor,
     * uint8_t patch, uint8_t buildHigh, uint8_t buildLow) but is a pointer to
     * uint8_t version[5]
     */
    TVMVersion(uint8_t *versionData) { this->SetVersion(versionData); }
    /**
     * @brief Construct a new 1.0.0.0-dev Version object
     *
     */
    TVMVersion() {
        this->major = 1;
        this->minor = 0;
        this->patch = 0;
        this->SetBuild(0, TVMVersionStage::DevVersion);
    }
    /**
     * @brief Construct a new Version object from a long
     *
     * @param v version serialized as long
     */
    TVMVersion(uint64_t v) { this->SetFromLong(v); }
    /**
     * @brief Compares to another version
     *
     * @param version another version object
     * @return int returns 1 if this is newer than other version, 0 if same, -1
     * if this is older than other version
     */
    int CompareTo(TVMVersion &version) {
        if (this->major > version.major)
            return 1;
        if (this->major < version.major)
            return -1;
        if (this->minor > version.minor)
            return 1;
        if (this->minor < version.minor)
            return -1;
        if (this->patch > version.patch)
            return 1;
        if (this->patch < version.patch)
            return -1;
        if (this->build > version.build)
            return 1;
        if (this->build < version.build)
            return -1;
        return 0;
    }
    /**
     * @brief Serializes to a long
     *
     * @return uint64_t serialized as a long
     */
    uint64_t AsLong() {
        uint64_t v = (uint64_t)major << 32;
        v |= (uint64_t)minor << 24;
        v |= (uint64_t)patch << 16;
        v |= (uint64_t)build;
        return v;
    }
    /**
     * @brief Compares this to the runtime version (for Bytecode)
     *
     * @return int CompareTo(RuntimeVersion) where RuntimeVersion is the runtime
     * version
     */
    int CompareToRuntime() {
        TVMVersion version(CROSSLANG_BYTECODE_MAJOR, CROSSLANG_BYTECODE_MINOR,
                           CROSSLANG_BYTECODE_PATCH, CROSSLANG_BYTECODE_BUILD,
                           CROSSLANG_BYTECODE_VERSIONSTAGE);
        return CompareTo(version);
    }
    /**
     * @brief Try to parse the version from a string
     *
     * @param versionStr the version string like 1.0.0.0-prod (or dev, alpha,
     * beta)
     * @param version a reference to a version
     * @return true the parsing succeeded
     * @return false the parsing failed
     */
    static bool TryParse(std::string versionStr, TVMVersion &version) {
        if (versionStr.empty())
            return false;
        size_t sep = versionStr.find_last_of('-');

        std::string left = versionStr;
        std::string right = "prod";

        if (sep != std::string::npos) {
            left = versionStr.substr(0, sep);
            right = versionStr.substr(sep + 1);
        }

        if (left.empty())
            return false;

        TVMVersionStage stage;
        if (right == "dev")
            stage = TVMVersionStage::DevVersion;
        else if (right == "alpha")
            stage = TVMVersionStage::AlphaVersion;
        else if (right == "beta")
            stage = TVMVersionStage::BetaVersion;
        else if (right == "prod")
            stage = TVMVersionStage::ProductionVersion;
        else
            return false;

        // 1 0.0.0
        sep = left.find_first_of('.');

        if (sep != std::string::npos) {
            auto lStr = left.substr(0, sep);

            right = left.substr(sep + 1);
            left = lStr;
        } else {
            version.SetMajor((uint8_t)std::stoul(left));
            version.SetBuild(0, stage);
            return true;
        }
        version.SetMajor((uint8_t)std::stoul(left));
        left = right;
        sep = left.find_first_of('.');

        if (sep != std::string::npos) {
            auto lStr = left.substr(0, sep);

            right = left.substr(sep + 1);
            left = lStr;
        } else {
            version.SetMinor((uint8_t)std::stoul(left));
            version.SetBuild(0, stage);
            return true;
        }
        version.SetMinor((uint8_t)std::stoul(left));
        left = right;
        sep = left.find_first_of('.');

        if (sep != std::string::npos) {
            auto lStr = left.substr(0, sep);

            right = left.substr(sep + 1);
            left = lStr;
        } else {
            version.SetPatch((uint8_t)std::stoul(left));
            version.SetBuild(0, stage);
            return true;
        }
        version.SetPatch((uint8_t)std::stoul(left));

        version.SetBuild((uint16_t)std::stoul(right), stage);
        return true;
    }
    /**
     * @brief Converts version to string
     *
     * @return std::string the version string like 1.0.0.0-prod (or dev, alpha,
     * beta)
     */
    std::string ToString() {
        std::string str = {};
        str.append(std::to_string((int)this->Major()));
        str.push_back('.');
        str.append(std::to_string((int)this->Minor()));
        str.push_back('.');
        str.append(std::to_string((int)this->Patch()));
        str.push_back('.');
        str.append(std::to_string((int)this->Build()));
        if (this->VersionStage() == TVMVersionStage::DevVersion) {
            str.append("-dev");
        } else if (this->VersionStage() == TVMVersionStage::AlphaVersion) {
            str.append("-alpha");
        } else if (this->VersionStage() == TVMVersionStage::BetaVersion) {
            str.append("-beta");
        } else if (this->VersionStage() == TVMVersionStage::ProductionVersion) {
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
void CrossArchiveCreate(
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs,
    std::shared_ptr<Tesses::Framework::Streams::Stream> strm, std::string name,
    TVMVersion version, std::string info, std::string icon = "");
/**
 * @brief
 *
 * @param strm the input stream
 * @param vfs vfs to extract to (as root)
 * @return std::pair<std::pair<std::string,TVMVersion>,std::string> ((name,
 * version),info)
 */
std::pair<std::pair<std::string, TVMVersion>, std::string>
CrossArchiveExtract(std::shared_ptr<Tesses::Framework::Streams::Stream> strm,
                    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs);

/**
 * @brief The token type for lexer
 *
 */
typedef enum { Identifier, Symbol, String, Char, Documentation } LexTokenType;
/**
 * @brief The line info for lextoken
 *
 */
class LexTokenLineInfo {
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
int Lex(std::string filename, std::istream &strm,
        std::vector<LexToken> &tokens);

using Undefined = std::monostate;
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
    JMPIFDEFINED,
    DECLARECONSTVARIABLE,
    LINEINFO,
    PUSHRESOURCESTREAM,
    PUSHRESOUURCEDIR,
    PUSHPRIVATEXPRESSION
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
    virtual size_t Size() = 0;
    /**
     * @brief Write the instruction to vector
     *
     * @param data the vector of uint8_t
     */
    virtual void Write(std::vector<uint8_t> &data) = 0;
    virtual ~ByteCodeInstruction() {}
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
    void Write(std::vector<uint8_t> &data);
};

class StringInstruction : public ByteCodeInstruction {
  public:
    uint32_t n;
    StringInstruction(uint32_t n);
    size_t Size();
    void Write(std::vector<uint8_t> &data);
};
class ScopeEndTimesInstruction : public ByteCodeInstruction {
  public:
    uint32_t n;
    ScopeEndTimesInstruction(uint32_t n);
    size_t Size();
    void Write(std::vector<uint8_t> &data);
};

class LabelInstruction : public ByteCodeInstruction {
  public:
    std::string label;
    LabelInstruction(std::string lbl);
    size_t Size();
    void Write(std::vector<uint8_t> &data);
};
class LabelableInstruction : public ByteCodeInstruction {
  public:
    uint32_t n;
    std::string label;
    virtual size_t Size();
    virtual void Write(std::vector<uint8_t> &data);
};

class JumpStyleInstruction : public LabelableInstruction {
  public:
    Instruction type;

    JumpStyleInstruction(Instruction conditional, std::string label);
    size_t Size();
    void Write(std::vector<uint8_t> &data);
};

class EmbedInstruction : public ByteCodeInstruction {
  public:
    uint32_t n;
    EmbedInstruction(uint32_t n);
    size_t Size();
    void Write(std::vector<uint8_t> &data);
};
class EmbedStreamInstruction : public ByteCodeInstruction {
  public:
    uint32_t n;
    EmbedStreamInstruction(uint32_t n);
    size_t Size();
    void Write(std::vector<uint8_t> &data);
};

class ClosureInstruction : public ByteCodeInstruction {
  public:
    bool hasScope;
    uint32_t n;
    ClosureInstruction(uint32_t n, bool hasScope = true);
    size_t Size();
    void Write(std::vector<uint8_t> &data);
};

class DoubleInstruction : public ByteCodeInstruction {
  public:
    double n;
    DoubleInstruction(double n);
    size_t Size();
    void Write(std::vector<uint8_t> &data);
};

class LongInstruction : public ByteCodeInstruction {
  public:
    int64_t n;
    LongInstruction(int64_t n);
    size_t Size();
    void Write(std::vector<uint8_t> &data);
};

class CharInstruction : public ByteCodeInstruction {
  public:
    char n;
    CharInstruction(char n);
    size_t Size();
    void Write(std::vector<uint8_t> &data);
};

using SyntaxNode =
    std::variant<Undefined, int64_t, double, std::string, char, bool,
                 std::nullptr_t, std::vector<uint8_t>, AdvancedSyntaxNode,
                 std::shared_ptr<Tesses::Framework::Streams::Stream>,
                 std::shared_ptr<Tesses::Framework::Filesystem::VFSPath>>;

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
    virtual uint32_t
    GetLength(std::shared_ptr<Tesses::Framework::Filesystem::VFS> embedFS) = 0;
    virtual void
    Write(std::shared_ptr<Tesses::Framework::Streams::Stream> output) = 0;
    virtual ~ResourceBase();
    virtual bool IsEqual(ResourceBase *base);
};

class ResourceFile : public ResourceBase {
    std::shared_ptr<Tesses::Framework::Streams::Stream> strm = nullptr;

  public:
    ResourceFile();
    ResourceFile(std::string file);
    std::string file;
    uint32_t
    GetLength(std::shared_ptr<Tesses::Framework::Filesystem::VFS> embedFS);
    void Write(std::shared_ptr<Tesses::Framework::Streams::Stream> output);
    bool IsEqual(ResourceBase *base);
    ~ResourceFile();
};

class ResourceByteArray : public ResourceBase {
  public:
    std::vector<uint8_t> data;
    uint32_t
    GetLength(std::shared_ptr<Tesses::Framework::Filesystem::VFS> embedFS);
    void Write(std::shared_ptr<Tesses::Framework::Streams::Stream> output);
};
class CodeGen {
    uint32_t id;
    uint32_t NewId();

    void GetFunctionArgs(std::vector<uint32_t> &args,
                         std::vector<ByteCodeInstruction *> &instructions,
                         SyntaxNode n);
    void FlattenCommas(std::vector<uint32_t> &args, SyntaxNode n);
    void GetFunctionName(std::vector<uint32_t> &name, SyntaxNode n);
    void FlattenCommas(std::vector<SyntaxNode> &args, SyntaxNode n);
    SyntaxNode StringifyListOfVars(SyntaxNode n);

    uint32_t GetString(std::string str);
    uint32_t GetResource(std::shared_ptr<ResourceBase> resource);
    std::vector<std::string> strs;
    std::vector<std::shared_ptr<ResourceBase>> res;

    std::vector<
        std::pair<std::vector<uint32_t>, std::vector<ByteCodeInstruction *>>>
        chunks;
    std::vector<std::pair<std::vector<uint32_t>, uint32_t>> funcs;
    std::vector<CodeGenClass> classes;

    std::vector<std::vector<uint8_t>> meta;
    SyntaxNode OptimizeNode(SyntaxNode n);
    void WriteMetadataObject(std::vector<uint8_t> &bytes, SyntaxNode n);
    void GenNode(std::vector<ByteCodeInstruction *> &instructions, SyntaxNode n,
                 int32_t scope, int32_t contscope, int32_t brkscope,
                 int32_t contI, int32_t brkI);
    void GenPop(std::vector<ByteCodeInstruction *> &instrs, SyntaxNode n);

  public:
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> embedFS;
    std::vector<std::pair<std::string, TVMVersion>> dependencies;
    std::vector<std::pair<std::string, TVMVersion>> tools;
    TVMVersion version;
    std::string name;
    std::string info;
    std::string icon;
    void GenRoot(SyntaxNode n);
    void Save(std::shared_ptr<Tesses::Framework::Streams::Stream> output);
    ~CodeGen();
};
/**
 * @brief A html expression
 *
 */
constexpr std::string_view HtmlRootExpression = "htmlRootExpression";
/**
 * @brief an intrinsic function that embeds a resource from the filename based
 * on the constant string argument
 *
 */
constexpr std::string_view EmbedExpression = "embedExpression";
/**
 * @brief an intrinsic function that embeds a resource as a stream from the
 * filename based on the constant string argument
 *
 */
constexpr std::string_view EmbedStreamExpression = "embedStreamExpression";
/**
 * @brief an intrinsic function that embeds a resource directory from the
 * directory name based on the constant string argument
 *
 */
constexpr std::string_view EmbedDirectoryExpression =
    "embedDirectoryExpression";
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
constexpr std::string_view PrefixIncrementExpression =
    "prefixIncrementExpression";
/**
 * @brief --i
 *
 */
constexpr std::string_view PrefixDecrementExpression =
    "prefixDecrementExpression";
/**
 * @brief i++
 *
 */
constexpr std::string_view PostfixIncrementExpression =
    "postfixIncrementExpression";
/**
 * @brief i--
 *
 */
constexpr std::string_view PostfixDecrementExpression =
    "postfixDecrementExpression";
/**
 * @brief Gets variable value
 *
 */
constexpr std::string_view GetVariableExpression = "getVariableExpression";
/**
 * @brief Get field or get property (EXPR.getNAME()) (unless it is wrapped in
 * AssignExpression) which then it sets the field or calls EXPR.setNAME(VALUE)
 *
 */
constexpr std::string_view GetFieldExpression = "getFieldExpression";
/**
 * @brief Syntax sugar for EXPR.GetAt(index) (unless it is wrapped in
 * AssignExpresion) which then it is EXPR.SetAt(index, VALUE)
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
constexpr std::string_view CompoundAssignExpression =
    "compoundAssignExpression";
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
 * @brief const v = 59;
 *
 */

constexpr std::string_view ConstExpression = "constExpression";
/**
 * @brief Closure expression (a,b)=> a * b
 *
 */
constexpr std::string_view ClosureExpression = "closureExpression";
/**
 * @brief Just like normal closures but doesn't create its own scope
 *
 */
constexpr std::string_view ScopelessClosureExpression =
    "scopelessClosureExpression";
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
 * @brief using statement using(EXPR)
 *
 */
constexpr std::string_view UsingStatement = "usingStatement";
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
constexpr std::string_view LessThanEqualsExpression =
    "lessThanEqualsExpression";
/**
 * @brief The greater than equals expression EXPR >= EXPR
 *
 */
constexpr std::string_view GreaterThanEqualsExpression =
    "greaterThanEqualsExpression";
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
 * @brief The relative path expression ./ "path" / "to" / "subdir" (fullfills
 * the ./)
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
constexpr std::string_view NullCoalescingExpression =
    "nullCoalescingExpression";
/**
 * @brief For debugging (store line info and filename)
 */
constexpr std::string_view LineNode = "lineNode";
/**
 * @brief For storing generic metadata
 */
constexpr std::string_view MetadataStatement = "MetadataStatement";
/**
 * @brief For storing private data (isolated to specific crvm file)
 */
constexpr std::string_view GetPrivateExpression = "getPrivateExpression";
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
    static AdvancedSyntaxNode Create(std::string_view nodeName,
                                     bool isExpression,
                                     std::vector<SyntaxNode> n);
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
    bool IsIdentifier(std::string txt, bool pop = true);
    bool IsAnyIdentifier(std::initializer_list<std::string> idents,
                         bool pop = true);
    bool IsSymbol(std::string txt, bool pop = true);
    bool IsAnySymbol(std::initializer_list<std::string> idents,
                     bool pop = true);

    SyntaxNode ParseExpression();
    SyntaxNode ParseTernary();
    SyntaxNode ParseNode(bool isRoot = false);
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
    void ParseHtml(std::vector<SyntaxNode> &nodes, std::string var);
    std::shared_ptr<GC> gc;
    TRootEnvironment *env;
    int lastLine = -1;
    std::string lastFile = "";
    bool CanEmit(LexTokenLineInfo &token);

  public:
    bool debug = true;
    /**
     * @brief Construct a new Parser object
     *
     * @param tokens the tokens from lexer
     */
    Parser(std::vector<LexToken> tokens);
    Parser(std::vector<LexToken> tokens, std::shared_ptr<GC> gc,
           TRootEnvironment *env);
    /**
     * @brief Turn tokens into abstract syntax tree
     *
     * @return SyntaxNode the abstract syntax tree
     */
    SyntaxNode ParseRoot() { return ParseNode(true); }
};
class THeapObject;
class CallStackEntry;
class InterperterThread;
class THeapObject {
  public:
    bool marked;
    virtual void Mark() { marked = true; }
    THeapObject() = default;
    THeapObject(const THeapObject &) = delete;
    THeapObject &operator=(const THeapObject &) = delete;
    virtual ~THeapObject() = default;
};

class THeapObjectHolder {
  public:
    THeapObject *obj;
    THeapObjectHolder(THeapObject *obj) { this->obj = obj; }
    THeapObjectHolder() {}
};

// this is a dummy type with
class MethodInvoker {};

class TBreak {};
class TContinue {};
/**
 * @brief A script object
 *
 */

using TObject =
    std::variant<Undefined, int64_t, double, char, bool, std::string,
                 std::regex, Tesses::Framework::Filesystem::VFSPath,
                 std::nullptr_t, MethodInvoker, THeapObjectHolder, TVMVersion,
                 std::shared_ptr<Tesses::Framework::Date::DateTime>,
                 std::shared_ptr<Tesses::Framework::Date::TimeSpan>, TBreak,
                 TContinue, std::shared_ptr<Tesses::Framework::Streams::Stream>,
                 std::shared_ptr<Tesses::Framework::Filesystem::VFS>,
                 std::shared_ptr<Tesses::Framework::Http::IHttpServer>,
                 std::shared_ptr<Tesses::Framework::Http::HttpRequestBody>,
                 std::shared_ptr<Tesses::Framework::TextStreams::TextReader>,
                 std::shared_ptr<Tesses::Framework::TextStreams::TextWriter>,
                 std::shared_ptr<Tesses::Framework::Streams::ByteReader>,
                 std::shared_ptr<Tesses::Framework::Streams::ByteWriter>,
                 std::shared_ptr<Tesses::Framework::Http::ServerSentEvents>,
                 std::shared_ptr<Tesses::Framework::TF_Timer_Handle>,
                 Tesses::Framework::Uuid>;

class TRootEnvironment;
class GCList;

class GC : public std::enable_shared_from_this<GC> {
    Tesses::Framework::Threading::Thread *thrd;
    Tesses::Framework::Threading::Mutex mtx;
    Tesses::Framework::Threading::Cond cond;
    uint32_t allocs = 0;
    volatile std::atomic<bool> running;
    std::unordered_set<GCList *> roots;
    std::unordered_set<THeapObject *> objects;

    Tesses::Framework::Lazy<Tesses::Framework::Threading::ThreadPool *> *tpool;
    std::vector<
        std::function<void(std::shared_ptr<GC> gc, TRootEnvironment *env)>>
        register_everything;

    void SetRoot(GCList *obj);
    void UnsetRoot(GCList *obj);

  public:
    Tesses::Framework::Threading::ThreadPool *GetPool();
    bool UsingNullThreads();
    GC();
    GC(const GC &) = delete;
    GC &operator=(const GC &) = delete;
    void Start();
    bool IsRunning();
    void BarrierBegin();
    void BarrierEnd();
    void Collect(std::vector<THeapObject *> &to_delete);
    void Watch(TObject obj);
    void Unwatch(TObject obj);
    static void Mark(TObject obj);
    void RegisterEverythingCallback(
        std::function<void(std::shared_ptr<GC> gc, TRootEnvironment *env)> cb);
    void RegisterEverything(TRootEnvironment *env);
    ~GC();

    // uses thread_local or std::map
    static void SetCurrentFunction(CallStackEntry *fn);
    // uses thread local or std::map
    static CallStackEntry *GetCurrentFunction();
    friend class GCList;
};

std::string GetObjectTypeString(TObject obj);
std::string ToString(std::shared_ptr<GC> gc, TObject obj);

class GCList {
    std::vector<THeapObject *> items;

    std::shared_ptr<GC> gc;

  public:
    GCList(std::shared_ptr<GC> gc);
    GCList(const GCList &) = delete;
    GCList &operator=(const GCList &) = delete;
    std::shared_ptr<GC> GetGC() const;

    template <typename T, typename... TArgs> T *Create(TArgs &&...args) {
        T *obj = new T(std::forward<TArgs>(args)...);
        Add(obj);
        gc->Watch(obj);
        return obj;
    }

    void Add(TObject v);
    void Remove(TObject v);
    void Mark();

    ~GCList();
    friend class GC;
};

class TFile;

class TFileChunk : public THeapObject {
  public:
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TFileChunk *Create(GCList *gc);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TFileChunk *Create(GCList &gc);
    TFile *file;
    std::vector<uint8_t> code;
    std::vector<std::string> args;
    std::optional<std::string> name;
    void Mark();
};

class TByteArray : public THeapObject {
  public:
    std::vector<uint8_t> data;
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TByteArray *Create(GCList *gc);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TByteArray *Create(GCList &gc);
};
enum class TClassModifier { Private, Protected, Public, Static };
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
class TDictionary;
class TFile : public THeapObject {
  public:
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TFile *Create(GCList *gc);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TFile *Create(GCList &gc);
    std::vector<TFileChunk *> chunks;

    std::vector<std::string> strings;
    std::vector<std::pair<std::string, std::string>> vms;
    std::vector<std::pair<std::vector<std::string>, uint32_t>> functions;
    std::vector<std::pair<std::string, TVMVersion>> dependencies;
    std::vector<std::pair<std::string, TVMVersion>> tools;
    std::vector<std::pair<std::string, std::vector<uint8_t>>> sections;
    std::vector<std::pair<std::string, std::vector<uint8_t>>> metadata;
    std::vector<std::vector<uint8_t>> resources;
    std::vector<TClass> classes;
    std::string name;
    TVMVersion version;
    std::string info;
    int32_t icon = -1;

    void Load(std::shared_ptr<GC> gc,
              std::shared_ptr<Tesses::Framework::Streams::Stream> strm);
    void Skip(std::shared_ptr<Tesses::Framework::Streams::Stream> strm,
              size_t len);
    void Ensure(std::shared_ptr<Tesses::Framework::Streams::Stream> strm,
                uint8_t *buffer, size_t len);
    uint32_t
    EnsureInt(std::shared_ptr<Tesses::Framework::Streams::Stream> strm);
    std::string
    EnsureString(std::shared_ptr<Tesses::Framework::Streams::Stream> strm);

    std::string
    GetString(std::shared_ptr<Tesses::Framework::Streams::Stream> strm);
    void Mark();

    void EnsureCanRunInCrossLang();

    TDictionary *MetadataDecode(GCList &ls, size_t index);
};
class TAssociativeArray : public THeapObject {
  public:
    std::vector<std::pair<TObject, TObject>> items;
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TAssociativeArray *Create(GCList &ls);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TAssociativeArray *Create(GCList *ls);
    void Set(std::shared_ptr<GC> gc, TObject key, TObject value);
    TObject Get(std::shared_ptr<GC> gc, TObject key);
    TObject GetKey(int64_t index);
    TObject GetValue(int64_t index);
    void SetKey(int64_t index, TObject key);
    void SetValue(int64_t index, TObject value);
    int64_t Count();
    void Mark();
};

class TList : public THeapObject {
  public:
    TList() = default;
    template <typename Itterator>
    TList(Itterator begin, Itterator end) : items(begin, end) {}
    TList(std::initializer_list<TObject> il) : items(il) {}
    TList(int64_t capacity) { items.reserve(capacity); }
    std::vector<TObject> items;
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TList *Create(GCList *gc);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TList *Create(GCList &gc);
    template <typename Itterator>
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TList *Create(GCList *gc, Itterator begin, Itterator end) {
        return gc->Create<TList>(begin, end);
    }
    template <typename Itterator>
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TList *Create(GCList &gc, Itterator begin, Itterator end) {
        return gc.Create<TList>(begin, end);
    }
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TList *Create(GCList *gc, std::initializer_list<TObject> il) {
        return gc->Create<TList>(il);
    }
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TList *Create(GCList &gc, std::initializer_list<TObject> il) {
        return gc.Create<TList>(il);
    }
    virtual int64_t Count();
    virtual TObject Get(int64_t index);
    virtual void Set(int64_t index, TObject value);
    virtual void Insert(int64_t index, TObject value);
    virtual void Add(TObject value);
    virtual void RemoveAt(int64_t index);
    virtual void Clear();
    virtual void Mark();
};
using TDItem = std::pair<std::string, TObject>;
class TDictionary : public THeapObject {
  public:
    TDictionary() = default;
    template <typename Itterator> TDictionary(Itterator begin, Itterator end) {
        for (Itterator i = begin; i != end; ++i) {
            TDItem &item = *i;
            SetValue(item.first, item.second);
        }
    }
    TDictionary(std::initializer_list<TDItem> il) {
        for (auto &item : il) {
            SetValue(item.first, item.second);
        }
    }
    std::map<std::string, TObject> items;
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TDictionary *Create(GCList *gc);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TDictionary *Create(GCList &gc);
    template <typename Itterator>
    static TDictionary *Create(GCList *gc, Itterator begin, Itterator end) {
        return gc->Create<TDictionary>(begin, end);
    }
    template <typename Itterator>
    static TDictionary *Create(GCList &gc, Itterator begin, Itterator end) {
        return gc.Create<TDictionary>(begin, end);
    }

    static TDictionary *Create(GCList *gc, std::initializer_list<TDItem> il) {
        return gc->Create<TDictionary>(il);
    }
    static TDictionary *Create(GCList &gc, std::initializer_list<TDItem> il) {
        return gc.Create<TDictionary>(il);
    }

    virtual bool HasValue(std::string key);
    virtual bool MethodExists(GCList &ls, std::string key);
    virtual TObject GetValue(std::string key);
    virtual void SetValue(std::string key, TObject value);
    virtual void Mark();
    void DeclareFunction(
        std::shared_ptr<GC> gc, std::string key, std::string documentation,
        std::vector<std::string> argNames,
        std::function<TObject(GCList &ls, std::vector<TObject> args)> cb);
    void DeclareFunction(
        std::shared_ptr<GC> gc, std::string key, std::string documentation,
        std::vector<std::string> argNames,
        std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
        std::function<void()> destroy);
    TObject CallMethod(GCList &ls, std::string key, std::vector<TObject> args);
    TObject CallMethodWithFatalError(GCList &ls, std::string key,
                                     std::vector<TObject> args);
};

class TRootEnvironment;
class TSubEnvironment;

class TCallable : public THeapObject {
  public:
    TObject tag;
    std::string documentation;
    virtual TObject Call(GCList &ls, std::vector<TObject> args) = 0;
    TObject CallWithFatalError(GCList &ls, std::vector<TObject> args);
    virtual void Mark();

    Tesses::Framework::Http::ServerRequestHandler
    ToRouteServerRequestHandler(std::shared_ptr<GC> gc);
};

void ThrowFatalError(std::exception &ex);

void ThrowConstError(std::string key);
enum class TEnvironmentDeclType { TEDT_VAR, TEDT_CONST };

class TEnvironment : public THeapObject {
  protected:
    std::unordered_map<std::string, TEnvironmentDeclType> decls;

  public:
    std::vector<TCallable *> defers;
    TObject LoadFile(std::shared_ptr<GC> gc, TFile *f);
    TObject Eval(GCList &ls, std::string code);
    virtual bool HasVariable(std::string key) = 0;
    virtual bool HasVariableRecurse(std::string key) = 0;

    virtual bool HasVariableOrFieldRecurse(std::string key,
                                           bool setting = false) = 0;
    virtual TObject GetVariable(std::string key) = 0;

    virtual TObject GetVariable(GCList &ls, std::string key) = 0;
    virtual TObject SetVariable(GCList &ls, std::string key, TObject v) = 0;

    virtual void SetVariable(std::string key, TObject value) = 0;
    TDictionary *EnsureDictionary(std::shared_ptr<GC> gc, std::string key);
    virtual void DeclareVariable(std::string key, TObject value) = 0;
    void DeclareConstVariable(std::string key, TObject value);
    bool HasConstForDeclare(std::string key);
    virtual bool HasConstForSet(std::string key);
    void DeclareVariable(std::shared_ptr<GC> gc, std::vector<std::string> key,
                         TObject value);
    virtual TRootEnvironment *GetRootEnvironment() = 0;
    virtual TEnvironment *GetParentEnvironment() = 0;
    virtual TSubEnvironment *GetSubEnvironment(TDictionary *dict);
    TSubEnvironment *GetSubEnvironment(GCList *gc);
    TSubEnvironment *GetSubEnvironment(GCList &gc);

    void DeclareFunction(
        std::shared_ptr<GC> gc, std::string key, std::string documentation,
        std::vector<std::string> argNames,
        std::function<TObject(GCList &ls, std::vector<TObject> args)> cb);
    void DeclareFunction(
        std::shared_ptr<GC> gc, std::string key, std::string documentation,
        std::vector<std::string> argNames,
        std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
        std::function<void()> destroy);
    TObject CallFunction(GCList &ls, std::string key,
                         std::vector<TObject> args);
    TObject CallFunctionWithFatalError(GCList &ls, std::string key,
                                       std::vector<TObject> args);
};
class TClassEnvironment;
class TClassObject : public THeapObject {
    TClassObjectEntry *GetEntry(std::string classN, std::string key);

  public:
    TFile *file;
    uint32_t classIndex;
    TEnvironment *ogEnv;
    TClassEnvironment *env;
    std::string name;
    std::vector<std::string> inherit_tree;
    std::vector<TClassObjectEntry> entries;

    // This is the only place where the old way is
    static TClassObject *Create(GCList &ls, TFile *f, uint32_t classIndex,
                                TEnvironment *env, std::vector<TObject> args);
    // this too
    static TClassObject *Create(GCList *ls, TFile *f, uint32_t classIndex,
                                TEnvironment *env, std::vector<TObject> args);

    TObject GetValue(std::string className, std::string name);
    void SetValue(std::string className, std::string name, TObject value);
    bool HasValue(std::string className, std::string name);
    bool HasField(std::string className, std::string name);
    bool HasMethod(std::string className, std::string name);
    TObject CallMethod(GCList &ls, std::string className, std::string name,
                       std::vector<TObject> args);
    std::string TypeName();
    void Mark();
};
class TClassEnvironment : public TEnvironment {
    TEnvironment *env;
    TClassObject *clsObj;

  public:
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TClassEnvironment *Create(GCList *gc, TEnvironment *env,
                                     TClassObject *obj);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TClassEnvironment *Create(GCList &gc, TEnvironment *env,
                                     TClassObject *obj);
    TClassEnvironment(TEnvironment *env, TClassObject *obj);
    bool HasVariable(std::string key);
    bool HasVariableRecurse(std::string key);
    bool HasVariableOrFieldRecurse(std::string key, bool setting = false);

    TObject GetVariable(std::string key);
    void SetVariable(std::string key, TObject value);
    TObject GetVariable(GCList &ls, std::string key);
    TObject SetVariable(GCList &ls, std::string key, TObject v);

    void DeclareVariable(std::string key, TObject value);
    bool HasConstForSet(std::string key);

    TRootEnvironment *GetRootEnvironment();
    TEnvironment *GetParentEnvironment();

    void Mark();
};

class EnvironmentPermissions {
  public:
    EnvironmentPermissions();
    Tesses::Framework::Filesystem::VFSPath sqliteOffsetPath;
    std::shared_ptr<Tesses::Framework::Filesystem::RelativeFilesystem> localfs;
    bool canRegisterEverything;
    bool canRegisterConsole;
    bool canRegisterIO;
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
    TDictionary *customConsole = nullptr;
};

class TRootEnvironment : public TEnvironment {
    TDictionary *dict;
    TCallable *error = nullptr;
    std::map<uint64_t, TDictionary *> private_file_data;

    void LoadDependency(std::shared_ptr<GC> gc,
                        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs,
                        std::pair<std::string, TVMVersion> dep);

  public:
    EnvironmentPermissions permissions;

    std::vector<std::pair<std::string, TVMVersion>> dependencies;
    std::vector<std::pair<TFile *, uint32_t>> classes;

    TDictionary *GetPrivateFromFile(std::shared_ptr<GC> gc, TFile *file);

    bool TryFindClass(std::vector<std::string> &name, size_t &index);

    void LoadFileWithDependencies(
        std::shared_ptr<GC> gc,
        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs, TFile *f);
    void LoadFileWithDependencies(
        std::shared_ptr<GC> gc,
        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs,
        Tesses::Framework::Filesystem::VFSPath path);

    TDictionary *GetDictionary();
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TRootEnvironment *Create(GCList *gc, TDictionary *dict);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TRootEnvironment *Create(GCList &gc, TDictionary *dict);
    TRootEnvironment(TDictionary *dict);
    bool HasVariable(std::string key);
    bool HasVariableRecurse(std::string key);
    bool HasVariableOrFieldRecurse(std::string key, bool setting = false);

    TObject GetVariable(std::string key);
    void SetVariable(std::string key, TObject value);

    TObject GetVariable(GCList &ls, std::string key);
    TObject SetVariable(GCList &ls, std::string key, TObject v);
    void DeclareVariable(std::string key, TObject value);
    TRootEnvironment *GetRootEnvironment();
    TEnvironment *GetParentEnvironment();
    void RegisterOnError(TCallable *callable);
    bool HandleException(std::shared_ptr<GC> gc, TEnvironment *env,
                         TObject err);
    bool HandleBreakpoint(std::shared_ptr<GC> gc, TEnvironment *env,
                          TObject err);
    void Mark();
};
class TStd {
  private:
    static void RegisterHelpers(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterUuid(std::shared_ptr<GC> gc, TRootEnvironment *env);

  public:
    static void RegisterStd(
        std::shared_ptr<GC> gc, TRootEnvironment *env,
        std::shared_ptr<Tesses::Framework::Filesystem::RelativeFilesystem>
            localfs);
    static void RegisterStd(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterConsole(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterIO(
        std::shared_ptr<GC> gc, TRootEnvironment *env,
        std::shared_ptr<Tesses::Framework::Filesystem::RelativeFilesystem>
            local);
    static void RegisterIO(std::shared_ptr<GC> gc, TRootEnvironment *env,
                           bool enableLocalFS = true);
    static void RegisterNet(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterSqlite(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterVM(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterDictionary(std::shared_ptr<GC> gc,
                                   TRootEnvironment *env);
    static void RegisterJson(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterCrypto(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterRoot(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterPath(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterOGC(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterEnv(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterProcess(std::shared_ptr<GC> gc, TRootEnvironment *env);
    static void RegisterClass(std::shared_ptr<GC> gc, TRootEnvironment *env);
};

class TSubEnvironment : public TEnvironment {
    TEnvironment *env;
    TDictionary *dict;

  public:
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TSubEnvironment *Create(GCList *gc, TEnvironment *env,
                                   TDictionary *dict);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TSubEnvironment *Create(GCList &gc, TEnvironment *env,
                                   TDictionary *dict);
    TSubEnvironment(TEnvironment *env, TDictionary *dict);
    bool HasVariable(std::string key);
    bool HasVariableRecurse(std::string key);
    bool HasVariableOrFieldRecurse(std::string key, bool setting = false);
    bool HasConstForSet(std::string key);

    TObject GetVariable(std::string key);
    void SetVariable(std::string key, TObject value);
    TObject GetVariable(GCList &ls, std::string key);
    TObject SetVariable(GCList &ls, std::string key, TObject v);

    void DeclareVariable(std::string key, TObject value);
    TRootEnvironment *GetRootEnvironment();
    TEnvironment *GetParentEnvironment();
    TDictionary *GetDictionary();
    void Mark();
};

TDictionary *CreateThread(GCList &ls, TCallable *callable, bool detached);

class TArgWrapper : public TCallable {
  public:
    TCallable *callable;
    TArgWrapper(TCallable *callable);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TArgWrapper *Create(GCList &ls, TCallable *callable);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TArgWrapper *Create(GCList *ls, TCallable *callable);
    TObject Call(GCList &ls, std::vector<TObject> args);
    void Mark();
};

class TExternalMethod : public TCallable {
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb;
    std::function<void()> destroy;

  public:
    std::vector<std::string> args;
    std::vector<TObject> watch;
    TExternalMethod(
        std::string documentation, std::vector<std::string> argNames,
        std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
        std::function<void()> destroy = nullptr);

    [[deprecated("Use GCList::Create<T>() instead")]]
    static TExternalMethod *
    Create(GCList &ls, std::string documentation,
           std::vector<std::string> argNames,
           std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
           std::function<void()> destroy);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TExternalMethod *
    Create(GCList *ls, std::string documentation,
           std::vector<std::string> argNames,
           std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
           std::function<void()> destroy);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TExternalMethod *
    Create(GCList &ls, std::string documentation,
           std::vector<std::string> argNames,
           std::function<TObject(GCList &ls, std::vector<TObject> args)> cb);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TExternalMethod *
    Create(GCList *ls, std::string documentation,
           std::vector<std::string> argNames,
           std::function<TObject(GCList &ls, std::vector<TObject> args)> cb);
    TObject Call(GCList &ls, std::vector<TObject> args);

    ~TExternalMethod();
    void Mark() {
        if (this->marked)
            return;
        this->marked = true;
        for (auto item : watch)
            GC::Mark(item);
        GC::Mark(this->tag);
    }
};

class TEnumerator : public THeapObject {
  public:
    virtual bool MoveNext(std::shared_ptr<GC> ls) = 0;
    virtual TObject GetCurrent(GCList &ls) = 0;
    static TEnumerator *CreateFromObject(GCList &ls, TObject obj);
};
class TAssociativeArrayEnumerator : public TEnumerator {
    int64_t index;
    TAssociativeArray *ls;

  public:
    TAssociativeArrayEnumerator(TAssociativeArray *list);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TAssociativeArrayEnumerator *Create(GCList &ls,
                                               TAssociativeArray *list);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TAssociativeArrayEnumerator *Create(GCList *ls,
                                               TAssociativeArray *list);
    bool MoveNext(std::shared_ptr<GC> ls);
    TObject GetCurrent(GCList &ls);
    void Mark();
};

class TCustomEnumerator : public TEnumerator {
  public:
    TDictionary *dict;
    TCustomEnumerator(TDictionary *dict);
    bool MoveNext(std::shared_ptr<GC> ls);
    TObject GetCurrent(GCList &ls);
    void Mark();
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TCustomEnumerator *Create(GCList &ls, TDictionary *dict);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TCustomEnumerator *Create(GCList *ls, TDictionary *dict);
};

class TYieldEnumerator : public TEnumerator {
    bool hasStarted;
    TObject enumerator;
    TObject current;

  public:
    TYieldEnumerator(TObject v);
    bool MoveNext(std::shared_ptr<GC> ls);
    TObject GetCurrent(GCList &ls);
    void Mark();
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TYieldEnumerator *Create(GCList &ls, TObject v);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TYieldEnumerator *Create(GCList *ls, TObject v);
};

class TStringEnumerator : public TEnumerator {
    bool hasStarted;
    size_t index;
    std::string str;

  public:
    TStringEnumerator(std::string str);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TStringEnumerator *Create(GCList &ls, std::string str);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TStringEnumerator *Create(GCList *ls, std::string str);
    bool MoveNext(std::shared_ptr<GC> ls);
    TObject GetCurrent(GCList &ls);
};

class TListEnumerator : public TEnumerator {
    int64_t index;
    TList *ls;

  public:
    TListEnumerator(TList *list);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TListEnumerator *Create(GCList &ls, TList *list);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TListEnumerator *Create(GCList *ls, TList *list);
    bool MoveNext(std::shared_ptr<GC> ls);
    TObject GetCurrent(GCList &ls);
    void Mark();
};
class TDynamicList;
class TDynamicDictionary;
class TDynamicListEnumerator : public TEnumerator {
    int64_t index;
    TDynamicList *ls;

  public:
    TDynamicListEnumerator(TDynamicList *list);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TDynamicListEnumerator *Create(GCList &ls, TDynamicList *list);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TDynamicListEnumerator *Create(GCList *ls, TDynamicList *list);
    bool MoveNext(std::shared_ptr<GC> ls);
    TObject GetCurrent(GCList &ls);
    void Mark();
};
class TVFSPathEnumerator : public TEnumerator {
    Tesses::Framework::Filesystem::VFSPathEnumerator enumerator;

  public:
    TVFSPathEnumerator(Tesses::Framework::Filesystem::VFSPathEnumerator list);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TVFSPathEnumerator *
    Create(GCList &ls, Tesses::Framework::Filesystem::VFSPathEnumerator list);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TVFSPathEnumerator *
    Create(GCList *ls, Tesses::Framework::Filesystem::VFSPathEnumerator list);
    bool MoveNext(std::shared_ptr<GC> ls);
    TObject GetCurrent(GCList &ls);
};

class TObjectVFS : public Tesses::Framework::Filesystem::VFS {
  public:
    TObject obj;
    GCList *ls;
    TObjectVFS(std::shared_ptr<GC> gc, TObject obj);
    std::shared_ptr<Tesses::Framework::Streams::Stream>
    OpenFile(Tesses::Framework::Filesystem::VFSPath path, std::string mode);
    Tesses::Framework::Filesystem::FIFOCreationResult
    CreateFIFO(Tesses::Framework::Filesystem::VFSPath path);
    void CreateDirectory(Tesses::Framework::Filesystem::VFSPath path);
    void DeleteDirectory(Tesses::Framework::Filesystem::VFSPath path);
    void CreateSymlink(Tesses::Framework::Filesystem::VFSPath existingFile,
                       Tesses::Framework::Filesystem::VFSPath symlinkFile);
    void CreateHardlink(Tesses::Framework::Filesystem::VFSPath existingFile,
                        Tesses::Framework::Filesystem::VFSPath newName);
    void DeleteFile(Tesses::Framework::Filesystem::VFSPath path);
    void Lock(Tesses::Framework::Filesystem::VFSPath path);
    void Unlock(Tesses::Framework::Filesystem::VFSPath path);

    void DeleteDirectoryRecurse(Tesses::Framework::Filesystem::VFSPath path);
    Tesses::Framework::Filesystem::VFSPathEnumerator
    EnumeratePaths(Tesses::Framework::Filesystem::VFSPath path);
    void MoveFile(Tesses::Framework::Filesystem::VFSPath src,
                  Tesses::Framework::Filesystem::VFSPath dest);
    void MoveDirectory(Tesses::Framework::Filesystem::VFSPath src,
                       Tesses::Framework::Filesystem::VFSPath dest);
    Tesses::Framework::Filesystem::VFSPath
    ReadLink(Tesses::Framework::Filesystem::VFSPath path);
    std::string VFSPathToSystem(Tesses::Framework::Filesystem::VFSPath path);
    Tesses::Framework::Filesystem::VFSPath SystemToVFSPath(std::string path);
    void SetDate(Tesses::Framework::Filesystem::VFSPath path,
                 Tesses::Framework::Date::DateTime lastWrite,
                 Tesses::Framework::Date::DateTime lastAccess);
    void Chmod(Tesses::Framework::Filesystem::VFSPath path, uint32_t mode);
    void Chown(Tesses::Framework::Filesystem::VFSPath path, uint32_t uid,
               uint32_t gid);

    bool StatVFS(Tesses::Framework::Filesystem::VFSPath path,
                 Tesses::Framework::Filesystem::StatVFSData &data);

    bool Stat(Tesses::Framework::Filesystem::VFSPath path,
              Tesses::Framework::Filesystem::StatData &data);
    void Close();
    ~TObjectVFS();
};

class TObjectStream : public Tesses::Framework::Streams::Stream {
    int64_t _GetPosInternal();

  public:
    TObject obj;
    GCList *ls;
    TObjectStream(std::shared_ptr<GC> gc, TObject obj);
    bool EndOfStream();
    size_t Read(uint8_t *buff, size_t sz);
    size_t Write(const uint8_t *buff, size_t sz);
    bool CanRead();
    bool CanWrite();
    bool CanSeek();
    int64_t GetPosition();
    int64_t GetLength();
    void Flush();
    void Seek(int64_t pos, Tesses::Framework::Streams::SeekOrigin whence);
    void Close();
    ~TObjectStream();
};

class TObjectHttpServer : public Tesses::Framework::Http::IHttpServer {
  public:
    TObject obj;
    GCList *ls;
    TObjectHttpServer(std::shared_ptr<GC> gc, TObject obj);
    bool Handle(Tesses::Framework::Http::ServerContext &ctx);
    ~TObjectHttpServer();
};

class TDictionaryEnumerator : public TEnumerator {
    bool hasStarted;
    std::map<std::string, Tesses::CrossLang::TObject>::iterator ittr;
    TDictionary *dict;

  public:
    TDictionaryEnumerator(TDictionary *dict);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TDictionaryEnumerator *Create(GCList &ls, TDictionary *dict);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TDictionaryEnumerator *Create(GCList *ls, TDictionary *dict);
    bool MoveNext(std::shared_ptr<GC> ls);
    TObject GetCurrent(GCList &ls);
    void Mark();
};

class TClosure : public TCallable {
  public:
    TClosure(TEnvironment *env, TFile *file, uint32_t chunkId,
             bool ownScope = true);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TClosure *Create(GCList &ls, TEnvironment *env, TFile *file,
                            uint32_t chunkId, bool ownScope = true);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TClosure *Create(GCList *ls, TEnvironment *env, TFile *file,
                            uint32_t chunkId, bool ownScope = true);
    bool ownScope;
    TFileChunk *closure;
    TFile *file;
    uint32_t chunkId;
    TEnvironment *env;
    std::string className;
    TObject Call(GCList &ls, std::vector<TObject> args);
    void Mark();
};
class TDynamicList : public THeapObject {
  public:
    TCallable *cb;
    TDynamicList(TCallable *callable);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TDynamicList *Create(GCList &ls, TCallable *callable);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TDynamicList *Create(GCList *ls, TCallable *callable);

    void Mark();

    int64_t Count(GCList &ls);

    TObject GetAt(GCList &ls, int64_t index);

    TObject SetAt(GCList &ls, int64_t index, TObject val);

    TObject Add(GCList &ls, TObject v);
    TObject Insert(GCList &ls, int64_t index, TObject v);
    TObject RemoveAllEqual(GCList &ls, TObject v);
    TObject Remove(GCList &ls, TObject v);
    TObject RemoveAt(GCList &ls, int64_t v);
    TObject Clear(GCList &ls);
    TObject ToString(GCList &ls);

    ~TDynamicList();
};
class TDynamicDictionary : public THeapObject {
  public:
    TCallable *cb;
    TDynamicDictionary(TCallable *callable);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TDynamicDictionary *Create(GCList &ls, TCallable *callable);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TDynamicDictionary *Create(GCList *ls, TCallable *callable);

    void Mark();

    TObject GetField(GCList &ls, std::string key);

    TObject SetField(GCList &ls, std::string key, TObject value);

    TObject CallMethod(GCList &ls, std::string name, std::vector<TObject> args);

    bool MethodExists(GCList &ls, std::string name);

    TEnumerator *GetEnumerator(GCList &ls);

    ~TDynamicDictionary();
};
class InterperterThread;

class CallStackEntry : public THeapObject {
  public:
    [[deprecated("Use GCList::Create<T>() instead")]]
    static CallStackEntry *Create(GCList *ls);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static CallStackEntry *Create(GCList &ls);
    InterperterThread *thread = nullptr;
    std::vector<TObject> stack;
    TEnvironment *env = nullptr;
    TClosure *callable = nullptr;
    uint32_t ip;
    uint32_t scopes;
    int64_t srcline = -1;
    std::string srcfile = "";
    bool mustReturn = false;

    void Mark();
    void Push(std::shared_ptr<GC> gc, TObject v);
    TObject Pop(GCList &gcl);

    TObject Resume(GCList &ls);
};
class TTask : public THeapObject {
    TCallable *cont = nullptr;
    std::exception_ptr ex = nullptr;
    std::shared_ptr<GC> gc;
    TObject obj = Undefined();
    bool isCompleted = false;
    TTask(std::shared_ptr<GC> gc);

  public:
    // this still is needed
    static TTask *Create(GCList &ls);
    bool IsCompleted();

    void ContinueWith(TCallable *callable);
    TTask *ContinueWith(GCList &ls, TCallable *callable);

    void SetFailed(std::exception_ptr ex);
    void SetSucceeded(TObject v);

    TObject Wait();
    void Mark();

    static TTask *FromClosure(GCList &ls, TClosure *closure);

    static TTask *FromCallStackEntry(GCList &ls, CallStackEntry *ent);

    static TTask *Run(GCList &ls, TCallable *callable);

    static TTask *FromResult(GCList &ls, TObject v);
};

class InterperterThread : public THeapObject {
  private:
    bool InvokeTwo(GCList &ls, TObject fn, TObject left, TObject right);
    bool InvokeOne(GCList &ls, TObject fn, TObject arg);
    bool InvokeMethod(GCList &ls, TObject fn, TObject instance,
                      std::vector<TObject> args);
    bool ExecuteMethod2(std::shared_ptr<GC> gc, TObject instance,
                        std::string key, std::vector<TObject> args);

  protected:
    static void *ThreadCallback(void *ptr);
    bool JumpIfDefined(std::shared_ptr<GC> gc);
    bool Add(std::shared_ptr<GC> gc);
    bool Sub(std::shared_ptr<GC> gc);
    bool Times(std::shared_ptr<GC> gc);
    bool Divide(std::shared_ptr<GC> gc);
    bool Mod(std::shared_ptr<GC> gc);
    bool Neg(std::shared_ptr<GC> gc);
    bool Lt(std::shared_ptr<GC> gc);
    bool Gt(std::shared_ptr<GC> gc);
    bool Lte(std::shared_ptr<GC> gc);
    bool Gte(std::shared_ptr<GC> gc);
    bool Eq(std::shared_ptr<GC> gc);
    bool NEq(std::shared_ptr<GC> gc);
    bool LShift(std::shared_ptr<GC> gc);
    bool RShift(std::shared_ptr<GC> gc);
    bool BOr(std::shared_ptr<GC> gc);
    bool XOr(std::shared_ptr<GC> gc);
    bool LNot(std::shared_ptr<GC> gc);
    bool BNot(std::shared_ptr<GC> gc);
    bool BAnd(std::shared_ptr<GC> gc);
    bool ExecuteFunction(std::shared_ptr<GC> gc);
    bool ExecuteMethod(std::shared_ptr<GC> gc);
    bool GetVariable(std::shared_ptr<GC> gc);
    bool SetVariable(std::shared_ptr<GC> gc);
    bool GetField(std::shared_ptr<GC> gc);
    bool SetField(std::shared_ptr<GC> gc);
    bool GetArray(std::shared_ptr<GC> gc);
    bool SetArray(std::shared_ptr<GC> gc);
    bool DeclareVariable(std::shared_ptr<GC> gc);
    bool DeclareConstVariable(std::shared_ptr<GC> gc);
    bool PushLong(std::shared_ptr<GC> gc);
    bool PushDouble(std::shared_ptr<GC> gc);
    bool PushChar(std::shared_ptr<GC> gc);
    bool PushString(std::shared_ptr<GC> gc);
    bool PushClosure(std::shared_ptr<GC> gc);
    bool PushScopelessClosure(std::shared_ptr<GC> gc);
    bool PushResource(std::shared_ptr<GC> gc);
    bool Illegal(std::shared_ptr<GC> gc);
    bool Throw(std::shared_ptr<GC> gc);
    bool Yield(std::shared_ptr<GC> gc);
    bool Jump(std::shared_ptr<GC> gc);
    bool JumpConditional(std::shared_ptr<GC> gc);
    bool JumpUndefined(std::shared_ptr<GC> gc);
    bool Defer(std::shared_ptr<GC> gc);
    bool TryCatch(std::shared_ptr<GC> gc);
    bool Return(std::shared_ptr<GC> gc);
    bool ScopeBegin(std::shared_ptr<GC> gc);
    bool ScopeEnd(std::shared_ptr<GC> gc);
    bool ScopeEndTimes(std::shared_ptr<GC> gc);
    bool PushTrue(std::shared_ptr<GC> gc);
    bool PushFalse(std::shared_ptr<GC> gc);
    bool PushNull(std::shared_ptr<GC> gc);
    bool PushUndefined(std::shared_ptr<GC> gc);
    bool CreateDictionary(std::shared_ptr<GC> gc);
    bool CreateArray(std::shared_ptr<GC> gc);
    bool AppendList(std::shared_ptr<GC> gc);
    bool AppendDictionary(std::shared_ptr<GC> gc);
    bool PushRootPath(std::shared_ptr<GC> gc);
    bool PushRelativePath(std::shared_ptr<GC> gc);
    bool Pop(std::shared_ptr<GC> gc);
    bool Dup(std::shared_ptr<GC> gc);
    bool Nop(std::shared_ptr<GC> gc);
    bool Breakpoint(std::shared_ptr<GC> gc);
    bool PushBreak(std::shared_ptr<GC> gc);
    bool PushContinue(std::shared_ptr<GC> gc);
    bool JumpIfBreak(std::shared_ptr<GC> gc);
    bool JumpIfContinue(std::shared_ptr<GC> gc);
    bool LineInfo(std::shared_ptr<GC> gc);
    bool PushResourceStream(std::shared_ptr<GC> gc);
    bool PushResourceDirectory(std::shared_ptr<GC> gc);
    bool PushPrivateExpression(std::shared_ptr<GC> gc);

  public:
    [[deprecated("Use GCList::Create<T>() instead")]]
    static InterperterThread *Create(GCList *ls);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static InterperterThread *Create(GCList &ls);
    std::vector<CallStackEntry *> call_stack_entries;
    virtual void Execute(std::shared_ptr<GC> gc);
    void AddCallStackEntry(GCList &ls, TClosure *closure,
                           std::vector<TObject> args);
    void Mark();
};

class VMException : public std::exception {

    std::string error_message;

  public:
    VMException(std::string ex) {
        error_message = "VMException: ";
        error_message.append(ex);
    }

    const std::string &GetMessage() { return error_message; }

    const char *what() const noexcept override { return error_message.c_str(); }
};
class TAny : public THeapObject {
  public:
    std::any any;
    TObject other;
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TAny *Create(GCList &ls);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TAny *Create(GCList *ls);
    void Mark();
};
class TNativeObject : public THeapObject {
  public:
    template <typename T, typename... TArgs>
    [[deprecated("Use GCList::Create<T>() instead")]]
    static T *Create(GCList &ls, TArgs &&...args) {
        return ls.Create<T>(std::forward<TArgs>(args)...);
    }
    template <typename T, typename... TArgs>
    [[deprecated("Use GCList::Create<T>() instead")]]
    static T *Create(GCList *ls, TArgs &&...args) {
        return ls->Create<T>(std::forward<TArgs>(args)...);
    }

    virtual TObject CallMethod(GCList &ls, std::string name,
                               std::vector<TObject> args) = 0;
    virtual std::string TypeName() = 0;
    virtual bool ToBool();
    virtual bool Equals(std::shared_ptr<GC> gc, TObject right);
};
class TRandom : public TNativeObject {
  public:
    Tesses::Framework::Random random;
    TRandom();
    TRandom(uint64_t seed);
    std::string TypeName();
    TObject CallMethod(GCList &ls, std::string name, std::vector<TObject> args);
};
class TNative : public THeapObject {
    std::atomic<bool> destroyed;
    void *ptr;
    std::function<void(void *)> destroy;

  public:
    TObject other;
    TNative(void *ptr, std::function<void(void *)> destroy = nullptr);
    bool GetDestroyed();
    void *GetPointer();
    void Destroy();
    void Mark();
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TNative *Create(GCList &ls, void *ptr,
                           std::function<void(void *)> destroy);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TNative *Create(GCList *ls, void *ptr,
                           std::function<void(void *)> destroy);
    ~TNative();
};

class ThreadHandle : public THeapObject {
  public:
    Tesses::Framework::Threading::Thread *thrd;
    std::atomic<bool> hasInit;
    std::atomic<bool> hasReturned;
    std::atomic<bool> detached;
    TCallable *callable;
    TObject returnValue;
    std::shared_ptr<GC> gc;

    void Mark() {
        if (this->marked)
            return;
        this->marked = true;
        callable->Mark();
        GC::Mark(returnValue);
    }
    ~ThreadHandle() {
        if (this->detached) {
            this->thrd->Join();
            delete this->thrd;
        }
    }
};

TObject ExecuteFunction(GCList &ls, TCallable *callable,
                        std::vector<TObject> args);
class VMByteCodeException : public std::exception {
    std::string lastErrorText;
    std::shared_ptr<GCList> gcList;

  public:
    TObject exception;
    std::vector<CallStackEntry *> stack_trace;

    VMByteCodeException() {}

    VMByteCodeException(std::shared_ptr<GC> gc, TObject obj,
                        CallStackEntry *ent) {
        gcList = std::make_shared<GCList>(gc);
        gcList->Add(obj);
        if (ent != nullptr && ent->thread != nullptr) {
            this->stack_trace = ent->thread->call_stack_entries;
            for (auto item : this->stack_trace)
                gcList->Add(item);
        }
        this->exception = obj;
        UpdateError();
    }

    void UpdateError() { lastErrorText = ToString(gcList->GetGC(), exception); }

    const char *what() const noexcept override { return lastErrorText.c_str(); }
    std::shared_ptr<GCList> GetGCList() { return this->gcList; }
};

class SyntaxException : public std::exception {
    std::string msg = {};
    LexTokenLineInfo line;
    std::string message;

  public:
    SyntaxException(LexTokenLineInfo lineInfo, std::string message)
        : line(lineInfo), message(message) {
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
    const char *what() const noexcept override { return msg.c_str(); }

    std::string Message() { return message; }
    LexTokenLineInfo LineInfo() { return line; }
};

template <typename T> bool GetObject(TObject &obj, T &res) {
    if (!std::holds_alternative<T>(obj))
        return false;
    res = std::get<T>(obj);
    return true;
}
template <typename T>
bool GetArgument(std::vector<TObject> &args, size_t index, T &obj) {
    if (index >= args.size())
        return false;
    return GetObject(args[index], obj);
}

template <typename T> bool GetObjectHeap(TObject &obj, T &res) {
    THeapObjectHolder h;
    if (!GetObject<THeapObjectHolder>(obj, h))
        return false;
    auto v = dynamic_cast<T>(h.obj);
    if (v == nullptr)
        return false;
    res = v;
    return true;
}

template <typename T>
bool GetArgumentHeap(std::vector<TObject> &args, size_t index, T &obj) {
    if (index >= args.size())
        return false;
    return GetObjectHeap(args[index], obj);
}
bool GetObjectAsPath(TObject &obj, Tesses::Framework::Filesystem::VFSPath &path,
                     bool allowString = true);
bool GetArgumentAsPath(std::vector<TObject> &args, size_t index,
                       Tesses::Framework::Filesystem::VFSPath &path,
                       bool allowString = true);
bool ToBool(TObject obj);
bool Equals(std::shared_ptr<GC> gc, TObject left, TObject right);
typedef void (*PluginFunction)(std::shared_ptr<GC> gc, TRootEnvironment *env);
#if !defined(_WIN32)
#define DLLEXPORT
#else
#define DLLEXPORT __declspec(dllexport)
#endif
#define CROSSLANG_PLUGIN(plugin)                                               \
    DLLEXPORT extern "C" void CrossLangPluginInit(std::shared_ptr<GC> gc,      \
                                                  TRootEnvironment *env) {     \
        plugin(gc, env);                                                       \
    }

void LoadPlugin(std::shared_ptr<GC> gc, TRootEnvironment *env,
                Tesses::Framework::Filesystem::VFSPath sharedObjectPath);
std::string Json_Encode(TObject o, bool indent = false);
TObject Json_Decode(GCList &ls, std::string str);
std::string Json_DocEncode(TObject o, bool indent);
TObject Json_DocDecode(GCList &ls, std::string str);
// DO NOT USE DIRECTLY
class SharedPtrTObject {
    GCList *ls;
    TObject o;

  public:
    SharedPtrTObject(std::shared_ptr<GC> gc, TObject o);
    TObject &GetObject();
    std::shared_ptr<GC> GetGC();
    ~SharedPtrTObject();
};
using MarkedTObject = std::shared_ptr<SharedPtrTObject>;

MarkedTObject CreateMarkedTObject(std::shared_ptr<GC> gc, TObject o);
MarkedTObject CreateMarkedTObject(GCList *gc, TObject o);
MarkedTObject CreateMarkedTObject(GCList &gc, TObject o);
std::string JoinPeriod(std::vector<std::string> &p);
TObject GetClassInfo(GCList &ls, TFile *f, uint32_t index);

extern Tesses::Framework::Filesystem::VFSPath CrossLangConfigPath;

std::shared_ptr<Tesses::Framework::Http::IHttpServer>
ToHttpServer(std::shared_ptr<GC> gc, TObject obj);

class EmbedStream : public Tesses::Framework::Streams::Stream {
    size_t offset;
    MarkedTObject file;
    uint32_t resource;

  public:
    EmbedStream(std::shared_ptr<GC> gc, TFile *file, uint32_t resource);
    bool CanRead();
    bool CanSeek();
    bool EndOfStream();
    size_t Read(uint8_t *buff, size_t len);
    int64_t GetPosition();
    int64_t GetLength();
    void Seek(int64_t pos, Tesses::Framework::Streams::SeekOrigin whence);
};

class EmbedDirectory : public Tesses::Framework::Filesystem::VFS {
    MarkedTObject dir;
    TObject getEntry(Tesses::Framework::Filesystem::VFSPath path);

  public:
    EmbedDirectory(std::shared_ptr<GC> gc, TDictionary *dict);
    std::shared_ptr<Tesses::Framework::Streams::Stream>
    OpenFile(Tesses::Framework::Filesystem::VFSPath path, std::string mode);

    Tesses::Framework::Filesystem::VFSPathEnumerator
    EnumeratePaths(Tesses::Framework::Filesystem::VFSPath path);
    std::string VFSPathToSystem(Tesses::Framework::Filesystem::VFSPath path);
    Tesses::Framework::Filesystem::VFSPath SystemToVFSPath(std::string path);

    bool Stat(Tesses::Framework::Filesystem::VFSPath path,
              Tesses::Framework::Filesystem::StatData &data);
};

class TNativeException : public TNativeObject {
    std::string what;
    std::exception_ptr ptr;

  public:
    TNativeException() = delete;
    TNativeException(const char *what, std::exception_ptr ptr);
    TObject CallMethod(GCList &ls, std::string key, std::vector<TObject> args);
    std::string TypeName();
    std::string What();
    std::exception_ptr GetPointer();
    void ThrowIt();
};

class TException : public TNativeObject {
  private:
    TDictionary *extraFields;
    TObject innerException;

    std::string message;
    std::string type;
    std::optional<std::string> filename;
    std::optional<uint32_t> line;
    std::optional<uint32_t> column;
    std::optional<uint32_t> offset;

  public:
    TException() = delete;
    TException(std::string message, std::string type = "Exception",
               TDictionary *extraFields = nullptr,
               TObject innerException = Undefined());
    TObject CallMethod(GCList &ls, std::string key, std::vector<TObject> args);
    std::string TypeName();
    bool ToBool();
    void Mark();
    ~TException() = default;
};

namespace Programs {
int64_t CrossArchiveCreate(std::vector<std::string> &argv);
int64_t CrossArchiveExtract(std::vector<std::string> &argv);
TObject CrossLangInterperter(GCList &ls, TRootEnvironment *env,
                             std::vector<std::string> &argv);
TObject CrossLangShell(GCList &ls, std::vector<std::string> &argv);

TObject CrossLangVM(GCList &ls, TRootEnvironment *env,
                    std::vector<std::string> &argv);

void CrossLangDump(std::shared_ptr<Tesses::Framework::Streams::Stream> strm);
void CrossLangCompiler(std::vector<std::string> &argv);
} // namespace Programs

enum class TQueryableMode {
    Passthrough,
    Skip,
    SkipWhile,
    Take,
    TakeWhile,
    Select,
    Where
};

class TQueryable : public THeapObject {

    TObject parent;
    TQueryableMode mode;
    std::vector<TObject> args;

  public:
    TQueryable(TObject parent);
    TQueryable(TObject parent, TQueryableMode mode, std::vector<TObject> args);

    [[deprecated("Use GCList::Create<T>() instead")]]
    static TQueryable *Create(GCList &ls, TObject parent);
    [[deprecated("Use GCList::Create<T>() instead")]]
    static TQueryable *Create(GCList &ls, TObject parent, TQueryableMode mode,
                              std::vector<TObject> args);

    TQueryable *Skip(GCList &ls, int64_t no);
    TQueryable *SkipWhile(GCList &ls, TCallable *call);
    TQueryable *Take(GCList &ls, int64_t no);
    TQueryable *TakeWhile(GCList &ls, TCallable *call);
    TQueryable *Select(GCList &ls, TCallable *call);
    TQueryable *Where(GCList &ls, TCallable *call);
    void ForEach(std::shared_ptr<GC> gc, TCallable *call);
    int64_t Count(std::shared_ptr<GC> gc, TCallable *call);
    int64_t Count(std::shared_ptr<GC> gc);
    bool Contains(std::shared_ptr<GC>, TObject value);
    bool Any(std::shared_ptr<GC> gc, TCallable *call);
    bool All(std::shared_ptr<GC> gc, TCallable *call);

    TList *ToList(GCList &ls);
    TEnumerator *GetEnumerator(GCList &ls);

    void Mark();
};
}; // namespace Tesses::CrossLang
