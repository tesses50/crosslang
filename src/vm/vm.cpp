#include "CrossLang.hpp"
#include <TessesFramework/Filesystem/VFS.hpp>
#include <cstddef>
#include <exception>
#include <iostream>
#include <cmath>
#include <cstring>
#include <sstream>
#include <variant>
namespace Tesses::CrossLang {
    
    thread_local CallStackEntry* current_function=nullptr;

    bool ToBool(TObject obj)
    {
        if(std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(obj))
        {
            return true;
        }
        if(std::holds_alternative<TVMVersion>(obj))
        {
            auto v = std::get<TVMVersion>(obj);
            return v.AsLong() != 0;
        }
        if(std::holds_alternative<std::string>(obj))
        {
            return !std::get<std::string>(obj).empty();
        }
        else if(std::holds_alternative<int64_t>(obj))
        {
            return std::get<int64_t>(obj) != 0;
        }
        else if(std::holds_alternative<double>(obj))
        {
            return std::get<double>(obj);
        }
        else if(std::holds_alternative<bool>(obj))
        {
            return std::get<bool>(obj);
        }
        else if(std::holds_alternative<char>(obj))
        {
            return std::get<char>(obj) != 0;
        }
        else if(std::holds_alternative<TDateTime>(obj))
        {
            auto& dt = std::get<TDateTime>(obj).GetDate();
            return !(dt.Year() == 1970 && dt.Month() == 1 && dt.Day() == 1 && dt.Hour() == 0 && dt.Minute() == 0 && dt.Second() == 0 && !dt.IsLocal());
           
        }
        else if(std::holds_alternative<THeapObjectHolder>(obj))
        {
            auto o = std::get<THeapObjectHolder>(obj).obj;
            auto ls = dynamic_cast<TList*>(o);
            auto aarray = dynamic_cast<TAssociativeArray*>(o);
            auto dict = dynamic_cast<TDictionary*>(o);
            auto ba = dynamic_cast<TByteArray*>(o);
            auto nat = dynamic_cast<TNative*>(o);
            auto thrd = dynamic_cast<ThreadHandle*>(o);
            auto dt = dynamic_cast<TDateTime*>(o);
            auto natObj = dynamic_cast<TNativeObject*>(o);

            auto any = dynamic_cast<TAny*>(o);
            auto cls = dynamic_cast<TClassObject*>(o);
            if(cls!=nullptr) return true;
            if(natObj != nullptr) return natObj->ToBool();
            if(any != nullptr) return any->any.has_value();
           
            if(ls != nullptr)
            {
                return ls->Count() != 0;
            }
            if(aarray != nullptr)
            {
                return aarray->Count() != 0;
            }
            else if(dict != nullptr)
            {
                return !dict->items.empty();
            }
            else if(ba != nullptr)
            {
                return !ba->data.empty();
            }
            else if(nat != nullptr)
            {
                return !nat->GetDestroyed();
            }
            else if(thrd != nullptr)
            {
                return !thrd->hasReturned;
            }
            return true;
        }
        
        return false;
    }

    bool Equals(GC* gc, TObject left, TObject right)
    {
        GCList ls(gc);
        if(std::holds_alternative<std::nullptr_t>(left) && std::holds_alternative<std::nullptr_t>(right))
        {

            return true;
        }

        else if(std::holds_alternative<Undefined>(left) && std::holds_alternative<Undefined>(right))
        {
            return true;
        }

        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            return std::get<int64_t>(left) == std::get<int64_t>(right);
        }

        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            return std::get<double>(left) == std::get<double>(right);
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            return std::get<double>(left) == std::get<int64_t>(right);
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            return std::get<int64_t>(left) == std::get<double>(right);
        }

        else if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
        {
            return std::get<std::string>(left) == std::get<std::string>(right);
        }

        else if(std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right))
        {
            return std::get<bool>(left) == std::get<bool>(right);
        }

        else if(std::holds_alternative<char>(left) && std::holds_alternative<char>(right))
        {
            return std::get<char>(left) == std::get<char>(right);
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<int64_t>(right))
        {
            return std::get<char>(left) == std::get<int64_t>(right);
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<char>(right))
        {
            return std::get<int64_t>(left) == std::get<char>(right);
        }
        else if(std::holds_alternative<TDateTime>(left) && std::holds_alternative<TDateTime>(right))
        {
            return std::get<TDateTime>(left).GetDate().ToEpoch() == std::get<TDateTime>(right).GetDate().ToEpoch();
        }
        else if(std::holds_alternative<TVMVersion>(left) && std::holds_alternative<TVMVersion>(right))
        {
            auto lver= std::get<TVMVersion>(left);
            auto rver = std::get<TVMVersion>(right);
            auto r = lver.CompareTo(rver);
            return r == 0;
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
                auto obj=cls->GetValue("","operator==");
                gc->BarrierEnd();
                TCallable* callable;
                
                if(GetObjectHeap(obj,callable))
                {
                    return ToBool(callable->Call(ls,{}));
                   
                }
                else if(std::holds_alternative<std::nullptr_t>(right)) {
                    return false;
                }
                else if(std::holds_alternative<Undefined>(right)) 
                {
                    return false;
                }
                else if(std::holds_alternative<THeapObjectHolder>(right))
                {
                    return cls == std::get<THeapObjectHolder>(right).obj;
                }
            }
             else 
            if(natObj != nullptr)
            {
                return natObj->Equals(gc, right);
            }
            if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator==");
                gc->BarrierEnd();
                if(!std::holds_alternative<Undefined>(fn))
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
                                    return ToBool(obj->Call(ls,{left,right}));
                                }
                                else
                                {
                                    return ToBool(obj->Call(ls,{right}));
                                }

                            }
                            else
                            {
                                return ToBool(obj->Call(ls,{right}));

                            }
                        }

                    }

                }
                return false;

            }

            else if(dynDict != nullptr)
            {
                auto res = dynDict->CallMethod(ls,"operator==",{right});
                if(!std::holds_alternative<std::nullptr_t>(res) && std::holds_alternative<Undefined>(res))
                {
                    return ToBool(res);
                }
            }
            else if(native != nullptr && std::holds_alternative<std::nullptr_t>(right))
            {
                return native->GetDestroyed();

            }

            if(std::holds_alternative<THeapObjectHolder>(right))
            {
                return obj == std::get<THeapObjectHolder>(right).obj;

            }

            else if(std::holds_alternative<std::nullptr_t>(right))
            {
                return false;
            }
            else if(std::holds_alternative<Undefined>(right))
            {

                return false;
            }
            else
            {
                return false;
            }

        }

        else if(std::holds_alternative<std::nullptr_t>(right))
        {
            return false;
        }
        else if(std::holds_alternative<Undefined>(right))
        {
            return false;
        }
        else
        {
            return false;

        }

        return false;

    }

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


    


    

    bool InterperterThread::InvokeTwo(GCList& ls, TObject fn, TObject left, TObject right)
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
                        this->AddCallStackEntry(ls,closure,{left,right});
                    }
                    else
                    {
                        this->AddCallStackEntry(ls,closure,{right});
                    }
                    
                }
                else
                {
                    this->call_stack_entries[this->call_stack_entries.size()-1]->Push(ls.GetGC(), obj->Call(ls,{right}));
                   
                    return false;
                }
                return true;
            }
            
        }
        return false;
    }
    bool InterperterThread::InvokeMethod(GCList& ls, TObject fn, TObject instance, std::vector<TObject> args)
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
                        std::vector<TObject> args2;
                        args2.push_back(instance);
                        args2.insert(args2.end(), args.begin(),args.end());
                        this->AddCallStackEntry(ls,closure,args2);
                    }
                    else
                    {
                        this->AddCallStackEntry(ls,closure,args);
                    }
                    
                }
                else
                {
                    auto val = obj->Call(ls,args);
                    this->call_stack_entries.back()->Push(ls.GetGC(), val);    
                    return false;
                }
                return true;
            }
            
        }
        this->call_stack_entries.back()->Push(ls.GetGC(),Undefined());
        return false;
    }
    bool InterperterThread::InterperterThread::Breakpoint(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto res = cse.back()->Pop(ls);

        auto env = cse.back()->env;
        if(!env->GetRootEnvironment()->HandleBreakpoint(gc, env, res))
        {
            throw std::exception();
        }

        return false;
    }
    bool InterperterThread::Sub(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        GCList ls(gc);
        auto right = cse.back()->Pop(ls);
        auto left = cse.back()->Pop(ls);

        if(std::holds_alternative<int64_t>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) - std::get<int64_t>(right));
        }
        
        else if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) - std::get<double>(right));
        }
        else if(std::holds_alternative<double>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc,std::get<double>(left) - std::get<int64_t>(right));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<double>(right))
        {
            cse.back()->Push(gc,std::get<int64_t>(left) - std::get<double>(right));
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc, (int64_t)(std::get<char>(left) - std::get<char>(right)));
        }
        else if(std::holds_alternative<char>(left) && std::holds_alternative<int64_t>(right))
        {
            cse.back()->Push(gc, (int64_t)(std::get<char>(left) - std::get<int64_t>(right)));
        }
        else if(std::holds_alternative<int64_t>(left) && std::holds_alternative<char>(right))
        {
            cse.back()->Push(gc, (int64_t)(std::get<int64_t>(left) - std::get<char>(right)));
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
                cse.back()->Push(gc, natObj->CallMethod(ls,"operator-",{right}));
                return false;
            }
            else if(dict != nullptr)
            {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("operator-");
                gc->BarrierEnd();
                return InvokeTwo(ls,fn,left,right);
            }
            else if(dynDict != nullptr)
            {
                cse.back()->Push(gc,dynDict->CallMethod(ls,"operator-",{right}));
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
        else if(std::holds_alternative<TDateTime>(left) && std::holds_alternative<TDateTime>(right))
        {
            cse.back()->Push(gc, std::get<TDateTime>(left).GetDate().ToEpoch() < std::get<TDateTime>(right).GetDate().ToEpoch());
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
        else if(std::holds_alternative<TDateTime>(left) && std::holds_alternative<TDateTime>(right))
        {
            cse.back()->Push(gc, std::get<TDateTime>(left).GetDate().ToEpoch() > std::get<TDateTime>(right).GetDate().ToEpoch());
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
        else if(std::holds_alternative<TDateTime>(left) && std::holds_alternative<TDateTime>(right))
        {
            cse.back()->Push(gc, std::get<TDateTime>(left).GetDate().ToEpoch() <= std::get<TDateTime>(right).GetDate().ToEpoch());
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
        else if(std::holds_alternative<TDateTime>(left) && std::holds_alternative<TDateTime>(right))
        {
            cse.back()->Push(gc, std::get<TDateTime>(left).GetDate().ToEpoch() >= std::get<TDateTime>(right).GetDate().ToEpoch());
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
        else if(std::holds_alternative<TDateTime>(left) && std::holds_alternative<TDateTime>(right))
        {
            cse.back()->Push(gc, std::get<TDateTime>(left).GetDate().ToEpoch() == std::get<TDateTime>(right).GetDate().ToEpoch());
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
        else if(std::holds_alternative<TDateTime>(left) && std::holds_alternative<TDateTime>(right))
        {
            cse.back()->Push(gc, std::get<TDateTime>(left).GetDate().ToEpoch() != std::get<TDateTime>(right).GetDate().ToEpoch());
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
    bool InterperterThread::Add(GC* gc)
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
     bool InterperterThread::ExecuteMethod2(GC* gc, TObject instance, std::string key, std::vector<TObject> args)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;
        if(!cse.empty())
        {
            GCList ls(gc);
            std::regex regex;
            TVMVersion version;
            if(GetObject(instance, version))
            {
                if(key == "ToString")
                {
                    cse.back()->Push(gc, version.ToString());
                    return false;
                }
                if(key == "ToByteArray")
                {
                    TByteArray* ba = TByteArray::Create(ls);
                    ba->data.resize(5);
                    version.ToArray(ba->data.data());
                    cse.back()->Push(gc,ba);
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
            else if(GetObject(instance,regex))
            {
                if(key == "Search")
                {
                    std::string str;
                    if(GetArgument(args,0,str))
                    {
                        std::smatch m;
                        if(std::regex_search(str,m,regex))
                        {
                            auto myLs = TList::Create(ls);
                            gc->BarrierBegin();
                            for(auto item : m)
                            {
                                auto itm = TDictionary::Create(ls);
                                itm->SetValue("Offset", (int64_t)(item.first-str.begin()));
                                itm->SetValue("Length",(int64_t)item.length());
                                itm->SetValue("Matched",item.matched);
                                itm->SetValue("Text",item.str());
                                myLs->Add(itm);
                            }


                            cse.back()->Push(gc, myLs);
                            gc->BarrierEnd();
                            return false;
                        }
                        
                    }
                }

                cse.back()->Push(gc, nullptr);
                return false;
            }
            else if(std::holds_alternative<bool>(instance))
            {
                bool flag = std::get<bool>(instance);
                if(key == "ToString")
                {
                    cse.back()->Push(gc, flag ? "true" : "false");
                    return false;
                }
                else if(key == "ToLong")
                {
                    cse.back()->Push(gc, flag ? (int64_t)1 : (int64_t)0);
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
            else if(std::holds_alternative<char>(instance))
            {
                char c = std::get<char>(instance);
                if(key == "ToString")
                {
                    cse.back()->Push(gc, std::string{c});
                    return false;
                }
                else if(key == "ToLong")
                {
                    uint8_t uc = (uint8_t)c;
                    cse.back()->Push(gc, (int64_t)uc);
                    return false;
                }
                else if(key == "IsAscii")
                {
                    bool isAscii = c >= 0;
                    cse.back()->Push(gc, isAscii);
                    return false;
                }
                if(key == "IsLetter")
                {
                    bool isLetter = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
                    cse.back()->Push(gc, isLetter);
                    return false;
                }
                if(key == "IsDigit")
                {
                    bool isDigit = (c >= '0' && c <= '9');
                    cse.back()->Push(gc, isDigit);
                    return false;
                }
                if(key == "IsLetterOrDigit")
                {
                    bool isDigit = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
                    cse.back()->Push(gc, isDigit);
                    return false;
                }
                if(key == "ToLower")
                {
                    if(c >= 'A' && c <= 'Z')
                        cse.back()->Push(gc, (char)tolower(c));
                    else
                        cse.back()->Push(gc, c);

                    return false;
                }
                if(key == "ToUpper")
                {
                    if(c >= 'a' && c <= 'z')
                        cse.back()->Push(gc, (char)toupper(c));
                    else
                        cse.back()->Push(gc, c);
                        
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
            else if(std::holds_alternative<double>(instance))
            {
                double number = std::get<double>(instance);
                if(key == "ToString")
                {
                    cse.back()->Push(gc, std::to_string(number));   
                    return false;
                }
                if(key == "ToLong")
                {
                    cse.back()->Push(gc, (int64_t)number);
                    return false;
                }
                if(key == "ToLongBits")
                {
                    cse.back()->Push(gc, *(int64_t*)&number);
                    return false;
                }
                if(key == "Floor")
                {
                    cse.back()->Push(gc,floor(number));
                    return false;
                }
                if(key == "Ceiling")
                {
                    cse.back()->Push(gc,ceil(number));
                    return false;
                }
                if(key == "Abs")
                {
                    cse.back()->Push(gc,fabs(number));
                    return false;
                }
                if(key == "Pow")
                {
                    if(args.size() != 1)
                    {
                        throw VMException("Double.Pow must only accept one argument");
                    }
                    if(!std::holds_alternative<double>(args[0])) 
                    {
                        throw VMException("Double.Pow must only accept a double");
                    } 
                    cse.back()->Push(gc,pow(number, std::get<double>(args[0])));
                    return false;
                }
                if(key == "Atan2")
                {
                    if(args.size() != 1)
                    {
                        throw VMException("Double.Atan2 must only accept one argument");
                    }
                    if(!std::holds_alternative<double>(args[0])) 
                    {
                        throw VMException("Double.Atan2 must only accept a double");
                    } 
                    cse.back()->Push(gc,atan2(number, std::get<double>(args[0])));
                    return false;
                }
                if(key == "Atan")
                {
                    cse.back()->Push(gc,tan(number));
                    return false;
                }
                if(key == "Tan")
                {
                    cse.back()->Push(gc,tan(number));
                    return false;
                }
                if(key == "Tanh")
                {
                    cse.back()->Push(gc,tanh(number));
                    return false;
                }
                if(key == "Asin")
                {
                    cse.back()->Push(gc,asin(number));
                    return false;
                }
                if(key == "Sin")
                {
                    cse.back()->Push(gc,sin(number));
                    return false;
                }
                if(key == "Sinh")
                {
                    cse.back()->Push(gc,sinh(number));
                    return false;
                }
                if(key == "Acos")
                {
                    cse.back()->Push(gc,acos(number));
                    return false;
                }
                if(key == "Cos")
                {
                    cse.back()->Push(gc,cos(number));
                    return false;
                }
                if(key == "Cosh")
                {
                    cse.back()->Push(gc,cosh(number));
                    return false;
                }
                if(key == "Log")
                {
                    cse.back()->Push(gc,log(number));
                    return false;
                }
                if(key == "Log10")
                {
                    cse.back()->Push(gc,log10(number));
                    return false;
                }
                if(key == "Sqrt")
                {
                    cse.back()->Push(gc,sqrt(number));
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
            else if(std::holds_alternative<int64_t>(instance))
            {
                int64_t number = std::get<int64_t>(instance);
                if(key == "ToHexString")
                {
                    int64_t width = 1;

                    if(!GetArgument(args,0,width)) width=1;
                    size_t _width = (size_t)width;
                    
                    std::stringstream strm;
                    if(_width > 1)
                    strm << std::setfill('0') << std::setw((int)_width) << std::hex << number;
                    else
                    strm << std::hex << number;
                    cse.back()->Push(gc,strm.str());
                    return false;
                }
                if(key == "ToString")
                {
                    cse.back()->Push(gc, std::to_string(number));   
                    return false;
                }
                if(key == "ToChar")
                {
                    uint8_t c = (uint8_t)number;
                    cse.back()->Push(gc, (char)c); 
                    return false;
                }
                if(key == "Abs")
                {
                    if(number < 0)
                        cse.back()->Push(gc, -number);
                    else
                        cse.back()->Push(gc, number);
                    return false;
                }
                if(key == "ToDouble")
                {
                    cse.back()->Push(gc,(double)number);
                    return false;
                }
                if(key == "ToDoubleBits")
                {
                    cse.back()->Push(gc,*(double*)&number);
                    return false;
                }
                
                cse.back()->Push(gc, Undefined());
                return false;
            }   
            else if(std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(instance))
            {
                auto path = std::get<Tesses::Framework::Filesystem::VFSPath>(instance);
                if(key == "GetEnumerator")
                {

                    TList* _ls = TList::Create(ls);
                    for(auto item : path.path)
                    {
                        _ls->Add(item);
                    }

                    cse.back()->Push(gc, TListEnumerator::Create(ls,_ls));
                    return false;
                }
                if(key == "ToString")
                {
                    cse.back()->Push(gc, path.ToString());
                    return false;
                }
                if(key == "GetAt")
                {
                    int64_t index;
                    if(GetArgument(args,0,index))
                    {
                        size_t idx = (size_t)idx;
                        if(idx < path.path.size())
                            cse.back()->Push(gc, path.path[idx]);
                        else
                            cse.back()->Push(gc, nullptr);
                        return false;
                    }
                }
                if(key == "Count" || key == "Length")
                {
                    cse.back()->Push(gc, (int64_t)path.path.size());
                    return false;
                }
                if(key == "GetParent")
                {
                    cse.back()->Push(gc, path.GetParent());
                    return false;
                }
                if(key == "GetFileName")
                {
                    cse.back()->Push(gc, path.GetFileName());
                    return false;
                }
                if(key == "GetExtension")
                {
                    cse.back()->Push(gc, path.GetExtension());
                    return false;
                }
                if(cse.back()->env->GetRootEnvironment()->permissions.canRegisterLocalFS && key == "MakeAbsolute")
                {
                    Tesses::Framework::Filesystem::VFSPath p;
                    if(GetArgumentAsPath(args,0,p))
                    {
                        cse.back()->Push(gc,path.MakeAbsolute(p));
                        return false;
                    }
                    else
                    {
                        cse.back()->Push(gc,path.MakeAbsolute());
                        return false;
                    }
                }
                if(cse.back()->env->GetRootEnvironment()->permissions.canRegisterLocalFS && key == "MakeRelative")
                {
                    Tesses::Framework::Filesystem::VFSPath p;
                    if(GetArgumentAsPath(args,0,p))
                    {
                        cse.back()->Push(gc,path.MakeRelative(p));
                        return false;
                    }
                    else
                    {
                        cse.back()->Push(gc,path.MakeRelative());
                        return false;
                    }
                }
                if(key == "ChangeExtension")
                {
                    Tesses::Framework::Filesystem::VFSPath newPath = path;
                    std::string ext;
                    if(GetArgument(args,0, ext))
                    {
                        newPath.ChangeExtension(ext);
                    }   
                    else
                    {
                        newPath.RemoveExtension();
                    }
                    cse.back()->Push(gc, newPath);
                    return false;
                }
                if(key == "CollapseRelativeParents")
                {
                    cse.back()->Push(gc, path.CollapseRelativeParents());
                    return false;
                }
                if(key == "IsRelative")
                {
                    cse.back()->Push(gc, path.relative);
                    return false;
                    //Path.FromString();
                }
            }
            else if(std::holds_alternative<std::string>(instance))
            {
                std::string str = std::get<std::string>(instance);
                if(key == "IndexOf")
                {   
                    std::string str2;
                     char c;
                    if(GetArgument(args,0,str2))
                    {

                        int64_t index = str.size();
                        GetArgument(args,1,index);
                        
                        auto res = str.find(str2,(std::size_t)index);
                        if(res == std::string::npos)
                            cse.back()->Push(gc, (int64_t)-1);
                        else
                            cse.back()->Push(gc, (int64_t)res);
                        return false;
                    }
                    
                    else if(GetArgument<char>(args,0,c))
                    {
                        int64_t index = 0;
                        GetArgument(args,1,index);
                        
                        auto res = str.find_first_of(c,(std::size_t)index);
                        if(res == std::string::npos)
                            cse.back()->Push(gc, (int64_t)-1);
                        else
                            cse.back()->Push(gc, (int64_t)res);
                        return false;
                    }
                    cse.back()->Push(gc,nullptr);
                    return false;
                }
                if(key == "LastIndexOf")
                {
                    std::string str2;
                     char c;
                    if(GetArgument(args,0,str2))
                    {

                        int64_t index = str.size();
                        GetArgument(args,1,index);
                        
                        auto res = str.rfind(str2,(std::size_t)index);
                        if(res == std::string::npos)
                            cse.back()->Push(gc, (int64_t)-1);
                        else
                            cse.back()->Push(gc, (int64_t)res);
                        return false;
                    }
                    else if(GetArgument<char>(args,0,c))
                    {
                        int64_t index = str.size();
                        GetArgument(args,1,index);
                        
                        auto res = str.find_last_of(c,(std::size_t)index);
                        if(res == std::string::npos)
                            cse.back()->Push(gc, (int64_t)-1);
                        else
                            cse.back()->Push(gc, (int64_t)res);
                        return false;
                    }
                    cse.back()->Push(gc,nullptr);
                    return false;
                }
                if(key == "GetEnumerator")
                {
                    cse.back()->Push(gc, TStringEnumerator::Create(ls,str));
                    return false;
                }
                if(key == "Substring")
                {
                    if(args.size() >=1 && std::holds_alternative<int64_t>(args[0]))
                    {
                        size_t offset = (size_t)std::get<int64_t>(args[0]);
                        size_t count = std::string::npos;
                        if(args.size() == 2 && std::holds_alternative<int64_t>(args[1]))
                        {   
                            count = (size_t)std::get<int64_t>(args[1]);
                        }
                        cse.back()->Push(gc, str.substr(offset,count));
                        return false;
                    }
                   
                }
                if(key == "Remove")
                {
                    if(args.size() >=1 && std::holds_alternative<int64_t>(args[0]))
                    {
                        size_t offset = (size_t)std::get<int64_t>(args[0]);
                        size_t count = std::string::npos;
                        if(args.size() == 2 && std::holds_alternative<int64_t>(args[1]))
                        {   
                            count = (size_t)std::get<int64_t>(args[1]);
                        }
                        cse.back()->Push(gc, str.erase(offset,count));
                        return false;
                    }
                   
                }
                if(key == "TrimStart")
                {
                    if(args.size() >= 0)
                    {
                        char c = (args.size() == 1 && std::holds_alternative<char>(args[0])) ? std::get<char>(args[0]) : ' ';
                        
                        size_t i = 0;
                        for(; i < str.size();i++)
                        {
                            if(str[i] != c) break;
                        }
                         cse.back()->Push(gc,str.substr(i));
                         return false;
                    }
                }
                if(key == "TrimEnd")
                {
                    if(args.size() >= 0)
                    {
                        char c = (args.size() == 1 && std::holds_alternative<char>(args[0])) ? std::get<char>(args[0]) : ' ';
                        size_t i = str.size()-1;
                        for(; i >= 0;i--)
                        {
                            if(str[i] != c) break;
                        }
                         cse.back()->Push(gc,str.substr(0,i+1));
                         return false;
                    }
                }
                if(key == "EndsWith")
                {
                    std::string v;
                    if(GetArgument(args,0,v))   
                    {
                        if(str.size() < v.size()) {
                            cse.back()->Push(gc,false);
                            return false;
                        }
                       
                        size_t _end = str.size()-v.size();
                        for(size_t i = 0; i < v.size(); i++)
                        {
                            if(v[i] != str[i+_end])
                            {
                                cse.back()->Push(gc,false);
                                return false;
                            }
                        }
                        cse.back()->Push(gc,true);
                        return false;
                    }
                    cse.back()->Push(gc,false);
                    return false;
                }
                if(key == "StartsWith")
                {
                    std::string v;
                    if(GetArgument(args,0,v))   
                    {
                        if(str.size() < v.size()) {
                            cse.back()->Push(gc,false);
                            return false;
                        }
                        for(size_t i = 0; i < v.size(); i++)
                        {
                            if(v[i] != str[i])
                            {
                                cse.back()->Push(gc,false);
                                return false;
                            }
                        }
                        cse.back()->Push(gc,true);
                        return false;
                    }
                    cse.back()->Push(gc,false);
                    return false;
                }
                if(key == "Escape")
                {
                    bool quote;
                    if(!GetArgument(args,0,quote)) quote=false;
                    cse.back()->Push(gc, EscapeString(str, quote));
                    return false;
                }
                if(key == "Replace")
                {
                    std::string oldStr;
                    std::string newStr;

                    std::string _str={};
                    if(GetArgument(args,0,oldStr) && GetArgument(args,1,newStr))
                    {
                        _str = Tesses::Framework::Http::HttpUtils::Replace(str, oldStr,newStr);

                        
                    }
                    cse.back()->Push(gc,_str);

                    return  false;
                }
                if(key == "Split")
                {
                    std::string delimiter; 
                    bool removeEmpty=false;
                    size_t count=std::string::npos;

                    if(args.size() < 1 || args.size() > 3)  throw VMException("String.Split must only accept 1-3 arguments");
                    if(!std::holds_alternative<std::string>(args[0])) throw VMException("String.Split first arg must be a string");
                    else 
                        delimiter = std::get<std::string>(args[0]);
                   
                    if(args.size() > 1 && !std::holds_alternative<bool>(args[1])) throw VMException("String.Split second arg must be a bool");
                    else if(args.size() > 1)
                        removeEmpty = std::get<bool>(args[1]);
                    if(args.size() > 2 && !std::holds_alternative<int64_t>(args[2])) throw VMException("String.Split third arg must be a int64_t");
                    else if(args.size() > 2)
                        count = (size_t)std::get<int64_t>(args[2]);

                    auto res = Tesses::Framework::Http::HttpUtils::SplitString(str, delimiter,count);
                    TList* mls = TList::Create(ls);
                    for(auto item : res)
                    {
                        if(!removeEmpty || !item.empty()) mls->Add(item);
                    }
                    cse.back()->Push(gc,mls);
                    return false;
                   // SplitString()
                }
                if(key == "GetAt")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("String.GetAt must only accept one argument");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("String.GetAt must only accept a long");
                        }

                        size_t index = (size_t)std::get<int64_t>(args[0]);
                        size_t sz = str.size();
                        if(index >= 0 && index < sz)
                        {
                            cse.back()->Push(gc, str[index]);
                            return false;
                        }
                        
                    }
                if(key == "Count" || key == "Length")
                {
                    int64_t len = (int64_t)str.size();
                    if(len < 0) len = 0;

                    cse.back()->Push(gc, len);
                    return false;
                }
                if(key == "ToString")
                {
                    cse.back()->Push(gc, str);
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
            else if(std::holds_alternative<TDateTime>(instance))
            {
                auto& date = std::get<TDateTime>(instance).GetDate();
                
                
                if(key == "ToString")
                {
                    std::string fmt;
                    if(GetArgument(args,0,fmt))
                    {
                        cse.back()->Push(gc, date.ToString(fmt));
                    }
                    else
                    {
                        cse.back()->Push(gc, date.ToString());
                    }
                    return false;
                }
                if(key == "ToHttpDate")
                {
                    cse.back()->Push(gc, date.ToHttpDate());
                    return false;
                }
                if(key == "ToEpoch")
                {
                    cse.back()->Push(gc, date.ToEpoch());
                    return false;
                }
                if(key == "ToLocal")
                {
                    cse.back()->Push(gc, date.ToLocal());
                    return false;
                }
                if(key == "ToUTC")
                {
                    cse.back()->Push(gc, date.ToUTC());
                    return false;
                }
                cse.back()->Push(gc, nullptr);
                return false;
                
            }
            else if(std::holds_alternative<THeapObjectHolder>(instance))
            {
                auto obj = std::get<THeapObjectHolder>(instance).obj;
                auto list = dynamic_cast<TList*>(obj);
                auto dynList = dynamic_cast<TDynamicList*>(obj);
                auto bArray = dynamic_cast<TByteArray*>(obj);
                auto dict = dynamic_cast<TDictionary*>(obj);
                auto dynDict = dynamic_cast<TDynamicDictionary*>(obj);
                auto ittr = dynamic_cast<TEnumerator*>(obj);
                auto strm = dynamic_cast<TStreamHeapObject*>(obj);
                auto vfs = dynamic_cast<TVFSHeapObject*>(obj);
                auto env = dynamic_cast<TEnvironment*>(obj);

                auto subEnv = dynamic_cast<TSubEnvironment*>(obj);
                auto rootEnv = dynamic_cast<TRootEnvironment*>(obj);
                auto callable = dynamic_cast<TCallable*>(obj);
                auto callstackEntry = dynamic_cast<CallStackEntry*>(obj);
                
                
                auto svr = dynamic_cast<TServerHeapObject*>(obj);
                auto natObj = dynamic_cast<TNativeObject*>(obj);
                auto cls = dynamic_cast<TClassObject*>(obj);
                auto aArray=dynamic_cast<TAssociativeArray*>(obj);
                auto ttask = dynamic_cast<TTask*>(obj);

                if(ttask != nullptr)
                {
                    if(key == "ContinueWith")
                    {
                        TCallable* callable2;
                        if(GetArgumentHeap(args,0,callable2))
                        {
                            cse.back()->Push(gc,ttask->ContinueWith(ls,callable2));
                            return false;
                        }
                    }
                    if(key == "Wait")
                    {
                        
                        cse.back()->Push(gc,ttask->Wait());
                        return false;
                    }
                    cse.back()->Push(gc,Undefined());
                    return false;
                }

                if(natObj != nullptr)
                {
                    cse.back()->Push(gc, natObj->CallMethod(ls,key,args));
                    return false;
                }
                
                if(callstackEntry != nullptr)
                {
                    if(key == "Resume")
                    {
                        gc->BarrierBegin();
                        cse.push_back(callstackEntry);
                        gc->BarrierEnd();
                        return true;
                    }
                    if(key == "Push")
                    {
                        if(!args.empty())
                        callstackEntry->Push(gc, args[0]);
                        else
                        callstackEntry->Push(gc,Undefined());
                        return false;
                    }
                    if(key == "Pop")
                    {
                        cse.back()->Push(gc, callstackEntry->Pop(ls));
                        return false;
                    }
                }

                if(svr != nullptr)
                {
                    auto mountable = dynamic_cast<Tesses::Framework::Http::MountableServer*>(svr->server);
                    if(mountable != nullptr)
                    {
                        if(key == "Mount")
                        {
                            Tesses::Framework::Filesystem::VFSPath p;
                            
                            if(args.size() > 1 && GetArgumentAsPath(args,0,p))
                            {
                                mountable->Mount(p.ToString(),new TObjectHttpServer(gc,args[1]),true);
                                cse.back()->Push(gc,nullptr);
                                return false;
                            }
                        }
                        if(key == "Unmount")
                        {
                            Tesses::Framework::Filesystem::VFSPath p;
                            
                            if(GetArgumentAsPath(args,0,p))
                            {
                                mountable->Unmount(p.ToString());
                                cse.back()->Push(gc,nullptr);
                                return false;
                            }
                        }
                    }
                    if(key == "Handle")
                    {
                       
                        if(GetArgumentHeap(args,0,dict))
                        {
                            gc->BarrierBegin();
                            auto nat = dict->GetValue("native");
                            gc->BarrierEnd();
                            TNative* nat2;
                            if(GetObjectHeap(nat,nat2))
                            {
                                if(!nat2->GetDestroyed()) 
                                {
                                    auto ctx = static_cast<Tesses::Framework::Http::ServerContext*>(nat2->GetPointer());
                                    if(ctx != nullptr)
                                    {
                                        cse.back()->Push(gc,svr->server->Handle(*ctx));
                                        return false;
                                    }
                                    
                                }
                            }
                        }
                        cse.back()->Push(gc,false);
                        return false;
                    }
                    if(key == "Close")
                    {
                        svr->Close();
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    cse.back()->Push(gc,Undefined());
                    return false;
                }
                if(rootEnv != nullptr)
                {
                    //TStd::RegisterCrypto
                    //TStd::RegisterDictionary
                    //TStd::RegisterEnv
                    //TStd::RegisterIO
                    //TStd::RegisterJson
                    //TStd::RegisterNet
                    //TStd::RegisterOGC
                    //TStd::RegisterPath
                    //TStd::RegisterRoot
                    //TStd::RegisterSqlite
                    //TStd::RegisterVM
                    auto myEnv = cse.back()->env->GetRootEnvironment();

                    if(key == "RegisterOnError")
                    {
                        TCallable* callable;
                        if(!rootEnv->permissions.locked && GetArgumentHeap(args,0,callable))
                        {
                            gc->BarrierBegin();
                            rootEnv->RegisterOnError(callable);
                            gc->BarrierEnd();
                        }
                    }
                    if(key == "RegisterEverything")
                    {
                        if(myEnv->permissions.canRegisterEverything)
                        {
                            TStd::RegisterStd(gc, rootEnv);
                        } 
                        else
                        {
                        if(myEnv->permissions.canRegisterConsole && !rootEnv->permissions.locked)
                        TStd::RegisterConsole(gc, rootEnv);

                        if(myEnv->permissions.canRegisterCrypto && !rootEnv->permissions.locked)
                        TStd::RegisterCrypto(gc, rootEnv);

                        if(myEnv->permissions.canRegisterDictionary && !rootEnv->permissions.locked)
                        TStd::RegisterDictionary(gc, rootEnv);

                        if(myEnv->permissions.canRegisterEnv && !rootEnv->permissions.locked)
                            TStd::RegisterEnv(gc, rootEnv);


                        if(myEnv->permissions.canRegisterIO && !rootEnv->permissions.locked)
                            TStd::RegisterIO(gc, rootEnv, myEnv->permissions.canRegisterLocalFS);
                        
                        if(myEnv->permissions.canRegisterJSON && !rootEnv->permissions.locked)
                        TStd::RegisterJson(gc, rootEnv);

                         if(myEnv->permissions.canRegisterNet && !rootEnv->permissions.locked)
                        TStd::RegisterNet(gc, rootEnv);

                        if(myEnv->permissions.canRegisterOGC && !rootEnv->permissions.locked)
                        TStd::RegisterOGC(gc, rootEnv);

                        if(myEnv->permissions.canRegisterPath && !rootEnv->permissions.locked)
                        TStd::RegisterPath(gc, rootEnv);

                        if(myEnv->permissions.canRegisterRoot && !rootEnv->permissions.locked)
                        TStd::RegisterRoot(gc, rootEnv);


                        if(myEnv->permissions.canRegisterSqlite && !rootEnv->permissions.locked)
                        TStd::RegisterSqlite(gc, rootEnv);

                         if(myEnv->permissions.canRegisterVM && !rootEnv->permissions.locked)
                        TStd::RegisterVM(gc, rootEnv);
                        if(myEnv->permissions.canRegisterProcess && !rootEnv->permissions.locked)
                        TStd::RegisterProcess(gc, rootEnv);
                        }

                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterClass")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterClass) && !rootEnv->permissions.locked)
                        TStd::RegisterClass(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterSDL2")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterSDL2)&& !rootEnv->permissions.locked)
                        TStd::RegisterSDL2(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterConsole")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterConsole) && !rootEnv->permissions.locked)
                        TStd::RegisterConsole(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterProcess")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterProcess) && !rootEnv->permissions.locked)
                        TStd::RegisterProcess(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterCrypto")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterCrypto) && !rootEnv->permissions.locked)
                        TStd::RegisterCrypto(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterDictionary")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterDictionary) && !rootEnv->permissions.locked)
                        TStd::RegisterDictionary(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterEnv")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterEnv) && !rootEnv->permissions.locked)
                        TStd::RegisterDictionary(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterIO")
                    {
                        bool r;
                        if(GetArgument(args,0,r))
                        {
                            if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterIO) && !rootEnv->permissions.locked)
                                TStd::RegisterIO(gc, rootEnv, myEnv->permissions.canRegisterLocalFS ? r : false);
                        }
                        cse.back()->Push(gc,nullptr);
                        return false;
                    
                    }
                    if(key == "RegisterJson")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterJSON) && !rootEnv->permissions.locked)
                        TStd::RegisterJson(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterNet")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterNet) && !rootEnv->permissions.locked)
                        TStd::RegisterNet(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterOGC")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterOGC) && !rootEnv->permissions.locked)
                        TStd::RegisterOGC(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterPath")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterPath) && !rootEnv->permissions.locked)
                        TStd::RegisterPath(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterRoot")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterRoot) && !rootEnv->permissions.locked)
                        TStd::RegisterRoot(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }

                    if(key == "RegisterSqlite")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterSqlite) && !rootEnv->permissions.locked)
                        TStd::RegisterSqlite(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    
                    if(key == "SetSqliteRoot")
                    {
                        Tesses::Framework::Filesystem::VFSPath p;
                        if(!rootEnv->permissions.locked)
                        {
                        if(GetArgumentAsPath(args,0,p))
                        {
                            if(myEnv->permissions.sqlite3Scoped)
                            {
                               rootEnv->permissions.sqliteOffsetPath = myEnv->permissions.sqliteOffsetPath / p.CollapseRelativeParents();
                               rootEnv->permissions.sqlite3Scoped = true;
                            }
                            else
                            {
                               rootEnv->permissions.sqliteOffsetPath = p;
                               rootEnv->permissions.sqlite3Scoped = true;
                            }
                        }
                        else {
                            if(myEnv->permissions.sqlite3Scoped)
                            {
                               rootEnv->permissions.sqliteOffsetPath = myEnv->permissions.sqliteOffsetPath;
                               rootEnv->permissions.sqlite3Scoped = true;
                            }
                            else
                            {
                               rootEnv->permissions.sqlite3Scoped = false;
                            }
                        }
                        }
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "RegisterVM")
                    {
                        if((myEnv->permissions.canRegisterEverything || myEnv->permissions.canRegisterVM) && !rootEnv->permissions.locked)
                        TStd::RegisterVM(gc, rootEnv);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "LockRegister")
                    {
                        if(!rootEnv->permissions.locked)
                        {
                            rootEnv->permissions.locked=true;
                        }
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "GetDictionary")
                    {

                        cse.back()->Push(gc,rootEnv->GetDictionary());
                        return false;
                    }
                    if(key == "LoadFileWithDependencies")
                    {
                        TVFSHeapObject* vfs0;
                        TFile* f;
                        Tesses::Framework::Filesystem::VFSPath p;
                        if(GetArgumentHeap(args,0,vfs0) )
                        {
                            if(GetArgumentHeap(args,1,f))
                            {
                                rootEnv->LoadFileWithDependencies(gc,vfs0->vfs,f);
                            }
                            else if(GetArgumentAsPath(args,1,p))
                            {
                                rootEnv->LoadFileWithDependencies(gc,vfs0->vfs,p);
                            }
                        }
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                }
                if(subEnv != nullptr)
                {
                    if(key == "GetDictionary")
                    {
                        cse.back()->Push(gc,subEnv->GetDictionary());
                        return false;
                    }
                }
                
                if(env != nullptr)
                {
                    if(key ==  "Eval")
                    {
                        std::string str;
                        if(GetArgument(args,0,str))
                        {
            
                            
                            cse.back()->Push(gc,env->Eval(ls,str));
                            return false;
                        }
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "GetRootEnvironment")
                    {
                        cse.back()->Push(gc,env->GetRootEnvironment());
                        return false;
                    }

                    if(key == "GetSubEnvironment")
                    {
                        cse.back()->Push(gc,env->GetSubEnvironment(ls));
                        return false;
                    }
                    if(key == "GetParentEnvironment")
                    {
                        cse.back()->Push(gc,env->GetParentEnvironment());
                        return false;
                    }
                    if(key == "GetVariable")
                    {
                        std::string key;
                        gc->BarrierBegin();
                        if(GetArgument(args,0,key))
                        cse.back()->Push(gc,env->GetVariable(key));
                        gc->BarrierEnd();
                        return false;
                    }
                    if(key == "SetVariable")
                    {
                        std::string key;
                       
                        gc->BarrierBegin();
                        if(args.size() > 1 && GetArgument(args,0,key))
                        env->SetVariable(key,args[1]);
                        gc->BarrierEnd();
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }

                    if(key == "DeclareVariable")
                    {
                        std::string key;
                       
                        gc->BarrierBegin();
                        if(args.size() > 1 && GetArgument(args,0,key))
                        env->DeclareVariable(key,args[1]);
                        gc->BarrierEnd();
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                    if(key == "LoadFile")
                    {
                        TFile* f;
                        if(GetArgumentHeap(args,0,f))
                        {
                            env->LoadFile(gc,f);
                        }
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }

                    cse.back()->Push(gc,nullptr);
                    return false;
                }
                if(vfs != nullptr)
                {
                    auto myvfs = dynamic_cast<TObjectVFS*>(vfs->vfs);

                    auto mountable = dynamic_cast<Tesses::Framework::Filesystem::MountableFilesystem*>(vfs->vfs);
                    if(myvfs != nullptr)
                    {
                        TDictionary* dict2;
                        if(GetObjectHeap(myvfs->obj, dict2))
                        {

                            gc->BarrierBegin();
                            auto o = dict2->GetValue(key);
                            gc->BarrierEnd();

                            return InvokeMethod(ls,o,dict2,args);
                        }
                    }
                    if(mountable != nullptr)
                    {
                        if(key == "Unmount")
                        {
                            Tesses::Framework::Filesystem::VFSPath path;
                            
                            if(GetArgumentAsPath(args,0,path))
                            {
                                mountable->Unmount(path);
                            }

                        cse.back()->Push(gc, nullptr);
                        return false;
                        }
                        if(key == "Mount")
                        {
                            Tesses::Framework::Filesystem::VFSPath path;
                            TVFSHeapObject* vfs2;
                            if(GetArgumentAsPath(args,0,path) && GetArgumentHeap(args,1,vfs2))
                            {
                                TObjectVFS* vfs3 = new TObjectVFS(gc, vfs2);
                                //mountable->Mount(path, , true);
                                mountable->Mount(path, vfs3, true);
                            }

                        cse.back()->Push(gc, nullptr);
                        return false;
                        }
                    }

                    if(key == "EnumeratePaths")
                    {
                        Tesses::Framework::Filesystem::VFSPath dir;
                        if(GetArgumentAsPath(args,0,dir))
                        {
                            auto tem = TExternalMethod::Create(ls,"Get the enumerator",{},[vfs,dir](GCList& ls, std::vector<TObject> args)->TObject{
                                return TVFSPathEnumerator::Create(ls,vfs->vfs->EnumeratePaths(dir));
                            });
                            auto d1=TDictionary::Create(ls);
                            gc->BarrierBegin();
                            tem->watch.push_back(vfs);
                            d1->SetValue("GetEnumerator", tem);
                            gc->BarrierEnd();
                            
                            cse.back()->Push(gc,d1);
                            return false;
                        }

                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                     if(key == "MoveDirectory")
                    {
                        Tesses::Framework::Filesystem::VFSPath existingFile;

                        Tesses::Framework::Filesystem::VFSPath symlinkFile;
                        if(GetArgumentAsPath(args,0,existingFile) && GetArgumentAsPath(args,1,symlinkFile))
                        {
                            vfs->vfs->MoveDirectory(existingFile,symlinkFile);
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "SetDate")
                    {
                        Tesses::Framework::Filesystem::VFSPath path;
                        TDateTime lastWrite;
                        TDateTime lastAccess;
                        if(GetArgumentAsPath(args,0,path) && GetArgument(args,1,lastWrite) && GetArgument(args,2,lastAccess))
                        {
                            vfs->vfs->SetDate(path,lastWrite.GetDate(), lastAccess.GetDate());
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "GetDate")
                    {
                        Tesses::Framework::Filesystem::VFSPath path;
                        if(GetArgumentAsPath(args,0,path))
                        {
                            Tesses::Framework::Date::DateTime lastWrite;
                            Tesses::Framework::Date::DateTime lastAccess;
                            vfs->vfs->GetDate(path,lastWrite,lastAccess);
                            
                            auto dict = TDictionary::Create(ls);
                    ls.GetGC()->BarrierBegin();
                    
                    dict->SetValue("LastWrite", TDateTime(lastWrite));
                    dict->SetValue("LastAccess", TDateTime(lastAccess));
                    ls.GetGC()->BarrierEnd();

                        cse.back()->Push(gc, dict);
                        return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "MoveFile")
                    {
                        Tesses::Framework::Filesystem::VFSPath existingFile;

                        Tesses::Framework::Filesystem::VFSPath symlinkFile;
                        if(GetArgumentAsPath(args,0,existingFile) && GetArgumentAsPath(args,1,symlinkFile))
                        {
                            vfs->vfs->MoveFile(existingFile,symlinkFile);
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "CreateHardlink")
                    {
                        Tesses::Framework::Filesystem::VFSPath existingFile;

                        Tesses::Framework::Filesystem::VFSPath symlinkFile;
                        if(GetArgumentAsPath(args,0,existingFile) && GetArgumentAsPath(args,1,symlinkFile))
                        {
                            vfs->vfs->CreateHardlink(existingFile,symlinkFile);
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "CreateSymlink")
                    {
                        Tesses::Framework::Filesystem::VFSPath existingFile;

                        Tesses::Framework::Filesystem::VFSPath symlinkFile;
                        if(GetArgumentAsPath(args,0,existingFile) && GetArgumentAsPath(args,1,symlinkFile))
                        {
                            vfs->vfs->CreateSymlink(existingFile,symlinkFile);
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "RegularFileExists")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->RegularFileExists(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "SpecialFileExists")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->SpecialFileExists(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "FIFOFileExists")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->FIFOFileExists(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "SocketFileExists")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->SocketFileExists(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "BlockDeviceExists")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->BlockDeviceExists(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "CharacterDeviceExists")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->CharacterDeviceExists(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "SymlinkExists")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->SymlinkExists(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "DirectoryExists")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->DirectoryExists(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "FileExists")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->FileExists(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "ReadLink")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->ReadLink(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                     if(key == "VFSPathToSystem")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->VFSPathToSystem(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "SystemToVFSPath")
                    {
                        std::string filename;
                        if(GetArgument(args,0,filename))
                        {
                            cse.back()->Push(gc,vfs->vfs->SystemToVFSPath(filename));
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "DeleteFile")
                    {
                        Tesses::Framework::Filesystem::VFSPath filename;
                        if(GetArgumentAsPath(args,0,filename))
                        {
                            vfs->vfs->DeleteFile(filename);
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "DeleteDirectoryRecurse")
                    {
                        Tesses::Framework::Filesystem::VFSPath dirname;
                        if(GetArgumentAsPath(args,0,dirname))
                        {
                            vfs->vfs->DeleteDirectoryRecurse(dirname);
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "DeleteDirectory")
                    {   
                        Tesses::Framework::Filesystem::VFSPath dirname;
                        if(GetArgumentAsPath(args,0,dirname))
                        {
                            vfs->vfs->DeleteDirectory(dirname);
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "CreateDirectory")
                    {

                        Tesses::Framework::Filesystem::VFSPath dirname;
                        if(GetArgumentAsPath(args,0,dirname))
                        {
                            vfs->vfs->CreateDirectory(dirname);
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "OpenFile")
                    {
                        
                        Tesses::Framework::Filesystem::VFSPath path;
                        std::string mode;
                        if(GetArgumentAsPath(args,0,path) && GetArgument(args,1,mode))
                        {
                            auto res = vfs->vfs->OpenFile(path,mode);
                            TStreamHeapObject* strm = TStreamHeapObject::Create(ls,res);
                            cse.back()->Push(gc, strm);
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "Close")
                    {
                        vfs->Close();
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                
                if(strm != nullptr)
                {
                    auto memStrm = dynamic_cast<Tesses::Framework::Streams::MemoryStream*>(strm->stream);
                    auto netStrm = dynamic_cast<Tesses::Framework::Streams::NetworkStream*>(strm->stream);
                    
                    auto mystrm = dynamic_cast<TObjectStream*>(strm->stream);

                    if(mystrm != nullptr)
                    {
                        TDictionary* dict2;
                        if(GetObjectHeap(mystrm->obj, dict2))
                        {

                            gc->BarrierBegin();
                            auto o = dict2->GetValue(key);
                            gc->BarrierEnd();

                            return InvokeMethod(ls,o,dict2,args);
                        }
                    }
                    if(memStrm != nullptr)
                    {
                        if(key == "GetBytes")
                        {
                            auto res = TByteArray::Create(ls);
                            res->data = memStrm->GetBuffer();
                            cse.back()->Push(gc, res);
                            return false;
                        }
                    }
                    if(netStrm != nullptr)
                    {
                        if(key == "GetPort")
                        {
                            cse.back()->Push(gc, (int64_t)netStrm->GetPort());
                            return false;
                        }
                        if(key == "Bind")
                        {
                            std::string ip;
                            int64_t port;
                            if(GetArgument(args,0,ip) && GetArgument(args,1,port))
                                netStrm->Bind(ip,(uint16_t)port);
                            
                            cse.back()->Push(gc, nullptr);
                            return false;
                        }
                        if(key == "Accept")
                        {
                            std::string ip;
                            uint16_t port;
                            auto strm = netStrm->Accept(ip,port);
                            TDictionary* dict = TDictionary::Create(ls);
                            gc->BarrierBegin();
                            dict->SetValue("IP",ip);
                            dict->SetValue("Port",(int64_t)port);
                            dict->SetValue("Stream", TStreamHeapObject::Create(ls,strm));
                            
                            gc->BarrierEnd();
                            cse.back()->Push(gc, dict);
                            return false;
                        }
                        if(key == "Listen")
                        {
                            int64_t backlog;
                            if(GetArgument(args,0,backlog))
                            {
                                netStrm->Listen((int32_t)backlog);
                            }
                            else
                            {
                                netStrm->Listen(10);
                            }

                            cse.back()->Push(gc, nullptr);
                            return false;
                        }
                        if(key == "ReadFrom")
                        {
                            TByteArray* data;
                        int64_t offset;
                        int64_t length;
                        if(GetArgumentHeap<TByteArray*>(args, 0, data) && GetArgument<int64_t>(args, 1, offset) && GetArgument<int64_t>(args,2,length))
                        {
                            size_t off = (size_t)offset;
                            size_t len = (size_t)length;
                            std::string ip={};
                            uint16_t port=0;

                            if(off < len)
                                
                                len = netStrm->ReadFrom(data->data.data()+off,std::min(len,std::min(data->data.size() - off, data->data.size())),ip,port);
                            
                            else
                                len = 0;

                            TDictionary* dict = TDictionary::Create(ls);
                            gc->BarrierBegin();
                            dict->SetValue("IP",ip);
                            dict->SetValue("Port",(int64_t)port);
                            dict->SetValue("Read", (int64_t)len);
                            
                            gc->BarrierEnd();
                            cse.back()->Push(gc, dict);
                            
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                        }
                        if(key == "WriteTo")
                        {
                            TByteArray* data;
                        int64_t offset;
                        int64_t length;
                        std::string ip;
                        int64_t port;
                        if(GetArgumentHeap<TByteArray*>(args, 0, data) && GetArgument<int64_t>(args, 1, offset) && GetArgument<int64_t>(args,2,length) && GetArgument(args,3,ip) && GetArgument(args,4,port))
                        {
                            size_t off = (size_t)offset;
                            size_t len = (size_t)length;

                            if(off < len)
                                
                                len = netStrm->WriteTo(data->data.data()+off,std::min(len, std::min(data->data.size() - off, data->data.size())), ip, (int64_t)port);
                                
                            else
                                len = 0;
                            
                            cse.back()->Push(gc, (int64_t)len);
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                        }
                    }

                    if(key == "Read")
                    {
                        TByteArray* data;
                        int64_t offset;
                        int64_t length;
                        if(GetArgumentHeap<TByteArray*>(args, 0, data) && GetArgument<int64_t>(args, 1, offset) && GetArgument<int64_t>(args,2,length))
                        {
                            size_t off = (size_t)offset;
                            size_t len = (size_t)length;

                            if(off < len)
                               
                                len = strm->stream->Read(data->data.data()+off,std::min(len,std::min(data->data.size() - off, data->data.size())));
                               
                            else
                                len = 0;
                            
                            cse.back()->Push(gc, (int64_t)len);
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "Write")
                    {
                        TByteArray* data;
                        int64_t offset;
                        int64_t length;
                        if(GetArgumentHeap<TByteArray*>(args, 0, data) && GetArgument<int64_t>(args, 1, offset) && GetArgument<int64_t>(args,2,length))
                        {
                            size_t off = (size_t)offset;
                            size_t len = (size_t)length;

                            if(off < len)
                               
                                len = strm->stream->Write(data->data.data()+off,std::min(len, std::min(data->data.size() - off, data->data.size())));
                                
                            else
                                len = 0;
                            
                            cse.back()->Push(gc, (int64_t)len);
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "ReadBlock")
                    {
                        TByteArray* data;
                        int64_t offset;
                        int64_t length;
                        if(GetArgumentHeap<TByteArray*>(args, 0, data) && GetArgument<int64_t>(args, 1, offset) && GetArgument<int64_t>(args,2,length))
                        {
                            size_t off = (size_t)offset;
                            size_t len = (size_t)length;

                            if(off < len)
                                
                                len = strm->stream->ReadBlock(data->data.data()+off,std::min(len, std::min(data->data.size() - off, data->data.size())));
                               
                            else
                                len = 0;
                            
                            cse.back()->Push(gc, (int64_t)len);
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "WriteText")
                    {
                        std::string text;

                        if(GetArgument(args,0,text))
                        {
                            strm->stream->WriteBlock((const uint8_t*)text.data(), text.size());
                        }

                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "WriteBlock")
                    {
                        TByteArray* data;
                        int64_t offset;
                        int64_t length;
                        if(GetArgumentHeap<TByteArray*>(args, 0, data) && GetArgument<int64_t>(args, 1, offset) && GetArgument<int64_t>(args,2,length))
                        {
                            size_t off = (size_t)offset;
                            size_t len = (size_t)length;

                            if(off < len)
                               
                                strm->stream->WriteBlock(data->data.data()+off,std::min(len,std::min(data->data.size() - off, data->data.size())));
                                

                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "CopyTo")
                    {
                        TStreamHeapObject* data;
                        int64_t buffSize;
                        if(GetArgumentHeap(args,0,data))
                        {
                            if(!GetArgument<int64_t>(args,1,buffSize)) buffSize=1024;
                            strm->stream->CopyTo(*data->stream,(size_t)buffSize);
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "Flush")
                    {
                        strm->stream->Flush();
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "Seek")
                    {
                        int64_t pos,whence;
                        if(!GetArgument<int64_t>(args,0,pos)) pos=0;

                        if(!GetArgument<int64_t>(args,1,whence)) whence=0;
                        
                     
                        strm->stream->Seek(pos, whence == 0 ? Tesses::Framework::Streams::SeekOrigin::Begin : whence == 1 ? Tesses::Framework::Streams::SeekOrigin::Current : Tesses::Framework::Streams::SeekOrigin::End);

                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if(key == "Close")
                    {
                        strm->Close();
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }

                if(ittr != nullptr)
                {
                    if(key == "MoveNext")
                    {
                        cse.back()->Push(gc, ittr->MoveNext(gc));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
               
                else if(bArray != nullptr)
                {
                    if(key == "Count" || key == "Length")
                    {
                        int64_t len = (int64_t)bArray->data.size();
                        if(len < 0) len = 0;

                        cse.back()->Push(gc, len);
                        return false;
                
                    }
                    if(key == "ToString")
                    {
                         cse.back()->Push(gc,std::string(bArray->data.begin(),bArray->data.end()));
                         return false;
                    }
                    if(key == "GetAt")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("ByteArray.GetAt must only accept one argument");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("ByteArray.GetAt must only accept a long");
                        }

                        size_t index = (size_t)std::get<int64_t>(args[0]);
                        size_t sz = bArray->data.size();
                        if(index >= 0 && index < sz)
                        {
                            cse.back()->Push(gc, (int64_t)bArray->data[index]);
                            return false;
                        }
                        
                    }

                    if(key == "SetAt")
                    {
                        if(args.size() != 2)
                        {
                            throw VMException("ByteArray.SetAt must only accept two arguments");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("ByteArray.SetAt first argument must only accept a long");
                        }

                        uint8_t v = 0;

                        if(std::holds_alternative<int64_t>(args[1]))
                        {
                            v = (uint8_t)(std::get<int64_t>(args[1]) & 0xFF);
                        }

                        size_t index = (size_t)std::get<int64_t>(args[0]);
                        size_t sz = bArray->data.size();
                        if(index >= 0 && index < sz)
                        {
                            bArray->data[index]=v;
                            return false;
                        }
                        
                    }
                    if(key == "CopyTo")
                    {
                        //CopyTo(destBuff, offsetSrc, offsetDest, length)

                        if(args.size() != 4)
                        {
                            throw VMException("ByteArray.CopyTo must only accept 4 arguments");
                        }

                        TByteArray* bArray2;
                        int64_t offsetSrc;
                        int64_t offsetDest;
                        int64_t count;
                        if(!GetArgumentHeap<TByteArray*>(args,0,bArray2)) 
                        {
                            throw VMException("ByteArray.CopyTo first argument must be a ByteArray");
                        }
                        if(!GetArgument<int64_t>(args,1, offsetSrc))
                        {
                            throw VMException("ByteArray.CopyTo second argument must be a Long (offsetSrc)");
                        }
                        if(!GetArgument<int64_t>(args,2, offsetDest))
                        {
                            throw VMException("ByteArray.CopyTo third argument must be a Long (offsetDest)");
                        }
                        if(!GetArgument<int64_t>(args,3,count))
                        {
                            throw VMException("ByteArray.CopyTo fourth argument must be a Long (length)");
                        }
                        size_t offsrc = (size_t)offsetSrc;
                        size_t offdest = (size_t)offsetDest;
                        size_t len = (size_t)count;

                        if(offsrc > bArray->data.size())
                        {
                            offsrc = bArray->data.size();
                        }

                        if(offdest > bArray2->data.size())
                        {
                            offdest = bArray2->data.size();
                        }
                        
                        len = std::min<size_t>(std::min<size_t>(bArray->data.size()-offsrc,bArray2->data.size()-offdest),len);
                        
                        if(len > 0)
                        memcpy(bArray2->data.data()+offdest,bArray->data.data()+offsrc,len);
                        cse.back()->Push(gc,bArray2);
                        return false;
                    }
                    if(key == "Insert")
                    {
                        if(args.size() != 4)
                        {
                            throw VMException("ByteArray.Insert must only accept 4 arguments");
                        }
                        //Insert(offsetDest, buffer, offsetSrc, length)
                        TByteArray* bArray2;
                        int64_t offsetSrc;
                        int64_t offsetDest;
                        int64_t count;
                        if(!GetArgumentHeap<TByteArray*>(args,1,bArray2)) 
                        {
                            throw VMException("ByteArray.Insert second argument must be a ByteArray");
                        }
                        if(!GetArgument<int64_t>(args,0, offsetSrc))
                        {
                            throw VMException("ByteArray.Insert first argument must be a Long (offsetSrc)");
                        }
                        if(!GetArgument<int64_t>(args,2, offsetDest))
                        {
                            throw VMException("ByteArray.Insert third argument must be a Long (offsetDest)");
                        }
                        if(!GetArgument<int64_t>(args,3,count))
                        {
                            throw VMException("ByteArray.Insert fourth argument must be a Long (length)");
                        }
                
                        size_t offsrc = (size_t)offsetSrc;
                        size_t offdest = (size_t)offsetDest;
                        size_t len = (size_t)count;
                
                        if(offsrc > bArray->data.size())
                        {
                            offsrc = bArray->data.size();
                        }
                        if(offdest > bArray->data.size())
                        {
                            offdest = bArray->data.size();
                        }
                        
                        len = std::min(len,bArray2->data.size()-offsrc);
                        

                        bArray->data.insert(bArray->data.begin()+offdest,bArray2->data.begin()+offsrc,bArray2->data.begin()+offsrc+len);
                        cse.back()->Push(gc, bArray);
                        return false;
                    }
                    if(key == "Append")
                    {
                        if(args.size() != 3)
                        {
                            throw VMException("ByteArray.Append must only accept 3 arguments");
                        }
                        TByteArray* bArray2;
                        int64_t offset;
                        int64_t count;
                        if(!GetArgumentHeap<TByteArray*>(args,0,bArray2)) 
                        {
                            throw VMException("ByteArray.Append first argument must be a ByteArray");
                        }
                        if(!GetArgument<int64_t>(args,1, offset))
                        {
                            throw VMException("ByteArray.Append second argument must be a Long (offset)");
                        }
                        if(!GetArgument<int64_t>(args,2,count))
                        {
                            throw VMException("ByteArray.Append third argument must be a Long (length)");
                        }
                        size_t off = (size_t)offset;
                        size_t len = (size_t)count;

                        if(off > bArray->data.size())
                        {
                            off = bArray->data.size();
                        }
                       
                        len = std::min<size_t>(bArray->data.size() - off,len);
                        bArray->data.insert(bArray->data.end(),bArray2->data.begin()+off,bArray2->data.begin()+off+len);
                        cse.back()->Push(gc,bArray);
                        return false;
                    }
               
                    if(key == "Resize")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("ByteArray.Resize must only accept one argument");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("ByteArray.Resize must only accept a long");
                        } 

                        size_t len = (size_t)std::get<int64_t>(args[0]);
                        if(len >= 0)
                        {
                            bArray->data.resize(len);
                        }
                        cse.back()->Push(gc, bArray);
                        return false;
                    }
                    

                    cse.back()->Push(gc, Undefined());
                    return false;
                }
                else if(aArray != nullptr)
                {
                    if(key == "ToString")
                    {
                        cse.back()->Push(gc,ToString(gc,aArray));
                        return false;
                    }
                    else if(key == "GetAt")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("AArray.GetAt must only accept one argument");
                        }
                        cse.back()->Push(gc, aArray->Get(gc,args[0]));
                        return false;
                    }
                    else if(key == "SetAt")
                    {
                        if(args.size() != 2)
                        {
                            throw VMException("AArray.SetAt must only accept two arguments");
                        }
                        aArray->Set(gc,args[0],args[1]);
                        cse.back()->Push(gc, args[1]);
                        return false;
                    }
                    else if(key == "GetKey")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("AArray.GetKey must only accept one argument");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("AArray.GetKey must only accept a long");
                        }

                        gc->BarrierBegin();
                        auto res = aArray->GetKey(std::get<int64_t>(args[0]));
                        gc->BarrierEnd();
                        cse.back()->Push(gc, res);
                        return false;
                    }
                    else if(key == "GetValue")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("AArray.GetValue must only accept one argument");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("AArray.GetValue must only accept a long");
                        }

                        gc->BarrierBegin();
                        auto res = aArray->GetValue(std::get<int64_t>(args[0]));
                        gc->BarrierEnd();
                        cse.back()->Push(gc, res);
                        return false;
                    }
                    else if(key == "SetKey")
                    {
                        if(args.size() != 2)
                        {
                            throw VMException("AArray.SetKey must only accept two arguments");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("AArray.SetKey first argument must only accept a long");
                        }

                        gc->BarrierBegin();
                        aArray->SetKey(std::get<int64_t>(args[0]),args[1]);
                        gc->BarrierEnd();
                        cse.back()->Push(gc, args[1]);
                        return false;
                    }
                    else if(key == "SetValue")
                    {
                        if(args.size() != 2)
                        {
                            throw VMException("AArray.SetValue must only accept two arguments");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("AArray.SetValue first argument must only accept a long");
                        }

                        gc->BarrierBegin();
                        aArray->SetValue(std::get<int64_t>(args[0]),args[1]);
                        gc->BarrierEnd();
                        cse.back()->Push(gc, args[1]);
                        return false;
                    }
                    else if(key == "Count" || key == "Length")
                    {
                        gc->BarrierBegin();
                        cse.back()->Push(gc, aArray->Count());
                        gc->BarrierEnd();
                        return false;
                    }
                    else if(key == "GetEnumerator")
                    {
                        
                        cse.back()->Push(gc,TAssociativeArrayEnumerator::Create(ls,aArray));
                        return false;
                    }
                    cse.back()->Push(gc,Undefined());
                    return false;
                }
                else if(list != nullptr)
                {
                    
                    if(key == "GetEnumerator")
                    {
                        cse.back()->Push(gc, TListEnumerator::Create(ls,list));
                        return false;
                    }
                    else if(key == "ToString")
                    {
                        
                        cse.back()->Push(gc,Json_Encode(list));
                        return false;
                    
                    }
                    else if(key == "Insert")
                    {
                        if(args.size() != 2)
                        {
                            throw VMException("List.Insert must only accept two arguments");
                        }
                        int64_t index;

                        if(!GetArgument(args,0,index))
                        {
                            throw VMException("List.Insert first argument must be Long");
                        }

                        gc->BarrierBegin();
                        list->Insert(index,args[1]);
                        gc->BarrierEnd();
                        cse.back()->Push(gc, Undefined());
                        return false;
                    }
                    else if(key == "Add")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("List.Add must only accept one argument");
                        }
                        gc->BarrierBegin();
                        list->Add(args[0]);
                        gc->BarrierEnd();
                        cse.back()->Push(gc, Undefined());
                        return false;
                    }
                    else if(key == "Contains")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("List.Contains must only accept one argument");
                        }
                        gc->BarrierBegin();
                        for(int64_t i = 0; i < list->Count(); i++)
                        {
                            auto item = list->Get(i);
                            gc->BarrierEnd();
                            if(Equals(gc,args[0],item))
                            {
                                cse.back()->Push(gc, true);
                                return false;
                            }
                            gc->BarrierBegin();
                        }
                        gc->BarrierEnd();
                        cse.back()->Push(gc, false);
                        return false;
                    }
                    else if(key == "IndexOf")
                    {
                        //IndexOf(obj, $idx)
                        if(args.size() < 1 || args.size() > 2)
                        {
                            throw VMException("List.IndexOf must either have one or two arguments");
                        }

                        int64_t i=0;
                        
                        GetArgument(args,1,i);
                        gc->BarrierBegin();
                        for(; i < list->Count(); i++)
                        {
                            auto item = list->Get(i);
                            gc->BarrierEnd();
                            if(Equals(gc,args[0],item))
                            {
                                
                                cse.back()->Push(gc,i);
                                return false;
                            }
                            gc->BarrierBegin();
                        }
                        gc->BarrierEnd();
                        cse.back()->Push(gc,(int64_t)-1);
                        return false;
                    }
                    else if(key == "RemoveAllEqual")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("List.RemoveAllEqual must only accept one argument");
                        }


                        gc->BarrierBegin();
                        for(int64_t i = 0; i < list->Count(); i++)
                        {
                            auto item = list->Get(i);
                            gc->BarrierEnd();
                            if(Equals(gc,args[0],item))
                            {
                                gc->BarrierBegin();
                                list->RemoveAt(i);
                                i--;
                            }else gc->BarrierBegin();
                        }
                        gc->BarrierEnd();
                        cse.back()->Push(gc, Undefined());
                        return false;
                    }
                    else if(key == "Remove")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("List.Remove must only accept one argument");
                        }


                        gc->BarrierBegin();
                        for(int64_t i = 0; i < list->Count(); i++)
                        {
                            auto item = list->Get(i);
                            gc->BarrierEnd();
                            if(Equals(gc,args[0],item))
                            {
                                gc->BarrierBegin();
                                list->RemoveAt(i);
                                gc->BarrierEnd();
                                break;
                            }
                            gc->BarrierBegin();
                        }
                        gc->BarrierEnd();
                        cse.back()->Push(gc, Undefined());
                        return false;
                    }
                    else if(key == "RemoveAt")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("List.RemoveAt must only accept one argument");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("List.RemoveAt must only accept a long");
                        }
                        gc->BarrierBegin();
                        list->RemoveAt(std::get<int64_t>(args[0]));
                        gc->BarrierEnd();
                        cse.back()->Push(gc, Undefined());
                        return false;
                    }
                    else if(key == "Clear")
                    {
                        gc->BarrierBegin();
                        list->Clear();
                        gc->BarrierEnd();
                        cse.back()->Push(gc, Undefined());
                        return false;
                    }
                    else if(key == "GetAt")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("List.GetAt must only accept one argument");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("List.GetAt must only accept a long");
                        }

                        int64_t index = std::get<int64_t>(args[0]);
                        if(index >= 0 && index < list->Count())
                        {
                            cse.back()->Push(gc, list->Get(index));
                            return false;
                        }
                        
                    }
                    else if(key == "SetAt")
                    {
                        if(args.size() != 2)
                        {
                            throw VMException("List.SetAt must only accept two arguments");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("List.SetAt first argument must only accept a long");
                        }

                        int64_t index = std::get<int64_t>(args[0]);
                        if(index >= 0 && index < list->Count())
                        {
                            list->Set(index,args[1]);
                            return false;
                        }
                        
                    }
                    
                    else if(key == "Count" || key == "Length")
                    {
                        gc->BarrierBegin();
                        cse.back()->Push(gc, list->Count());
                        gc->BarrierEnd();
                        return false;
                    }
                    cse.back()->Push(gc, Undefined());
                    return false;
                }
                else if(dynList != nullptr)
                {

                    if(key == "GetEnumerator")
                    {
                        cse.back()->Push(gc, TDynamicListEnumerator::Create(ls, dynList));
                        return false;
                    }
                    else if(key == "ToString")
                    {
                        cse.back()->Push(gc, dynList->ToString(ls));
                        return false;
                    }
                    else if(key == "Insert")
                    {
                        if(args.size() != 2)
                        {
                            throw VMException("List.Insert must only accept two arguments");
                        }
                        int64_t index;

                        if(!GetArgument(args,0,index))
                        {
                            throw VMException("List.Insert first argument must be Long");
                        }

                        cse.back()->Push(gc, dynList->Insert(ls,index,args[0]));
                        return false;
                    }
                    if(key == "Add")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("List.Add must only accept one argument");
                        }
                       
                        cse.back()->Push(gc,dynList->Add(ls,args[0]));
                        return false;
                    }
                    if(key == "RemoveAllEqual")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("List.RemoveAllEqual must only accept one argument");
                        }

                        cse.back()->Push(gc,dynList->RemoveAllEqual(ls, args[0]));
                        return false;
                    }
                    if(key == "Remove")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("List.Remove must only accept one argument");
                        }


                                                
                        cse.back()->Push(gc,dynList->Remove(ls, args[0]));
                        return false;
                    }
                    if(key == "RemoveAt")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("List.RemoveAt must only accept one argument");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("List.RemoveAt must only accept a long");
                        }
                        cse.back()->Push(gc,dynList->RemoveAt(ls,std::get<int64_t>(args[0])));
                        
                        return false;
                    }
                    if(key == "Clear")
                    {
                        
                      
                        cse.back()->Push(gc, dynList->Clear(ls));
                        return false;
                    }
                    if(key == "GetAt")
                    {
                        if(args.size() != 1)
                        {
                            throw VMException("List.GetAt must only accept one argument");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("List.GetAt must only accept a long");
                        }

                        int64_t index = std::get<int64_t>(args[0]);
                        if(index >= 0)
                        {
                            cse.back()->Push(gc, dynList->GetAt(ls,index));
                            return false;
                        }
                        
                    }
                    if(key == "SetAt")
                    {
                        if(args.size() != 2)
                        {
                            throw VMException("List.SetAt must only accept two arguments");
                        }

                        if(!std::holds_alternative<int64_t>(args[0])) 
                        {
                            throw VMException("List.SetAt first argument must only accept a long");
                        }

                        int64_t index = std::get<int64_t>(args[0]);
                        if(index >= 0)
                        {
                            cse.back()->Push(gc,dynList->SetAt(ls,index,args[1]));
                            return false;
                        }
                        
                    }
                    
                    if(key == "Count" || key == "Length")
                    {
                        cse.back()->Push(gc, dynList->Count(ls));
                       
                        return false;
                    }
                    cse.back()->Push(gc, Undefined());
                    return false;
                }
                else if(cls != nullptr)
                {
                    gc->BarrierBegin();
                    auto obj=cls->GetValue(cse.back()->callable->className,key);
                    gc->BarrierEnd();
                    TClosure* clos;
                    TCallable* callable;
                    if(GetObjectHeap(obj,clos)) 
                    {
                        this->AddCallStackEntry(ls,clos,args);
                        return true;
                    }
                    else if(GetObjectHeap(obj,callable))
                    {
                        cse.back()->Push(gc,callable->Call(ls,args));
                        return false;
                    }
                    cse.back()->Push(gc,Undefined());
                    return false;
                }
                else if(dict != nullptr)
                {
                    if(key == "ToString" && !dict->MethodExists(ls, key) && args.empty())
                    {
                        cse.back()->Push(gc,Json_Encode(dict));
                        return false;
                    }
                    gc->BarrierBegin();
                    auto o = dict->GetValue(key);
                    gc->BarrierEnd();
                   

                    return InvokeMethod(ls,o,dict,args);
                }
                else if(dynDict != nullptr)
                {
                    
                    
                    cse.back()->Push(gc,dict->CallMethod(ls, key, args));

                    return false;
                }
                else if(callable != nullptr)
                {
                    if(key == "Call")
                    {
                        TList* argls;
                        if(GetArgumentHeap(args,0,argls))
                        {
                            TClosure* clo = dynamic_cast<TClosure*>(callable);
                            if(clo != nullptr)
                            {
                                AddCallStackEntry(ls,clo,argls->items);
                                return true;
                            }
                            else
                            {
                                cse.back()->Push(gc,callable->Call(ls, argls->items));
                                return false;
                            }
                        }
                    }
                    cse.back()->Push(gc,nullptr);
                    return false;
                }
                else
                {
                    cse.back()->Push(gc, Undefined());
                }
            }
            else
            {
                cse.back()->Push(gc,Undefined());
            }
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
    bool InterperterThread::GetField(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;

        if(!cse.empty())
        {
            auto stk = cse.back();
            GCList ls(gc);
            TObject _key = stk->Pop(ls);
            
            
            TObject instance = stk->Pop(ls);
            
            if(!std::holds_alternative<std::string>(_key))
            {
                stk->Push(gc,Undefined());
                return false;
            }

            std::string key = std::get<std::string>(_key);


            if(std::holds_alternative<std::string>(instance))
            {
                std::string str = std::get<std::string>(instance);
                
                if(key == "Count" || key == "Length")
                {
                    int64_t len = (int64_t)str.size();
                    if(len < 0) len = 0;

                    cse.back()->Push(gc, len);
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
            if(std::holds_alternative<TVMVersion>(instance))
            {
                TVMVersion& version = std::get<TVMVersion>(instance);
                if(key == "Major")
                {
                    stk->Push(gc, (int64_t)version.Major());
                    return false;
                }
                if(key == "Minor")
                {
                    stk->Push(gc, (int64_t)version.Minor());
                    return false;
                }
                if(key == "Patch")
                {
                    stk->Push(gc, (int64_t)version.Patch());
                    return false;
                }
                if(key == "Build")
                {
                    stk->Push(gc, (int64_t)version.Build());
                    return false;
                }
                if(key == "VersionInt")
                {
                    stk->Push(gc,(int64_t)version.AsLong());
                    return false;
                }
                if(key == "Stage")
                {
                    switch(version.VersionStage())
                    {
                        case TVMVersionStage::DevVersion:
                            stk->Push(gc,"dev");
                            break;
                        case TVMVersionStage::AlphaVersion:
                            stk->Push(gc,"alpha");
                            break;
                        case TVMVersionStage::BetaVersion:
                            stk->Push(gc,"beta");
                            break;
                        case TVMVersionStage::ProductionVersion:
                            stk->Push(gc,"prod");
                            break;
                    }
                    return false;
                }
                
                stk->Push(gc, Undefined());
                return false;
            }
            if(std::holds_alternative<TDateTime>(instance))
            {
                auto& date = std::get<TDateTime>(instance).GetDate();
                if(key == "IsLocal")
                {
                    stk->Push(gc, date.IsLocal());
                    return false;
                }
                if(key == "Year")
                {
                    stk->Push(gc, (int64_t)date.Year());
                    return false;
                }
                if(key == "Month")
                {
                    stk->Push(gc, (int64_t)date.Month());
                    return false;
                }
                if(key == "Day")
                {
                    stk->Push(gc, (int64_t)date.Day());
                    return false;
                }
                if(key == "Hour")
                {
                    stk->Push(gc, (int64_t)date.Hour());
                    return false;
                }
                if(key == "Minute")
                {
                    stk->Push(gc, (int64_t)date.Minute());
                    return false;
                }
                if(key == "Second")
                {
                    stk->Push(gc, (int64_t)date.Second());
                    return false;
                }
                if(key == "DayOfWeek")
                {
                    stk->Push(gc, (int64_t)date.DayOfWeek());
                    return false;
                }
                
                stk->Push(gc, Undefined());
                return false;
            }
            if(std::holds_alternative<THeapObjectHolder>(instance))
            {
                auto obj = std::get<THeapObjectHolder>(instance).obj;
                auto bA = dynamic_cast<TByteArray*>(obj);
                auto list = dynamic_cast<TList*>(obj);
                auto dict = dynamic_cast<TDictionary*>(obj);
                auto dynDict = dynamic_cast<TDynamicDictionary*>(obj);
                auto dynList = dynamic_cast<TDynamicList*>(obj);
                auto tcallable = dynamic_cast<TCallable*>(obj);
                auto closure = dynamic_cast<TClosure*>(obj);
                auto externalMethod = dynamic_cast<TExternalMethod*>(obj);
                auto ittr = dynamic_cast<TEnumerator*>(obj);
                auto strm = dynamic_cast<TStreamHeapObject*>(obj);
                auto vfs = dynamic_cast<TVFSHeapObject*>(obj);
                auto callstackEntry = dynamic_cast<CallStackEntry*>(obj);
                auto file = dynamic_cast<TFile*>(obj);
                auto chunk = dynamic_cast<TFileChunk*>(obj);
                auto natObj = dynamic_cast<TNativeObject*>(obj);
                auto cls = dynamic_cast<TClassObject*>(obj);
                auto aarray = dynamic_cast<TAssociativeArray*>(obj);
                auto task = dynamic_cast<TTask*>(obj);
                if(task != nullptr)
                {
                    if(key == "IsCompleted")
                    {
                        cse.back()->Push(gc,task->IsCompleted());
                        return false;
                    }
                    cse.back()->Push(gc,Undefined());
                    return false;
                }
                if(aarray != nullptr)
                {
                    if(key == "Count" || key == "Length")
                    {
                        cse.back()->Push(gc,aarray->Count());
                        return false;
                    }
                    cse.back()->Push(gc, Undefined());
                    return false;
                }
                else if(cls != nullptr)
                {
                    gc->BarrierBegin();
                    auto obj=cls->GetValue(cse.back()->callable->className,"get"+key);
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
                    cse.back()->Push(gc,cls->GetValue(cse.back()->callable->className,key));
                    return false;
               
                }
                else
                if(natObj != nullptr)
                {
                    cse.back()->Push(gc,natObj->CallMethod(ls,"get"+key,{}));
                    return false;
                }
               
                if(file != nullptr)
                {
                    if(key == "Version")
                    {
                        cse.back()->Push(gc,file->version);
                        return false;
                    }
                    else if(key == "Name")
                    {
                        cse.back()->Push(gc,file->name);
                        return false;
                    }
                    else if(key == "Info")
                    {
                        cse.back()->Push(gc, file->info);
                        return false;
                    }
                    else if(key == "Dependencies")
                    {
                        auto list = TList::Create(ls);
                        gc->BarrierBegin();
                        for(auto item : file->dependencies)
                        {
                            auto res = TDictionary::Create(ls);
                            res->SetValue("Name", item.first);
                            res->SetValue("Version", item.second);
                            list->Add(res);
                        }
                        gc->BarrierEnd();
                        cse.back()->Push(gc, list);
                        return false;
                    }
                    else if(key == "Tools")
                    {
                        auto list = TList::Create(ls);
                        gc->BarrierBegin();
                        for(auto item : file->tools)
                        {
                            auto res = TDictionary::Create(ls);
                            res->SetValue("Name", item.first);
                            res->SetValue("Version", item.second);
                            list->Add(res);
                        }
                        gc->BarrierEnd();
                        cse.back()->Push(gc, list);
                        return false;
                    }
                    else if(key == "Strings")
                    {
                        auto list = TList::Create(ls);
                        gc->BarrierBegin();
                        for(auto item : file->name)
                        {
                            list->Add(item);
                        }
                        gc->BarrierEnd();

                        cse.back()->Push(gc, list);
                        return false;
                    }
                    else if(key == "Sections")
                    {
                        TList* sections = TList::Create(ls);
                        gc->BarrierBegin();
                        for(auto& item : file->sections)
                        {
                            TByteArray* ba = TByteArray::Create(ls);
                            ba->data = item.second;
                            sections->Add(TDictionary::Create(ls,{
                                TDItem("Name", item.first),
                                TDItem("Data", ba)
                            }));
                        }
                        gc->BarrierEnd();

                        cse.back()->Push(gc, sections);
                        return false;
                    }
                    else if(key == "SupportedVMs")
                    {
                        TList* supported = TList::Create(ls);
                        gc->BarrierBegin();
                        if(file->vms.empty()) 
                        {
                            supported->Add(TDictionary::Create(ls,{
                                TDItem("Name",std::string(VMName)),
                                TDItem("HowToGet",std::string(VMHowToGet))
                            }));
                        }
                        else {
                            for(auto item : file->vms)
                                supported->Add(TDictionary::Create(ls,{
                                    TDItem("Name",item.first),
                                    TDItem("HowToGet",item.second)
                                }));
                        }
                        gc->BarrierEnd();
                        cse.back()->Push(gc, supported);
                        return false;
                    }
                    else if(key == "Chunks")
                    {
                        auto list = TList::Create(ls);
                        gc->BarrierBegin();
                        for(auto item : file->chunks)
                        {
                            list->Add(item);
                        }

                        gc->BarrierEnd();

                        cse.back()->Push(gc, list);
                        return false;
                    }
                    else if(key == "Classes")
                    {
                        auto list = TList::Create(ls);
                        gc->BarrierBegin();

                        for(uint32_t i = 0; i < (uint32_t)file->classes.size(); i++)
                        {
                            list->Add(GetClassInfo(ls,file,i));
                        }

                        cse.back()->Push(gc, list);
                        gc->BarrierEnd();
                        return false;
                    }
                    else if(key == "Functions")
                    {
                        auto list = TList::Create(ls);
                        gc->BarrierBegin();
                        for(auto& item : file->functions)
                        {
                            TDictionary* dict = TDictionary::Create(ls);
                            if(!item.first.empty())
                                dict->SetValue("Documentation", item.first[0]);
                            TList* nameParts = TList::Create(ls);
                            for(size_t i = 1; i < item.first.size(); i++)
                            {
                                nameParts->Add(item.first[i]);
                            }
                            dict->SetValue("NameParts", nameParts);
                            dict->SetValue("ChunkId", (int64_t)item.second);

                            list->Add(dict);
                        }
                        
                        cse.back()->Push(gc, list);
                        gc->BarrierEnd();
                        return false;
                    }
                    else if(key == "Icon")
                    {
                        if(file->icon >= 0 && file->icon < file->resources.size())
                        {
                            TByteArray* ba = TByteArray::Create(ls);
                            ba->data = file->resources[file->icon];
                            cse.back()->Push(gc, ba);
                            return false;
                        }
                        else {
                            cse.back()->Push(gc, nullptr);
                            return false;
                        }
                    }
                    cse.back()->Push(gc, Undefined());
                    return false;
                }

                if(callstackEntry != nullptr)
                {
                    if(key == "IP") 
                    {
                        cse.back()->Push(gc, (int64_t)callstackEntry->ip);
                        return false;
                    }
                    if(key == "IsDone")
                    {
                        cse.back()->Push(gc, callstackEntry->ip >= callstackEntry->callable->closure->code.size());
                        return false;
                    }
                    if(key == "Closure")
                    {
                        cse.back()->Push(gc, callstackEntry->callable);
                        return false;
                    }
                    if(key == "StackEmpty")
                    {
                        cse.back()->Push(gc, callstackEntry->stack.empty());
                        return false;
                    }
                    cse.back()->Push(gc,Undefined());
                    return false;
                }
            
                if(strm != nullptr)
                {
                    auto netStrm = dynamic_cast<Tesses::Framework::Streams::NetworkStream*>(strm->stream);
                    auto objStrm = dynamic_cast<TObjectStream*>(strm->stream);
                    if(objStrm != nullptr)
                    {
                        GetObjectHeap(objStrm->obj,dict);
                    }else{
                        if(key == "CanRead")
                        {
                            bool r = strm->stream != nullptr ? strm->stream->CanRead() : false;
                            
                            cse.back()->Push(gc, r);
                            return false;
                        }
                        if(key == "CanWrite")
                        {
                            bool r = strm->stream != nullptr ? strm->stream->CanWrite() : false;
                            
                            cse.back()->Push(gc, r);
                            return false;
                        }
                        if(key == "CanSeek")
                        {
                            bool r = strm->stream != nullptr ? strm->stream->CanSeek() : false;
                            
                            cse.back()->Push(gc, r);
                            return false;
                        }
                        if(key == "EndOfStream")
                        {
                            bool r = strm->stream != nullptr ? strm->stream->EndOfStream() : false;
                            
                            cse.back()->Push(gc, r);
                            return false;
                        }
                        if(key == "Length")
                        {
                            int64_t r = strm->stream != nullptr ? strm->stream->GetLength() : 0;
                            
                            cse.back()->Push(gc, r);
                            return false;
                        }
                         if(key == "Position")
                        {
                            int64_t r = strm->stream != nullptr ? strm->stream->GetPosition() : 0;
                            
                            cse.back()->Push(gc, r);
                            return false;
                        }

                        cse.back()->Push(gc, Undefined());

                        return false;
                    }
                }
                if(vfs != nullptr)
                {
                    auto d = dynamic_cast<TObjectVFS*>(vfs->vfs);
                    if(d != nullptr)
                    {
                        GetObjectHeap(d->obj,dict);
                    }
                }
                
                if(ittr != nullptr)
                {
                    if(key == "Current")
                    {
                        cse.back()->Push(gc, ittr->GetCurrent(ls));
                        return false;
                    }
                    cse.back()->Push(gc, Undefined());
                    return false;
                }
                if(closure != nullptr)
                {
                    if(key == "Arguments")
                    {
                        GCList ls2(gc);
                        TList* ls = TList::Create(ls2);
                        for(auto arg : closure->closure->args)
                        {
                            ls->Add(arg);
                        }
                        cse.back()->Push(gc,ls);
                        return false;
                    }
                    if(key == "File")
                    {
                        cse.back()->Push(gc,closure->file);   
                        return false;                 
                    }
                }
                if(externalMethod != nullptr)
                {
                    if(key == "Arguments")
                    {
                        GCList ls2(gc);
                        TList* ls = TList::Create(ls2);
                        for(auto arg : externalMethod->args)
                        {
                            ls->Add(arg);
                        }
                        cse.back()->Push(gc,ls);
                        return false;
                    }
                }

                if(tcallable != nullptr)
                {
                    if(key == "Documentation")
                    {
                        cse.back()->Push(gc, tcallable->documentation);
                        return false;
                    }
                    if(key == "Tag")
                    {
                        gc->BarrierBegin();
                        cse.back()->Push(gc, tcallable->tag);
                        gc->BarrierEnd();
                        return false;
                    }
                    cse.back()->Push(gc,Undefined());
                    return false;
                }
                
                if(bA != nullptr)
                {
                    if(key == "Count" || key == "Length")
                    {
                        int64_t len = (int64_t)bA->data.size();
                        if(len < 0) len = 0;

                        stk->Push(gc, len);
                        return false;
                    }   
                }
                if(list != nullptr)
                {
                    if(key == "Count" || key == "Length")
                    {
                        int64_t len = list->Count();
                        if(len < 0) len = 0;

                        stk->Push(gc, len);
                        return false;
                    }   
                }
                if(dynList != nullptr)
                {
                    if(key == "Count" || key == "Length")
                    {
                        int64_t len = dynList->Count(ls);
                        if(len < 0) len = 0;

                        stk->Push(gc, len);
                        return false;
                    }   
                }
                if(dynDict != nullptr)
                {
                    if(dynDict->MethodExists(ls,"get" + key))
                    {
                        cse.back()->Push(gc,dynDict->CallMethod(ls,"get" + key, {}));
                    }
                    else
                    {
                        cse.back()->Push(gc, dynDict->GetField(ls,key));
                    }
                    return false;
                }

                if(dict != nullptr)
                {
                    gc->BarrierBegin();
                    TObject fn = dict->GetValue("get" + key);
                    gc->BarrierEnd();
                    if(std::holds_alternative<THeapObjectHolder>(fn) && dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(fn).obj) != nullptr)
                    {
                        return InvokeOne(ls,fn, dict);
                    }
                    else
                    {
                        gc->BarrierBegin();
                       fn = dict->GetValue(key);
                        stk->Push(gc, fn);
                        gc->BarrierEnd();
                        return false;
                    }
                }
           
            }

            stk->Push(gc, Undefined());
        }
        
        return false;
    }
    bool InterperterThread::SetField(GC* gc)
    {
        std::vector<CallStackEntry*>& cse=this->call_stack_entries;

        if(!cse.empty())
        {
            auto stk = cse.back();
            GCList ls(gc);
            TObject value = stk->Pop(ls);
            TObject _key = stk->Pop(ls);
            TObject instance = stk->Pop(ls);
            
            if(!std::holds_alternative<std::string>(_key))
            {
                stk->Push(gc,Undefined());
                return false;
            }

            std::string key = std::get<std::string>(_key);
            if(std::holds_alternative<THeapObjectHolder>(instance))
            {
                auto obj = std::get<THeapObjectHolder>(instance).obj;
                auto vfs = dynamic_cast<TVFSHeapObject*>(obj);
                auto strm = dynamic_cast<TStreamHeapObject*>(obj);
                auto dict = dynamic_cast<TDictionary*>(obj);
                auto dynDict = dynamic_cast<TDynamicDictionary*>(obj);
                auto natObj = dynamic_cast<TNativeObject*>(obj);
                auto cls = dynamic_cast<TClassObject*>(obj);
                if(cls != nullptr)
                {
                    gc->BarrierBegin();
                    auto obj=cls->GetValue(cse.back()->callable->className,"set"+key);
                    gc->BarrierEnd();
                    TClosure* clos;
                    TCallable* callable;
                    if(GetObjectHeap(obj,clos)) 
                    {
                        this->AddCallStackEntry(ls,clos,{value});
                        return true;
                    }
                    else if(GetObjectHeap(obj,callable))
                    {
                        cse.back()->Push(gc,callable->Call(ls,{value}));
                        return false;
                    }
                    gc->BarrierBegin();
                    cls->SetValue(cse.back()->callable->className,key,value);
                    gc->BarrierEnd();
                    cse.back()->Push(gc,value);
                    return false;
               
                }
                else 
                if(natObj != nullptr)
                {
                    cse.back()->Push(gc,natObj->CallMethod(ls,"set"+key,{value}));
                    return false;
                }
                auto tcallable = dynamic_cast<TCallable*>(obj);
                if(tcallable != nullptr)
                {
                    if(key == "Tag")
                    {
                        gc->BarrierBegin();
                        tcallable->tag = value;
                        gc->BarrierEnd();
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                }
                if(strm != nullptr)
                {
                    auto netStrm = dynamic_cast<Tesses::Framework::Streams::NetworkStream*>(strm->stream);
                    auto objStrm = dynamic_cast<TObjectStream*>(strm->stream);
                    if(objStrm != nullptr)
                    {
                        GetObjectHeap(objStrm->obj,dict);
                    }
                    if(netStrm != nullptr && key == "Broadcast")
                    {
                        bool r;
                        if(GetObject(value,r)) netStrm->SetBroadcast(r);
                        cse.back()->Push(gc,nullptr);
                        return false;
                    }
                }
                if(vfs != nullptr)
                {
                    auto d = dynamic_cast<TObjectVFS*>(vfs->vfs);
                    if(d != nullptr)
                    {
                        GetObjectHeap(d->obj,dict);
                        
                    }
                }
                if(dynDict != nullptr)
                {
                    if(dynDict->MethodExists(ls,"set" + key))
                    {
                        cse.back()->Push(gc,dynDict->CallMethod(ls,"set" + key, {value}));
                    }
                    else
                    {
                        dynDict->SetField(ls,key,value);
                        cse.back()->Push(gc,value);
                    }
                    return false;
                }
                if(dict != nullptr)
                {
                    gc->BarrierBegin();
                    TObject fn = dict->GetValue("set" + key);
                    gc->BarrierEnd();
                    if(std::holds_alternative<THeapObjectHolder>(fn) && dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(fn).obj) != nullptr)
                    {
                        return InvokeTwo(ls,fn, dict, value);
                    }
                    else
                    {
                        gc->BarrierBegin();
                        dict->SetValue(key, value);
                        stk->Push(gc, value);
                        gc->BarrierEnd();
                        return false;
                    }
                }
                
            }
            stk->Push(gc, Undefined());
      
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
                            stk->env->DeclareVariable(mkey, value);
                        }
                    }
                    stk->Push(gc, value);
                }
                gc->BarrierEnd();
            }
            else
            {
                throw VMException("[DECLAREVARIABLE] Can't pop string.");
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
            throw VMByteCodeException(gc,_res2);
        }
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
                            catch(TextException& ex)
                            {

                                TDictionary* dict = TDictionary::Create(ls);
                                auto gc = ls.GetGC();
                                gc->BarrierBegin();
                                dict->SetValue("Type","NativeException");
                                dict->SetValue("Text",ex.what());
                                gc->BarrierEnd();
                                stk->Push(gc, catchC->Call(ls,{dict}));
                            }
                            catch(VMException& ex)
                            {
                                
                                TDictionary* dict = TDictionary::Create(ls);
                                auto gc = ls.GetGC();
                                gc->BarrierBegin();
                                dict->SetValue("Type","NativeException");
                                dict->SetValue("Text",ex.what());
                                gc->BarrierEnd();
                                stk->Push(gc, catchC->Call(ls,{dict}));
                            }
                            catch(VMByteCodeException& ex)
                            {

                                stk->Push(gc, catchC->Call(ls,{ex.exception}));
                            }
                            catch(std::exception& ex)
                            {
                                TDictionary* dict = TDictionary::Create(ls);
                                auto gc = ls.GetGC();
                                gc->BarrierBegin();
                                dict->SetValue("Type","NativeException");
                                dict->SetValue("Text",ex.what());
                                gc->BarrierEnd();

                                stk->Push(gc, catchC->Call(ls,{dict}));
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
             catch(VMByteCodeException& ex)
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
                throw ex;
            }
             catch(VMException& ex)
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
                throw ex;
            }
             catch(std::exception& ex)
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
                throw ex;
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
        GC* _gc = ls.GetGC();
        ls.Add(cse);
        _gc->Watch(cse);
        return cse;
    }

    CallStackEntry* CallStackEntry::Create(GCList* ls)
    {
        CallStackEntry* cse = new CallStackEntry();
        cse->mustReturn=false;
        GC* _gc = ls->GetGC();
        ls->Add(cse);
        _gc->Watch(cse);
        return cse;
    }
    void InterperterThread::AddCallStackEntry(GCList& ls, TClosure* closure, std::vector<TObject> args)
    {
        ls.GetGC()->BarrierBegin();
        CallStackEntry* cse = CallStackEntry::Create(ls);
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
            if(i == closure->closure->args.size()-1)
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

    std::string ToString(GC* gc, TObject o)
    {
        if(std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(o))
        {
            return std::get<Tesses::Framework::Filesystem::VFSPath>(o).ToString();
        }
        if(std::holds_alternative<std::string>(o))
        {
            return std::get<std::string>(o);
        }
        if(std::holds_alternative<TVMVersion>(o))
        {
            return std::get<TVMVersion>(o).ToString();
        }
        if(std::holds_alternative<int64_t>(o))
        {
            return std::to_string(std::get<int64_t>(o));
        }

        if(std::holds_alternative<double>(o))
        {
            return std::to_string(std::get<double>(o));
        }

        if(std::holds_alternative<char>(o))
        {
            return std::string{std::get<char>(o)};
        }
        if(std::holds_alternative<std::nullptr_t>(o))
        {
            return "null";
        }
        if(std::holds_alternative<Undefined>(o))
        {
            return "undefined";
        }
        if(std::holds_alternative<bool>(o))
        {
            return std::get<bool>(o) ? "true" : "false";
        }
        if(std::holds_alternative<TDateTime>(o))
        {
            return std::get<TDateTime>(o).GetDate().ToString();
        }
        if(std::holds_alternative<THeapObjectHolder>(o))
        {
            auto obj = std::get<THeapObjectHolder>(o).obj;
            auto dict = dynamic_cast<TDictionary*>(obj);
            auto list = dynamic_cast<TList*>(obj);
            auto bArray = dynamic_cast<TByteArray*>(obj);
            auto natObj = dynamic_cast<TNativeObject*>(obj);
            auto cls = dynamic_cast<TClassObject*>(obj);
            auto aArray = dynamic_cast<TAssociativeArray*>(obj);
            if(aArray != nullptr)
            {

                std::string str={};
                
                gc->BarrierBegin();
                bool first=true;
                for(auto item : aArray->items)
                {
                    if(!first) str.push_back('\n');
                    first=false;
                    str.push_back('[');
                    str.append(Json_Encode(item.first));
                    str.append("] = ");
                    str.append(Json_Encode(item.second));
                    str.append(";");
                }
                gc->BarrierEnd();
                return str;
            }
            if(cls != nullptr)
            {
                auto res = cls->GetValue("","ToString");
                TCallable* call;
                GCList ls(gc);
                if(GetObjectHeap(res,call)) return ToString(gc, call->Call(ls,{}));
                return cls->TypeName();
            }
            if(natObj != nullptr)
            {
                GCList ls(gc);
                TObject o=natObj->CallMethod(ls,"ToString",{});
               

                return ToString(gc, o);
            }
            
            if(dict != nullptr)
            {
                GCList ls(gc);
                if(dict->MethodExists(ls,"ToString"))
                    return ToString(gc,dict->CallMethod(ls,"ToString",{}));
                else
                {
                    return Json_Encode(dict);
                }
            }
            else if(bArray != nullptr)
            {
                return std::string(bArray->data.begin(),bArray->data.end());
            }
            else if(list != nullptr) 
            {
                return Json_Encode(list);
            }
      }

        return "";
    }
}
