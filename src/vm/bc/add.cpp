#include "CrossLang.hpp"
#include "TessesFramework/Serialization/BitConverter.hpp"
#include "TessesFramework/Streams/ByteReader.hpp"
#include "TessesFramework/Uuid.hpp"
#include <cstddef>
#include <exception>
#include <iostream>
#include <cmath>
#include <cstring>
#include <sstream>
#include <variant>
namespace Tesses::CrossLang {
    bool InterperterThread::Add(std::shared_ptr<GC> gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);



        if(std::holds_alternative<std::string>(left) && std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(right))
        {
            cse.back()->Push(gc,std::get<std::string>(left) + std::get<Tesses::Framework::Filesystem::VFSPath>(right));
        
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,(char)(std::get<char>(left) + std::get<char>(right)));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc, (char)(std::get<int64_t>(left) + std::get<char>(right)));
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc, (char)(std::get<char>(left) + std::get<int64_t>(right)));
        }
        else if(std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(left) && std::holds_alternative<std::string>(right))
        {
            cse.back()->Push(gc,std::get<Tesses::Framework::Filesystem::VFSPath>(left) + std::get<std::string>(right));
        
        }

        else if(std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(left) && std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(right))
        {
            cse.back()->Push(gc,std::get<Tesses::Framework::Filesystem::VFSPath>(left) + std::get<Tesses::Framework::Filesystem::VFSPath>(right));
        
        }

        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) + std::get<int64_t>(right));
        }
        
        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) + std::get<double>(right));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) + std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) + std::get<double>(right));
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right))
        {
            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right);
            cse.back()->Push(gc,std::make_shared<Tesses::Framework::Date::TimeSpan>((*l) + (*r)));
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right))
        {
            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right);
            cse.back()->Push(gc,std::make_shared<Tesses::Framework::Date::DateTime>((*l) + (*r)));
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right))
        {
            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right);
            cse.back()->Push(gc,std::make_shared<Tesses::Framework::Date::DateTime>((*l) + (*r)));
        }
        else if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
        {
            std::string str={};
            str.append(std::get<std::string>(left));
            str.append(std::get<std::string>(right));
            cse.back()->Push(gc,str);
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<std::string>(right))
        {
            std::string str={};
            str.push_back(std::get<char>(left));
            str.append(std::get<std::string>(right));
            cse.back()->Push(gc,str);
        }
        else if(std::holds_alternative<std::string>(left) && std::holds_alternative<char>(right))
        {
            std::string str={};
            str.append(std::get<std::string>(left));
            str.push_back(std::get<char>(right));
            cse.back()->Push(gc,str);
        }
        else if(std::holds_alternative<THeapObjectHolder>(left))
        {
            auto obj = std::get<THeapObjectHolder>(left).obj;
            auto dict = dynamic_cast<TDictionary*>(obj);
            auto dynDict = dynamic_cast<TDynamicDictionary*>(obj);
            auto natObj = dynamic_cast<TNativeObject*>(obj);
            auto cls = dynamic_cast<TClassObject*>(obj);
            if(cls != nullptr)
            {
                gc->BarrierBegin();
                auto obj=cls->GetValue(cse.back()->callable->className,"operator+");
                gc->BarrierEnd();
                TClosure* clos;
                TCallable* callable;
                if(GetObjectHeap(obj,clos)) 
                {
                    this->AddCallStackEntry(ls,clos,{right});
                    return true;
                }
                else if(GetObjectHeap(obj,callable))
                {
                    cse.back()->Push(gc,callable->Call(ls,{right}));
                    return false;
                }
                cse.back()->Push(gc,Undefined());
                return false;
               
            }
            else if(natObj != nullptr)
            {
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator+",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator+");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator+",{right}));
                return false;
            }
            else
            {
                cse.back()->Push(gc, Undefined());
            }
           
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
        return false;
    }
}