#include "CrossLang.hpp"

namespace Tesses::CrossLang  {
    TObject TDictionary::CallMethod(GCList& ls, std::string key, std::vector<TObject> args)
    {
        ls.GetGC()->BarrierBegin();
        auto res = this->GetValue(key);
        ls.GetGC()->BarrierEnd();
        TCallable* callable;

            if(GetObjectHeap(res,callable))
            {
                 auto closure = dynamic_cast<TClosure*>(callable);
                if(closure != nullptr && !closure->closure->args.empty() && closure->closure->args.front() == "this") 
                {
                   std::vector<TObject> args2;
                        args2.push_back(this);
                        args2.insert(args2.end(), args.begin(),args.end());
                    return closure->Call(ls,args2);
                    
                }
                else
                {
                    return callable->Call(ls,args);
                    
                }
            }
        return Undefined();
    }
    void TDictionary::DeclareFunction(GC* gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb)
    {
        gc->BarrierBegin();
        GCList ls(gc);
        this->SetValue(key, TExternalMethod::Create(ls,documentation,argNames,cb));
        gc->BarrierEnd();
    }
    void TDictionary::DeclareFunction(GC& gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb)
    {
        gc.BarrierBegin();
        GCList ls(gc);
        this->SetValue(key, TExternalMethod::Create(ls,documentation,argNames,cb));
        gc.BarrierEnd();
    }
    void TDictionary::DeclareFunction(GC* gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb,std::function<void()> destroy)
    {
        gc->BarrierBegin();
        GCList ls(gc);
        this->SetValue(key, TExternalMethod::Create(ls,documentation,argNames,cb,destroy));
        gc->BarrierEnd();
    }
    void TDictionary::DeclareFunction(GC& gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb,std::function<void()> destroy)
    {
        gc.BarrierBegin();
        GCList ls(gc);
        this->SetValue(key, TExternalMethod::Create(ls,documentation,argNames,cb,destroy));
        gc.BarrierEnd();
    }
    TObject TDictionary::GetValue(std::string key)
    {
        if(this->items.empty()) return Undefined();
        for(auto item : this->items)
        {
            if(item.first == key) return item.second;
        }
        return Undefined();
    }
    void TDictionary::SetValue(std::string key, TObject value)
    {
        if(std::holds_alternative<Undefined>(value))
        {
            if(this->items.count(key) > 0)
                this->items.erase(key);
        }
        else
        {
            this->items[key] = value;
        }
    }
    bool TDictionary::HasValue(std::string key)
    {
        return this->items.count(key) > 0;
    }
    void TDictionary::Mark()
    {
        if(this->marked) return;
        this->marked = true;
        for(auto item : this->items)
        {
            GC::Mark(item.second);
        }
    }
    TDictionary* TDictionary::Create(GCList* gc)
    {
        TDictionary* dict=new TDictionary();
        GC* _gc = gc->GetGC();
        gc->Add(dict);
        _gc->Watch(dict);
        return dict;
    }
    TDictionary* TDictionary::Create(GCList& gc)
    {
        TDictionary* dict=new TDictionary();
        GC* _gc = gc.GetGC();
        gc.Add(dict);
        _gc->Watch(dict);
        return dict;
    }
};