#include "CrossLang.hpp"
#include <iostream>
namespace Tesses::CrossLang
{
    std::string EscapeString(std::string text,bool quote)
    {
        std::string str={};
        if(quote) str.push_back('\"');
        for(auto item : text)
        {
            if(item == '\\' || item == '\"' || item == '\'')
            {
                str.push_back('\\');
                str.push_back(item);
            }
            else if(item == '\n')
            {
                str.push_back('\\');
                str.push_back('n');
            }
            else if(item == '\r')
            {
                str.push_back('\\');
                str.push_back('r');
            }
            else if(item == '\t')
            {
                str.push_back('\\');
                str.push_back('t');
            }
            else if(item == '\f')
            {
                str.push_back('\\');
                str.push_back('f');
            }
            else if(item == '\0')
            {
                str.push_back('\\');
                str.push_back('0');
            }
            else if(item == '\b')
            {
                str.push_back('\\');
                str.push_back('b');
            }
            else if(item == '\a')
            {
                str.push_back('\\');
                str.push_back('a');
            }
            else if(item == '\v')
            {
                str.push_back('\\');
                str.push_back('v');
            }
            else if(item == '\e')
            {
                str.push_back('\\');
                str.push_back('e');
            }
            else if((uint8_t)item < 32 || (uint8_t)item > 126)
            {
                str.append("\\x");
                str.push_back(Tesses::Framework::Http::HttpUtils::NibbleToHex(((uint8_t)item >> 4)&0x0F));
                str.push_back(Tesses::Framework::Http::HttpUtils::NibbleToHex((uint8_t)item & 0x0F));
            }
            else 
            {
                str.push_back(item);
            }
        }

        if(quote) str.push_back('\"');
        return str;
    }

    void LexTokenLineInfo::Subtract(size_t len)
    {
        this->offset -= len;
        this->column -= len;
    }
    void LexTokenLineInfo::Add(int c)
    {
        this->offset++;
        switch(c)
        {
            case ' ':
                this->column++;
                break;
            case '\n':
                this->column=1;
                this->line++;
                break;
            case '\t':
                this->column += 4;
                break;
            case '\r':
                this->column++;
                break;
            default:
                this->column++;
                break;
        }
    }
    
       int Lex(std::string filename, std::istream& strm, std::vector<LexToken>& tokens)
    {
        int _peeked=-1;

        auto Read = [&_peeked,&strm]()->int {
            if(_peeked > -1)
            {
                int _peek2 = _peeked;
                _peeked=-1;
                return _peek2;
            }
            uint8_t b;

            strm.read((char*)&b,1);
            if(strm.eof()) return -1;
            return b;
        };

        auto Peek = [&_peeked,Read]()->int {
            if(_peeked > -1) return _peeked;
            _peeked = Read();
            return _peeked;
        };

        int read;
        int peek;

        std::string buffer={};

        LexTokenLineInfo lineInfo;

        lineInfo.filename = filename;
        lineInfo.column = 1;
        lineInfo.line = 1;
        lineInfo.offset = 0;

       

        auto Flush = [&buffer,&tokens,&lineInfo]() -> void {
            if(!buffer.empty())
            {
                LexToken token;
                token.text = buffer;
                token.type = LexTokenType::Identifier;
                token.lineInfo = lineInfo;
                token.lineInfo.Subtract(buffer.size());
                tokens.push_back(token);
                buffer.clear();
            }
        };

        auto Symbol = [&tokens,&lineInfo](std::initializer_list<int> chrs)-> void {
            LexToken token;
            
            token.type = LexTokenType::Symbol;
            token.lineInfo = lineInfo;

            token.text.reserve(chrs.size());

            for(auto i : chrs)
                token.text.push_back((char)i);

            tokens.push_back(token);
        };

        auto ReadChr = [&lineInfo, &strm, Read]() -> std::pair<int,bool> {
             int read=Read();
             lineInfo.Add(read);

            if(read == -1) 
            {
                
                return std::pair<int,bool>(-1,false);
            }
            


            if(read == '\\')
            {
                read = Read();
                lineInfo.Add(read);
                if(read == -1)
                {
                    return std::pair<int,bool>(-1,true);
                }
                else if(read == 'n')
                {
                    return std::pair<int,bool>('\n',true);
                }
                else if(read == 'r')
                {
                    return std::pair<int,bool>('\r',true);
                }
                else if(read == 'f')
                {
                    return std::pair<int,bool>('\f',true);
                }
                else if(read == 'b')
                {
                    return std::pair<int,bool>('\b',true);
                }
                else if(read == 'a')
                {
                    return std::pair<int,bool>('\a',true);
                }
                else if(read == '0')
                {
                    return std::pair<int,bool>('\0',true);
                }
                else if(read == 'v')
                {
                    return std::pair<int,bool>('\v',true);
                }
                else if(read == 'e')
                {
                    return std::pair<int,bool>('\e',true);
                }
                else if(read == 't')
                {
                    return std::pair<int,bool>('\t',true);
                }
                else if(read == 'x')
                {
                    int r1 = Read();
                    lineInfo.Add(r1);
                    if(r1 == -1)
                    {
                        return std::pair<int,bool>(-1,true);
                    }
                    int r2 = Read();
                    lineInfo.Add(r2);
                    if(r2 == -1)
                    {
                        return std::pair<int,bool>(-1,true);
                    }
                    
                    uint8_t c = (uint8_t)std::stoi(std::string{(char)r1,(char)r2},nullptr,16);

                    return std::pair<int,bool>(c,true);
                }
                else
                {
                    return std::pair<int,bool>(read,true);
                }
            }
            else
            {
                return std::pair<int,bool>(read,false);
            }
        };


        auto ParseString = [&lineInfo, &strm, Read, ReadChr,&tokens](bool interopolated)->int {
            auto lI = lineInfo;

            std::string b={};


            auto rChr = ReadChr();
            lineInfo.Add(rChr.first);

            while(rChr.first != '\"' || rChr.second)
            {
                if(rChr.first == -1) return lineInfo.line;

                b.push_back((char)rChr.first);
                rChr = ReadChr();
                lineInfo.Add(rChr.first);
            }

            if(interopolated)
            {
                int e = 0;
                int escapeI = 0;
                std::string b2 = {};

                for(size_t i = 0; i< b.size();i++)
                {
                    if(b[i] == '{')
                    {
                        if((i+1 < b.size() && b[i+1] != '{') || escapeI >= 1)
                        {
                            if(b2.size() > 0 && escapeI < 1)
                            {
                                if(e > 0)
                                {
                                    LexToken _tkn;
                                    _tkn.type = LexTokenType::Symbol;
                                    _tkn.text = "+";
                                    _tkn.lineInfo = lI;
                                    tokens.push_back(_tkn);
                                }
                                 LexToken _tkn2;
                                _tkn2.type = LexTokenType::String;
                                _tkn2.text = b2;
                                _tkn2.lineInfo = lI;
                                tokens.push_back(_tkn2);
                                b2.clear();
                                e++;
                                
                            }
                            escapeI++;
                            if(escapeI > 1)
                            {
                                b2.push_back('{');
                            }
                        }
                        else 
                        {
                            b2.push_back('{');
                            i++;
                        }
                    } 
                    else if(b[i] == '}')
                    {
                        if(escapeI >= 1)
                        {
                            escapeI--;
                            if(b2.size() > 0 && escapeI == 0)
                            {
                                if(e > 0)
                                {
                                    LexToken _tkn;
                                    _tkn.type = LexTokenType::Symbol;
                                    _tkn.text = "+";
                                    _tkn.lineInfo = lI;
                                    tokens.push_back(_tkn);
                                }
                                LexToken _tkn2;
                                _tkn2.type = LexTokenType::Symbol;
                                _tkn2.text = "(";
                                _tkn2.lineInfo = lI;
                                tokens.push_back(_tkn2);
                                std::stringstream strm2(b2,std::ios_base::in | std::ios_base::binary);
                                int res = Lex("lexGen", strm2, tokens);
                                if(res != 0) return res;
                               
                                _tkn2.text = ")";
                                tokens.push_back(_tkn2);

                                _tkn2.text = ".";
                                tokens.push_back(_tkn2);
                                _tkn2.type = LexTokenType::Identifier;
                                _tkn2.text = "ToString";
                                tokens.push_back(_tkn2);
                                _tkn2.type = LexTokenType::Symbol;
                                _tkn2.text = "(";
                                tokens.push_back(_tkn2);
                                _tkn2.text = ")";
                                tokens.push_back(_tkn2);
                                b2.clear();
                                e++;
                            }
                            if(escapeI >= 1)
                            {
                                b2.push_back('}');
                            }
                        }
                    } else {
                        b2.push_back(b[i]);
                    }
                }
                if(b2.size() > 0)
                {
                    if(escapeI > 0)
                    {
                        if(e > 0)
                        {
                            LexToken _tkn;
                            _tkn.type = LexTokenType::Symbol;
                            _tkn.text = "+";
                            _tkn.lineInfo = lI;
                            tokens.push_back(_tkn);
                        }
                        LexToken _tkn2;
                        _tkn2.type = LexTokenType::Symbol;
                        _tkn2.text = "(";
                        _tkn2.lineInfo = lI;
                        tokens.push_back(_tkn2);
                        std::stringstream strm2(b2,std::ios_base::in | std::ios_base::binary);
                        int res = Lex("lexGen", strm2, tokens);
                        if(res != 0) return res;
                               
                        _tkn2.text = ")";
                        tokens.push_back(_tkn2);

                        _tkn2.text = ".";
                        tokens.push_back(_tkn2);
                        _tkn2.type = LexTokenType::Identifier;
                        _tkn2.text = "ToString";
                        tokens.push_back(_tkn2);
                        _tkn2.type = LexTokenType::Symbol;
                        _tkn2.text = "(";
                        tokens.push_back(_tkn2);
                        _tkn2.text = ")";
                        tokens.push_back(_tkn2);
                        b2.clear();
                        e++;
                     
                    } 
                    else
                    {
                        if(e > 0)
                        {
                            LexToken _tkn;
                            _tkn.type = LexTokenType::Symbol;
                            _tkn.text = "+";
                            _tkn.lineInfo = lI;
                            tokens.push_back(_tkn);
                        }
                        LexToken _tkn2;
                        _tkn2.type = LexTokenType::String;
                        _tkn2.text = b2;
                        _tkn2.lineInfo = lI;
                        tokens.push_back(_tkn2);
                        b2.clear();
                        e++;
                    }
                }
            } else {
                LexToken _tkn2;
                _tkn2.type = LexTokenType::String;
                _tkn2.text = b;
                _tkn2.lineInfo = lI;
                tokens.push_back(_tkn2);
                                
            }
            return 0;
        };

        while((read = Read()) != -1)
        {
            
            peek = Peek();

            switch(read)
            {
                case '$':
                    if(peek == '\"')
                    {
                        Flush();
                        lineInfo.Add(Read());
                        int re = ParseString(true);
                        if(re != 0) return re;
                    }
                    else
                    {
                        Flush();
                        Symbol({read});
                    }
                    break;
                case '\"':
                {
                    Flush();
                    int re = ParseString(false);
                    if(re != 0) return re;
                }
                    break;
                case '\'':
                {
                    Flush();
                    auto res = ReadChr();
                    if(res.first == -1) return lineInfo.line;
                    int r = Read();
                    lineInfo.Add(r);
                    if(r != '\'')
                        return lineInfo.line;
                    LexToken token;
                    token.text = {(char)(uint8_t)res.first};
                    token.lineInfo = lineInfo;
                    token.type = LexTokenType::Char;

                    tokens.push_back(token);
                }
                    break;
                case '#':
                    Flush();
                    while(true)
                    {
                        int r = Read();
                        lineInfo.Add(r);
                        if(r == '\n' || r == -1) break;
                    }
                    break;
                case '/':
                    if(peek == '/')
                    {
                        Flush();
                        while(true)
                        {
                            int r = Read();
                            lineInfo.Add(r);
                            if(r == '\n' || r == -1) break;
                        }
                    }
                    else if(peek == '*')
                    {
                        Flush();
                        while(true)
                        {
                            int r = Read();
                            lineInfo.Add(r);
                            if(r == -1) 
                            {
                                return lineInfo.line;
                            }
                            if(r == '*')
                            {
                                r = Read();
                                lineInfo.Add(r);
                                if(r == -1)
                                    return lineInfo.line;
                                if(r == '/')
                                    break;
                            }
                        }
                    }
                    else if(peek == '^')
                    {
                        Flush();
                        lineInfo.Add(Read());
                        std::string str={};
                        while(true)
                        {
                            int r = Read();
                            std::cout << r <<std::endl;
                            lineInfo.Add(r);
                            if(r == -1) 
                            {
                                return lineInfo.line;
                            }
                            if(r == '^')
                            {
                                r = Read();
                                lineInfo.Add(r);
                                if(r == -1)
                                    return lineInfo.line;
                                if(r == '^')
                                {
                                    str.push_back('^');
                                    continue;
                                }
                                if(r == '/')
                                    break;
                                str.push_back('^');
                            }
                            str.push_back((char)r);
                        }
                        LexToken token;
            
                        token.type = LexTokenType::Documentation;
                        token.lineInfo = lineInfo;
                        token.text = str;
                        tokens.push_back(token);
                    }
                    else if(peek == '=')
                    {
                        Flush();
                        lineInfo.Add(Read());
                        Symbol({read,peek});
                    }
                    else
                    {
                        Flush();
                        Symbol({read});
                    }
                    break;
                case '<':
                case '>':
                    if(peek == read)
                    {
                        Flush();
                        lineInfo.Add(Read());
                        int peek2=Peek();
                        if(peek2 == '=')
                        {
                            lineInfo.Add(Read());
                            Symbol({read,peek,peek2});
                        }
                        else
                        {
                            Symbol({read,peek});
                        }
                    }
                    else if(peek == '=')
                    {
                        Flush();
                        lineInfo.Add(Read());
                        Symbol({read,peek});
                    }
                    else
                    {
                        Flush();
                        Symbol({read});
                    }
                    break;
                case '+':
                case '-':
                case '|':
                case '&':
                    if(peek == '=' || peek == read)
                    {
                        Flush();
                        lineInfo.Add(Read());
                        Symbol({read,peek});
                    }
                    else
                    {
                        Flush();
                        Symbol({read});
                    }
                    break;
                case '=':
                    if(peek == '>')
                    {
                        Flush();
                        lineInfo.Add(Read());
                        Symbol({read,peek});
                    }
                    else if(peek == '=')
                    {
                        Flush();
                        lineInfo.Add(Read());
                        Symbol({read,peek});
                    }
                    else
                    {
                        Flush();
                        Symbol({read});
                    }
                    break;
                case '^':
                case '~':
                case '!':
                case '*':
                case '%':
                    //*
                    //*=
                    if(peek == '=')
                    {
                        Flush();
                        lineInfo.Add(Read());
                        Symbol({read,peek});
                    }
                    else
                    {
                        Flush();
                        Symbol({read});
                    }
                    break;
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case '.':
                case ':':
                case ';':
                case ',':
                case '?':
                    Flush();
                    Symbol({read});
                    break;
                case '\n':
                case '\t':
                case '\r':
                case ' ':
                    Flush();
                    
                    break;
                default:
                    buffer.push_back((char)read);
                    break;
            }
            
            lineInfo.Add(read);
        }
        Flush();
        return 0;
    }
}