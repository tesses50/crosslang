#include "CrossLang.hpp"
#include <iostream>
#include <stdexcept>
namespace Tesses::CrossLang
{
    std::string LexTokenType_ToString(LexTokenType t)
    {  
        switch(t)
        {   
            case LexTokenType::Char:
                return "char";
            case LexTokenType::Documentation:
                return "documentation";
            case LexTokenType::Identifier:
                return "identifier";
            case LexTokenType::String:
                return "string";
            case LexTokenType::Symbol:
                return "symbol";
        }

        return "";
    }
    bool Parser::IsAnyIdentifier(std::initializer_list<std::string> idents, bool pop)
    {
        if(i < tokens.size())
        {
            if(tokens[i].type != LexTokenType::Identifier) return false;
            for(auto item : idents)
            {
                if(item == tokens[i].text)
                {
                    tkn = tokens[i];
                    if(pop) i++;
                    return true;
                }
            }
        }
        return false;
    }
    bool Parser::IsIdentifier(std::string txt,bool pop)
    {
        if(i < tokens.size())
        {
            if(tokens[i].type != LexTokenType::Identifier) return false;
            if(tokens[i].text == txt)
            {
                tkn = tokens[i];
                if(pop) i++;
                return true;
            }
        }
        return false;
    }
  
    bool Parser::IsAnySymbol(std::initializer_list<std::string> idents, bool pop)
    {
        if(i < tokens.size())
        {
            if(tokens[i].type != LexTokenType::Symbol) return false;
            for(auto item : idents)
            {
                if(item == tokens[i].text)
                {
                    tkn = tokens[i];
                    if(pop) i++;
                    return true;
                }
            }
        }
        return false;
    }
    bool Parser::IsSymbol(std::string txt,bool pop)
    {
        if(i < tokens.size())
        {
            if(tokens[i].type != LexTokenType::Symbol) return false;
            if(tokens[i].text == txt)
            {
                tkn = tokens[i];
                if(pop) i++;
                return true;
            }
        }
        return false;
    }
    void Parser::EnsureSymbol(std::string txt)
    {
        if(i < tokens.size())
        {
            if(tokens[i].type != LexTokenType::Symbol)
            {
                throw SyntaxException(tokens[i].lineInfo, "expected the symbol \"" + txt + "\" but got the " + LexTokenType_ToString(tokens[i].type) + " \"" + tokens[i].text + "\" which is not a symbol at all.");
            }
            if(tokens[i].text != txt)
            {
              
                throw SyntaxException(tokens[i].lineInfo, "expected the symbol \"" + txt + "\" but got the symbol \"" + tokens[i].text + "\"");
            }
            tkn = tokens[i];
            i++;
            return;
        }
        throw std::out_of_range("End of file");
    }
    Parser::Parser(std::vector<LexToken> tokens)
    {
        this->i = 0;
        this->tokens = tokens;
    }
    void Parser::ParseHtml(std::vector<SyntaxNode>& nodes,std::string var)
    {
        if(this->IsSymbol("!",true))
        {
            if(this->i < this->tokens.size() && this->tokens[this->i].type == LexTokenType::Identifier)
            {
                std::string identifier = this->tokens[this->i++].text;

                if(identifier == "DOCTYPE")
                {
                    if(this->i < this->tokens.size() && this->tokens[this->i].type == LexTokenType::Identifier)
                    {
                        std::string doctype_secArg = this->tokens[this->i++].text;
                        std::string r = "<!DOCTYPE " + doctype_secArg + ">";

                        nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),r})}));
                        this->EnsureSymbol(">");
                        if(this->IsSymbol("<"))
                        {
                            ParseHtml(nodes,var);
                        }
                    }
                }
                
            }
        }
        else
        {
            auto parseFn = [this,var](std::vector<SyntaxNode>& nodes,std::string tagName)->void{
                        while(this->i < this->tokens.size())
                        {
                            if(this->IsSymbol("<",false))
                            {
                                nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),Tesses::Framework::Http::HttpUtils::HtmlEncode(this->tokens[i].whiteSpaceCharsBefore)})}));
                                
                                this->i++;
                                if(this->IsIdentifier("else",false) || this->IsIdentifier("elif",false))
                                {
                                    this->i--;
                                    return;
                                }

                                if(this->IsSymbol("/"))
                                {
                                    if(!this->IsIdentifier(tagName))
                                    {
                                        //error
                                        
                                    }
                                    this->EnsureSymbol(">");
                                    if(tagName != "if" && tagName != "for" && tagName != "while" && tagName != "do" && tagName != "each")
                                    {
                                        std::string myVal = "</";
                                        myVal += tagName;
                                        myVal += ">";
                                        nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),myVal})}));
                                    }
                                    break;
                                }
                                else
                                {
                                    ParseHtml(nodes,var);
                                }
                            }
                            else if(this->IsSymbol("{",false))
                            {
                                nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),Tesses::Framework::Http::HttpUtils::HtmlEncode(this->tokens[i].whiteSpaceCharsBefore)})}));
                                this->i++;

                                auto expr = ParseExpression();

                                this->EnsureSymbol("}");

                                //Net.Http.HtmlEncode(expr.ToString())
                                
                                nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{
                                    AdvancedSyntaxNode::Create(AddExpression,true,{
                                        AdvancedSyntaxNode::Create(GetVariableExpression,true,{var})
                                        ,
                                        AdvancedSyntaxNode::Create(FunctionCallExpression,true,{
                                                AdvancedSyntaxNode::Create(GetFieldExpression,true,{
                                                    AdvancedSyntaxNode::Create(GetFieldExpression,true, {
                                                        AdvancedSyntaxNode::Create(GetVariableExpression,true, {
                                                            "Net"
                                                        }),
                                                        "Http"
                                                    }),
                                                    "HtmlEncode"
                                                }),
                                                AdvancedSyntaxNode::Create(FunctionCallExpression,true,{
                                                    AdvancedSyntaxNode::Create(GetFieldExpression,true,{
                                                        AdvancedSyntaxNode::Create(ParenthesesExpression,true,{expr})
                                                        ,
                                                        "ToString"
                                                    })
                                                })
                                            })

                                        
                                    })
                                }));

                            }
                            else
                            {
                                std::string str = "";
                                while(this->i < this->tokens.size() && !this->IsSymbol("{",false) && !this->IsSymbol("<",false))
                                {
                                    str+=this->tokens[this->i].whiteSpaceCharsBefore + this->tokens[this->i].text;
                                    this->i++;
                                }

                                nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),Tesses::Framework::Http::HttpUtils::HtmlEncode(str)})}));
                                
                            }
                        }
                    
            };

            if(this->i < this->tokens.size() && this->tokens[this->i].type == LexTokenType::Identifier)
            {
                std::string tagName = this->tokens[this->i++].text;
                

                if(tagName == "if")
                {
                    std::function<SyntaxNode()> readIf;
                    readIf = [this,&parseFn,var,&readIf]()->SyntaxNode {
                        EnsureSymbol("(");
                        auto expr = ParseExpression();
                        EnsureSymbol(")");
                        EnsureSymbol(">");
                        
                        std::vector<SyntaxNode> trueNodes;

                        SyntaxNode falseNode=nullptr;
                        
                        parseFn(trueNodes,"if"); 

                        if(this->IsSymbol("<",false) && this->i + 1 < this->tokens.size())
                        {
                            auto tkn = this->tokens[i+1];
                            if(tkn.type == LexTokenType::Identifier && tkn.text == "else")
                            {
                                i += 2;
                                EnsureSymbol(">");
                                std::vector<SyntaxNode> falseNodes;
                                parseFn(falseNodes,"if");
                                falseNode = AdvancedSyntaxNode::Create(ScopeNode,false,falseNodes);
                            }
                            else if(tkn.type == LexTokenType::Identifier && tkn.text == "elif")
                            {
                               
                                i += 2;
                                falseNode = readIf();

                            }
                        }


                        return AdvancedSyntaxNode::Create(IfStatement,false,{
                            expr,
                            AdvancedSyntaxNode::Create(ScopeNode,false,trueNodes),
                            falseNode
                        });
                    } ;
                    nodes.push_back(readIf());
                }
                else if(tagName == "raw")
                {
                    EnsureSymbol("(");
                    SyntaxNode expr = ParseExpression();
                    
                    EnsureSymbol(")");
                    EnsureSymbol(">");

                    nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{
                        AdvancedSyntaxNode::Create(AddExpression,true,{
                            AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),
                            AdvancedSyntaxNode::Create(FunctionCallExpression,true,{
                                AdvancedSyntaxNode::Create(GetFieldExpression,true,{
                                    expr
                                    ,
                                    "ToString"
                                })
                            })
                        })
                    }));
                }
                else if(tagName == "while")
                {
                    
                    EnsureSymbol("(");
                    SyntaxNode expr = ParseExpression();
                    SyntaxNode body = nullptr;
                    
                    EnsureSymbol(")");
                    EnsureSymbol(">");
                    
                    
                    std::vector<SyntaxNode> _nodes;
                    parseFn(_nodes,"while");
                    body = AdvancedSyntaxNode::Create(ScopeNode,false,_nodes);
                    
                    nodes.push_back(AdvancedSyntaxNode::Create(WhileStatement,false,{expr,body}));
              
                }
                else if(tagName == "do")
                {
                    EnsureSymbol("(");
                    SyntaxNode expr = ParseExpression();
                    SyntaxNode body = nullptr;
                    
                    EnsureSymbol(")");
                    EnsureSymbol(">");
                    
                    std::vector<SyntaxNode> _nodes;
                    parseFn(_nodes,"do");
                    body = AdvancedSyntaxNode::Create(ScopeNode,false,_nodes);
                    
                    nodes.push_back(AdvancedSyntaxNode::Create(DoStatement,false,{expr,body}));
                }
                else if(tagName == "for")
                {
                    SyntaxNode init = nullptr;
            SyntaxNode cond = true;
            SyntaxNode inc = nullptr;
            SyntaxNode body = nullptr;
            EnsureSymbol("(");
            if(!IsSymbol(";",false))
            {
                init = ParseExpression();
            }
            EnsureSymbol(";");
            if(!IsSymbol(";",false))
            {
                cond = ParseExpression();
            }
            EnsureSymbol(";");
            if(!IsSymbol(")",false))
            {
                inc = ParseExpression();
            }

            EnsureSymbol(")");
            EnsureSymbol(">");
            std::vector<SyntaxNode> _nodes;
            parseFn(_nodes,"for");
            body = AdvancedSyntaxNode::Create(ScopeNode,false,_nodes);
            
            nodes.push_back(AdvancedSyntaxNode::Create(ForStatement,false,{init,cond,inc,body}));


                }
                else if(tagName == "each")
                {
                    SyntaxNode item = nullptr;
                    EnsureSymbol("(");
                    SyntaxNode list = ParseExpression();
                    SyntaxNode body = nullptr;
                    if(IsSymbol(":"))
                    {
                        item = list;
                        list = ParseExpression();
                    }
                    EnsureSymbol(")");
                    EnsureSymbol(">");
                    
                    std::vector<SyntaxNode> _nodes;
                    parseFn(_nodes,"each");
                    body = AdvancedSyntaxNode::Create(ScopeNode,false,_nodes);
                    
                    nodes.push_back(AdvancedSyntaxNode::Create(EachStatement,false,{item,list,body}));
                }
                else 
                {
                    std::string s = "<";
                    s.append(tagName);

                    nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),s})}));

                    while(!this->IsSymbol(">"))
                    {
                        //we need to get a name=value
                        if(this->i < this->tokens.size() && this->tokens[this->i].type == LexTokenType::Identifier)
                        {
                            std::string key = "";

                            while(this->i < this->tokens.size())
                            {
                                if(this->tokens[i].type == Identifier)
                                {
                                    key += this->tokens[i].text;
                                    i++;
                                }
                                else if(this->tokens[i].type == Symbol)
                                {
                                    if(this->tokens[i].text == "-" || this->tokens[i].text == ":" || this->tokens[i].text == "--" || tokens[i].text == ".")
                                    {
                                        key += this->tokens[i].text;
                                        i++;
                                    }
                                    else break;
                                }
                                else break;
                            }
                            
                            if(this->IsSymbol(">"))
                            {
                                std::string myVal = " " + key;
                                

                                nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),myVal})}));
                                
                                break;
                            }
                            else if(this->IsSymbol("="))
                            {

                                std::string myVal = " " + key + "=\"";
                                
                               
                                
                                EnsureSymbol("{");
                                auto expr = ParseExpression();
                                EnsureSymbol("}");
                                if(std::holds_alternative<AdvancedSyntaxNode>(expr))
                                {
                                  nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{
                                        AdvancedSyntaxNode::Create(AddExpression,true,{
                                            AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),
                                            AdvancedSyntaxNode::Create(AddExpression,true,{
                                                myVal,
                                                    AdvancedSyntaxNode::Create(AddExpression,true,{
                                                        AdvancedSyntaxNode::Create(FunctionCallExpression,true,{
                                                            AdvancedSyntaxNode::Create(GetFieldExpression,true,{
                                                                AdvancedSyntaxNode::Create(GetFieldExpression,true, {
                                                                    AdvancedSyntaxNode::Create(GetVariableExpression,true, {
                                                                        "Net"
                                                                    }),
                                                                    "Http"
                                                                }),
                                                                "HtmlEncode"
                                                            }),
                                                            AdvancedSyntaxNode::Create(FunctionCallExpression,true,{
                                                                AdvancedSyntaxNode::Create(GetFieldExpression,true,{
                                                                    expr
                                                                    ,
                                                                    "ToString"
                                                                })
                                                            })
                                                        }),
                                                        "\"",
                                                    })
                                                    
                                            })
                                        })
                                  }));
                                
                              

                                } 
                                else if(std::holds_alternative<double>(expr)) 
                                {
                                    myVal += std::to_string(std::get<double>(expr)) + "\"";
                                    
                                    nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),myVal})}));
                                }
                                else if(std::holds_alternative<int64_t>(expr)) 
                                {
                                    myVal += std::to_string(std::get<int64_t>(expr)) + "\"";

                                    nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),myVal})}));
                                }
                                else if(std::holds_alternative<bool>(expr))
                                {
                                    myVal += std::holds_alternative<bool>(expr) ? "true\"" : "false\"";

                                    
                                    nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),myVal})}));
                                }
                                else if(std::holds_alternative<std::string>(expr))
                                {
                                    myVal += Tesses::Framework::Http::HttpUtils::HtmlEncode(std::get<std::string>(expr)) + "\"";
                                    nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),myVal})}));
                                }
                                //key = value


                                
                            }
                            else {

                                std::string myVal = " " + key;
                                
                                
                                nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),myVal})}));
                                
                            }
                        }
                    }

                    nodes.push_back(AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{AdvancedSyntaxNode::Create(GetVariableExpression,true,{var}),">"})}));
                    
                    if(tagName !=  "img" && tagName != "input" && tagName != "br" && tagName != "hr" && tagName != "link" && tagName != "meta")
                    {
                        parseFn(nodes,tagName);
                    }
                }
            }
        }
    }
    uint32_t Parser::NewId()
    {
        return id++;
    }
    SyntaxNode Parser::ParseValue()
    {
        if(i >= tokens.size()) throw std::out_of_range("End of file");
        auto tkn2 = tokens[i];
        SyntaxNode node = nullptr;
        if(tokens[i].type == LexTokenType::String)
        {
            node = tkn2.text;
            i++;
            
        }
        else if(tokens[i].type == LexTokenType::Char)
        {
            node = tkn2.text.empty() ? '\0' : tkn2.text.front();
            i++;
            
        }
        else if(IsSymbol("<"))
        {
            uint32_t htmlId = NewId();
            std::string compHtml = "__compGenHtml";
            compHtml.append(std::to_string(htmlId));
            std::vector<SyntaxNode> syntaxNode = {
                compHtml
            };
            ParseHtml(syntaxNode,compHtml);    
            
            return AdvancedSyntaxNode::Create(HtmlRootExpression,true,syntaxNode);
        }
        else if(IsSymbol("["))
        {
            if(IsSymbol("]",false))
            node = AdvancedSyntaxNode::Create(ArrayExpression, true, {});
            else
            node = AdvancedSyntaxNode::Create(ArrayExpression, true, {ParseExpression()});
            EnsureSymbol("]");
        }
        else if(IsSymbol("{"))
        {
            if(IsSymbol("}",false))
                node = AdvancedSyntaxNode::Create(DictionaryExpression,true,{});
            else
                node = AdvancedSyntaxNode::Create(DictionaryExpression,true,{ParseExpression()});
            
            EnsureSymbol("}");
        }
        else if(IsSymbol("("))
        {
            if(IsSymbol(")",false))
                node = AdvancedSyntaxNode::Create(ParenthesesExpression,true,{});
            else
                node = AdvancedSyntaxNode::Create(ParenthesesExpression,true,{ParseExpression()});
            EnsureSymbol(")");
        }
        else if(IsIdentifier("var"))
        {
            if(i >= tokens.size()) throw std::out_of_range("End of file");
            auto variable = tokens[i];
            i++;
            if(variable.type != LexTokenType::Identifier) throw SyntaxException(variable.lineInfo, "Expected an identifier got a " + LexTokenType_ToString(variable.type) + " \"" + variable.text + "\"");
            node = AdvancedSyntaxNode::Create(DeclareExpression,true,{variable.text});
        }
        else if(IsIdentifier("operator"))
        {
            if(i >= tokens.size()) throw std::out_of_range("End of file");
            auto variable = tokens[i];
            i++;
            if(variable.type != LexTokenType::Identifier && variable.type != LexTokenType::Symbol) throw SyntaxException(variable.lineInfo, "Expected an identifier or a symbol got a " + LexTokenType_ToString(variable.type) + " \"" + variable.text + "\"");
            node = AdvancedSyntaxNode::Create(GetVariableExpression, true,{"operator"+variable.text});
            
        }
        else if(IsIdentifier("new"))
        {
            if(i >= tokens.size()) throw std::out_of_range("End of file");
            auto variable = tokens[i];
            i++;
            if(variable.type != LexTokenType::Identifier && variable.type != LexTokenType::Symbol) throw SyntaxException(variable.lineInfo, "Expected an identifier or a symbol got a " + LexTokenType_ToString(variable.type) + " \"" + variable.text + "\"");
            node = AdvancedSyntaxNode::Create(GetFieldExpression, true, {AdvancedSyntaxNode::Create(GetVariableExpression,true,{"New"}), variable.text});
        }
        else if(IsIdentifier("embed"))
        {
            EnsureSymbol("(");
            if(i >= tokens.size()) throw std::out_of_range("End of file");
            auto embed = tokens[i];
            i++;
            if(embed.type != LexTokenType::String) throw SyntaxException(embed.lineInfo, "Expected an string for embed got a " + LexTokenType_ToString(embed.type) + " \"" + embed.text + "\"");
            EnsureSymbol(")");
            node = AdvancedSyntaxNode::Create(EmbedExpression, true,{embed.text});
        }   
        else if(tokens[i].type == LexTokenType::Identifier)
        {
            std::string token = tokens[i].text;
            
            i++;
        
            bool hasNumber=true;    
            int64_t lngNum = 0;
            
            if(token.size() == 1 && token[0] == '0')
            {
                lngNum = 0;
            }
            else
            if(token.size() > 0 && token[0] == '0')
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

            if(hasNumber && this->IsSymbol(".",false) && i+1 < tokens.size() && tokens[i+1].type == LexTokenType::Identifier)
            {
                std::string myToken = tokens[i+1].text;
                if(myToken.size() > 0 && myToken[0] >= '0' && myToken[0] <= '9')
                {
                    i+=2;
                    std::string myN = std::to_string(lngNum) + "." + myToken;

                    double v = std::stod(myN,nullptr);

                    node = v;
                }
                else
                {
                    node = lngNum;
                }
            }
            else if(hasNumber)
            {
                
                node = lngNum;
            }
        
            if(!hasNumber)
            {
                if(token == "true")
                    node = true;
                else if(token == "false")
                    node = false;
                else if(token == "null")
                    node = nullptr;
                else if(token == "undefined")
                    node = Undefined();
                else {
                    node = AdvancedSyntaxNode::Create(GetVariableExpression,true,{token});
                }
            }
        }
        

        if(IsSymbol("=>"))
        {
            if(IsSymbol("{",false))
            {
                node = AdvancedSyntaxNode::Create(ClosureExpression,true,{node,ParseNode()});
            }
            else
            {
                node = AdvancedSyntaxNode::Create(ClosureExpression,true,{node, AdvancedSyntaxNode::Create(ReturnStatement,false,{ParseExpression()})});
            }
        }
        while(IsAnySymbol({".","[","("}))
        {
            if(tkn.text == ".")
            {
                if(i>=tokens.size()) throw std::out_of_range("End of file");
                if(tokens[i].type != LexTokenType::Identifier) throw std::exception();
                std::string name = tokens[i].text; 
                if(name == "operator")
                {
                    if(i >= tokens.size()) throw std::out_of_range("End of file");
                    auto op = tokens[i];
                    if(op.type != LexTokenType::Identifier && op.type != LexTokenType::Symbol) throw SyntaxException(op.lineInfo, "Expected an identifier or a symbol got a " + LexTokenType_ToString(op.type) + " \"" + op.text + "\"");
            
                    name += tokens[i+1].text;
                    i++;
                }
                i++;    
                node = AdvancedSyntaxNode::Create(GetFieldExpression, true, {node, name});
            }
            else if(tkn.text == "[")
            {
                node = AdvancedSyntaxNode::Create(GetArrayExpression,true,{node,ParseExpression()});
                EnsureSymbol("]");
            }
            else if(tkn.text == "(")
            {
                if(IsSymbol(")",false))
                {
                    node = AdvancedSyntaxNode::Create(FunctionCallExpression,true,{node});
                }
                else
                {
                    node = AdvancedSyntaxNode::Create(FunctionCallExpression,true,{node,ParseExpression()});
               
                }
                 EnsureSymbol(")");
            }
        }
        
        
        if(IsSymbol("++"))
        {
            node = AdvancedSyntaxNode::Create(PostfixIncrementExpression,true,{node});
        }
        else if(IsSymbol("--"))
        {
            node = AdvancedSyntaxNode::Create(PostfixDecrementExpression,true,{node});
        }
        
        
        return node;
    }
    SyntaxNode Parser::ParseUnary()
    {
        if(IsSymbol("-"))
        {
            return AdvancedSyntaxNode::Create(NegativeExpression,true,{ParseUnary()});
        }
        else if(IsSymbol("!"))
        {
            return AdvancedSyntaxNode::Create(NotExpression,true,{ParseUnary()});
        }
        else if(IsSymbol("~"))
        {
            return AdvancedSyntaxNode::Create(BitwiseNotExpression,true,{ParseUnary()});
        }
        else if(IsSymbol("++"))
        {
            return AdvancedSyntaxNode::Create(PrefixIncrementExpression,true,{ParseUnary()});
        }
        else if(IsSymbol("--"))
        {
            return AdvancedSyntaxNode::Create(PrefixDecrementExpression,true,{ParseUnary()});
        }
        else if(IsSymbol("/"))
        {
            if(this->i < this->tokens.size() && (this->tokens[this->i].type == LexTokenType::String || this->tokens[this->i].type == LexTokenType::Identifier))
            {
                return AdvancedSyntaxNode::Create(DivideExpression,true,{
                    AdvancedSyntaxNode::Create(RootPathExpression,true,{}),
                    ParseValue()
                }); 
            }
            else {
                return AdvancedSyntaxNode::Create(RootPathExpression,true,{});
            }
        }
        else if(IsSymbol("."))
        {
            return AdvancedSyntaxNode::Create(RelativePathExpression, true, {});
        }
        
        return ParseValue();
    }

    SyntaxNode Parser::ParseFactor()
    {
        SyntaxNode expr = ParseUnary();
        while(IsAnySymbol({"*","/","%"},true))
        {
            if(tkn.text == "*")
            {
                expr = AdvancedSyntaxNode::Create(TimesExpression, true, {expr,ParseUnary()});
            }
            else if(tkn.text == "/")
            {
                expr = AdvancedSyntaxNode::Create(DivideExpression, true, {expr,ParseUnary()});
            }
            else if(tkn.text == "%")
            {
                expr = AdvancedSyntaxNode::Create(ModExpression, true, {expr,ParseUnary()});
            }
        }
        return expr;
    }
    SyntaxNode Parser::ParseSum()
    {
        SyntaxNode expr = ParseFactor();
        while(IsAnySymbol({"+","-"},true))
        {
            if(tkn.text == "+")
            {
                expr = AdvancedSyntaxNode::Create(AddExpression, true, {expr,ParseFactor()});
            }
            else if(tkn.text == "-")
            {
                expr = AdvancedSyntaxNode::Create(SubExpression, true, {expr,ParseFactor()});
            }
        }
        return expr;
    }
    SyntaxNode Parser::ParseAssignment()
    {
        SyntaxNode node = ParseTernary();
        if(IsSymbol("="))
        {
            return AdvancedSyntaxNode::Create(AssignExpression,true,{node,ParseAssignment()});
        }
        else if(IsSymbol("+="))
        {
            return AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(AddExpression,true,{ node,ParseAssignment()})});
        }
        else if(IsSymbol("-="))
        {
            return AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(SubExpression,true,{ node,ParseAssignment()})});
        }
        else if(IsSymbol("*="))
        {
            return AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(TimesExpression,true,{ node,ParseAssignment()})});
        }
        else if(IsSymbol("/="))
        {
            return AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(DivideExpression,true,{ node,ParseAssignment()})});
        }
        else if(IsSymbol("%="))
        {
            return AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(ModExpression,true,{ node,ParseAssignment()})});
        }
        else if(IsSymbol("<<="))
        {
            return AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(LeftShiftExpression,true,{ node,ParseAssignment()})});
        }
        else if(IsSymbol(">>="))
        {
            return AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(RightShiftExpression,true,{ node,ParseAssignment()})});
        }
        else if(IsSymbol("|="))
        {
            return AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(BitwiseOrExpression,true,{ node,ParseAssignment()})});
        }
        else if(IsSymbol("&="))
        {
            return AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(BitwiseAndExpression,true,{ node,ParseAssignment()})});
        }
        else if(IsSymbol("^="))
        {
            return AdvancedSyntaxNode::Create(CompoundAssignExpression,true,{AdvancedSyntaxNode::Create(XOrExpression,true,{ node,ParseAssignment()})});
        }
        return node;
    }
    SyntaxNode Parser::ParseNode(bool isRoot)
    {
        if(i < tokens.size() && !isRoot && tokens[i].type == Documentation)
        {
            auto txt = tokens[i].text;
            i++;
            return AdvancedSyntaxNode::Create(DocumentationStatement,false,{txt,ParseNode()});
        }
        if(IsSymbol("{") || isRoot)
        {
            AdvancedSyntaxNode aSN;
            aSN.isExpression=false;
            aSN.nodeName = isRoot ? NodeList : ScopeNode;
            
            while(i < tokens.size() && (isRoot || !IsSymbol("}",false)))
            {
                aSN.nodes.push_back(ParseNode());
                IsSymbol(";");
            }
            
            if(!isRoot) i++;
            return aSN;
        }

        if(IsIdentifier("if"))
        {
            EnsureSymbol("(");
            SyntaxNode cond = ParseExpression();
            EnsureSymbol(")");
            SyntaxNode truthy = nullptr;
            SyntaxNode falsey = nullptr;

            if(!IsIdentifier("else",false))
            {
                truthy = ParseNode();
            }

            if(IsIdentifier("else"))
            {
                falsey = ParseNode();
            }

            return AdvancedSyntaxNode::Create(IfStatement, false, {cond, truthy,falsey});
        }
        if(IsIdentifier("switch"))
        {
            EnsureSymbol("(");
            SyntaxNode cond = ParseExpression();
            EnsureSymbol(")");

            SyntaxNode body = ParseNode();
            
            return AdvancedSyntaxNode::Create(SwitchStatement,false,{cond,body});
        }
        if(IsIdentifier("while"))
        {
            EnsureSymbol("(");
            SyntaxNode cond = ParseExpression();
            EnsureSymbol(")");

            SyntaxNode body = nullptr;
            if(!IsSymbol(";"))
            {
                body = ParseNode();
            }
            return AdvancedSyntaxNode::Create(WhileStatement,false,{cond,body});
        }
        if(IsIdentifier("do"))
        {
            EnsureSymbol("(");
            SyntaxNode cond = ParseExpression();
            EnsureSymbol(")");

            SyntaxNode body = nullptr;
            if(!IsSymbol(";"))
            {
                body = ParseNode();
            }
            return AdvancedSyntaxNode::Create(DoStatement,false,{cond,body});
        }
        if(IsIdentifier("for"))
        {
            SyntaxNode init = nullptr;
            SyntaxNode cond = true;
            SyntaxNode inc = nullptr;
            SyntaxNode body = nullptr;
            EnsureSymbol("(");
            if(!IsSymbol(";",false))
            {
                init = ParseExpression();
            }
            EnsureSymbol(";");
            if(!IsSymbol(";",false))
            {
                cond = ParseExpression();
            }
            EnsureSymbol(";");
            if(!IsSymbol(")",false))
            {
                inc = ParseExpression();
            }

            EnsureSymbol(")");


            if(!IsSymbol(";"))
            {
                body = ParseNode();
            }
            return AdvancedSyntaxNode::Create(ForStatement,false,{init,cond,inc,body});
        }
        if(IsIdentifier("each"))
        {
            SyntaxNode item = nullptr;
            EnsureSymbol("(");
            SyntaxNode list = ParseExpression();
            SyntaxNode body = nullptr;
            if(IsSymbol(":"))
            {
                item = list;
                list = ParseExpression();
            }
            EnsureSymbol(")");
            
            if(!IsSymbol(";"))
            {
                body = ParseNode();
            }
            return AdvancedSyntaxNode::Create(EachStatement,false,{item,list,body});
        }
        if(IsIdentifier("object"))
        {
            //TODO: complete this
            if(i < tokens.size())
            {
                std::string name = tokens[i++].text;
                EnsureSymbol("{");
                std::vector<SyntaxNode> name_and_methods;
                name_and_methods.push_back(name);

                while(!IsSymbol("}",false) && i < tokens.size())
                {
                    std::string documentation = "";
                    bool hasDocumentation=false;

                    

                    if(tokens[i].type == LexTokenType::Documentation)
                    {
                        hasDocumentation=true;
                        documentation = tokens[i++].text;
                    }

                    if(i < tokens.size())
                    {
                        if(IsIdentifier("method")) 
                        {
                            auto nameAndArgs = ParseExpression();
            
                            if(IsSymbol("{",false))
                            {
                                auto r = AdvancedSyntaxNode::Create(MethodDeclaration,false,{nameAndArgs,ParseNode()});
                                if(hasDocumentation)
                                {
                                    name_and_methods.push_back(AdvancedSyntaxNode::Create(DocumentationStatement,false,{documentation,r}));
                                }
                                else 
                                {
                                    name_and_methods.push_back(r);    
                                }
                            }
                            else
                            {
                                auto v = ParseExpression();

                                EnsureSymbol(";");
                                auto r= AdvancedSyntaxNode::Create(MethodDeclaration,false,{nameAndArgs,AdvancedSyntaxNode::Create(ReturnStatement,false,{v})});
                                if(hasDocumentation)
                                {
                                    name_and_methods.push_back(AdvancedSyntaxNode::Create(DocumentationStatement,false,{documentation,r}));
                                }
                                else 
                                {
                                    name_and_methods.push_back(r);    
                                }
                            }
                        }
                        else if(IsIdentifier("static")) 
                        {
                            auto nameAndArgs = ParseExpression();
            
                            if(IsSymbol("{",false))
                            {
                                auto r = AdvancedSyntaxNode::Create(StaticStatement,false,{nameAndArgs,ParseNode()});
                                if(hasDocumentation)
                                {
                                    name_and_methods.push_back(AdvancedSyntaxNode::Create(DocumentationStatement,false,{documentation,r}));
                                }
                                else 
                                {
                                    name_and_methods.push_back(r);    
                                }
                            }
                            else
                            {
                                auto v = ParseExpression();

                                EnsureSymbol(";");
                                auto r= AdvancedSyntaxNode::Create(StaticStatement,false,{nameAndArgs,AdvancedSyntaxNode::Create(ReturnStatement,false,{v})});
                                if(hasDocumentation)
                                {
                                    name_and_methods.push_back(AdvancedSyntaxNode::Create(DocumentationStatement,false,{documentation,r}));
                                }
                                else 
                                {
                                    name_and_methods.push_back(r);    
                                }
                            }
                        }
                    }
                }
                EnsureSymbol("}");
                return AdvancedSyntaxNode::Create(ObjectStatement, false, name_and_methods);
            }
            else throw std::out_of_range("End of file");
        }
        if(IsIdentifier("enumerable"))
        {
            auto nameAndArgs = ParseExpression();
            
            if(IsSymbol("{",false))
            {
                return AdvancedSyntaxNode::Create(EnumerableStatement,false,{nameAndArgs,ParseNode()});
            }
            else
            {   
                throw SyntaxException(tokens[i].lineInfo, "expected the symbol \"{\" on enumerable but got the symbol or other token \"" + tokens[i].text + "\"");
            }
        }
        if(IsIdentifier("func"))
        {
            auto nameAndArgs = ParseExpression();
            
            if(IsSymbol("{",false))
            {
                return AdvancedSyntaxNode::Create(FunctionStatement,false,{nameAndArgs,ParseNode()});
            }
            else
            {
                auto v = ParseExpression();

                EnsureSymbol(";");
                return AdvancedSyntaxNode::Create(FunctionStatement,false,{nameAndArgs,AdvancedSyntaxNode::Create(ReturnStatement,false,{v})});
            }
        }
        if(IsIdentifier("break"))
        {
            EnsureSymbol(";");
            return AdvancedSyntaxNode::Create(BreakStatement,false,{});
        }
        if(IsIdentifier("continue"))
        {
            EnsureSymbol(";");
            return AdvancedSyntaxNode::Create(ContinueStatement,false,{});
        }
        if(IsIdentifier("case"))
        {
            auto r = AdvancedSyntaxNode::Create(CaseStatement,false,{ParseExpression()});
            EnsureSymbol(":");
            return r;
        }
        if(IsIdentifier("default"))
        {
            auto r = AdvancedSyntaxNode::Create(DefaultStatement,false,{});
            EnsureSymbol(":");
            return r;
        }
        if(IsIdentifier("return"))
        {
            SyntaxNode v = Undefined();
            if(!IsSymbol(";",true))
            {
                v = ParseExpression();
                EnsureSymbol(";");
            }
            return AdvancedSyntaxNode::Create(ReturnStatement,false,{v});
        }

        if(IsIdentifier("yield"))
        {
            SyntaxNode v = Undefined();
            if(!IsSymbol(";",true))
            {
                v = ParseExpression();
                EnsureSymbol(";");
            }
            return AdvancedSyntaxNode::Create(YieldStatement,false,{v});
        }
        if(IsIdentifier("throw"))
        {
            auto v = ParseExpression();
            EnsureSymbol(";");
            return AdvancedSyntaxNode::Create(ThrowStatement,false,{v});
        }
        if(IsIdentifier("try"))
        {
            auto tryBody = ParseNode();
            SyntaxNode catchNode = nullptr;
            SyntaxNode catchEx = nullptr;
            SyntaxNode finally=nullptr;
            if(IsIdentifier("catch"))
            {
                EnsureSymbol("(");
                catchEx = ParseExpression();
                EnsureSymbol(")");
                catchNode = ParseNode();
            }
            if(IsIdentifier("finally"))
            {
                finally = ParseNode();
            }

            return AdvancedSyntaxNode::Create(TryStatement, false, {tryBody,catchNode,catchEx, finally});
        }
        if(IsIdentifier("defer"))
        {
            if(IsSymbol("{",false))
            {
                return AdvancedSyntaxNode::Create(DeferStatement,false,{ParseNode()});
            }
            else
            {
                auto v = ParseExpression();
                
                EnsureSymbol(";");
                return AdvancedSyntaxNode::Create(DeferStatement,false,{AdvancedSyntaxNode::Create(ReturnStatement,false,{v})});
            }
        }
        auto v = ParseExpression();
        EnsureSymbol(";");
        return v;
    }
    SyntaxNode Parser::ParseTernary()
    {
        SyntaxNode node = ParseLOr();
        if(IsSymbol("?"))
        {
            auto yes = ParseTernary();
            EnsureSymbol(":");
            auto no = ParseTernary();

            return AdvancedSyntaxNode::Create(TernaryExpression,true,{node,yes,no});
        }
        return node;
    }
    SyntaxNode Parser::ParseShift()
    {
        SyntaxNode expr = ParseSum();
        while(IsAnySymbol({"<<",">>"},true))
        {
            if(tkn.text == "<<")
            {
                expr = AdvancedSyntaxNode::Create(LeftShiftExpression, true, {expr,ParseSum()});
            }
            else if(tkn.text == ">>")
            {
                expr = AdvancedSyntaxNode::Create(RightShiftExpression, true, {expr,ParseSum()});
            }
        }
        return expr;
    }
   
    SyntaxNode Parser::ParseRel()
    {
        SyntaxNode expr = ParseShift();
        while(IsAnySymbol({"<",">","<=",">="},true))
        {
            if(tkn.text == "<")
            {
                expr = AdvancedSyntaxNode::Create(LessThanExpression, true, {expr,ParseShift()});
            }
            else if(tkn.text == ">")
            {
                expr = AdvancedSyntaxNode::Create(GreaterThanExpression, true, {expr,ParseShift()});
            }
            else if(tkn.text == "<=")
            {
                expr = AdvancedSyntaxNode::Create(LessThanEqualsExpression, true, {expr,ParseShift()});
            }
            else if(tkn.text == ">=")
            {
                expr = AdvancedSyntaxNode::Create(GreaterThanEqualsExpression, true, {expr,ParseShift()});
            }
        }
        return expr;
    }
    SyntaxNode Parser::ParseEq()
    {
        SyntaxNode expr = ParseRel();
        while(IsAnySymbol({"==","!="},true))
        {
            if(tkn.text == "==")
            {
                expr = AdvancedSyntaxNode::Create(EqualsExpression, true, {expr,ParseRel()});
            }
            else if(tkn.text == "!=")
            {
                expr = AdvancedSyntaxNode::Create(NotEqualsExpression, true, {expr,ParseRel()});
            }
        }
        return expr;
    }
    SyntaxNode Parser::ParseBAnd()
    {
        SyntaxNode expr = ParseEq();
        while(IsSymbol("&"))
        {
            expr = AdvancedSyntaxNode::Create(BitwiseAndExpression,true,{expr,ParseEq()});
        }
        return expr;
    }
    SyntaxNode Parser::ParseExpression()
    {
        SyntaxNode expr = ParseAssignment();
        while(IsSymbol(","))
        {
            expr = AdvancedSyntaxNode::Create(CommaExpression,true,{expr,ParseAssignment()});
        }
        return expr;
    }
    SyntaxNode Parser::ParseXOr()
    {
        SyntaxNode expr = ParseBAnd();
        while(IsSymbol("^"))
        {
            expr = AdvancedSyntaxNode::Create(XOrExpression,true,{expr,ParseBAnd()});
        }
        return expr;
    }
    SyntaxNode Parser::ParseBOr()
    {
        SyntaxNode expr = ParseXOr();
        while(IsSymbol("|"))
        {
            expr = AdvancedSyntaxNode::Create(BitwiseOrExpression,true,{expr,ParseXOr()});
        }
        return expr;
    }
    SyntaxNode Parser::ParseLAnd()
    {
        SyntaxNode expr = ParseBOr();
        while(IsSymbol("&&"))
        {
            expr = AdvancedSyntaxNode::Create(LogicalAndExpression,true,{expr,ParseBOr()});
        }
        return expr;
    }
    SyntaxNode Parser::ParseLOr()
    {
        SyntaxNode expr = ParseLAnd();
        while(IsSymbol("||"))
        {
            expr = AdvancedSyntaxNode::Create(LogicalOrExpression,true,{expr,ParseLAnd()});
        }
        return expr;
    }
}