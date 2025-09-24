#include "CrossLang.hpp"
#include <iostream>
#include <array>
#include <map>
#include <cstring>
#include <variant>

namespace Tesses::CrossLang
{
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
    
    
    CodeGen::~CodeGen()
    {
        for(auto& item : this->chunks) 
        {
            for(auto instr : item.second)
                delete instr;
        }
    }


    void CodeGen::Save(Tesses::Framework::Filesystem::VFS* vfs, Tesses::Framework::Streams::Stream* stream)
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
            this->GetResource(std::make_shared<ResourceFile>(this->icon));
            
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
        for(auto& chunk : this->chunks)
        {
            clength += (2 + chunk.first.size())*4;
            size_t offset=0;
            std::map<std::string, int64_t> items;
       
            for(auto instr : chunk.second)
            {
                
                auto lbl = dynamic_cast<LabelInstruction*>(instr);
                if(lbl != nullptr)
                {
                    items[lbl->label] = offset;
                }

                offset += instr->Size();

            }
            
            clength += (uint32_t)offset;
            
            for(auto instr : chunk.second)
            {
                auto jmp = dynamic_cast<LabelableInstruction*>(instr);
                if(jmp != nullptr)
                {
                    jmp->n = items[jmp->label];
                }
            }
            
        }

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
            WriteInt(stream,reso->GetLength(vfs));
            reso->Write(stream);
        }
        if(!this->icon.empty())
        {
            memcpy(buffer,"ICON",4);
            Write(stream,buffer,4);
            WriteInt(stream,4);
            WriteInt(stream,this->GetResource(std::make_shared<ResourceFile>(this->icon)));
            
        }
    }


    size_t SimpleInstruction::Size()
    {
        return 1;
    }
    SimpleInstruction::SimpleInstruction(Instruction instr)
    {
        this->instruction = instr;
    }
    void SimpleInstruction::Write(std::vector<uint8_t>& instr)
    {
        instr.push_back(instruction);
    }


    ScopeEndTimesInstruction::ScopeEndTimesInstruction(uint32_t s)
    {
        this->n = s;
    }
    size_t ScopeEndTimesInstruction::Size()
    {
        return 5;
    }
    void ScopeEndTimesInstruction::Write(std::vector<uint8_t>& instr)
    {
        instr.push_back(SCOPEENDTIMES);
        std::array<uint8_t,4> buff;
        BitConverter::FromUint32BE(buff[0],this->n);
        instr.insert(instr.end(),buff.begin(),buff.end());
    }

    LabelInstruction::LabelInstruction(std::string lbl)
    {
        this->label = lbl;
    }

    size_t LabelInstruction::Size()
    {
        return 0;
    }

    void LabelInstruction::Write(std::vector<uint8_t>& instr)
    {
        //label instructions do not write a thing
    }

    JumpStyleInstruction::JumpStyleInstruction(Instruction instr, std::string lbl)
    {
        this->type = instr;
        this->label = lbl;
        this->n = 0;
    }
    
    size_t JumpStyleInstruction::Size()
    {
        return 5;
    }
    void JumpStyleInstruction::Write(std::vector<uint8_t>& instr)
    {
        instr.push_back((uint8_t)type);
        std::array<uint8_t,4> buff;
        BitConverter::FromUint32BE(buff[0],this->n);
        instr.insert(instr.end(),buff.begin(),buff.end());
    }

    size_t LabelableInstruction::Size()
    {
        return 4;
    }
    void LabelableInstruction::Write(std::vector<uint8_t>& instr)
    {
        
        std::array<uint8_t,4> buff;
        BitConverter::FromUint32BE(buff[0],this->n);
        instr.insert(instr.end(),buff.begin(),buff.end());
    }
    StringInstruction::StringInstruction(uint32_t s)
    {
        this->n = s;
    }
    size_t StringInstruction::Size()
    {
        return 5;
    }
    void StringInstruction::Write(std::vector<uint8_t>& instr)
    {
        instr.push_back(PUSHSTRING);
        std::array<uint8_t,4> buff;
        BitConverter::FromUint32BE(buff[0],this->n);
        instr.insert(instr.end(),buff.begin(),buff.end());
    }
    EmbedInstruction::EmbedInstruction(uint32_t s)
    {
        this->n = s;
    }
    size_t EmbedInstruction::Size()
    {
        return 5;
    }
    void EmbedInstruction::Write(std::vector<uint8_t>& instr)
    {
        instr.push_back(PUSHRESOURCE);
        std::array<uint8_t,4> buff;
        BitConverter::FromUint32BE(buff[0],this->n);
        instr.insert(instr.end(),buff.begin(),buff.end());
    }
    ClosureInstruction::ClosureInstruction(uint32_t s,bool hasScope)
    {
        this->n = s;
        this->hasScope = hasScope;
    }
    size_t ClosureInstruction::Size()
    {
        return 5;
    }
    void ClosureInstruction::Write(std::vector<uint8_t>& instr)
    {
        instr.push_back(this->hasScope ? PUSHCLOSURE : PUSHSCOPELESSCLOSURE);
        std::array<uint8_t,4> buff;
        BitConverter::FromUint32BE(buff[0],this->n);
        instr.insert(instr.end(),buff.begin(),buff.end());
    }
    
    DoubleInstruction::DoubleInstruction(double s)
    {
        this->n = s;
    }
    size_t DoubleInstruction::Size()
    {
        return 9;
    }
    void DoubleInstruction::Write(std::vector<uint8_t>& instr)
    {
        instr.push_back(PUSHDOUBLE);
        std::array<uint8_t,8> buff;
        BitConverter::FromDoubleBE(buff[0],this->n);
        instr.insert(instr.end(),buff.begin(),buff.end());
    }

     CharInstruction::CharInstruction(char s)
    {
        this->n = s;
    }
    size_t CharInstruction::Size()
    {
        return 2;
    }
    void CharInstruction::Write(std::vector<uint8_t>& instr)
    {
        instr.push_back(PUSHCHAR);
        uint8_t buff[8];
        instr.push_back((uint8_t)this->n);
    }
     LongInstruction::LongInstruction(int64_t s)
    {
        this->n = s;
    }
    size_t LongInstruction::Size()
    {
        return 9;
    }
    void LongInstruction::Write(std::vector<uint8_t>& instr)
    {
        instr.push_back(PUSHLONG);
        std::array<uint8_t,8> buff;
        BitConverter::FromUint64BE(buff[0],(uint64_t)this->n);
        instr.insert(instr.end(),buff.begin(),buff.end());
    }
    uint32_t CodeGen::GetString(std::string str)
    {
        for(uint32_t i = 0; i < (uint32_t)this->strs.size();i++)
        {
            if(this->strs[i] == str) return i;
        }
        uint32_t strI = (uint32_t)this->strs.size();
        this->strs.push_back(str);
        return strI;
    }
    uint32_t CodeGen::GetResource(std::shared_ptr<ResourceBase> resource)
    {
        for(uint32_t i = 0; i < (uint32_t)this->res.size();i++)
        {
            if(this->res[i]->IsEqual(resource.get())) return i;
        }
        uint32_t resI = (uint32_t)this->res.size();
        this->res.push_back(resource);
        return resI;
    }
    #define ONE_EXPR(EXPRESSION, INSTRUCTION) if(adv.nodeName == EXPRESSION && adv.nodes.size() == 1) {GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);instructions.push_back(new SimpleInstruction(INSTRUCTION));}
    #define TWO_EXPR(EXPRESSION, INSTRUCTION) if(adv.nodeName == EXPRESSION && adv.nodes.size() == 2) {GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);instructions.push_back(new SimpleInstruction(INSTRUCTION));}

    SyntaxNode CodeGen::StringifyListOfVars(SyntaxNode n)
    {
        if(std::holds_alternative<std::string>(n))
        {
            return n;
        }
        else if(std::holds_alternative<AdvancedSyntaxNode>(n))
        {
            auto itemA=std::get<AdvancedSyntaxNode>(n);
            if(itemA.nodeName == GetVariableExpression && itemA.nodes.size()==1 &&  std::holds_alternative<std::string>(itemA.nodes[0]))
                {
                    return std::get<std::string>(itemA.nodes[0]);
                }
            else if(itemA.nodeName == CommaExpression && itemA.nodes.size()==2)
            {
                return AdvancedSyntaxNode::Create(CommaExpression,true,{StringifyListOfVars(itemA.nodes[0]),StringifyListOfVars(itemA.nodes[1])});
            }



        }
        return nullptr;
    }


    void CodeGen::GenNode(std::vector<ByteCodeInstruction*>& instructions, SyntaxNode n,int32_t scope,int32_t contscope, int32_t brkscope, int32_t contI, int32_t brkI)
    {
        if(std::holds_alternative<std::nullptr_t>(n))
        {
            instructions.push_back(new SimpleInstruction(PUSHNULL));
        }
        else if(std::holds_alternative<Undefined>(n))
        {
            instructions.push_back(new SimpleInstruction(PUSHUNDEFINED));
        }
        else if(std::holds_alternative<bool>(n))
        {
            instructions.push_back(new SimpleInstruction(std::get<bool>(n) ? PUSHTRUE : PUSHFALSE));
        }
        else if(std::holds_alternative<std::string>(n))
        {
            instructions.push_back(new StringInstruction(GetString(std::get<std::string>(n))));
        }
        else if(std::holds_alternative<char>(n))
        {
            instructions.push_back(new CharInstruction(std::get<char>(n)));
        }
        else if(std::holds_alternative<int64_t>(n))
        {
            instructions.push_back(new LongInstruction(std::get<int64_t>(n)));
        }
        else if(std::holds_alternative<double>(n))
        {
            instructions.push_back(new DoubleInstruction(std::get<double>(n)));
        }
        else if(std::holds_alternative<std::vector<uint8_t>>(n))
        {
            ResourceByteArray ba;
            ba.data = std::get<std::vector<uint8_t>>(n);
            instructions.push_back(new EmbedInstruction(GetResource(std::make_shared<ResourceByteArray>(ba))));
        }
        else if(std::holds_alternative<AdvancedSyntaxNode>(n))
        {
            auto adv = std::get<AdvancedSyntaxNode>(n);
            ONE_EXPR(NotExpression, NOT)
            ONE_EXPR(BitwiseNotExpression, BITWISENOT)
            ONE_EXPR(NegativeExpression,NEGATIVE)
            TWO_EXPR(AddExpression, ADD)
            TWO_EXPR(SubExpression, SUB)
            TWO_EXPR(TimesExpression, TIMES)
            TWO_EXPR(DivideExpression, DIVIDE)
            TWO_EXPR(ModExpression, MODULO)
            TWO_EXPR(LeftShiftExpression, LEFTSHIFT)
            TWO_EXPR(RightShiftExpression, RIGHTSHIFT)
            TWO_EXPR(BitwiseOrExpression, BITWISEOR)
            TWO_EXPR(BitwiseAndExpression, BITWISEAND)
            TWO_EXPR(LessThanExpression, LESSTHAN)
            TWO_EXPR(GreaterThanExpression, GREATERTHAN)
            TWO_EXPR(LessThanEqualsExpression, LESSTHANEQ)
            TWO_EXPR(GreaterThanEqualsExpression, GREATERTHANEQ)
            TWO_EXPR(NotEqualsExpression, NEQ)
            TWO_EXPR(EqualsExpression, EQ)
            TWO_EXPR(XOrExpression, XOR)
            if(adv.nodeName == NullCoalescingExpression && adv.nodes.size() == 2)
            {
                uint32_t ifId = NewId();
                std::string ifIdTrue = "__compGenTrue";
                ifIdTrue.append(std::to_string(ifId));
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new JumpStyleInstruction(Instruction::JMPIFDEFINED, ifIdTrue));
                GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new LabelInstruction(ifIdTrue));    
                
            }
            else if(adv.nodeName == ClassStatement && adv.nodes.size() >= 3 && std::holds_alternative<std::string>(adv.nodes[0]))
            {
                CodeGenClass cls;
                cls.documentation = GetString(std::get<std::string>(adv.nodes[0]));
                GetFunctionName(cls.name,adv.nodes[1]);
                GetFunctionName(cls.inherits,adv.nodes[2]);
                for(size_t i = 3; i < adv.nodes.size(); i++)
                {
                    auto& node = adv.nodes[i];
                    if(std::holds_alternative<AdvancedSyntaxNode>(node))
                    {
                        auto& adv2 = std::get<AdvancedSyntaxNode>(node);
                        CodeGenClassEntry ent;
                        ent.type = 0;
                        if(adv2.nodes.size() >= 2 && std::holds_alternative<std::string>(adv2.nodes[0]) && std::holds_alternative<std::string>(adv2.nodes[1]))
                        {
                            ent.documentation=GetString(std::get<std::string>(adv2.nodes[0]));
                            std::string type=std::get<std::string>(adv2.nodes[1]);
                            if(type == "private") ent.type = 0;
                            else if(type == "protected") ent.type = 1;
                            else if(type == "public") ent.type = 2;
                            else if(type == "static") ent.type = 3;
                        }

                        if(adv2.nodeName == MethodStatement && adv2.nodes.size() == 4 && std::holds_alternative<AdvancedSyntaxNode>(adv2.nodes[2]) )
                        {
                            

                            //documentation,myTkn.text,nameAndArgs,closureData
                            size_t fnindex=this->chunks.size();
                            ent.closure=(uint32_t)fnindex;
                            this->chunks.resize(fnindex+1);
                            auto& nameAndArgs= std::get<AdvancedSyntaxNode>(adv2.nodes[2]);
                            if(nameAndArgs.nodeName == FunctionCallExpression && !nameAndArgs.nodes.empty() && std::holds_alternative<AdvancedSyntaxNode>(nameAndArgs.nodes[0]))
                            {
                                auto& getvar = std::get<AdvancedSyntaxNode>(nameAndArgs.nodes[0]);
                                if(getvar.nodeName == GetVariableExpression && getvar.nodes.size() == 1 && std::holds_alternative<std::string>(getvar.nodes[0]))
                                {
                               
                                    ent.name = GetString(std::get<std::string>(getvar.nodes[0]));
                                    if(nameAndArgs.nodes.size() > 1)
                                    {
                                        GetFunctionArgs(ent.arguments, nameAndArgs.nodes[1]);
                                    }
                                }
                                else continue;
                            }

                            std::vector<ByteCodeInstruction*> fnInstructions;
                            
                            GenNode(fnInstructions,adv2.nodes[3],0,-1,-1,-1,-1);

                            this->chunks[fnindex] = std::pair<std::vector<uint32_t>,std::vector<ByteCodeInstruction*>>(ent.arguments, fnInstructions);
                        }
                        else if(adv2.nodeName == AbstractMethodStatement && adv2.nodes.size() == 3 && std::holds_alternative<AdvancedSyntaxNode>(adv2.nodes[2]))
                        {
                            ent.closure = 0;
                            ent.type |= 0b00001000;
                            //documentation,myTkn.text,nameAndArgs
                            auto& nameAndArgs= std::get<AdvancedSyntaxNode>(adv2.nodes[2]);
                            if(nameAndArgs.nodeName == FunctionCallExpression && !nameAndArgs.nodes.empty() && std::holds_alternative<AdvancedSyntaxNode>(nameAndArgs.nodes[0]))
                            {
                                auto& getvar = std::get<AdvancedSyntaxNode>(nameAndArgs.nodes[0]);
                                if(getvar.nodeName == GetVariableExpression && getvar.nodes.size() == 1 && std::holds_alternative<std::string>(getvar.nodes[0]))
                                {
                                    ent.name = GetString(std::get<std::string>(getvar.nodes[0]));
                                    if(nameAndArgs.nodes.size() > 1)
                                    {
                                        GetFunctionArgs(ent.arguments, nameAndArgs.nodes[1]);
                                    }
                                } else continue;

                               
                            }

                        }
                        else if(adv2.nodeName == FieldStatement && adv2.nodes.size() == 3 && std::holds_alternative<AdvancedSyntaxNode>(adv2.nodes[2]))
                        {
                            auto& setter = std::get<AdvancedSyntaxNode>(adv2.nodes[2]);    
                            if(setter.nodeName == GetVariableExpression && setter.nodes.size() == 1 && std::holds_alternative<std::string>(setter.nodes[0]))
                            {
                                ent.closure = 0;
                                ent.type |= 0b00001100;
                               
                                ent.name = GetString(std::get<std::string>(setter.nodes[0]));
                                
                            }
                            else if(setter.nodeName == AssignExpression && setter.nodes.size() == 2 && std::holds_alternative<AdvancedSyntaxNode>(setter.nodes[0]))
                            {
                                auto& getvar = std::get<AdvancedSyntaxNode>(setter.nodes[0]);
                                if(getvar.nodeName == GetVariableExpression && getvar.nodes.size() == 1 && std::holds_alternative<std::string>(getvar.nodes[0]))
                                {
                                    ent.type |= 0b00000100;
                                    ent.name = GetString(std::get<std::string>(getvar.nodes[0]));
                                    size_t fnindex=this->chunks.size();
                                    ent.closure=(uint32_t)fnindex;
                                    ent.arguments={};
                                
                                    this->chunks.resize(fnindex+1);

                                    std::vector<ByteCodeInstruction*> fnInstructions;
                            
                                    GenNode(fnInstructions,AdvancedSyntaxNode::Create(ReturnStatement,false,{setter.nodes[1]}),0,-1,-1,-1,-1);

                                    this->chunks[fnindex] = std::pair<std::vector<uint32_t>,std::vector<ByteCodeInstruction*>>({}, fnInstructions);
                                }
                                else continue;
                            }
                            else continue;
                        }
                        cls.entries.push_back(ent);
                    }
                }
                classes.push_back(cls);
            }
            else if(adv.nodeName == RelativePathExpression)
            {
                instructions.push_back(new SimpleInstruction(Instruction::PUSHRELATIVEPATH));
            }
            else if(adv.nodeName == RootPathExpression)
            {
                instructions.push_back(new SimpleInstruction(Instruction::PUSHROOTPATH));
            }
            else if(adv.nodeName == SwitchStatement && adv.nodes.size() == 2)
            {
                //THIS CODE WORKED FIRST TRY, I DON'T SEE THAT EVERY DAY, PRAISE GOD!!!!!!!
                auto expr = adv.nodes[0];

                std::vector<SyntaxNode> nodes_before;
            
                SyntaxNode currentCase = nullptr;
                std::vector<SyntaxNode> currentNodes;
                std::string defaultJmp = {};

                std::vector<std::pair<std::pair<std::string,AdvancedSyntaxNode>,std::vector<SyntaxNode>>> snodes;

                

                if(std::holds_alternative<AdvancedSyntaxNode>(adv.nodes[1]))
                {
                    auto body = std::get<AdvancedSyntaxNode>(adv.nodes[1]);
                    if(body.nodeName == ScopeNode)
                    {
                        for(auto item : body.nodes)
                        {
                            if(std::holds_alternative<AdvancedSyntaxNode>(item))
                            {
                                auto no = std::get<AdvancedSyntaxNode>(item);
                                if(no.nodeName == CaseStatement || no.nodeName == DefaultStatement)
                                {
                                    if(std::holds_alternative<AdvancedSyntaxNode>(currentCase))
                                    {
                                         
                                        uint32_t jmpId = NewId();
                                        std::string jmpIdStr = "__compGenJmp";
                                        jmpIdStr.append(std::to_string(jmpId));
                                        snodes.push_back(std::pair<std::pair<std::string,AdvancedSyntaxNode>,std::vector<SyntaxNode>>(std::pair<std::string,AdvancedSyntaxNode>(jmpIdStr,std::get<AdvancedSyntaxNode>(currentCase)),currentNodes));
                                        currentNodes={};
                                    }
                                    currentCase = no;
                                    continue;
                                }
                                
                            }

                            if(std::holds_alternative<AdvancedSyntaxNode>(currentCase))
                            {
                                currentNodes.push_back(item);
                            }
                            else
                            {
                                nodes_before.push_back(item);
                            }

                        }
                        if(std::holds_alternative<AdvancedSyntaxNode>(currentCase))
                        {
                                         
                            uint32_t jmpId = NewId();
                            std::string jmpIdStr = "__compGenJmp";
                            jmpIdStr.append(std::to_string(jmpId));
                            snodes.push_back(std::pair<std::pair<std::string,AdvancedSyntaxNode>,std::vector<SyntaxNode>>(std::pair<std::string,AdvancedSyntaxNode>(jmpIdStr,std::get<AdvancedSyntaxNode>(currentCase)),currentNodes));
                            currentNodes={};
                        }

                        uint32_t endId = NewId();
                        std::string endIdStr = "__compGenBrk";
                        endIdStr.append(std::to_string(endId));
                        for(auto item : nodes_before)
                        {
                            GenNode(instructions,item,scope,contscope,brkscope,contI,brkI);
                        }
                        
                        for(auto item : snodes)
                        {
                            if(item.first.second.nodeName == CaseStatement)
                            {
                                auto eq = AdvancedSyntaxNode::Create(EqualsExpression,true,{
                                    item.first.second.nodes[0],
                                    adv.nodes[0]
                                });
                                GenNode(instructions,eq,scope,contscope,brkscope,contI,brkI);
                                instructions.push_back(new JumpStyleInstruction(JMPC,item.first.first));
                            }
                            if(item.first.second.nodeName == DefaultStatement)
                            {
                                if(!defaultJmp.empty()) std::cout << "ERROR: multiple default in switch statement will cause undefined behaviour, this is not an exception due to not allowing exceptions in codegen stage (the compilation shouldn't fail)" << std::endl;
                                defaultJmp = item.first.first;
                            }
                        }

                        if(defaultJmp.empty())
                        {
                            instructions.push_back(new JumpStyleInstruction(JMP,endIdStr));
                        }
                        else
                        {
                            instructions.push_back(new JumpStyleInstruction(JMP,defaultJmp));
                        }

                        for(auto item : snodes)
                        {
                            instructions.push_back(new LabelInstruction(item.first.first));
                            for(auto item2 : item.second)
                            {
                                GenNode(instructions,item2,scope,contscope,scope,contI,endId);
                            }
                        }

                        instructions.push_back(new LabelInstruction(endIdStr));
                    }
                }
                return;
            }
            if(adv.nodeName == TernaryExpression && adv.nodes.size() == 3)
            {
                 uint32_t ifId = NewId();
                std::string ifIdTrue = "__compGenTrue";
                ifIdTrue.append(std::to_string(ifId));
                std::string ifIdEnd = "__compGenEnd";
                ifIdEnd.append(std::to_string(ifId));

                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new JumpStyleInstruction(JMPC,ifIdTrue));
                if(!std::holds_alternative<std::nullptr_t>(adv.nodes[2]))
                {
                    GenNode(instructions,adv.nodes[2],scope,contscope,brkscope,contI,brkI);
                   
                }
                instructions.push_back(new JumpStyleInstruction(JMP,ifIdEnd));
                instructions.push_back(new LabelInstruction(ifIdTrue));    
                if(!std::holds_alternative<std::nullptr_t>(adv.nodes[1]))
                {
                    GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                   
                }
                instructions.push_back(new LabelInstruction(ifIdEnd));

            }
            else if(adv.nodeName == CompoundAssignExpression && adv.nodes.size() == 1 && std::holds_alternative<AdvancedSyntaxNode>(adv.nodes[0]))
            {
                auto data = std::get<AdvancedSyntaxNode>(adv.nodes[0]);
                if(data.nodes.size() == 2)
                {
                    auto d = AdvancedSyntaxNode::Create(AssignExpression,true,{
                        data.nodes[0],
                        data
                    });
                    GenNode(instructions,d,scope,contscope,brkscope,contI,brkI);
                }
            }
            else if(adv.nodeName == PrefixIncrementExpression && adv.nodes.size() == 1)
            {  
                GenNode(instructions,
                AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{
                    AdvancedSyntaxNode::Create(AddExpression,true,{
                        adv.nodes[0]
                        ,
                        (int64_t)1
                    })
                })
                ,scope,contscope,brkscope,contI,brkI);
            }
            else if(adv.nodeName == PrefixDecrementExpression && adv.nodes.size() == 1)
            {    
                GenNode(instructions,
                AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{
                    AdvancedSyntaxNode::Create(SubExpression,true,{
                        adv.nodes[0]
                        ,
                        (int64_t)1
                    })
                })
                ,scope,contscope,brkscope,contI,brkI);
            }
            else if(adv.nodeName == PostfixIncrementExpression && adv.nodes.size() == 1)
            {
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                GenNode(instructions,
                AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{
                    AdvancedSyntaxNode::Create(AddExpression,true,{
                        adv.nodes[0]
                        ,
                        (int64_t)1
                    })
                })
                ,scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new SimpleInstruction(POP));
            }
            else if(adv.nodeName == PostfixDecrementExpression && adv.nodes.size() == 1)
            {
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                GenNode(instructions,
                AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{
                    AdvancedSyntaxNode::Create(SubExpression,true,{
                        adv.nodes[0]
                        ,
                        (int64_t)1
                    })
                })
                ,scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new SimpleInstruction(POP));
            }
            else if(adv.nodeName == LogicalOrExpression && adv.nodes.size() == 2)
            {
                auto data= AdvancedSyntaxNode::Create(TernaryExpression,true,{adv.nodes[0],true, AdvancedSyntaxNode::Create(TernaryExpression,true,{adv.nodes[1],true,false}) });
                GenNode(instructions,data,scope,contscope,brkscope,contI,brkI);
            }
            else if(adv.nodeName == LogicalAndExpression && adv.nodes.size() == 2)
            {
                auto data= AdvancedSyntaxNode::Create(TernaryExpression,true,{adv.nodes[0], AdvancedSyntaxNode::Create(TernaryExpression,true,{adv.nodes[1],true,false}),false});
                GenNode(instructions,data,scope,contscope,brkscope,contI,brkI);
            }
            else if(adv.nodeName == EachStatement && adv.nodes.size() == 3)
            {
                auto item = adv.nodes[0];
                auto list = adv.nodes[1];
                auto body = adv.nodes[2];

                uint32_t compGenId = NewId();
                std::string compGenIttr = "__compGenIttr";
                compGenIttr.append(std::to_string(compGenId));

                auto each = AdvancedSyntaxNode::Create(NodeList,false,{
                    AdvancedSyntaxNode::Create(AssignExpression,true, {
                        AdvancedSyntaxNode::Create(DeclareExpression,true,{compGenIttr}),
                        AdvancedSyntaxNode::Create(FunctionCallExpression,true,{AdvancedSyntaxNode::Create(GetFieldExpression,true,{list,"GetEnumerator"})})
                    }),
                    AdvancedSyntaxNode::Create(DeferStatement,false,{AdvancedSyntaxNode::Create(FunctionCallExpression,true,{AdvancedSyntaxNode::Create(GetFieldExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{compGenIttr}), "Dispose"})})}),
                    AdvancedSyntaxNode::Create(WhileStatement,false,{
                        AdvancedSyntaxNode::Create(FunctionCallExpression,true,{AdvancedSyntaxNode::Create(GetFieldExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{compGenIttr}),"MoveNext" }),}),
                        AdvancedSyntaxNode::Create(NodeList,false,{
                            AdvancedSyntaxNode::Create(AssignExpression,true,{
                                item,
                                AdvancedSyntaxNode::Create(GetFieldExpression,true,{
                                    AdvancedSyntaxNode::Create(GetVariableExpression,true,{
                                        compGenIttr
                                    }),
                                    "Current"
                                })
                            }),
                            body
                        })
                    })
                });
                GenNode(instructions,each,scope,contscope,brkscope,contI,brkI);

            }
            else if(adv.nodeName == ArrayExpression)
            {
                instructions.push_back(new SimpleInstruction(CREATEARRAY));
                std::vector<SyntaxNode> itms;
                 if(adv.nodes.size() > 0)
                
                GetFunctionArgs(itms,adv.nodes[0]);
                for(auto item : itms)
                {
                    GenNode(instructions,item,scope,contscope,brkscope,contI,brkI);
                    instructions.push_back(new SimpleInstruction(APPENDLIST));
                }
            }
            else if(adv.nodeName == DictionaryExpression)
            {
                instructions.push_back(new SimpleInstruction(CREATEDICTIONARY));
                std::vector<SyntaxNode> itms;
                if(adv.nodes.size() > 0)
                GetFunctionArgs(itms,adv.nodes[0]);
                for(auto item : itms)
                {
                    if(std::holds_alternative<AdvancedSyntaxNode>(item))
                    {
                        auto tkn = std::get<AdvancedSyntaxNode>(item);
                        if(tkn.nodeName == GetVariableExpression && !tkn.nodes.empty())
                        {
                            GenNode(instructions,tkn.nodes[0],scope,contscope,brkscope,contI,brkI);
                            GenNode(instructions,item,scope,contscope,brkscope,contI,brkI);
                            instructions.push_back(new SimpleInstruction(APPENDDICT));
                        }
                        else if(tkn.nodeName == AssignExpression && tkn.nodes.size()==2 && std::holds_alternative<AdvancedSyntaxNode>(tkn.nodes[0]))
                        {
                            auto myTn =  std::get<AdvancedSyntaxNode>(tkn.nodes[0]);
                            if(myTn.nodeName == GetVariableExpression && !myTn.nodes.empty())
                            {
                                GenNode(instructions,myTn.nodes[0],scope,contscope,brkscope,contI,brkI);
                                GenNode(instructions,tkn.nodes[1],scope,contscope,brkscope,contI,brkI);
                                instructions.push_back(new SimpleInstruction(APPENDDICT));
                            }
                        }
                    }
                }
            }
            else if(adv.nodeName == FunctionCallExpression && adv.nodes.size() >= 1 && std::holds_alternative<AdvancedSyntaxNode>(adv.nodes[0]))
            {
                auto v = std::get<AdvancedSyntaxNode>(adv.nodes[0]);
                if(v.nodeName == GetFieldExpression && v.nodes.size() == 2)
                {
                    GenNode(instructions,v.nodes[0],scope,contscope,brkscope,contI,brkI);
                    GenNode(instructions,v.nodes[1],scope,contscope,brkscope,contI,brkI);
                    if(adv.nodes.size()==2)
                    {
                        std::vector<SyntaxNode> nodes;
                        GetFunctionArgs(nodes,adv.nodes[1]);
                        for(auto item : nodes)
                        GenNode(instructions,item,scope,contscope,brkscope,contI,brkI);
                        instructions.push_back(new LongInstruction((int64_t)nodes.size()));
                    }
                    else {
                        instructions.push_back(new LongInstruction(0));
                    }
                    instructions.push_back(new SimpleInstruction(CALLMETHOD));
                }
                else if(v.nodeName != GetFieldExpression)
                {
                    GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                    if(adv.nodes.size()==2)
                    {
                        std::vector<SyntaxNode> nodes;
                        GetFunctionArgs(nodes,adv.nodes[1]);
                        for(auto item : nodes)
                        GenNode(instructions,item,scope,contscope,brkscope,contI,brkI);
                        instructions.push_back(new LongInstruction((int64_t)nodes.size()));
                    }
                    else {
                        instructions.push_back(new LongInstruction(0));
                    }
                    instructions.push_back(new SimpleInstruction(CALLFUNCTION));
                }
            }
            else if(adv.nodeName == IfStatement && adv.nodes.size() == 3)
            {
                uint32_t ifId = NewId();
                std::string ifIdTrue = "__compGenTrue";
                ifIdTrue.append(std::to_string(ifId));
                std::string ifIdEnd = "__compGenEnd";
                ifIdEnd.append(std::to_string(ifId));

                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new JumpStyleInstruction(JMPC,ifIdTrue));
                if(!std::holds_alternative<std::nullptr_t>(adv.nodes[2]))
                {
                    GenNode(instructions,adv.nodes[2],scope,contscope,brkscope,contI,brkI);
                    GenPop(instructions,adv.nodes[2]);
                }
                instructions.push_back(new JumpStyleInstruction(JMP,ifIdEnd));
                instructions.push_back(new LabelInstruction(ifIdTrue));    
                if(!std::holds_alternative<std::nullptr_t>(adv.nodes[1]))
                {
                    GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                    GenPop(instructions,adv.nodes[1]);
                }
                instructions.push_back(new LabelInstruction(ifIdEnd));

            }
            else if(adv.nodeName == WhileStatement && adv.nodes.size() == 2)
            {
                auto old_contI = contI;
                auto old_brkI = brkI;
                auto old_contscope = contscope;
                auto old_brkscope = brkscope;

                contscope = scope;
                brkscope = scope;

                uint32_t whileId = NewId();
                std::string whileIdCont = "__compGenCont";
                whileIdCont.append(std::to_string(whileId));
                std::string whileIdBrk = "__compGenBrk";
                whileIdBrk.append(std::to_string(whileId));
                
                contI = whileId;
                brkI = whileId;

                instructions.push_back(new LabelInstruction(whileIdCont));
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new SimpleInstruction(NOT));
                instructions.push_back(new JumpStyleInstruction(JMPC,whileIdBrk));
                
                if(!std::holds_alternative<std::nullptr_t>(adv.nodes[1]))
                {
                    GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                    GenPop(instructions,adv.nodes[1]);
                }

                instructions.push_back(new JumpStyleInstruction(JMP,whileIdCont));
                instructions.push_back(new LabelInstruction(whileIdBrk));

                contI = old_contI;
                brkI = old_brkI;
                contscope = old_contscope;
                brkscope = old_brkscope;

            }
            else if(adv.nodeName == DoStatement && adv.nodes.size() == 2)
            {
                auto old_contI = contI;
                auto old_brkI = brkI;
                auto old_contscope = contscope;
                auto old_brkscope = brkscope;

                contscope = scope;
                brkscope = scope;

                uint32_t doId = NewId();
                std::string doIdCont = "__compGenCont";
                doIdCont.append(std::to_string(doId));
                std::string doIdBrk = "__compGenBrk";
                doIdBrk.append(std::to_string(doId));
                std::string doIdStart = "__compGenStart";
                doIdStart.append(std::to_string(doId));
                
                contI = doId;
                brkI = doId;

                
                instructions.push_back(new LabelInstruction(doIdStart));
                
                if(!std::holds_alternative<std::nullptr_t>(adv.nodes[1]))
                {
                    GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                    GenPop(instructions,adv.nodes[1]);
                }
                instructions.push_back(new LabelInstruction(doIdCont));
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);

                instructions.push_back(new JumpStyleInstruction(JMPC,doIdStart));
                instructions.push_back(new LabelInstruction(doIdBrk));

                contI = old_contI;
                brkI = old_brkI;
                contscope = old_contscope;
                brkscope = old_brkscope;

            }
            else if(adv.nodeName == ForStatement && adv.nodes.size() == 4)
            {
                if(!std::holds_alternative<std::nullptr_t>(adv.nodes[0]))
                {
                    GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                    GenPop(instructions,adv.nodes[0]);
                }
                auto old_contI = contI;
                auto old_brkI = brkI;
                auto old_contscope = contscope;
                auto old_brkscope = brkscope;

                contscope = scope;
                brkscope = scope;

                uint32_t doId = NewId();
                std::string doIdCont = "__compGenCont";
                doIdCont.append(std::to_string(doId));
                std::string doIdBrk = "__compGenBrk";
                doIdBrk.append(std::to_string(doId));
                std::string doIdStart = "__compGenStart";
                doIdStart.append(std::to_string(doId));
                std::string doIdStart2 = "__compGenCond";
                doIdStart2.append(std::to_string(doId));
                
                contI = doId;
                brkI = doId;

                instructions.push_back(new JumpStyleInstruction(JMP,doIdStart2));
                instructions.push_back(new LabelInstruction(doIdStart));
                if(!std::holds_alternative<std::nullptr_t>(adv.nodes[3]))
                {
                    GenNode(instructions,adv.nodes[3],scope,contscope,brkscope,contI,brkI);
                    GenPop(instructions,adv.nodes[3]);
                }

                instructions.push_back(new LabelInstruction(doIdCont));
                if(!std::holds_alternative<std::nullptr_t>(adv.nodes[2]))
                {
                    GenNode(instructions,adv.nodes[2],scope,contscope,brkscope,contI,brkI);
                    GenPop(instructions,adv.nodes[2]);
                }
                instructions.push_back(new LabelInstruction(doIdStart2));
                GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new JumpStyleInstruction(JMPC,doIdStart));
                instructions.push_back(new LabelInstruction(doIdBrk));
                 contI = old_contI;
                brkI = old_brkI;
                contscope = old_contscope;
                brkscope = old_brkscope; 
            }
            else if(adv.nodeName == CommaExpression && adv.nodes.size() == 2)
            {
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                GenPop(instructions,adv.nodes[0]);
                GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                
            }
            else if(adv.nodeName == ContinueStatement)
            {
                if( contscope == -1)
                {
                    std::cout << "WARN: continue does nothing here\n";
                }
                else if(contscope == -2)
                {
                    instructions.push_back(new SimpleInstruction(PUSHCONTINUE));
                    instructions.push_back(new SimpleInstruction(RET));
                }
                else
                {
                    auto cont = scope-contscope;
                    if(cont > 0)
                    instructions.push_back(new ScopeEndTimesInstruction((uint32_t)cont));
                    
                    std::string myJmp = "__compGenCont";
                    myJmp.append(std::to_string(contI));

                    instructions.push_back(new JumpStyleInstruction(JMP,myJmp));
                }
            }
            else if(adv.nodeName == BreakStatement)
            {
                if( brkscope == -1)
                {
                    std::cout << "WARN: break does nothing here\n";
                }
                else if(brkscope == -2)
                {
                    instructions.push_back(new SimpleInstruction(PUSHBREAK));
                    instructions.push_back(new SimpleInstruction(RET));
                }
                else
                {
                    auto _brk = scope-brkscope;
                    if(_brk > 0)
                    instructions.push_back(new ScopeEndTimesInstruction((uint32_t)_brk));
                    
                    std::string myJmp = "__compGenBrk";
                    myJmp.append(std::to_string(brkI));

                    instructions.push_back(new JumpStyleInstruction(JMP,myJmp));
                }
            }
            else if(adv.nodeName == GetVariableExpression && adv.nodes.size() == 1)
            {
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new SimpleInstruction(GETVARIABLE));
            }
            else if(adv.nodeName == GetFieldExpression && adv.nodes.size() == 2)
            {
                
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                
                instructions.push_back(new SimpleInstruction(GETFIELD));
             
            }
            else if(adv.nodeName == GetArrayExpression && adv.nodes.size() == 2)
            {
                SyntaxNode n = AdvancedSyntaxNode::Create(FunctionCallExpression,true,{
                    AdvancedSyntaxNode::Create(GetFieldExpression,true,{adv.nodes[0],"GetAt"}),
                    adv.nodes[1]
                });
                GenNode(instructions,n,scope,contscope,brkscope,contI,brkI);
            }
            else if(adv.nodeName == AssignExpression && adv.nodes.size() == 2 && std::holds_alternative<AdvancedSyntaxNode>(adv.nodes[0]))
            {
                auto varNode = std::get<AdvancedSyntaxNode>(adv.nodes[0]);

                if(varNode.nodeName == GetVariableExpression && varNode.nodes.size() == 1)
                {
                    GenNode(instructions,varNode.nodes[0],scope,contscope,brkscope,contI,brkI);
                    GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                    instructions.push_back(new SimpleInstruction(SETVARIABLE));
                }
                else if(varNode.nodeName == ArrayExpression && varNode.nodes.size() >= 1)
                {

                    auto vars= StringifyListOfVars(varNode.nodes[0]);


                    auto nArray = AdvancedSyntaxNode::Create(ArrayExpression,true,{vars});

                    this->GenNode(instructions,nArray ,scope ,contscope ,brkscope ,contI , brkI);
                    GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                    instructions.push_back(new SimpleInstruction(SETVARIABLE));
                }
                else if(varNode.nodeName == DeclareExpression && varNode.nodes.size() == 1 && std::holds_alternative<std::string>(varNode.nodes[0]))
                {
                    GenNode(instructions,varNode.nodes[0],scope,contscope,brkscope,contI,brkI);
                    GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                    instructions.push_back(new SimpleInstruction(DECLAREVARIABLE));
                }
                else if(varNode.nodeName == DeclareExpression && varNode.nodes.size() == 1 && std::holds_alternative<AdvancedSyntaxNode>(varNode.nodes[0]))
                {
                    auto adv2 = std::get<AdvancedSyntaxNode>(varNode.nodes[0]);

                    if(adv2.nodeName == ArrayExpression && adv2.nodes.size() == 1)
                    {

                        auto vars= StringifyListOfVars(adv2.nodes[0]);

                        auto nArray = AdvancedSyntaxNode::Create(ArrayExpression,true,{vars});

                        this->GenNode(instructions,nArray ,scope ,contscope ,brkscope ,contI , brkI);
                        GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                        instructions.push_back(new SimpleInstruction(DECLAREVARIABLE));

                    }
                }
                else if(varNode.nodeName == GetFieldExpression && varNode.nodes.size() == 2)
                {
                    GenNode(instructions,varNode.nodes[0],scope,contscope,brkscope,contI,brkI);

                    GenNode(instructions,varNode.nodes[1],scope,contscope,brkscope,contI,brkI);
                    GenNode(instructions,adv.nodes[1],scope,contscope,brkscope,contI,brkI);
                    instructions.push_back(new SimpleInstruction(SETFIELD));
                }
                else if(varNode.nodeName == GetArrayExpression && varNode.nodes.size() == 2)
                {
                    SyntaxNode n = AdvancedSyntaxNode::Create(FunctionCallExpression,true,{
                        AdvancedSyntaxNode::Create(GetFieldExpression,true,{varNode.nodes[0],"SetAt"}),
                       AdvancedSyntaxNode::Create(CommaExpression,true,
                       {
                        varNode.nodes[1],
                        adv.nodes[1]
                       })
                    });
                    GenNode(instructions,n,scope,contscope,brkscope,contI,brkI);
                }
            }
            else if(adv.nodeName == BreakpointStatement && adv.nodes.size() == 5)
            {
                instructions.push_back(new SimpleInstruction(CREATEDICTIONARY));
                instructions.push_back(new StringInstruction(GetString("Data")));
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new SimpleInstruction(APPENDDICT));
                instructions.push_back(new StringInstruction(GetString("Filename")));
                GenNode(instructions,adv.nodes[1] ,scope,contscope ,brkscope ,contI ,brkI);
                instructions.push_back(new SimpleInstruction(APPENDDICT));
                instructions.push_back(new StringInstruction(GetString("Line")));
                GenNode(instructions,adv.nodes[2] ,scope,contscope ,brkscope ,contI ,brkI);
                instructions.push_back(new SimpleInstruction(APPENDDICT));
                instructions.push_back(new StringInstruction(GetString("Column")));
                GenNode(instructions,adv.nodes[3] ,scope,contscope ,brkscope ,contI ,brkI);
                instructions.push_back(new SimpleInstruction(APPENDDICT));
                instructions.push_back(new StringInstruction(GetString("Offset")));
                GenNode(instructions,adv.nodes[4] ,scope,contscope ,brkscope ,contI ,brkI);
                instructions.push_back(new SimpleInstruction(APPENDDICT));
                instructions.push_back(new SimpleInstruction(BREAKPOINT));
            }
            else if(adv.nodeName == ThrowStatement && adv.nodes.size() == 5)
            {
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new StringInstruction(GetString("Filename")));
                GenNode(instructions,adv.nodes[1] ,scope,contscope ,brkscope ,contI ,brkI);
                instructions.push_back(new SimpleInstruction(APPENDDICT));
                instructions.push_back(new StringInstruction(GetString("Line")));
                GenNode(instructions,adv.nodes[2] ,scope,contscope ,brkscope ,contI ,brkI);
                instructions.push_back(new SimpleInstruction(APPENDDICT));
                instructions.push_back(new StringInstruction(GetString("Column")));
                GenNode(instructions,adv.nodes[3] ,scope,contscope ,brkscope ,contI ,brkI);
                instructions.push_back(new SimpleInstruction(APPENDDICT));
                instructions.push_back(new StringInstruction(GetString("Offset")));
                GenNode(instructions,adv.nodes[4] ,scope,contscope ,brkscope ,contI ,brkI);
                instructions.push_back(new SimpleInstruction(APPENDDICT));
                instructions.push_back(new SimpleInstruction(THROW));
            }
            else if(adv.nodeName == ReturnStatement && adv.nodes.size() == 1)
            {
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new SimpleInstruction(RET));
            }
            else if(adv.nodeName == YieldStatement && adv.nodes.size() == 1)
            {
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new SimpleInstruction(YIELD));
            }
            else if(adv.nodeName == ParenthesesExpression && adv.nodes.size() == 1)
            {
                GenNode(instructions,adv.nodes[0],scope,contscope,brkscope,contI,brkI);
            }
            else if(adv.nodeName == EmbedExpression && adv.nodes.size() == 1 && std::holds_alternative<std::string>(adv.nodes[0]))
            {
                std::string filename = std::get<std::string>(adv.nodes[0]);
                instructions.push_back(new EmbedInstruction(GetResource(std::make_shared<ResourceFile>(filename))));

            }
            else if(adv.nodeName == HtmlRootExpression)
            {
                scope++;
                instructions.push_back(new SimpleInstruction(SCOPEBEGIN));
               
                instructions.push_back(new StringInstruction(GetString(std::get<std::string>(adv.nodes[0]))));
                instructions.push_back(new StringInstruction(GetString("")));
                instructions.push_back(new SimpleInstruction(DECLAREVARIABLE));
                instructions.push_back(new SimpleInstruction(POP));

                for(size_t i = 1; i < adv.nodes.size(); i++)
                {
                    GenNode(instructions,adv.nodes[i],scope,contscope,brkscope,contI,brkI);
                    GenPop(instructions,adv.nodes[i]);
                }
                
                instructions.push_back(new StringInstruction(GetString(std::get<std::string>(adv.nodes[0]))));
                instructions.push_back(new SimpleInstruction(GETVARIABLE));
                instructions.push_back(new SimpleInstruction(SCOPEEND));
                scope--;
            }
            else if(adv.nodeName == ScopeNode)
            {
                scope++;
                instructions.push_back(new SimpleInstruction(SCOPEBEGIN));
                for(size_t i = 0; i < adv.nodes.size(); i++)
                {
                    GenNode(instructions,adv.nodes[i],scope,contscope,brkscope,contI,brkI);
                    if(!adv.isExpression || i < adv.nodes.size()-1)
                        GenPop(instructions,adv.nodes[i]);
                }
                instructions.push_back(new SimpleInstruction(SCOPEEND));
                scope--;
            }
             else if(adv.nodeName == NodeList)
            {
                
                for(auto item : adv.nodes)
                {
                    GenNode(instructions,item,scope,contscope,brkscope,contI,brkI);
                    GenPop(instructions,item);
                }
            }
            else if(adv.nodeName == DeferStatement && adv.nodes.size() == 1)
            {
                GenNode(instructions,AdvancedSyntaxNode::Create(ScopelessClosureExpression,true,{AdvancedSyntaxNode::Create(ParenthesesExpression,true,{}),adv.nodes[0]}),scope,contscope,brkscope,contI,brkI);
                instructions.push_back(new SimpleInstruction(DEFER));
            }
            else if(adv.nodeName == TryStatement && adv.nodes.size() == 4)
            {
                //AdvancedSyntaxNode::Create(TryStatement, false, {tryBody,catchNode,catchEx, finally});
                if(std::holds_alternative<AdvancedSyntaxNode>(adv.nodes[3]))
                {
                    GenNode(instructions,AdvancedSyntaxNode::Create(DeferStatement,false, {adv.nodes[3]}),scope,contscope,brkscope,contI,brkI);
                }
                if(std::holds_alternative<AdvancedSyntaxNode>(adv.nodes[0]) && std::holds_alternative<AdvancedSyntaxNode>(adv.nodes[1]) && std::holds_alternative<AdvancedSyntaxNode>(adv.nodes[2]))
                {
                    GenNode(instructions,AdvancedSyntaxNode::Create(ScopelessClosureExpression,true,{AdvancedSyntaxNode::Create(ParenthesesExpression,true,{}),adv.nodes[0]}),scope,contscope == -1 ? -1 : -2,brkscope == -1 ? -1 : -2,contI,brkI);
                    GenNode(instructions,AdvancedSyntaxNode::Create(ScopelessClosureExpression,true,{
                       AdvancedSyntaxNode::Create(ParenthesesExpression,true,{adv.nodes[2]}),
                       adv.nodes[1]
                    }),scope,contscope == -1 ? -1 : -2,brkscope == -1 ? -1 : -2,contI,brkI);
                    instructions.push_back(new SimpleInstruction(TRYCATCH));
                    uint32_t compGenId = NewId();
                    std::string compGenIttr = "__compGenRetThing";
                    compGenIttr.append(std::to_string(compGenId));
                    if(contscope != -1)
                    {
                        instructions.push_back(new JumpStyleInstruction(JMPIFCONTINUE,compGenIttr+"_cont"));
                    }
                    if(brkscope != -1)
                    {
                        instructions.push_back(new JumpStyleInstruction(JMPIFBREAK,compGenIttr+"_brk"));
                    }
                    instructions.push_back(new JumpStyleInstruction(JMPUNDEFINED, compGenIttr));
                    instructions.push_back(new SimpleInstruction(RET));
                    if(contscope == -2)
                    {
                        instructions.push_back(new LabelInstruction(compGenIttr+"_cont"));
                        instructions.push_back(new SimpleInstruction(PUSHCONTINUE));
                        instructions.push_back(new SimpleInstruction(RET));
                    }
                    else if(contscope != -1)
                    {
                        instructions.push_back(new LabelInstruction(compGenIttr+"_cont"));
                    auto cont = scope-contscope;
                    if(cont > 0)
                    instructions.push_back(new ScopeEndTimesInstruction((uint32_t)cont));
                    
                    std::string myJmp = "__compGenCont";
                    myJmp.append(std::to_string(contI));

                    instructions.push_back(new JumpStyleInstruction(JMP,myJmp));
                    }
                    if(brkscope == -2)
                    {
                        instructions.push_back(new LabelInstruction(compGenIttr+"_brk"));
                        instructions.push_back(new SimpleInstruction(PUSHBREAK));
                        instructions.push_back(new SimpleInstruction(RET));
                    }
                    else if(brkscope != -1)
                    {
                        instructions.push_back(new LabelInstruction(compGenIttr+"_brk"));
                        auto _brk = scope-brkscope;
                    if(_brk > 0)
                    instructions.push_back(new ScopeEndTimesInstruction((uint32_t)_brk));
                    
                    std::string myJmp = "__compGenBrk";
                    myJmp.append(std::to_string(brkI));

                    instructions.push_back(new JumpStyleInstruction(JMP,myJmp));
                        
                    }
                    
                    instructions.push_back(new LabelInstruction(compGenIttr));
                }
                
            }
            else if(adv.nodeName == ScopelessClosureExpression && adv.nodes.size() == 2)
            {
                //()=>{}
                //Name => {}
                //(a,b) => {}
                //it has two args
                std::vector<uint32_t> args;
                GetFunctionArgs(args, adv.nodes[0]);

                std::vector<ByteCodeInstruction*> fnInstructions;
                size_t fnindex=this->chunks.size();
                this->chunks.resize(fnindex+1);

                auto body = adv.nodes[1];

                if(std::holds_alternative<AdvancedSyntaxNode>(body))
                {
                    auto res = std::get<AdvancedSyntaxNode>(body);
                    if(res.nodeName == ScopeNode)
                    {
                        res.nodeName = NodeList;
                    }
                    body = res;
                }
                

                GenNode(fnInstructions,body,0,contscope == -2 ? -2 : -1,brkscope == -2 ? -2 : -1,-1,-1);

                this->chunks[fnindex] = std::pair<std::vector<uint32_t>,std::vector<ByteCodeInstruction*>>(args, fnInstructions);
                instructions.push_back(new ClosureInstruction((uint32_t)fnindex,false));
            }
            else if(adv.nodeName == ClosureExpression && adv.nodes.size() == 2)
            {
                //()=>{}
                //Name => {}
                //(a,b) => {}
                //it has two args
                std::vector<uint32_t> args;
                GetFunctionArgs(args, adv.nodes[0]);

                std::vector<ByteCodeInstruction*> fnInstructions;
                size_t fnindex=this->chunks.size();
                this->chunks.resize(fnindex+1);

                auto body = adv.nodes[1];

                if(std::holds_alternative<AdvancedSyntaxNode>(body))
                {
                    auto res = std::get<AdvancedSyntaxNode>(body);
                    if(res.nodeName == ScopeNode)
                    {
                        res.nodeName = NodeList;
                    }
                    body = res;
                }
                

                GenNode(fnInstructions,body,0,-1,-1,-1,-1);

                this->chunks[fnindex] = std::pair<std::vector<uint32_t>,std::vector<ByteCodeInstruction*>>(args, fnInstructions);
                instructions.push_back(new ClosureInstruction((uint32_t)fnindex));
            }
            else if(adv.nodeName == FunctionStatement && adv.nodes.size() == 2 && std::holds_alternative<AdvancedSyntaxNode>(adv.nodes[0])) 
            {
                //func NAME(ARGS) {}
                //we need to disect NAME(ARGS) and {}
                //then disect NAME and (ARGS)
                auto fcall = std::get<AdvancedSyntaxNode>(adv.nodes[0]);
                if(fcall.nodeName == FunctionCallExpression && adv.nodes.size() >= 1 && std::holds_alternative<AdvancedSyntaxNode>(fcall.nodes[0]))
                {
                    SyntaxNode args = AdvancedSyntaxNode::Create(ParenthesesExpression,true,{});
                    if(fcall.nodes.size() == 2)
                    {
                        args = AdvancedSyntaxNode::Create(ParenthesesExpression,true,{fcall.nodes[1]});
                    }
                    SyntaxNode closure = AdvancedSyntaxNode::Create(ClosureExpression,true,{args,adv.nodes[1]});
                    
                    SyntaxNode assign = AdvancedSyntaxNode::Create(AssignExpression,false,{
                        fcall.nodes[0],
                        closure
                    });
                    GenNode(instructions,assign,scope,contscope,brkscope,contI,brkI);

                    instructions.push_back(new SimpleInstruction(POP));
                }
            }
        
        }
    }
    void CodeGen::GetFunctionArgs(std::vector<SyntaxNode>& args, SyntaxNode n)
    {
        AdvancedSyntaxNode sn;
        if(std::holds_alternative<AdvancedSyntaxNode>(n) && (sn=std::get<AdvancedSyntaxNode>(n)).nodeName == CommaExpression && sn.nodes.size()==2)
        {
            GetFunctionArgs(args,sn.nodes[0]);
            GetFunctionArgs(args,sn.nodes[1]);
        }
        else
        {
            args.push_back(n);
        }
    }
    void CodeGen::GetFunctionArgs(std::vector<uint32_t>& name, SyntaxNode n)
    {
        if(std::holds_alternative<std::nullptr_t>(n))
            return;


        if(std::holds_alternative<AdvancedSyntaxNode>(n))
        {
            auto res = std::get<AdvancedSyntaxNode>(n);
            if(res.nodeName == ParenthesesExpression)
            {
                for(auto n : res.nodes)
                    GetFunctionArgs(name,n);
            }
            else if(res.nodeName == CommaExpression && res.nodes.size() == 2)
            {
                GetFunctionArgs(name,res.nodes[0]);
                GetFunctionArgs(name,res.nodes[1]);
            }
            else if(res.nodeName == GetVariableExpression && res.nodes.size() == 1)
            {
                if(std::holds_alternative<std::string>(res.nodes[0]))
                {
                    name.push_back(GetString(std::get<std::string>(res.nodes[0])));
                }
            }
        }
    }

    void CodeGen::GetFunctionName(std::vector<uint32_t>& name,SyntaxNode n)
    {
        if(std::holds_alternative<AdvancedSyntaxNode>(n))
        {
            auto res = std::get<AdvancedSyntaxNode>(n);
            if(res.nodeName == GetFieldExpression && res.nodes.size() == 2)
            {
                GetFunctionName(name, res.nodes[0]);
                if(std::holds_alternative<std::string>(res.nodes[1]))
                {
                    name.push_back(GetString(std::get<std::string>(res.nodes[1])));
                }
                
            }
            else if(res.nodeName == GetVariableExpression && res.nodes.size() == 1)
            {
                if(std::holds_alternative<std::string>(res.nodes[0]))
                {
                    name.push_back(GetString(std::get<std::string>(res.nodes[0])));
                }
            }
        }
    }

    void CodeGen::GenRoot(SyntaxNode n)
    {
        this->id = 0;
        std::vector<ByteCodeInstruction*> rootInstructions;
        size_t index=this->chunks.size();
        this->chunks.resize(index+1);

        if(std::holds_alternative<AdvancedSyntaxNode>(n))
        {
            auto res = std::get<AdvancedSyntaxNode>(n);

            if(res.nodeName == NodeList)
            {
                for(auto item : res.nodes)
                {
                    if(std::holds_alternative<AdvancedSyntaxNode>(item))
                    {
                        std::string documentation = "";
                        auto res2 = std::get<AdvancedSyntaxNode>(item);

                        if(res2.nodeName == DocumentationStatement)
                        {
                            if(res2.nodes.size() == 2)
                            {
                                if(std::holds_alternative<std::string>(res2.nodes[0]) && std::holds_alternative<AdvancedSyntaxNode>(res2.nodes[1]))
                                {
                                    documentation = std::get<std::string>(res2.nodes[0]);
                                    auto j = std::get<AdvancedSyntaxNode>(res2.nodes[1]);
                                    if(j.nodeName == FunctionStatement || j.nodeName == EnumerableStatement) {
                                        res2 = j;
                                    }
                                }
                            }
                        }
                        if(res2.nodeName == EnumerableStatement)
                        {
                            res2 = AdvancedSyntaxNode::Create(FunctionStatement,false,{
                                res2.nodes[0],
                                AdvancedSyntaxNode::Create(ReturnStatement,false,{
                                    AdvancedSyntaxNode::Create(FunctionCallExpression,true,{
                                        AdvancedSyntaxNode::Create(GetVariableExpression,true,{"YieldEmumerable"}),
                                        AdvancedSyntaxNode::Create(ClosureExpression,true,{AdvancedSyntaxNode::Create(ParenthesesExpression,true,{}), res2.nodes[1]})
                                     })
                                })
                            });
                        }
                        if(res2.nodeName == FunctionStatement)
                        {
                            if(res2.nodes.size()==2)
                            {
                                auto fcall = res2.nodes[0];
                                if(std::holds_alternative<AdvancedSyntaxNode>(fcall))
                                {
                                    auto fcalli = std::get<AdvancedSyntaxNode>(fcall);
                                    if(fcalli.nodeName == FunctionCallExpression)
                                    {
                                        if(fcalli.nodes.size() >= 1)
                                        {
                                            std::vector<uint32_t> functionName;
                                            std::vector<uint32_t> args;
                                            GetFunctionName(functionName,fcalli.nodes[0]);
                                            functionName.insert(functionName.begin(),{GetString(documentation)});
                                            if(fcalli.nodes.size()==2)
                                            GetFunctionArgs(args, fcalli.nodes[1]);

                                            std::vector<ByteCodeInstruction*> fnInstructions;
                                            size_t fnindex=this->chunks.size();
                                            this->chunks.resize(fnindex+1);

                                            GenNode(fnInstructions,res2.nodes[1],0,-1,-1,-1,-1);

                                            this->funcs.push_back(std::pair<std::vector<uint32_t>,uint32_t>(functionName,(uint32_t)fnindex));

                                            this->chunks[fnindex] = std::pair<std::vector<uint32_t>,std::vector<ByteCodeInstruction*>>(args, fnInstructions);
                                        }
                                        else
                                        {
                                            GenNode(rootInstructions,item,0,-1,-1,-1,-1);
                                            GenPop(rootInstructions,item);
                                        }
                                    }
                                    else
                                    {
                                        GenNode(rootInstructions,item,0,-1,-1,-1,-1);
                                        GenPop(rootInstructions,item);
                                    }
                                }
                            }
                            else
                            {
                                GenNode(rootInstructions,item,0,-1,-1,-1,-1);
                                GenPop(rootInstructions,item);
                            }
                        }
                        else
                        {
                            GenNode(rootInstructions,item,0,-1,-1,-1,-1);
                            GenPop(rootInstructions,item);
                        }
                        
                    }
                }
            }
            else
            {
                GenNode(rootInstructions,n,0,-1,-1,-1,-1);
                GenPop(rootInstructions,n);
            }
            
        }
        else
        {
            GenNode(rootInstructions,n,0,-1,-1,-1,-1);
            GenPop(rootInstructions,n);
        }

        this->chunks[index] = std::pair<std::vector<uint32_t>,std::vector<ByteCodeInstruction*>>({},rootInstructions);
    }

    uint32_t CodeGen::NewId()
    {
        return id++;
    }

    void CodeGen::GenPop(std::vector<ByteCodeInstruction*>& instrs,SyntaxNode n)
    {
        if(std::holds_alternative<AdvancedSyntaxNode>(n))
        {
            if(std::get<AdvancedSyntaxNode>(n).isExpression)
            instrs.push_back(new SimpleInstruction(POP));
        }
        else
        {
            instrs.push_back(new SimpleInstruction(POP));
        }
    }

    ResourceBase::~ResourceBase()
    {

    }

    bool ResourceBase::IsEqual(ResourceBase* base)
    {
        return this == base;
    }
    
    ResourceFile::ResourceFile()
    {

    }
    ResourceFile::ResourceFile(std::string f)
    {
        this->file = f;
    }
    ResourceFile::~ResourceFile()
    {
        delete this->strm;
    }

    uint32_t ResourceFile::GetLength(Tesses::Framework::Filesystem::VFS* embedFS)
    {
        if(embedFS == nullptr) return 0;
        if(strm != nullptr) return strm->GetLength();
        this->strm = embedFS->OpenFile(this->file,"rb");
        if(strm != nullptr) return this->strm->GetLength();

        return 0;
    }
    bool ResourceFile::IsEqual(ResourceBase* base)
    {
        auto res = dynamic_cast<ResourceFile*>(base);
        if(res != nullptr) return this->file == res->file;
        return ResourceBase::IsEqual(base);
    }
    void ResourceFile::Write(Tesses::Framework::Streams::Stream* output)
    {
        if(this->strm != nullptr)
        this->strm->CopyTo(output);
    }
   
    uint32_t ResourceByteArray::GetLength(Tesses::Framework::Filesystem::VFS* embedFS)
    {
        return (uint32_t)this->data.size();
    }
    void ResourceByteArray::Write(Tesses::Framework::Streams::Stream* output)
    {
        output->WriteBlock(this->data.data(),this->data.size());
    }

}
