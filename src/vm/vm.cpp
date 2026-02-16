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

    
    thread_local CallStackEntry* current_function=nullptr;

    


    TObject ExecuteFunction(GCList& ls,TCallable* callable, std::vector<TObject> args)
    {
        return callable->Call(ls,args);
    }
    
    void InterperterThread::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        for(auto item : this->call_stack_entries)
        {
            item->Mark();
        }
    }

    #define TVM_HANDLER(hndl) if(hndl(gc)) goto execute

    typedef bool (InterperterThread::*opcode)(GC* gc);

    bool InterperterThread::InterperterThread::Breakpoint(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto res = cse.back()->Pop(ls);

        auto env = cse.back()->env;
        if(!env->GetRootEnvironment()->HandleBreakpoint(gc, env, res))
        {
            throw std::runtime_error("Breakpoint unhandled");
        }

        return false;
    }
    
    bool InterperterThread::Times(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) * std::get<int64_t>(right));
        }
        
        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) * std::get<double>(right));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) * std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) * std::get<double>(right));
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator*");
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
            else  if(natObj != nullptr)
            {
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator*",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator*");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator*",{right}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::Divide(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);
        if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
        {
            cse.back()->Push(gc,Tesses::Framework::Filesystem::VFSPath(std::get<std::string>(left)) / std::get<std::string>(right));
        }
        else if(std::holds_alternative<std::string>(left) && std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(right))
        {
            cse.back()->Push(gc,std::get<std::string>(left) / std::get<Tesses::Framework::Filesystem::VFSPath>(right));
        
        }


        else if(std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(left) && std::holds_alternative<std::string>(right))
        {
            cse.back()->Push(gc,std::get<Tesses::Framework::Filesystem::VFSPath>(left) / std::get<std::string>(right));
        
        }

        else if(std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(left) && std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(right))
        {
            cse.back()->Push(gc,std::get<Tesses::Framework::Filesystem::VFSPath>(left) / std::get<Tesses::Framework::Filesystem::VFSPath>(right));
        
        }

        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) / std::get<int64_t>(right));
        }
        
        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) / std::get<double>(right));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) / std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) / std::get<double>(right));
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator/");
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
            else 
             if(natObj != nullptr)
            {
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator/",{right}));
                return false;
            }
            else  
           
            
             if(natObj != nullptr)
            {
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator/",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator/");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator/",{right}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
        return false;
    }
    bool InterperterThread::Mod(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) % std::get<int64_t>(right));
        }
        
        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,fmod(std::get<double>(left), std::get<double>(right)));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,fmod(std::get<double>(left), std::get<int64_t>(right)));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,fmod(std::get<int64_t>(left), std::get<double>(right)));
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator%");
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
            else  if(natObj != nullptr)
            {
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator%",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator%");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator%",{right}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::Neg(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left))
        {
            cse.back()->Push(gc,-std::get<int64_t>(left));
        }        
        else if(std::holds_alternative<double>(left))
        {
            cse.back()->Push(gc,-std::get<double>(left));
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator-");
                gc->BarrierEnd();
                TClosure* clos;
                TCallable* callable;
                if(GetObjectHeap(obj,clos)) 
                {
                    this->AddCallStackEntry(ls,clos,{});
                    return true;
                }
                else if(GetObjectHeap(obj,callable))
                {
                    cse.back()->Push(gc,callable->Call(ls,{}));
                    return false;
                }
                cse.back()->Push(gc,Undefined());
                return false;
               
            }
             else if(natObj != nullptr)
            {
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator-",{}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject negfn = dict->GetValue("operator-");
                gc->BarrierEnd();
                return InvokeOne(ls,negfn,left);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator-",{}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,nullptr);
            }
            
        }
        else 
        {
            cse.back()->Push(gc,nullptr);
        }
 
        return false;
    }
    bool InterperterThread::LNot(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<Undefined>(left))
        {
            cse.back()->Push(gc,true);
        }
        else if(std::holds_alternative<std::nullptr_t>(left))
        {
            cse.back()->Push(gc,true);
        }

        else if(std::holds_alternative<int64_t>(left))
        {
            cse.back()->Push(gc,!std::get<int64_t>(left));
        }
        
        else if(std::holds_alternative<bool>(left))
        {
            cse.back()->Push(gc,!std::get<bool>(left));
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator!");
                gc->BarrierEnd();
                TClosure* clos;
                TCallable* callable;
                if(GetObjectHeap(obj,clos)) 
                {
                    this->AddCallStackEntry(ls,clos,{});
                    return true;
                }
                else if(GetObjectHeap(obj,callable))
                {
                    cse.back()->Push(gc,callable->Call(ls,{}));
                    return false;
                }
                cse.back()->Push(gc,Undefined());
                return false;
               
            }
             else 
            if(natObj != nullptr)
            {
                cse.back()->Push(gc, !natObj->ToBool());
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject negfn = dict->GetValue("operator!");
                gc->BarrierEnd();
                return InvokeOne(ls,negfn,left);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator!",{}));
                return false;
            }
            
            else
            {
                cse.back()->Push(gc, !ToBool(left));
            }
            
        }
        else
        {
            cse.back()->Push(gc, !ToBool(left));
        }
 
        return false;
    }
    bool InterperterThread::BNot(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left))
        {
            cse.back()->Push(gc,~std::get<int64_t>(left));
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator~");
                gc->BarrierEnd();
                TClosure* clos;
                TCallable* callable;
                if(GetObjectHeap(obj,clos)) 
                {
                    this->AddCallStackEntry(ls,clos,{});
                    return true;
                }
                else if(GetObjectHeap(obj,callable))
                {
                    cse.back()->Push(gc,callable->Call(ls,{}));
                    return false;
                }
                cse.back()->Push(gc,Undefined());
                return false;
               
            }
             else if(natObj != nullptr)
            {
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator~",{}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject negfn = dict->GetValue("operator~");
                gc->BarrierEnd();
                return InvokeOne(ls,negfn,left);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator~",{}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::Lt(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) < std::get<int64_t>(right));
        }
        
        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) < std::get<double>(right));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) < std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) < std::get<double>(right));
        }

        else if(std::holds_alternative<char>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) < std::get<int64_t>(right));
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) < std::get<char>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) < std::get<char>(right));
        }
        else if(std::holds_alternative<TVMVersion>(left) && std::holds_alternative<TVMVersion>(right))
        {
            auto lver= std::get<TVMVersion>(left);
            auto rver = std::get<TVMVersion>(right);
            auto r = lver.CompareTo(rver);
            cse.back()->Push(gc, r < 0);
        }
        else if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
        {
            cse.back()->Push(gc, std::get<std::string>(left) < std::get<std::string>(right));
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right))
        {

            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right);
            cse.back()->Push(gc,l->ToEpoch() < r->ToEpoch());
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right))
        {
            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right);
            cse.back()->Push(gc,l->TotalSeconds() < r->TotalSeconds());
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator<");
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
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator<",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator<");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator<",{right}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::Gt(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) > std::get<int64_t>(right));
        }
        
        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) > std::get<double>(right));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) > std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) > std::get<double>(right));
        }

        else if(std::holds_alternative<char>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) > std::get<int64_t>(right));
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) > std::get<char>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) > std::get<char>(right));
        }
        else if(std::holds_alternative<TVMVersion>(left) && std::holds_alternative<TVMVersion>(right))
        {
            auto lver= std::get<TVMVersion>(left);
            auto rver = std::get<TVMVersion>(right);
            auto r = lver.CompareTo(rver);
            cse.back()->Push(gc, r > 0);
        }
        else if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
        {
            cse.back()->Push(gc, std::get<std::string>(left) > std::get<std::string>(right));
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right))
        {

            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right);
            cse.back()->Push(gc,l->ToEpoch() > r->ToEpoch());
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right))
        {
            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right);
            cse.back()->Push(gc,l->TotalSeconds() > r->TotalSeconds());
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator>");
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
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator>",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator>");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator>",{right}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::Lte(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) <= std::get<int64_t>(right));
        }
        
        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) <= std::get<double>(right));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) <= std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) <= std::get<double>(right));
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) <= std::get<char>(right));
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) <= std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) <= std::get<char>(right));
        }
        else if(std::holds_alternative<TVMVersion>(left) && std::holds_alternative<TVMVersion>(right))
        {
            auto lver= std::get<TVMVersion>(left);
            auto rver = std::get<TVMVersion>(right);
            auto r = lver.CompareTo(rver);
            cse.back()->Push(gc, r <= 0);
        }
        else if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
        {
            cse.back()->Push(gc, std::get<std::string>(left) <= std::get<std::string>(right));
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right))
        {

            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right);
            cse.back()->Push(gc,l->ToEpoch() <= r->ToEpoch());
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right))
        {
            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right);
            cse.back()->Push(gc,l->TotalSeconds() <= r->TotalSeconds());
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator<=");
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
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator<=",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator<=");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator<=",{right}));
                return false;
            }
          
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::Gte(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) >= std::get<int64_t>(right));
        }
        
        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) >= std::get<double>(right));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) >= std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) >= std::get<double>(right));
        }

        else if(std::holds_alternative<char>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) >= std::get<int64_t>(right));
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) >= std::get<char>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) >= std::get<char>(right));
        }
        else if(std::holds_alternative<TVMVersion>(left) && std::holds_alternative<TVMVersion>(right))
        {
            auto lver= std::get<TVMVersion>(left);
            auto rver = std::get<TVMVersion>(right);
            auto r = lver.CompareTo(rver);
            cse.back()->Push(gc, r >= 0);
        }
        else if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
        {
            cse.back()->Push(gc, std::get<std::string>(left) >= std::get<std::string>(right));
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right))
        {

            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right);
            cse.back()->Push(gc,l->ToEpoch() >= r->ToEpoch());
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right))
        {
            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right);
            cse.back()->Push(gc,l->TotalSeconds() >= r->TotalSeconds());
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator>=");
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
            else 
            if(natObj != nullptr)
            {
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator>=",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator>=");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator>=",{right}));
                return false;
            }
            
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::Eq(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);
        if(std::holds_alternative<std::nullptr_t>(left) && std::holds_alternative<std::nullptr_t>(right))
        {
            cse.back()->Push(gc,true);
            return false;
        }

        else if(std::holds_alternative<Undefined>(left) && std::holds_alternative<Undefined>(right))
        {
            cse.back()->Push(gc,true);
            return false;
        }

        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) == std::get<int64_t>(right));
        }
        
        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) == std::get<double>(right));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) == std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) == std::get<double>(right));
        }

        else if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
        {
            cse.back()->Push(gc,std::get<std::string>(left) == std::get<std::string>(right));
        }

        else if(std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right))
        {
            cse.back()->Push(gc,std::get<bool>(left) == std::get<bool>(right));
        }

        else if(std::holds_alternative<char>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) == std::get<char>(right));
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) == std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) == std::get<char>(right));
        }
        else if(std::holds_alternative<TVMVersion>(left) && std::holds_alternative<TVMVersion>(right))
        {
            auto lver= std::get<TVMVersion>(left);
            auto rver = std::get<TVMVersion>(right);
            auto r = lver.CompareTo(rver);
            cse.back()->Push(gc, r == 0);
        }
        else if(std::holds_alternative<Tesses::Framework::Uuid>(left) && std::holds_alternative<Tesses::Framework::Uuid>(right))
        {
            auto l= std::get<Tesses::Framework::Uuid>(left);
            auto r = std::get<Tesses::Framework::Uuid>(right);
            
            return l == r;
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right))
        {

            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right);
            cse.back()->Push(gc,(l->ToEpoch() == r->ToEpoch()) && (l->IsLocal() == r->IsLocal()));
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right))
        {
            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right);
            cse.back()->Push(gc,l->TotalSeconds() == r->TotalSeconds());
        }
        else if(std::holds_alternative<THeapObjectHolder>(left))
        {
            auto obj = std::get<THeapObjectHolder>(left).obj;
            auto dict = dynamic_cast<TDictionary*>(obj);

            auto dynDict = dynamic_cast<TDynamicDictionary*>(obj);
            auto native = dynamic_cast<TNative*>(obj);
            auto natObj = dynamic_cast<TNativeObject*>(obj);
            auto cls = dynamic_cast<TClassObject*>(obj);
            if(cls != nullptr)
            {
                gc->BarrierBegin();
                auto obj=cls->GetValue(cse.back()->callable->className,"operator==");
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
                cse.back()->Push(gc, natObj->Equals(gc,right));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator==");
                gc->BarrierEnd();
                if(!std::holds_alternative<Undefined>(fn))
                return InvokeTwo(ls,fn,left,right);
                
            }
            
            else if(dynDict != nullptr)
            {
                auto res = dynDict->CallMethod(ls,"operator==",{right});
                if(!std::holds_alternative<std::nullptr_t>(res) && std::holds_alternative<Undefined>(res))
                {
                    cse.back()->Push(gc,res);
                    return false;
                }
            }
            
            else if(native != nullptr && std::holds_alternative<std::nullptr_t>(right)){
                cse.back()->Push(gc, native->GetDestroyed());
                return false;
            }

            if(std::holds_alternative<THeapObjectHolder>(right))
            {
                cse.back()->Push(gc,obj == std::get<THeapObjectHolder>(right).obj);
                return false;
            }

            else if(std::holds_alternative<std::nullptr_t>(right))
            {
                cse.back()->Push(gc, false);
                return false;
            }
            else if(std::holds_alternative<Undefined>(right))
            {
                cse.back()->Push(gc, false);
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }

        else if(std::holds_alternative<std::nullptr_t>(right))
        {
            cse.back()->Push(gc, false);
            return false;
        }
        else if(std::holds_alternative<Undefined>(right))
        {
            cse.back()->Push(gc, false);
            return false;
        }
        else
        {
            cse.back()->Push(gc, Undefined());
            
        }
 
        return false;
    }
    bool InterperterThread::NEq(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<std::nullptr_t>(left) && std::holds_alternative<std::nullptr_t>(right))
        {
            cse.back()->Push(gc,false);
            return false;
        }
        else if(std::holds_alternative<Undefined>(left) && std::holds_alternative<Undefined>(right))
        {
            cse.back()->Push(gc,false);
            return false;
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) != std::get<int64_t>(right));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) != std::get<double>(right));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) != std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) != std::get<double>(right));
        }
        else if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
        {
            cse.back()->Push(gc,std::get<std::string>(left) != std::get<std::string>(right));
        }
        else if(std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right))
        {
            cse.back()->Push(gc,std::get<bool>(left) != std::get<bool>(right));
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) != std::get<char>(right));
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<char>(left) != std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) != std::get<char>(right));
        }
        else if(std::holds_alternative<TVMVersion>(left) && std::holds_alternative<TVMVersion>(right))
        {
            auto lver= std::get<TVMVersion>(left);
            auto rver = std::get<TVMVersion>(right);
            auto r = lver.CompareTo(rver);
            cse.back()->Push(gc, r != 0);
        }
        else if(std::holds_alternative<Tesses::Framework::Uuid>(left) && std::holds_alternative<Tesses::Framework::Uuid>(right))
        {
            auto l= std::get<Tesses::Framework::Uuid>(left);
            auto r = std::get<Tesses::Framework::Uuid>(right);
            
            return l != r;
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right))
        {

            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(right);
            cse.back()->Push(gc,!((l->ToEpoch() == r->ToEpoch()) && (l->IsLocal() == r->IsLocal())));
        }
        else if(std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left) && std::holds_alternative<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right))
        {
            auto& l = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left);
            auto& r = std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right);
            cse.back()->Push(gc,l->TotalSeconds() != r->TotalSeconds());
        }
        else if(std::holds_alternative<THeapObjectHolder>(left))
        {
            auto obj = std::get<THeapObjectHolder>(left).obj;
            auto dict = dynamic_cast<TDictionary*>(obj);
            auto native = dynamic_cast<TNative*>(obj);
            auto natObj = dynamic_cast<TNativeObject*>(obj);
            auto dynDict = dynamic_cast<TDynamicDictionary*>(obj);
            auto cls = dynamic_cast<TClassObject*>(obj);
            if(cls != nullptr)
            {
                gc->BarrierBegin();
                auto obj=cls->GetValue(cse.back()->callable->className,"operator!=");
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
            else 
            if(natObj != nullptr)
            {
                cse.back()->Push(gc, !natObj->Equals(gc,right));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator!=");
                gc->BarrierEnd();
                if(!std::holds_alternative<Undefined>(fn))           
                return InvokeTwo(ls,fn,left,right);
            }

            else if(dynDict != nullptr)
            {
                
                auto res = dynDict->CallMethod(ls,"operator!=",{right});
                if(!std::holds_alternative<std::nullptr_t>(res) && std::holds_alternative<Undefined>(res))
                {
                    cse.back()->Push(gc,res);
                    return false;
                }
            }
            
            else if(native != nullptr && std::holds_alternative<std::nullptr_t>(right)){
                cse.back()->Push(gc, !native->GetDestroyed());
                return false;
            }
            if(std::holds_alternative<THeapObjectHolder>(right))
            {
                cse.back()->Push(gc,obj != std::get<THeapObjectHolder>(right).obj);
                return false;
            }
            else if(std::holds_alternative<std::nullptr_t>(right))
            {
                cse.back()->Push(gc, true);
                return false;
            }
            else if(std::holds_alternative<Undefined>(right))
            {
                cse.back()->Push(gc, true);
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
                
            }
            
        }
        else if(std::holds_alternative<Undefined>(right))
        {
            cse.back()->Push(gc, true);
            return false;
        }
        else if(std::holds_alternative<std::nullptr_t>(right))
        {
            cse.back()->Push(gc, true);
            return false;
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::LShift(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) << (int)std::get<int64_t>(right));
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator<<");
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
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator<<",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator<<");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }

            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator<<",{right}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::RShift(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) >> (int)std::get<int64_t>(right));
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator>>");
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
            else 
            if(natObj != nullptr)
            {
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator>>",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator>>");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }

            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator>>",{right}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::BOr(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) | std::get<int64_t>(right));
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator|");
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
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator|",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator|");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }

            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator|",{right}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::XOr(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) ^ std::get<int64_t>(right));
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator^");
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
            else 
            if(natObj != nullptr)
            {
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator^",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator^");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }

            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator^",{right}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::BAnd(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) & std::get<int64_t>(right));
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
                auto obj=cls->GetValue(cse.back()->callable->className,"operator&");
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
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator&",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator&");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator&",{right}));
                return false;
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
            
        }
        else
        {
            cse.back()->Push(gc, Undefined());
        }
 
        return false;
    }
    bool InterperterThread::InvokeOne(GCList& ls, TObject fn, TObject arg)
    {
        if(std::holds_alternative<THeapObjectHolder>(fn))
        {
            
            auto obj = dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(fn).obj);
            if(obj != nullptr)
            {
                auto closure = dynamic_cast<TClosure*>(obj);
                if(closure != nullptr)
                {
                    
                    if(!closure->closure->args.empty() && closure->closure->args[0] == "this")
                    {
                        this->AddCallStackEntry(ls,closure,{arg});
                    }
                    else
                    {
                        this->AddCallStackEntry(ls,closure,{});
                    }
                    
                }
                else
                {
                    auto val = obj->Call(ls,{});
                    this->call_stack_entries.back()->Push(ls.GetGC(), val);
                    return false;
                }
                return true;
            }
            
        }
        return false;
    }
    
    bool InterperterThread::ExecuteFunction(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            auto stk = cse.back();
            std::vector<uint8_t>& code = stk->callable->closure->code;
             GCList ls(gc);
            auto res = stk->Pop(ls);


            if(std::holds_alternative<int64_t>(res))
            {
                uint32_t n=(uint32_t)std::get<int64_t>(res);
               

                std::vector<TObject> args;

                for(size_t i = 0;i<n;i++)
                {
                    args.insert(args.begin(),{stk->Pop(ls)});
                }
                


                TObject fn = stk->Pop(ls);
                if(std::holds_alternative<MethodInvoker>(fn))
                {
                    if(args.size() == 3)
                    {
                       
                        if(std::holds_alternative<std::string>(args[1]) && std::holds_alternative<THeapObjectHolder>(args[2]))
                        {
                            std::string key = std::get<std::string>(args[1]);
                            TList* ls = dynamic_cast<TList*>(std::get<THeapObjectHolder>(args[2]).obj);
                            if(ls != nullptr)
                                return ExecuteMethod2(gc,args[0],key,ls->items);
                        }
                    }
                    stk->Push(gc, Undefined());
                    return false;
                }

                if(std::holds_alternative<THeapObjectHolder>(fn))
                {
            
                    auto obj = dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(fn).obj);
                    if(obj != nullptr)
                    {
                        auto closure = dynamic_cast<TClosure*>(obj);
                        if(closure != nullptr)
                        {
                    
                            this->AddCallStackEntry(ls,closure,args);
                            return true;
                        }
                        else
                        {
                            
                            auto val = obj->Call(ls,args);
                            this->call_stack_entries.back()->Push(ls.GetGC(), val);
                      
                           return false;
                        }
                        return true;
                    }
                    stk->Push(gc,Undefined());
                    return false;
                }
                stk->Push(gc, Undefined());
            }
            else
                stk->Push(gc, Undefined());
        }
        return false;
    }
    
    bool InterperterThread::Yield(GC* gc)
    {
        GCList ls(gc);
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            gc->BarrierBegin();
            cse.back()->mustReturn=true;
            cse.back()->Push(gc, cse.back());
            gc->BarrierEnd();
        }
        return false;
    }
    bool InterperterThread::ExecuteMethod(GC* gc)
    {
         GCList ls(gc);
         std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            auto stk = cse.back();
            std::vector<uint8_t>& code = stk->callable->closure->code;
           
            auto cnt = stk->Pop(ls);
            if(std::holds_alternative<int64_t>(cnt))
            {
                uint32_t n=(uint32_t)std::get<int64_t>(cnt);
               

                std::vector<TObject> args;

                for(size_t i = 0;i<n;i++)
                {
                    args.insert(args.begin(),{stk->Pop(ls)});
                }
                

                TObject key = stk->Pop(ls);
                TObject instance = stk->Pop(ls);
                
                if(std::holds_alternative<std::string>(key))
                {
                    return ExecuteMethod2(gc,instance,std::get<std::string>(key),args);
                }
                stk->Push(gc, Undefined());
                return false;   
            }
        }
        return false;
    }
   
    bool InterperterThread::GetVariable(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {            
            auto stk = cse.back();
            GCList ls(gc);
            auto key = stk->Pop(ls);
            if(std::holds_alternative<std::string>(key))
            {
                gc->BarrierBegin();
                stk->Push(gc, 
                    stk->env->GetVariable(ls,std::get<std::string>(key)));
                gc->BarrierEnd();
            }
            else
            {
                throw VMException("[GETVARIABLE] Can't pop string.");
            }
        }
        return false;
    }
    bool InterperterThread::SetVariable(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {            
            auto stk = cse.back();
            GCList ls(gc);
            auto value = stk->Pop(ls);
            auto key = stk->Pop(ls);
            TList* mls;
            if(std::holds_alternative<std::string>(key))
            {
                gc->BarrierBegin();

                if(stk->env->HasConstForSet(std::get<std::string>(key)))
                {
                    gc->BarrierEnd();
                    ThrowConstError(std::get<std::string>(key));
                }    
                stk->Push(gc,stk->env->SetVariable(ls,std::get<std::string>(key),value));
                 
                gc->BarrierEnd();
            }
            else if(GetObjectHeap(key,mls))
            {
                gc->BarrierBegin();

                TList* valueLs;
                TDynamicList* valueDynList;
                TDictionary* valueDict;
                TDynamicDictionary* valueDynDict;
                if(GetObjectHeap(value, valueLs))
                {
                    TDictionary* result = TDictionary::Create(ls);
                    int64_t len = std::min(valueLs->Count(), mls->Count());
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            auto val = valueLs->Get(i);
                            result->SetValue(mkey, val);

                            if(stk->env->HasConstForSet(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }    
                            stk->env->SetVariable(ls,mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else if(GetObjectHeap(value, valueDynList))
                {
                    TDictionary* result = TDictionary::Create(ls);
                    gc->BarrierEnd();
                    int64_t len = std::min(valueDynList->Count(ls), mls->Count());
                    gc->BarrierBegin();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            gc->BarrierEnd();
                            auto val = valueDynList->GetAt(ls,i);
                            gc->BarrierBegin();
                            result->SetValue(mkey, val);

                            if(stk->env->HasConstForSet(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }    
                            stk->env->SetVariable(ls,mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else if(GetObjectHeap(value, valueDict))
                {

                    TDictionary* result = TDictionary::Create(ls);
                    int64_t len = mls->Count();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            auto val = valueDict->GetValue(mkey);
                            result->SetValue(mkey, val);

                            if(stk->env->HasConstForSet(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }    
                            stk->env->SetVariable(ls,mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else if(GetObjectHeap(value, valueDynDict))
                {
                    TDictionary* result = TDictionary::Create(ls);
                    int64_t len =mls->Count();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            gc->BarrierEnd();
                            auto val = valueDynDict->GetField(ls,mkey);
                            gc->BarrierBegin();

                            result->SetValue(mkey, val);
                            
                            if(stk->env->HasConstForSet(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }    
                            stk->env->SetVariable(ls,mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else {
                    int64_t len =mls->Count();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            if(stk->env->HasConstForSet(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }    
                            
                            stk->env->SetVariable(ls,mkey, value);
                        }
                    }
                    stk->Push(gc, value);
                }
                gc->BarrierEnd();
            }
            else
            {
                throw VMException("[SETVARIABLE] Can't pop string.");
            }
        }
        return false;
    }
    bool InterperterThread::DeclareVariable(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {            
            auto stk = cse.back();
            GCList ls(gc);
            
            auto value = stk->Pop(ls);
            auto key = stk->Pop(ls);

            TList* mls;
        
            if(std::holds_alternative<std::string>(key))
            {
                gc->BarrierBegin();
                if(stk->env->HasConstForDeclare(std::get<std::string>(key)))
                {
                    gc->BarrierEnd();
                    ThrowConstError(std::get<std::string>(key));
                }
                stk->env->DeclareVariable(std::get<std::string>(key),value);
                stk->Push(gc, value);
                gc->BarrierEnd();
            }
            else if(GetObjectHeap(key,mls))
            {
                gc->BarrierBegin();

                TList* valueLs;
                TDynamicList* valueDynList;
                TDictionary* valueDict;
                TDynamicDictionary* valueDynDict;
                if(GetObjectHeap(value, valueLs))
                {
                    TDictionary* result = TDictionary::Create(ls);
                    int64_t len = std::min(valueLs->Count(), mls->Count());
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            auto val = valueLs->Get(i);
                            result->SetValue(mkey, val);
                            if(stk->env->HasConstForDeclare(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }
                            stk->env->DeclareVariable(mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else if(GetObjectHeap(value, valueDynList))
                {
                    TDictionary* result = TDictionary::Create(ls);
                    gc->BarrierEnd();
                    int64_t len = std::min(valueDynList->Count(ls), mls->Count());
                    gc->BarrierBegin();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            gc->BarrierEnd();
                            auto val = valueDynList->GetAt(ls,i);
                            gc->BarrierBegin();
                            result->SetValue(mkey, val);
                            if(stk->env->HasConstForDeclare(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }
                            stk->env->DeclareVariable(mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else if(GetObjectHeap(value, valueDict))
                {

                    TDictionary* result = TDictionary::Create(ls);
                    int64_t len = mls->Count();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            auto val = valueDict->GetValue(mkey);
                            result->SetValue(mkey, val);
                            if(stk->env->HasConstForDeclare(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }
                            stk->env->DeclareVariable(mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else if(GetObjectHeap(value, valueDynDict))
                {
                    TDictionary* result = TDictionary::Create(ls);
                    int64_t len =mls->Count();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            gc->BarrierEnd();
                            auto val = valueDynDict->GetField(ls,mkey);
                            gc->BarrierBegin();

                            result->SetValue(mkey, val);
                            if(stk->env->HasConstForDeclare(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }
                            stk->env->DeclareVariable(mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else {
                    int64_t len =mls->Count();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            if(stk->env->HasConstForDeclare(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }
                            stk->env->DeclareVariable(mkey, value);
                        }
                    }
                    stk->Push(gc, value);
                }
                gc->BarrierEnd();
            }
            else
            {

                throw VMException("[DECLAREVARIABLE] Can't pop string, got type " + GetObjectTypeString(key) + " = " + ToString(gc,key) + ".");
            }
        }
        return false;
    }
  

    bool InterperterThread::DeclareConstVariable(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {            
            auto stk = cse.back();
            GCList ls(gc);
            
            auto value = stk->Pop(ls);
            auto key = stk->Pop(ls);

            TList* mls;
        
            if(std::holds_alternative<std::string>(key))
            {
                gc->BarrierBegin();
                if(stk->env->HasConstForDeclare(std::get<std::string>(key)))
                {
                    gc->BarrierEnd();
                    ThrowConstError(std::get<std::string>(key));
                }
                stk->env->DeclareConstVariable(std::get<std::string>(key),value);
                stk->Push(gc, value);
                gc->BarrierEnd();
            }
            else if(GetObjectHeap(key,mls))
            {
                gc->BarrierBegin();

                TList* valueLs;
                TDynamicList* valueDynList;
                TDictionary* valueDict;
                TDynamicDictionary* valueDynDict;
                if(GetObjectHeap(value, valueLs))
                {
                    TDictionary* result = TDictionary::Create(ls);
                    int64_t len = std::min(valueLs->Count(), mls->Count());
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            auto val = valueLs->Get(i);
                            result->SetValue(mkey, val);
                            if(stk->env->HasConstForDeclare(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }
                            stk->env->DeclareConstVariable(mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else if(GetObjectHeap(value, valueDynList))
                {
                    TDictionary* result = TDictionary::Create(ls);
                    gc->BarrierEnd();
                    int64_t len = std::min(valueDynList->Count(ls), mls->Count());
                    gc->BarrierBegin();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            gc->BarrierEnd();
                            auto val = valueDynList->GetAt(ls,i);
                            gc->BarrierBegin();
                            result->SetValue(mkey, val);
                            if(stk->env->HasConstForDeclare(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }
                            stk->env->DeclareConstVariable(mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else if(GetObjectHeap(value, valueDict))
                {

                    TDictionary* result = TDictionary::Create(ls);
                    int64_t len = mls->Count();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            auto val = valueDict->GetValue(mkey);
                            result->SetValue(mkey, val);
                            if(stk->env->HasConstForDeclare(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }
                            stk->env->DeclareConstVariable(mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else if(GetObjectHeap(value, valueDynDict))
                {
                    TDictionary* result = TDictionary::Create(ls);
                    int64_t len =mls->Count();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            gc->BarrierEnd();
                            auto val = valueDynDict->GetField(ls,mkey);
                            gc->BarrierBegin();

                            result->SetValue(mkey, val);
                            if(stk->env->HasConstForDeclare(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }
                            stk->env->DeclareConstVariable(mkey,val);
                        }
                    }
                    stk->Push(gc,result);
                }
                else {
                    int64_t len =mls->Count();
                    for(int64_t i = 0; i < len; i++)
                    {
                        std::string mkey;
                        auto item = mls->Get(i);
                        if(GetObject(item,mkey))
                        {
                            if(stk->env->HasConstForDeclare(mkey))
                            {
                                gc->BarrierEnd();
                                ThrowConstError(mkey);
                            }
                            stk->env->DeclareConstVariable(mkey, value);
                        }
                    }
                    stk->Push(gc, value);
                }
                gc->BarrierEnd();
            }
            else
            {

                throw VMException("[DECLARECONSTVARIABLE] Can't pop string, got type " + GetObjectTypeString(key) + " = " + ToString(gc,key) + ".");
            }
        }
        return false;
    }
    bool InterperterThread::PushResourceStream(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            auto stk = cse.back();
            std::vector<uint8_t>& code = stk->callable->closure->code;
            if(stk->ip + 4 <= code.size())
            {
                uint32_t n=BitConverter::ToUint32BE(code[stk->ip]);
                if(n >= stk->callable->file->resources.size())
                    throw VMException("Can't read resource.");
                stk->ip = stk->ip + 4;

                gc->BarrierBegin();
                GCList ls(gc);
  //              TByteArray* arr = TByteArray::Create(ls);
//                arr->data = stk->callable->file->resources[n];
                stk->Push(gc, std::make_shared<EmbedStream>(gc,stk->callable->file,n));
                
                gc->BarrierEnd();
            }
            else
            {
                throw VMException("Can't read chunk.");
            }
        }
        return false;
    }

    bool InterperterThread::PushResource(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            auto stk = cse.back();
            std::vector<uint8_t>& code = stk->callable->closure->code;
            if(stk->ip + 4 <= code.size())
            {
                uint32_t n=BitConverter::ToUint32BE(code[stk->ip]);
                if(n >= stk->callable->file->resources.size())
                    throw VMException("Can't read resource.");
                stk->ip = stk->ip + 4;

                gc->BarrierBegin();
                GCList ls(gc);
                TByteArray* arr = TByteArray::Create(ls);
                arr->data = stk->callable->file->resources[n];
                stk->Push(gc, arr);

                gc->BarrierEnd();
            }
            else
            {
                throw VMException("Can't read chunk.");
            }
        }
        return false;
    }
    bool InterperterThread::Throw(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto _res2 = cse.back()->Pop(ls);

        if(!std::holds_alternative<Undefined>(_res2))
        {
            auto env = cse.back()->env;
            if(!env->GetRootEnvironment()->HandleException(gc,env, _res2))
            throw VMByteCodeException(gc,_res2,cse.back());
        }
        return false;
    }
    bool InterperterThread::PushResourceDirectory(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto _res2 = cse.back()->Pop(ls);
        TDictionary* dict;
        if(GetObjectHeap(_res2, dict))
        {
            cse.back()->Push(gc, std::make_shared<EmbedDirectory>(gc,dict));
        }
        else {
            cse.back()->Push(gc, Undefined());
        }
        return false;
    }
    bool InterperterThread::JumpIfDefined(GC* gc)
    {
        
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();

        if(stk->ip + 4 <= stk->callable->closure->code.size())
        {
            uint32_t n=BitConverter::ToUint32BE(stk->callable->closure->code[stk->ip]);
                            
            GCList ls(gc);
            auto _res2 = stk->Pop(ls);
                                
            stk->ip = stk->ip + 4;
            if(!std::holds_alternative<Undefined>(_res2) && !std::holds_alternative<std::nullptr_t>(_res2))
            {
                stk->ip = n;
                stk->Push(gc,_res2);
            }
                            
                            
        }
        else
            throw VMException("Can't read jmpifdefined pc.");
        return false;
    }
    bool InterperterThread::JumpIfBreak(GC* gc)
    {
        
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();

        if(stk->ip + 4 <= stk->callable->closure->code.size())
        {
            uint32_t n=BitConverter::ToUint32BE(stk->callable->closure->code[stk->ip]);
                            
            GCList ls(gc);
            auto _res2 = stk->Pop(ls);
                                
            stk->ip = stk->ip + 4;
            if(std::holds_alternative<TBreak>(_res2))
                stk->ip = n;
            else
                stk->Push(gc,_res2);
                            
                            
        }
        else
            throw VMException("Can't read jmpifbreak pc.");
        return false;
    }

    bool InterperterThread::JumpIfContinue(GC* gc)
    {
        
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();

        if(stk->ip + 4 <= stk->callable->closure->code.size())
        {
            uint32_t n=BitConverter::ToUint32BE(stk->callable->closure->code[stk->ip]);
                            
            GCList ls(gc);
            auto _res2 = stk->Pop(ls);
                                
            stk->ip = stk->ip + 4;
            if(std::holds_alternative<TContinue>(_res2))
                stk->ip = n;
            else
                stk->Push(gc,_res2);
                            
                            
        }
        else
            throw VMException("Can't read jmpifcontinue pc.");
        return false;
    }
    bool InterperterThread::JumpUndefined(GC* gc)
    {
        
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();

        if(stk->ip + 4 <= stk->callable->closure->code.size())
        {
            uint32_t n=BitConverter::ToUint32BE(stk->callable->closure->code[stk->ip]);
                            
            GCList ls(gc);
            auto _res2 = stk->Pop(ls);
                                
            stk->ip = stk->ip + 4;
            if(std::holds_alternative<Undefined>(_res2))
                stk->ip = n;
            else
                stk->Push(gc,_res2);
                            
                            
        }
        else
            throw VMException("Can't read jmpundefined pc.");
        return false;
    }
    bool InterperterThread::Jump(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();

        if(stk->ip + 4 <= stk->callable->closure->code.size())
        {
            uint32_t n=BitConverter::ToUint32BE(stk->callable->closure->code[stk->ip]);
            stk->ip = n;
        }
        else
            throw VMException("Can't read jmp pc.");
        return false;
    }
    bool InterperterThread::PushNull(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
       
        stk->Push(gc,nullptr);
        return false;
    }
    bool InterperterThread::PushBreak(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
       
        stk->Push(gc,TBreak());
        return false;
    }
    bool InterperterThread::PushContinue(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
       
        stk->Push(gc,TContinue());
        return false;
    }
     bool InterperterThread::PushUndefined(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
       
        stk->Push(gc,Undefined());
        return false;
    }
    bool InterperterThread::LineInfo(GC* gc)
    {
        GCList ls(gc);
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
        auto file = stk->Pop(ls);
        auto line = stk->Pop(ls);      
        
        //Set the fields in this cse
        GetObject(file,stk->srcfile);
        GetObject(line,stk->srcline);  

        //TODO: implement lines (this will make the language work until we get it rigged up)



        return false;
    }
      bool InterperterThread::PushFalse(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
       
        stk->Push(gc,false);
        return false;
    }
      bool InterperterThread::PushTrue(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
       
        stk->Push(gc,true);
        return false;
    }
    bool InterperterThread::CreateDictionary(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        stk->Push(gc,dict);
        return false;
    }
     bool InterperterThread::Nop(GC* gc)
    {
        return false;
    }   
    bool InterperterThread::AppendList(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
       
                        GCList ls(gc);
                        gc->BarrierBegin();
                        auto obj = stk->Pop(ls);
                        auto objhold= stk->Pop(ls);
                        if(std::holds_alternative<THeapObjectHolder>(objhold))
                        {
                            auto list= dynamic_cast<TList*>(std::get<THeapObjectHolder>(objhold).obj);
                            if(list != nullptr)
                            {
                                list->Add(obj);
                            }
                            /*
                            if(dict != nullptr)
                            {
                                auto potential_str = stk->Pop(ls);
                                if(std::holds_alternative<std::string>(potential_str))
                                {
                                    dict->SetValue(std::get<std::string>(potential_str), stk->Pop(ls));
                                }
                            }*/
                        }

                        stk->Push(gc, objhold);

                        gc->BarrierEnd();
        return false;
    }
     bool InterperterThread::AppendDictionary(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
        GCList ls(gc);
                        gc->BarrierBegin();
                        auto value = stk->Pop(ls);
                        auto k = stk->Pop(ls);
                        auto objhold= stk->Pop(ls);
                        if(std::holds_alternative<THeapObjectHolder>(objhold) && std::holds_alternative<std::string>(k))
                        {
                            auto dict= dynamic_cast<TDictionary*>(std::get<THeapObjectHolder>(objhold).obj);
                            auto cls = dynamic_cast<TClassObject*>(std::get<THeapObjectHolder>(objhold).obj); 
                            
                            if(dict != nullptr)
                            {
                                dict->SetValue(std::get<std::string>(k), value);
                            }
                            else if(cls != nullptr)
                            {
                                auto obj=cls->GetValue(cse.back()->callable->className,"set"+std::get<std::string>(k));
                    TCallable* callable;
                    if(GetObjectHeap(obj,callable))
                    {
                                     gc->BarrierEnd();
                   
                        callable->Call(ls,{value});
                        gc->BarrierBegin();
                    }else {
                    cls->SetValue(cse.back()->callable->className,std::get<std::string>(k),value);
                    }
                            }
                        }

                        stk->Push(gc, objhold);

                        gc->BarrierEnd();
        return false;
    }
     bool InterperterThread::CreateArray(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
        GCList ls(gc);
        TList* dict = TList::Create(ls);
        stk->Push(gc,dict);
        return false;
    }
    bool InterperterThread::Pop(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
        GCList ls(gc);
        stk->Pop(ls);
        return false;
    }
    bool InterperterThread::TryCatch(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
                        GCList ls(gc);
                        auto catchFn = stk->Pop(ls);
                        auto tryFn = stk->Pop(ls);

                        TCallable* tryC;
                        TCallable* catchC;

                        if(GetObjectHeap(tryFn,tryC) && GetObjectHeap(catchFn,catchC))
                        {
                            try 
                            {
                                stk->Push(gc, tryC->Call(ls,{}));
                            }   
                            
                            catch(std::exception& ex)
                            {
                                TDictionary* dict = TDictionary::Create(ls);
                                auto gc = ls.GetGC();
                                auto myEx = dynamic_cast<VMByteCodeException*>(&ex);
                                if(myEx != nullptr)
                                {
                                    stk->Push(gc, catchC->Call(ls,{myEx->exception}));
                                } else {
                                gc->BarrierBegin();
                                
                                dict->SetValue("Type","NativeException");
                                dict->SetValue("Text",ex.what());
                                gc->BarrierEnd();

                                stk->Push(gc, catchC->Call(ls,{dict}));
                                }
                            }
                           
                            
                        }
        return false;
    }
    bool InterperterThread::JumpConditional(GC* gc)
    {

        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        auto stk = cse.back();
        if(stk->ip + 4 <= stk->callable->closure->code.size())
        {
            uint32_t n=BitConverter::ToUint32BE(stk->callable->closure->code[stk->ip]);
                            
            GCList ls2(gc);
            auto _res2 = stk->Pop(ls2);
            auto _res = ToBool(_res2);
            stk->ip = stk->ip + 4;
            if(_res)
            stk->ip = n;
                            
                            
                            
        }
        else
            throw VMException("Can't read jmpc pc.");
        return false;
    }
    bool InterperterThread::PushClosure(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            auto stk = cse.back();
            std::vector<uint8_t>& code = stk->callable->closure->code;
            if(stk->ip + 4 <= code.size())
            {
                uint32_t n=BitConverter::ToUint32BE(code[stk->ip]);
                if(n >= stk->callable->file->chunks.size())
                    throw VMException("Can't read chunk.");
                stk->ip = stk->ip + 4;

                gc->BarrierBegin();
                GCList ls(gc);
                TClosure* closure = TClosure::Create(ls,stk->env,stk->callable->file,n,true);
                closure->className = cse.back()->callable->className;
                stk->Push(gc,closure);
                gc->BarrierEnd();
            }
            else
            {
                throw VMException("Can't read chunk.");
            }
        }
        return false;
    }
    bool InterperterThread::PushScopelessClosure(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            auto stk = cse.back();
            std::vector<uint8_t>& code = stk->callable->closure->code;
            if(stk->ip + 4 <= code.size())
            {
                uint32_t n=BitConverter::ToUint32BE(code[stk->ip]);
                if(n >= stk->callable->file->chunks.size())
                    throw VMException("Can't read chunk.");
                stk->ip = stk->ip + 4;

                gc->BarrierBegin();
                GCList ls(gc);
                TClosure* closure = TClosure::Create(ls,stk->env,stk->callable->file,n,false);
                closure->className = stk->callable->className;
                stk->Push(gc,closure);
                gc->BarrierEnd();
            }
            else
            {
                throw VMException("Can't read chunk.");
            }
        }
        return false;
    }
    
    bool InterperterThread::PushString(GC* gc)
    {
        
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            auto stk = cse.back();
            std::vector<uint8_t>& code = stk->callable->closure->code;
            if(stk->ip + 4 <= code.size())
            {
                uint32_t n=BitConverter::ToUint32BE(code[stk->ip]);
                if(n < stk->callable->file->strings.size())
                    stk->Push(gc,stk->callable->file->strings[n]);
                else
                    throw VMException("Can't read string.");
                stk->ip = stk->ip + 4;
            }
        }
        return false;
    }
    
    bool InterperterThread::PushLong(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            auto stk = cse.back();
            std::vector<uint8_t>& code = stk->callable->closure->code;
            if(stk->ip + 8 <= code.size())
            {
                uint64_t n=BitConverter::ToUint64BE(code[stk->ip]);
                stk->Push(gc,(int64_t)n);
                stk->ip = stk->ip + 8;
            }
        }
        return false;
    }
    bool InterperterThread::PushChar(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            auto stk = cse.back();
            std::vector<uint8_t>& code = stk->callable->closure->code;
            if(stk->ip + 1 <= code.size())
            {
                char c = (char)code[stk->ip];
                stk->Push(gc,c);
                stk->ip = stk->ip + 1;
            }
        }
        return false;
    }
    bool InterperterThread::PushDouble(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            auto stk = cse.back();
            std::vector<uint8_t>& code = stk->callable->closure->code;
            if(stk->ip + 8 <= code.size())
            {
                double dbl = BitConverter::ToDoubleBE(code[stk->ip]);
                stk->Push(gc,dbl);
                stk->ip = stk->ip + 8;
            }
        }
        return false;
    }
    bool InterperterThread::Return(GC* gc)
    {


       std::vector<CallStackEntry*>& cse=this->call_stack_entries;

            auto stk = cse.back();
             stk->ip = (uint32_t)stk->callable->closure->code.size();
            return false;
    }
    bool InterperterThread::ScopeBegin(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;

        auto stk = cse.back();

        gc->BarrierBegin();
        GCList ls(gc);
        stk->env = stk->env->GetSubEnvironment(ls);
        stk->scopes++;
        gc->BarrierEnd();
        
        return false;
    }
     bool InterperterThread::Defer(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;

        auto stk = cse.back();

                        gc->BarrierBegin();
                        GCList ls(gc);
                        auto item = stk->Pop(ls);
                        TCallable* call;
                        if(GetObjectHeap(item,call))
                        cse.back()->env->defers.insert(cse.back()->env->defers.begin(), {call});
                        gc->BarrierEnd();
        
        return false;
    }
     bool InterperterThread::Dup(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;

        auto stk = cse.back();
        GCList ls(gc);
        auto res = stk->Pop(ls);
        stk->Push(gc,res);
        stk->Push(gc,res);

        return false;
    }
    bool InterperterThread::ScopeEndTimes(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;

        auto stk = cse.back();

        gc->BarrierBegin();
                        GCList ls(gc);
                        std::vector<TCallable*> callable;
                        std::vector<uint8_t>& code = stk->callable->closure->code;
                        if(stk->ip + 4 <= code.size())
                        {
                            uint32_t n=BitConverter::ToUint32BE(code[stk->ip]);
                            stk->ip += 4;
                            for(uint32_t i = 0; i < n;i++)
                            {
                                if(!stk->env->defers.empty())
                                {
                                    ls.Add(stk->env);
                                    callable.insert(callable.end(), stk->env->defers.begin(),stk->env->defers.end());
                                }
                                stk->scopes--;
                                stk->env = stk->env->GetParentEnvironment();
                            }
                        }
                        
                        gc->BarrierEnd();
                        for(auto item : callable) 
                        {
                            GCList ls2(gc);
                            item->Call(ls2,{});
                        }
        
        return false;
    }
     bool InterperterThread::ScopeEnd(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;

        auto stk = cse.back();

        
                        gc->BarrierBegin();
                        GCList ls(gc);
                        std::vector<TCallable*> callable;
                    
                        if(!stk->env->defers.empty())
                        {
                            ls.Add(stk->env);
                            callable.insert(callable.end(), stk->env->defers.begin(),stk->env->defers.end());
                        }
                            
                        stk->scopes--;
                        stk->env = stk->env->GetParentEnvironment();
                        gc->BarrierEnd();


                        for(auto item : callable) 
                        {
                            GCList ls2(gc);
                            item->Call(ls2,{});
                        }
        
        return false;
    }
    bool InterperterThread::PushRelativePath(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;

        auto stk = cse.back();
        auto p = Framework::Filesystem::VFSPath();
        p.relative=true;
        p.path={};
        stk->Push(gc, p);
        return false;
    }
    bool InterperterThread::PushRootPath(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;

        auto stk = cse.back();
        auto p = Framework::Filesystem::VFSPath();
        p.relative=false;
        p.path={};
        stk->Push(gc, p);
        return false;
    }
    bool InterperterThread::Illegal(GC* gc)
    {


       std::vector<CallStackEntry*>& cse=this->call_stack_entries;

            auto stk = cse.back();
        char chr[3];
        snprintf(chr,3,"%02X",stk->callable->closure->code[stk->ip-1]);
        throw VMException("Illegal instruction: 0x" + std::string(chr)  + ".");
                    
    }

    void InterperterThread::Execute(GC* gc)
    {

       std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        #define VM_OPCODE_TABLE_INLINE
       #include "vm_opcode_table.h"
       #undef VM_OPCODE_TABLE_INLINE
    
        execute:

        if(!cse.empty())
        {
            auto stk = cse.back();
            current_function = stk;

            

            try{
            while(stk->ip < 0xFFFFFFFF && stk->ip < stk->callable->closure->code.size())
            {
               
                uint32_t ip = stk->ip;
                stk->ip = ip + 1;

                if(((*this).*(opcodes[stk->callable->closure->code[ip]]))(gc))
                    goto execute;

                if(stk->mustReturn) {

                    stk->mustReturn=false;
                    if(cse.size() > 1)
                    {
                        GCList ls(gc);
                         TObject o = cse[cse.size()-1]->Pop(ls);
                 cse[cse.size()-2]->Push(gc,o);
            
                cse.erase(cse.end()-1);
                current_function = cse.back();
                gc->BarrierEnd();
                goto execute;
                    } else {
                        return;
                    }
                }
               

                if(gc->UsingNullThreads()) gc->Collect();
                
            }

            stk->mustReturn=false;
            }
            catch(...)
            {
                
                {
                    gc->BarrierBegin();
                    GCList ls(gc);
                    std::vector<TCallable*> callable;
                    while(!cse.empty())
                    {
                        auto r= cse.back();
                        auto e = r->env;
                        for(uint32_t i = 0; i < r->scopes; i++)
                        {
                            if(!e->defers.empty())
                            {
                                ls.Add(e);
                                callable.insert(callable.end(), e->defers.begin(),e->defers.end());
                            }
                            e = e->GetParentEnvironment();
                        }
                        cse.erase(cse.end()-1);
                    }
                    gc->BarrierEnd();   

                    for(auto item : callable) 
                    {
                        GCList ls2(gc);
                        item->Call(ls2,{});
                    }
                }
                std::rethrow_exception(std::current_exception());
            }
            if(cse.size()==1)
            {
                  
               
                current_function=nullptr;
                {
             
                gc->BarrierBegin();
                
                
                
                GCList ls(gc);  


                    
                    std::vector<TCallable*> callable;
                    
                        auto r= cse.back();
                        auto e = r->env;
                        for(uint32_t i = 0; i < r->scopes; i++)
                        {
                            if(!e->defers.empty())
                            {
                                ls.Add(e);
                                callable.insert(callable.end(), e->defers.begin(),e->defers.end());
                            }
                            e = e->GetParentEnvironment();
                        }
                    
                gc->BarrierEnd();

                for(auto item : callable) 
                {
                    GCList ls2(gc);
                    item->Call(ls2,{});
                }
                }
 
                return;
            }
            else
            {
                
                {
                gc->BarrierBegin();
                
                GCList ls(gc);  


                    
                    std::vector<TCallable*> callable;
                   
                        auto r= cse.back();
                        auto e = r->env;
                        for(uint32_t i = 0; i < r->scopes; i++)
                        {
                            if(!e->defers.empty())
                            {
                                ls.Add(e);
                                callable.insert(callable.end(), e->defers.begin(),e->defers.end());
                            }
                            e = e->GetParentEnvironment();
                        }
                    


                 TObject o = cse[cse.size()-1]->Pop(ls);
                 cse[cse.size()-2]->Push(gc,o);
            
                cse.erase(cse.end()-1);
                current_function = cse.back();
                gc->BarrierEnd();

                for(auto item : callable) 
                {
                    GCList ls2(gc);
                    item->Call(ls2,{});
                }

                }

                goto execute;
            }
        }

    }


    void CallStackEntry::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        this->env->Mark();
        this->callable->Mark();
        for(auto item : this->stack) GC::Mark(item);
    }
    void CallStackEntry::Push(GC* gc,TObject o)
    {
        gc->BarrierBegin();
        this->stack.push_back(o);
        gc->BarrierEnd();
    }
    TObject CallStackEntry::Resume(GCList& ls)
    {
        auto cse = current_function;
        InterperterThread* thrd=InterperterThread::Create(ls);
        ls.GetGC()->BarrierBegin();
        thrd->call_stack_entries.push_back(this);
        ls.GetGC()->BarrierEnd();

        thrd->Execute(ls.GetGC());

        TObject v= thrd->call_stack_entries[0]->Pop(ls);
        current_function = cse;
        return v;
    }
    TObject CallStackEntry::Pop(GCList& gc)
    {
        if(this->stack.empty()) return Undefined();
        gc.GetGC()->BarrierBegin();
        TObject o = this->stack[this->stack.size()-1];
        gc.Add(o);
        this->stack.erase(this->stack.begin()+this->stack.size()-1);
        gc.GetGC()->BarrierEnd();
        return o;
    }

    InterperterThread* InterperterThread::Create(GCList& ls)
    {
        InterperterThread* it = new InterperterThread();
        GC* _gc = ls.GetGC();
        ls.Add(it);
        _gc->Watch(it);
        return it;
    }
    InterperterThread* InterperterThread::Create(GCList* ls)
    {
        InterperterThread* it = new InterperterThread();
        GC* _gc = ls->GetGC();
        ls->Add(it);
        _gc->Watch(it);
        return it;
    }
    CallStackEntry* CallStackEntry::Create(GCList& ls)
    {
        CallStackEntry* cse = new CallStackEntry();
        cse->mustReturn=false;
        cse->srcline = -1;
        cse->srcfile = "";
        cse->thread=nullptr;
        GC* _gc = ls.GetGC();
        ls.Add(cse);
        _gc->Watch(cse);
        return cse;
    }

    CallStackEntry* CallStackEntry::Create(GCList* ls)
    {
        CallStackEntry* cse = new CallStackEntry();
        cse->mustReturn=false;
        cse->srcline = -1;
        cse->srcfile = "";
        cse->thread=nullptr;
        GC* _gc = ls->GetGC();
        ls->Add(cse);
        _gc->Watch(cse);
        return cse;
    }
    void InterperterThread::AddCallStackEntry(GCList& ls, TClosure* closure, std::vector<TObject> args)
    {
        ls.GetGC()->BarrierBegin();
        CallStackEntry* cse = CallStackEntry::Create(ls);
        cse->thread = this;
        cse->callable = closure;
        cse->env = closure->chunkId == 0 ? closure->env : closure->ownScope ? closure->env->GetSubEnvironment(ls) : closure->env;
        cse->ip = 0;
        if(closure->closure->args.empty() && closure->chunkId != 0)
        {
            TList* list = TList::Create(ls);
            list->items = args;
            cse->env->DeclareVariable("arguments", list);
            
        }
        else 
        {
            auto requiredArguments = [closure]()->size_t
            {
                for(size_t i =0;i<closure->closure->args.size();i++)
                {
                    if(closure->closure->args[i].find("$") == 0)
                    {
                        return i;
                    }
                }
                return closure->closure->args.size();
            };
            auto optionalArguments = [closure](size_t argLen)->size_t 
            {
                for(size_t i =0;i<closure->closure->args.size();i++)
                {
                    if(closure->closure->args[i].find("$$") == 0)
                    {
                        
                        return std::min(argLen,i);
                       
                    }
                }
                
                return std::min(argLen,closure->closure->args.size());
            };
            auto trimStart = [](std::string txt)->std::string {
                if(txt.empty()) return {};
                if(txt[0] != '$') return txt;
                auto idx = txt.find_first_not_of('$');
                if(idx == std::string::npos) return {};
                return txt.substr(idx);
            };
            size_t required = requiredArguments();

            if(args.size() < required)
            {
                throw VMException("Called a function that expected at least " + std::to_string(required) + " args but got " + std::to_string(args.size()));
            }

            size_t i;
            for( i = 0; i < optionalArguments(args.size()); i++)
            {
                cse->env->DeclareVariable(trimStart(closure->closure->args[i]), args[i]);
            }
            std::string back = closure->closure->args.empty() ? std::string() : closure->closure->args.back();
            if(i == closure->closure->args.size()-1 && back.size() > 2 && back[0] == '$' && back[1] == '$')
            {
                auto argName = closure->closure->args[i];
                auto lsArgs = TList::Create(ls);
                for(;i<args.size(); i++)
                    lsArgs->Add(args[i]);
                cse->env->DeclareVariable(trimStart(argName), lsArgs);
                i = args.size();
            }
            if(i<args.size())
                throw VMException("Too many arguments");
        }
        
        current_function = cse;

        this->call_stack_entries.push_back(cse);
        ls.GetGC()->BarrierEnd();
    }

   
}
