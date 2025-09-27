#include "CrossLang.hpp"
using namespace Tesses::Framework::Serialization::Json;

namespace Tesses::CrossLang {
    
    class ChunkInstruction {

        public:
            virtual ~ChunkInstruction() {}
            virtual size_t Size()=0;
    };

    class SimpleChunkInstruction : public ChunkInstruction
    {
        public:
            SimpleChunkInstruction(Instruction instr) : instruction(instr)
            {

            }
            Instruction instruction;
            size_t Size()
            {
                return 1;
            }
          
    };

     class PushLongChunkInstruction : public ChunkInstruction {
        public:
            PushLongChunkInstruction(int64_t v) : value(v)
            {}
            int64_t value;

            size_t Size()
            {
                return 9;
            }
    };
    class PushDoubleChunkInstruction : public ChunkInstruction {
        public:
            PushDoubleChunkInstruction(double v) : value(v)
            {}
            double value;

            size_t Size()
            {
                return 9;
            }
    };
   

    class PushStringChunkInstruction : public ChunkInstruction {
        public:
            PushStringChunkInstruction(std::string v) : value(v)
            {}
            std::string value;

            size_t Size()
            {
                return 5;
            }
    };

    class PushCharChunkInstruction : public ChunkInstruction {
        public:
            PushCharChunkInstruction(char v) : value(v)
            {}
            char value;
            size_t Size()
            {
                return 2;
            }
    };

    class PushResourceChunkInstruction : public ChunkInstruction {
        public:
            PushResourceChunkInstruction(std::string v) : value(v)
            {}
            std::string value;
            size_t Size()
            {
                return 5;
            }
    };

    class LabelChunkInstruction : public ChunkInstruction {
        public:
            LabelChunkInstruction(std::string lbl) : lbl(lbl)
            {}
            std::string lbl;
            size_t Size()
            {
                return 0;
            }
    };
    class JumpStyleChunkInstruction : public ChunkInstruction {
        public:
            JumpStyleChunkInstruction(Instruction instr,std::string lbl) : instr(instr), lbl(lbl) 
            {

            }
            Instruction instr;
            std::string lbl;
            size_t Size()
            {
                return 5;
            }
    };

    class ScopeEndTimesChunkInstruction : public ChunkInstruction {
        public:
            ScopeEndTimesChunkInstruction(uint32_t times) : times(times)
            {}
            uint32_t times;
            size_t Size()
            {
                return 5;
            }
    };

    class ChunkData {
        public:
            std::vector<std::string> args;
            std::vector<std::shared_ptr<ChunkInstruction>> instructions;
            
    };
    class ClosureChunkInstruction : public ChunkInstruction {
        public:
            ClosureChunkInstruction(bool scoped) : scoped(scoped)
            {

            }
            ChunkData data;
            bool scoped;
            size_t Size()
            {
                return 5;
            }
    };


    class FunctionData {
        public:
            std::vector<std::string> name;
            ChunkData chunk;

    };
    class ClassDataEntry {
        public:
            ChunkData data;
            std::string documentation;
            std::string name;
            TClassModifier modifier;
            bool isFunction;
            bool isAbstract;
    };
    class ClassData {
        public:
        std::string documentation;
        std::vector<std::string> name;
        std::vector<std::string> inherits;
        std::vector<ClassDataEntry> entries;
    };
    class AssemblerParser {
        std::vector<LexToken>& tokens;
        size_t i=0;
        std::string ReadIdent()
        {
            if(i >= tokens.size()) throw std::out_of_range("End of file");
            if(tokens[i].type != LexTokenType::Identifier) throw std::runtime_error("Expected an identifier");
            return tokens[i++].text;
        }
        bool IsIdent(std::string val,bool pop=true)
        {
            if(i >= tokens.size()) return false;
            if(tokens[i].type != LexTokenType::Identifier) return false;
            if(tokens[i].text != val) return false;
            if(pop) i++;
            return true;
        }
        bool IsSym(std::string val,bool pop=true)
        {
            if(i >= tokens.size()) return false;
            if(tokens[i].type != LexTokenType::Symbol) return false;
            if(tokens[i].text != val) return false;
            if(pop) i++;
            return true;
        }

        void ParseClosureInstructions(std::vector<std::shared_ptr<ChunkInstruction>>& instrs, std::string lbl_prefix)
        {
            while(!IsSym("}",false))
            {
                auto name = ReadIdent();
                
                if(name == "add")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(ADD));
                }
                else if(name == "sub")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(SUB));
                }
                else if(name == "mul")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(TIMES));
                }
                else if(name == "div")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(DIVIDE));
                }
                else if(name == "mod")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(MODULO));
                }
                else if(name == "lsh")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(LEFTSHIFT));
                }
                else if(name == "rsh")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(RIGHTSHIFT));
                }
                else if(name == "bor")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(BITWISEOR));
                }
                else if(name == "band")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(BITWISEAND));
                }
                else if(name == "bnot")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(BITWISENOT));
                }
                else if(name == "lt")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(LESSTHAN));
                }
                else if(name == "gt")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(GREATERTHAN));
                }
                else if(name == "lte")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(LESSTHANEQ));
                }
                else if(name == "gte")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(GREATERTHANEQ));
                }
                else if(name == "eq")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(EQ));
                }
                else if(name == "neq")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(NEQ));
                }
                else if(name == "not")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(NOT));
                }
                else if(name == "neg")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(NEGATIVE));
                }
                else if(name == "xor")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(XOR));
                }
                else if(name == "pop")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(POP));
                }
                else if(name == "dup")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(DUP));
                }
                else if(name == "nop")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(NOP));
                }
                else if(name == "pushclosure")
                {
                    auto closure = std::make_shared<ClosureChunkInstruction>(true);
                    EnsureSymbol("(");
                    while(!IsSym(")",false) && i < tokens.size())
                    {
                        closure->data.args.push_back(ReadIdent());
                        IsSym(",");
                    }
                    EnsureSymbol(")");
                    EnsureSymbol("{");
                    ParseClosureInstructions(closure->data.instructions,"closure");
                    EnsureSymbol("}");
                    instrs.push_back(closure);
                    //instrs.push_back(std::make_shared<JumpStyleInstruction>(JMPC,lblName));
                }
                else if(name == "createdict")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(CREATEDICTIONARY));
                }
                else if(name == "createarray")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(CREATEARRAY));
                }
                else if(name == "appendlist")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(APPENDLIST));
                }
                else if(name == "appenddict")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(APPENDDICT));
                }
                else if(name == "embed")
                {
                    if(i < tokens.size() && tokens[i].type == LexTokenType::String)
                    {
                        std::string str = tokens[i++].text;
                        instrs.push_back(std::make_shared<PushResourceChunkInstruction>(str));
                    }
                }
                else if(name == "push")
                {
                    if(IsSym("."))
                    {
                        instrs.push_back(std::make_shared<SimpleChunkInstruction>(PUSHRELATIVEPATH));
                    }
                    else if(IsSym("/"))
                    {
                        instrs.push_back(std::make_shared<SimpleChunkInstruction>(PUSHROOTPATH));
                    }
                    else if(IsSym("}",false))
                    {
                        throw std::runtime_error("Push needs an argument");
                    }
                    else if(i < tokens.size() && tokens[i].type == LexTokenType::Identifier)
                    {

                        std::string token = tokens[i].text;
            
                        i++;
        
                        bool hasNumber=true;    
                        int64_t lngNum = 0;
            
                        if(token.size() == 1 && token[0] == '0')
                        {
                            lngNum = 0;
                        }
                        else if(token.size() > 0 && token[0] == '0')
                        {
                            if(token.size() > 1 && token[1] == 'x')
                            {
                                lngNum = std::stoll(token.substr(2),nullptr,16);
                            }
                            else if(token.size() > 1 && token[1] == 'b')
                            {
                                lngNum = std::stoll(token.substr(2),nullptr,2);
                            }
                            else
                            {
                                lngNum = std::stoll(token.substr(1),nullptr,8);
                            }

                        }
                        else if(token.size() > 0 && token[0] >= '0' && token[0] <= '9')
                        {
                            lngNum=std::stoll(token,nullptr,10);
                        }
                        else
                        {
                            hasNumber = false;
                        }

                        if(hasNumber && this->IsSym(".",false) && i+1 < tokens.size() && tokens[i+1].type == LexTokenType::Identifier)
                        {
                            std::string myToken = tokens[i+1].text;
                            if(myToken.size() > 0 && myToken[0] >= '0' && myToken[0] <= '9')
                            {
                                i+=2;
                                std::string myN = std::to_string(lngNum) + "." + myToken;

                                double v = std::stod(myN,nullptr);

                                
                                instrs.push_back(std::make_shared<PushDoubleChunkInstruction>(v));
                            }
                            else
                            {
                               
                                instrs.push_back(std::make_shared<PushLongChunkInstruction>(lngNum));
                            }
                        }
                        else if(hasNumber)
                        {
                
                            instrs.push_back(std::make_shared<PushLongChunkInstruction>(lngNum));
                        }

                        if(!hasNumber)
                        {
                            if(token == "true")
                                instrs.push_back(std::make_shared<SimpleChunkInstruction>(PUSHTRUE));
                            if(token == "false")
                                instrs.push_back(std::make_shared<SimpleChunkInstruction>(PUSHFALSE));
                            if(token == "null")
                                instrs.push_back(std::make_shared<SimpleChunkInstruction>(PUSHNULL));
                            if(token == "undefined")
                                instrs.push_back(std::make_shared<SimpleChunkInstruction>(PUSHUNDEFINED));
                            if(token == "break")
                                instrs.push_back(std::make_shared<SimpleChunkInstruction>(PUSHBREAK));
                            
                            if(token == "continue")
                                instrs.push_back(std::make_shared<SimpleChunkInstruction>(PUSHCONTINUE));
                        }
                    }
                    else if(i < tokens.size() && tokens[i].type == LexTokenType::String)
                    {
                        instrs.push_back(std::make_shared<PushStringChunkInstruction>(tokens[i++].text));

                    }
                    else if(i < tokens.size() && tokens[i].type == LexTokenType::Char)
                    {
                        instrs.push_back(std::make_shared<PushCharChunkInstruction>(tokens[i++].text[0]));
                    }
                }
                else if(name == "scopebegin")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(SCOPEBEGIN));
                }
                else if(name == "scopeend")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(SCOPEEND));
                }
                else if(name == "scopeendtimes")
                {
                    if(tokens[i].type == LexTokenType::Identifier)
                        instrs.push_back(std::make_shared<ScopeEndTimesChunkInstruction>((uint32_t)std::stoul(tokens[i++].text)));
                }
                else if(name == "setvariable")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(SETVARIABLE));
                }
                else if(name == "getvariable")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(GETVARIABLE));
                }
                else if(name == "declarevariable")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(DECLAREVARIABLE));
                }
                else if(name == "setfield")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(SETFIELD));
                }
                else if(name == "getfield")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(GETFIELD));
                }
                else if(name == "callfunction")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(CALLFUNCTION));
                }
                else if(name == "callmethod")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(CALLMETHOD));
                }
                else if(name == "ret")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(RET));
                }
                else if(name == "jmpc")
                {
                    auto lblName = lbl_prefix + "_" + ReadIdent();
                    instrs.push_back(std::make_shared<JumpStyleChunkInstruction>(JMPC,lblName));
                }
                else if(name == "jmp")
                {
                    auto lblName = lbl_prefix + "_" + ReadIdent();
                    instrs.push_back(std::make_shared<JumpStyleChunkInstruction>(JMP,lblName));
                }
                else if(name == "jmpundefined")
                {
                    auto lblName = lbl_prefix + "_" + ReadIdent();
                    instrs.push_back(std::make_shared<JumpStyleChunkInstruction>(JMPUNDEFINED,lblName));
                }
                else if(name == "defer")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(DEFER));
                }
                else if(name == "trycatch")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(TRYCATCH));
                }
                else if(name == "throw")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(THROW));
                }
                else if(name == "pushscopelessclosure")
                {
                    auto closure = std::make_shared<ClosureChunkInstruction>(false);
                    EnsureSymbol("(");
                    while(!IsSym(")",false) && i < tokens.size())
                    {
                        closure->data.args.push_back(ReadIdent());
                        IsSym(",");
                    }
                    EnsureSymbol(")");
                    EnsureSymbol("{");
                    ParseClosureInstructions(closure->data.instructions,"closure");
                    EnsureSymbol("}");
                    instrs.push_back(closure);
                    //instrs.push_back(std::make_shared<JumpStyleInstruction>(JMPC,lblName));
                }
                else if(name == "yield")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(YIELD));
                }                
                else if(name == "breakpoint")
                {
                    instrs.push_back(std::make_shared<SimpleChunkInstruction>(BREAKPOINT));
                }
                else if(name == "jmpifbreak")
                {
                    auto lblName = lbl_prefix + "_" + ReadIdent();
                    instrs.push_back(std::make_shared<JumpStyleChunkInstruction>(JMPIFBREAK,lblName));
                }
                else if(name == "jmpifcontinue")
                {
                    auto lblName = lbl_prefix + "_" + ReadIdent();
                    instrs.push_back(std::make_shared<JumpStyleChunkInstruction>(JMPIFCONTINUE,lblName));
                }
                else if(name == "jmpifdefined")
                {
                    auto lblName = lbl_prefix + "_" + ReadIdent();
                    instrs.push_back(std::make_shared<JumpStyleChunkInstruction>(JMPIFDEFINED,lblName));
                }
                else if(name == "lbl")
                {
                    auto lblName = lbl_prefix + "_" + ReadIdent();
                    instrs.push_back(std::make_shared<LabelChunkInstruction>(lblName));
                }
                else {
                    throw std::runtime_error("NOT IMPLEMENTED: " + name);
                }
            }
        }
        void EnsureSymbol(std::string txt)
        {
            if(i < tokens.size())
            {
                if(tokens[i].type != LexTokenType::Symbol)
                {
                    throw SyntaxException(tokens[i].lineInfo, "expected the symbol \"" + txt + "\" but did not get a symbol at all.");
                }
                if(tokens[i].text != txt)
                {
              
                    throw SyntaxException(tokens[i].lineInfo, "expected the symbol \"" + txt + "\" but got the symbol \"" + tokens[i].text + "\"");
                }
                i++;
                return;
            }
            throw std::out_of_range("End of file");
        }
        std::string ParseDocumentation()
        {
            if(i >= tokens.size()) return {};
            if(tokens[i].type != LexTokenType::Documentation) return {};
            return tokens[i++].text;
        }
        public:
            AssemblerParser(std::vector<LexToken>& tokens) : tokens(tokens)
            {
                size_t root=0;

                while(i < tokens.size())
                {
                    auto doc = ParseDocumentation();

                    if(IsIdent("root")) {
                        EnsureSymbol("{");
                        ParseClosureInstructions(this->root.instructions, "root_" + std::to_string(root));
                        EnsureSymbol("}");  
                        root++;
                    }
                    else if(IsIdent("func")) {
                        FunctionData fdata;
                        fdata.name.push_back(doc);
                        fdata.name.push_back(ReadIdent());
                        while(IsSym("."))
                        {
                            fdata.name.push_back(ReadIdent());
                        }
                        EnsureSymbol("(");
                        while(!IsSym(")",false) && i < tokens.size())
                        {
                            fdata.chunk.args.push_back(ReadIdent());
                            IsSym(",");
                        }
                        EnsureSymbol(")");

                        EnsureSymbol("{");
                        ParseClosureInstructions(fdata.chunk.instructions, "func_" + std::to_string(root));
                        EnsureSymbol("}");
                        this->functions.push_back(fdata);
                    }
                    else if(IsIdent("class"))
                    {
                        ClassData cdata;
                        cdata.documentation = doc;
                        cdata.name.push_back(ReadIdent());
                        while(IsSym("."))
                        {
                            cdata.name.push_back(ReadIdent());
                        }
                        if(IsSym(":"))
                        {
                            cdata.inherits.push_back(ReadIdent());
                            while(IsSym("."))
                            {
                                cdata.inherits.push_back(ReadIdent());
                            }
                        }
                        else {
                            cdata.inherits.push_back("ClassObject");
                        }

                        EnsureSymbol("{");

                        while(i < tokens.size() && !IsSym("}",false))
                        {
                            
                            ClassDataEntry ent;
                            ent.documentation = ParseDocumentation();
                            

                            if(IsIdent("public"))
                            {
                                ent.modifier = TClassModifier::Public;
                            }
                            else if(IsIdent("protected"))
                            {
                                ent.modifier = TClassModifier::Protected;
                            }
                            else if(IsIdent("private"))
                            {
                                ent.modifier = TClassModifier::Private;
                            }
                            else if(IsIdent("static"))
                            {
                                ent.modifier = TClassModifier::Static;
                            }
                            else throw std::runtime_error("Invalid modifier on class entry");

                            if(IsIdent("abstract"))
                            {
                                ent.name = ReadIdent();
                                ent.isAbstract = true;
                                ent.isFunction=true;
                                

                                EnsureSymbol("(");
                                while(!IsSym(")",false) && i < tokens.size())
                                {
                                    ent.data.args.push_back(ReadIdent());
                                    IsSym(",");
                                }
                                EnsureSymbol(")");
                                EnsureSymbol(";");
                            }
                            else {
                                ent.name = ReadIdent();
                                if(IsSym("("))
                                {
                                    ent.isFunction=true;
                                    ent.isAbstract=false;
                                    while(!IsSym(")",false) && i < tokens.size())
                                    {
                                        ent.data.args.push_back(ReadIdent());
                                        IsSym(",");
                                    }
                                    EnsureSymbol(")");
                                    EnsureSymbol("{");
                                    ParseClosureInstructions(ent.data.instructions,"cls_fn");
                                    EnsureSymbol("}");
                                }
                                else if(IsSym(";"))
                                {
                                    
                                    ent.isFunction=false;
                                    ent.isAbstract=true;
                                }
                                else if(IsSym("{"))
                                {

                                    ent.isFunction=false;
                                    ent.isAbstract=false;
                                    EnsureSymbol("{");
                                    ParseClosureInstructions(ent.data.instructions,"cls_field");
                                    EnsureSymbol("}");

                                }
                            }
                            cdata.entries.push_back(ent);
                        }

                        EnsureSymbol("}");

                        this->classes.push_back(cdata);
                    }
                }
            }

            ChunkData root;
            std::vector<FunctionData> functions;
            std::vector<ClassData> classes;

        
    };

    class CodeGen2 {
        public:
              void Write(Tesses::Framework::Streams::Stream* strm, uint8_t* buffer, size_t len)
    {
        strm->WriteBlock(buffer,len);
    }
    void WriteInt(Tesses::Framework::Streams::Stream* strm,uint32_t v)
    {
        uint8_t buffer[4];
        BitConverter::FromUint32BE(buffer[0],v);
        Write(strm,buffer,4);
    }
    void WriteString(Tesses::Framework::Streams::Stream* strm,std::string v)
    {
        WriteInt(strm,(uint32_t)v.size());
        Write(strm,(uint8_t*)v.data(),v.size());
    }
    void Save(Tesses::Framework::Filesystem::VFS* vfs, Tesses::Framework::Streams::Stream* stream)
    {

        TVMVersion runtime_version(TVM_MAJOR,TVM_MINOR,TVM_PATCH,TVM_BUILD,TVM_VERSIONSTAGE);
        uint8_t buffer[18];
        memcpy(buffer,"TCROSSVM",8);
        runtime_version.ToArray(buffer+8);
        version.ToArray(buffer+13);
        Write(stream,buffer,18);
        uint32_t sections=5;
        uint32_t name = GetString(this->name);
        uint32_t info = GetString(this->info);

        for(auto& dep : this->dependencies)
        {
            GetString(dep.first);
            sections++;
        }
        for(auto& tool : this->tools)
        {
            GetString(tool.first);
            sections++;
        }
        if(!this->icon.empty())
        {
            this->GetResource(this->icon);
            
        }
        for(auto& sec : this->sections)
            sections++;
        for(auto& vm : this->vms)
        {
            this->GetString(vm.first);
            this->GetString(vm.second);
            sections++;
        }

        for(auto& res : this->res)
            sections++;
        
        if(!this->icon.empty())
            sections++;
        if(!this->classes.empty())
            sections++;
        
       
        WriteInt(stream,sections);
        uint32_t strSz=4;
        for(auto& s : this->strs)
        {
            strSz += (uint32_t)s.size() + 4;
        }
        memcpy(buffer,"STRS",4);
        Write(stream,buffer,4);
        WriteInt(stream,strSz); //even though its ignored
        WriteInt(stream,this->strs.size());
        for(auto& str : this->strs)
            WriteString(stream,str);
        memcpy(buffer,"NAME",4);
        Write(stream,buffer,4);
        WriteInt(stream,4);
        WriteInt(stream,name);
        memcpy(buffer,"INFO",4);
        Write(stream,buffer,4);
        WriteInt(stream,4);
        WriteInt(stream,info);

        for(auto& dep : this->dependencies)
        {
            memcpy(buffer,"DEPS",4);
            Write(stream,buffer,4);
            WriteInt(stream,9); //even though its ignored
            WriteInt(stream,GetString(dep.first));
            dep.second.ToArray(buffer);
            Write(stream,buffer,5);
        }
        for(auto& tool : this->tools)
        {
            memcpy(buffer,"TOOL",4);
            Write(stream,buffer,4);
            WriteInt(stream,9); //even though its ignored
            WriteInt(stream,GetString(tool.first));
            tool.second.ToArray(buffer);
            Write(stream,buffer,5);
        }

        for(auto& vm : this->vms)
        {
            memcpy(buffer,"MACH",4);
            Write(stream,buffer,4);
            WriteInt(stream,8); //even though its ignored
            WriteInt(stream,GetString(vm.first));
            WriteInt(stream,GetString(vm.second));
            
        }

        uint32_t fnLen=4;

        for(auto& fn : this->funcs)        
        {
            fnLen += (fn.first.size() + 2) * 4;
        }

        memcpy(buffer,"FUNS",4);
        Write(stream,buffer,4);
        WriteInt(stream,fnLen);
        WriteInt(stream,(uint32_t)this->funcs.size());
        for(auto& fn : this->funcs)        
        {
            WriteInt(stream,(uint32_t)fn.first.size());
            for(auto namePart : fn.first)
            {
                WriteInt(stream,namePart);
            }
            WriteInt(stream,fn.second);
        }

        uint32_t clength = 4;
      
        memcpy(buffer,"CHKS",4);
        Write(stream,buffer,4);
        WriteInt(stream,clength);
        WriteInt(stream,(uint32_t)this->chunks.size());
        for(auto& chunk : this->chunks)
        {
            std::vector<uint8_t> buffer;
            WriteInt(stream,(uint32_t)chunk.first.size());
            for(auto arg : chunk.first)
            {
                WriteInt(stream,arg);
            }
            for(auto instr : chunk.second)
            {
                instr->Write(buffer);
            }
            WriteInt(stream,(uint32_t)buffer.size());
            Write(stream,buffer.data(),buffer.size());
        }
        
        if(!classes.empty())
        {
            uint32_t len = 4;
            for(auto& cls : classes)
            {
                len += 8;
                len += cls.name.size() * 4;
                len += 4;
                len += cls.inherits.size() * 4;
                len += 4;
                for(auto& clsEnt : cls.entries)
                {
                    len += 17;
                    for(auto& arg : clsEnt.arguments) len+=4;
                }
            }

            memcpy(buffer,"CLSS",4);
            Write(stream,buffer,4);
            WriteInt(stream,len);
            WriteInt(stream,(uint32_t)classes.size());
            for(auto& cls : classes)
            {
                WriteInt(stream,cls.documentation);
                WriteInt(stream,(uint32_t)cls.name.size());
                for(auto namePart : cls.name) WriteInt(stream,namePart);

                WriteInt(stream,(uint32_t)cls.inherits.size());
                for(auto inhPart : cls.inherits) WriteInt(stream,inhPart);

                WriteInt(stream,(uint32_t)cls.entries.size());
                
                for(auto& ent : cls.entries)
                {
                    buffer[0] = ent.type;
                    Write(stream,buffer,1);
                    WriteInt(stream,ent.documentation);
                    WriteInt(stream,ent.name);
                    WriteInt(stream,(uint32_t)ent.arguments.size());
                    for(auto ar : ent.arguments)
                    {
                        WriteInt(stream,ar);
                    }
                    WriteInt(stream,ent.closure);
                }
            }            
        }

        for(auto& reso : res)
        {
            memcpy(buffer,"RESO",4);
            Write(stream,buffer,4);

            auto strm = vfs->OpenFile(reso,"rb");
            if(strm->EndOfStream())
            {
                WriteInt(stream,0);

            }
            else {
                WriteInt(stream, (uint32_t)strm->GetLength());
                strm->CopyTo(stream);
            }
            
            delete strm;
        }
        if(!this->icon.empty())
        {
            memcpy(buffer,"ICON",4);
            Write(stream,buffer,4);
            WriteInt(stream,4);
            WriteInt(stream,this->GetResource(this->icon));
            
        }
        for(auto& sect : this->sections)
        {
            memcpy(buffer,sect.first.data(),4);
            Write(stream,buffer,4);
            WriteInt(stream,(uint32_t)sect.second.size());
            Write(stream,sect.second.data(),sect.second.size());
        }   
    }


            uint32_t GetResource(std::string resource)
            {
                for(uint32_t i = 0; i < (uint32_t)this->res.size();i++)
                {
                    if(this->res[i] == resource) return i;
                }
                uint32_t resI = (uint32_t)this->res.size();
                this->res.push_back(resource);
                return resI;
            }
            uint32_t GetString(std::string str)
            {
                for(uint32_t i = 0; i < (uint32_t)this->strs.size();i++)
                {
                    if(this->strs[i] == str) return i;
                }
                uint32_t strI = (uint32_t)this->strs.size();
                this->strs.push_back(str);
                return strI;
            }
           
            std::vector<std::string> strs;
            std::vector<std::string> res;
            std::vector<std::pair<std::vector<uint32_t>,uint32_t>> funcs;
            std::vector<CodeGenClass> classes;
            std::vector<std::pair<std::vector<uint32_t>, std::vector<std::shared_ptr<ByteCodeInstruction>>>> chunks;
            
            std::vector<std::pair<std::string, TVMVersion>> dependencies;
            std::vector<std::pair<std::string, TVMVersion>> tools;
            std::vector<std::pair<std::string,std::vector<uint8_t>>> sections;
            std::vector<std::pair<std::string,std::string>> vms;
            TVMVersion version;
            std::string name;
            std::string info;
            std::string icon;

            uint32_t PushChunk(ChunkData& data)
            {   
                uint32_t chunkId = chunks.size();
                
                chunks.push_back(std::pair<std::vector<uint32_t>,std::vector<std::shared_ptr<ByteCodeInstruction>>>());
                chunks[chunkId].first.resize(data.args.size());
                for(size_t i = 0; i < data.args.size(); i++)
                {
                    chunks[chunkId].first[i] = GetString(data.args[i]);
                }

                std::map<std::string,uint32_t> labels;

                uint32_t offset=0;

                for(auto item : data.instructions)
                {
                    offset += item->Size();
                    
                    auto lbl = std::dynamic_pointer_cast<LabelChunkInstruction>(item);
                    auto simp = std::dynamic_pointer_cast<SimpleChunkInstruction>(item);
                    auto lng = std::dynamic_pointer_cast<PushLongChunkInstruction>(item);
                    auto dbl = std::dynamic_pointer_cast<PushDoubleChunkInstruction>(item);
                    auto str = std::dynamic_pointer_cast<PushStringChunkInstruction>(item);
                    auto chr = std::dynamic_pointer_cast<PushCharChunkInstruction>(item);
                    auto chk = std::dynamic_pointer_cast<ClosureChunkInstruction>(item);
                    auto reso = std::dynamic_pointer_cast<PushResourceChunkInstruction>(item);
                    auto jmp = std::dynamic_pointer_cast<JumpStyleChunkInstruction>(item);
                    auto scopeend = std::dynamic_pointer_cast<ScopeEndTimesChunkInstruction>(item);
                    if(lbl)
                    {
                        labels[lbl->lbl] = offset;
                    }
                    if(simp)
                    {
                        chunks[chunkId].second.push_back(std::make_shared<SimpleInstruction>(simp->instruction));
                    }
                    if(lng)
                    {
                        chunks[chunkId].second.push_back(std::make_shared<LongInstruction>(lng->value));
                    }
                    if(dbl)
                    {
                        chunks[chunkId].second.push_back(std::make_shared<DoubleInstruction>(dbl->value));
                    }
                    if(str)
                    {
                        chunks[chunkId].second.push_back(std::make_shared<StringInstruction>(GetString(str->value)));
                    }
                    if(chr)
                    {
                        chunks[chunkId].second.push_back(std::make_shared<CharInstruction>(chr->value));
                    }
                    if(chk)
                    {
                        auto item = std::make_shared<ClosureInstruction>(PushChunk(chk->data),chk->scoped);
                        chunks[chunkId].second.push_back(item);
                    }
                    if(reso)
                    {
                        chunks[chunkId].second.push_back(std::make_shared<EmbedInstruction>(GetResource(reso->value)));
                    }
                    if(jmp)
                    {
                        chunks[chunkId].second.push_back(std::make_shared<JumpStyleInstruction>(jmp->instr,jmp->lbl));
                    }
                    if(scopeend)
                    {
                        chunks[chunkId].second.push_back(std::make_shared<ScopeEndTimesInstruction>(scopeend->times));
                    }
                }

                for(auto& item : chunks[chunkId].second)
                {
                    auto jmp = std::dynamic_pointer_cast<JumpStyleInstruction>(item);
                    if(jmp)
                    {
                        jmp->n = labels[jmp->label];
                    }
                }
                return chunkId;
            }
    };  

    



    Tesses::Framework::Filesystem::VFSPath Assemble(Tesses::Framework::Filesystem::VFS* vfs)
    {
        using namespace Tesses::Framework::Filesystem;
        using namespace Tesses::Framework::TextStreams;
        std::function<void(VFSPath)> getSrc;

        std::vector<LexToken> tokens;

        getSrc = [&](VFSPath path)->void {
            if(vfs->DirectoryExists(path))
            for(auto item : vfs->EnumeratePaths(path))
            {
                if(vfs->DirectoryExists(item))
                {
                    getSrc(item);
                }
                else {
                    if(item.GetExtension() == ".tcasm")
                    {
                        StreamReader reader(vfs->OpenFile(item,"rb"),true);
                        
                        std::stringstream strm(reader.ReadToEnd(),std::ios_base::binary | std::ios_base::in);
                        Lex(item.ToString(), strm, tokens);
                    }
                }
            }
        };

        getSrc(VFSPath() / "src");

        AssemblerParser parser(tokens);
        CodeGen2 cg2;
        cg2.name = "out";
        cg2.version = TVMVersion(1,0,0,0,TVMVersionStage::ProductionVersion);
        

        auto confFile = VFSPath() / "crossapp.json";
        if(vfs->FileExists(confFile))
        {
            Tesses::Framework::TextStreams::StreamReader reader(vfs->OpenFile(confFile,"rb"),true);
            auto jobj = Json::Decode(reader.ReadToEnd());
            JObject main;
            if(TryGetJToken(jobj,main))
            {
                TVMVersion version;
                std::string str0;
                std::string str1;
                JObject dict0;
                JArray array0;
                
                main.TryGetValueAsType("name",cg2.name);
                if(main.TryGetValueAsType("version",str0))
                    TVMVersion::TryParse(str0, cg2.version);
                
                if(main.TryGetValueAsType("info",dict0))
                    cg2.info = Json::Encode(dict0,false);
                
                main.TryGetValueAsType("icon", cg2.icon);

                if(main.TryGetValueAsType("dependencies",array0))
                {
                    for(auto item : array0)
                    {
                        if(TryGetJToken(item,dict0))
                        {
                            if(dict0.TryGetValueAsType("version", str1) && TVMVersion::TryParse(str1,version) && dict0.TryGetValueAsType("name",str0))
                            {
                                cg2.dependencies.emplace_back(str0, version);
                            }
                            
                        }
                    }
                }
                if(main.TryGetValueAsType("tools",array0))
                {
                    for(auto item : array0)
                    {
                        if(TryGetJToken(item,dict0))
                        {
                            if(dict0.TryGetValueAsType("version", str1) && TVMVersion::TryParse(str1,version) && dict0.TryGetValueAsType("name",str0))
                            {
                                cg2.tools.emplace_back(str0, version);
                            }
                            
                        }
                    }
                }
                 if(main.TryGetValueAsType("vms",array0))
                {
                    for(auto item : array0)
                    {
                        if(TryGetJToken(item,dict0))
                        {
                            if(dict0.TryGetValueAsType("name", str0) && dict0.TryGetValueAsType("how_to_get",str1))
                            {
                                cg2.vms.emplace_back(str0, str1);
                            }
                            
                        }
                    }
                }
                   
            }
        }
        
        cg2.PushChunk(parser.root);
        for(auto& fn : parser.functions)
        {
            std::vector<uint32_t> nameParts;
            nameParts.resize(fn.name.size());
            for(size_t i = 0; i < fn.name.size(); i++)
            {
                nameParts[i] = cg2.GetString(fn.name[i]);
            }

            auto chunk = cg2.PushChunk(fn.chunk);
            cg2.funcs.push_back(std::pair<std::vector<uint32_t>,uint32_t>(nameParts,chunk));
        }
        for(auto& cls : parser.classes)
        {
            CodeGenClass cls2;
            cls2.name.resize(cls.name.size());
            for(size_t i = 0; i < cls.name.size(); i++)
            {
                cls2.name[i] = cg2.GetString(cls.name[i]);
            }

            cls2.inherits.resize(cls.inherits.size());
            for(size_t i = 0; i < cls.inherits.size(); i++)
            {
                cls2.inherits[i] = cg2.GetString(cls.inherits[i]);
            }

            cls2.documentation = cg2.GetString(cls.documentation);

            for(auto clsItem : cls.entries)
            {
                CodeGenClassEntry ent;
                ent.type = 0;
                ent.type = (uint8_t)clsItem.modifier & 3;
                if(clsItem.isAbstract) ent.type |= 0b00001000;
                if(!clsItem.isFunction) ent.type |= 0b00000100;
                ent.documentation = cg2.GetString(clsItem.documentation);
                ent.name =  cg2.GetString(clsItem.name);
                ent.closure = 0;
                ent.arguments.resize(clsItem.data.args.size());
                for(size_t i = 0; i < clsItem.data.args.size(); i++)
                {
                    ent.arguments[i] = cg2.GetString(clsItem.data.args[i]);
                }
                if(!clsItem.isAbstract)
                {
                    ent.closure = cg2.PushChunk(clsItem.data);

                }

                cls2.entries.push_back(ent);
            }
            cg2.classes.push_back(cls2);
        }
        vfs->CreateDirectory(VFSPath() / "res");
        vfs->CreateDirectory(VFSPath() / "bin");
        vfs->CreateDirectory(VFSPath() / "sections");

        SubdirFilesystem sectionsdir(vfs,VFSPath() / "sections",false);
        SubdirFilesystem resdir(vfs,VFSPath() / "res",false);

        for(auto file : sectionsdir.EnumeratePaths(VFSPath()))
        {
            if(file.GetExtension() == ".tsec" && sectionsdir.FileExists(file))
            {
                auto strm0 = sectionsdir.OpenFile(file,"rb");
                int64_t len = strm0->GetLength();
                if(len > 4)
                {
                    std::string name(4,' ');
                    strm0->ReadBlock((uint8_t*)name.data(),4);

                    size_t off = cg2.sections.size();
                    cg2.sections.push_back(std::pair<std::string,std::vector<uint8_t>>(name,{}));
                    cg2.sections[off].second.resize((size_t)len-4);
                    strm0->ReadBlock(cg2.sections[off].second.data(), cg2.sections[off].second.size());
                }

                delete strm0;
            }
        }

        

        auto strm = vfs->OpenFile(VFSPath() / "bin" / cg2.name + "-" + cg2.version.ToString() + ".crvm","wb");
        cg2.Save(&resdir, strm);
        delete strm;


        return VFSPath() / "bin" / cg2.name + "-" + cg2.version.ToString() + ".crvm";

    }
    
}
