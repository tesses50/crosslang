#include "CrossLang.hpp"

namespace Tesses::CrossLang  {
    TDynamicDictionary* TDynamicDictionary::Create(GCList& ls,TCallable* callable)
    {

        TDynamicDictionary* dict=new TDynamicDictionary();
        dict->cb = callable;
        GC* _gc = ls.GetGC();
        ls.Add(dict);
        _gc->Watch(dict);
        return dict;
    }
    TDynamicDictionary* TDynamicDictionary::Create(GCList* ls,TCallable* callable)
    {
        TDynamicDictionary* dict=new TDynamicDictionary();
        dict->cb = callable;
        GC* _gc = ls->GetGC();
        ls->Add(dict);
        _gc->Watch(dict);
        return dict;
    }

    void TDynamicDictionary::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        this->cb->Mark();
    }

    TObject TDynamicDictionary::GetField(GCList& ls, std::string key)
    {
        auto dict = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();
        dict->SetValue("Type", "GetField");
        dict->SetValue("Key", key);
        ls.GetGC()->BarrierEnd();
        return this->cb->Call(ls,{dict});
    }

    TObject TDynamicDictionary::SetField(GCList& ls, std::string key, TObject value)
    {
        auto dict = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();
        dict->SetValue("Type", "SetField");
        dict->SetValue("Key", key);
        dict->SetValue("Value", value);
        ls.GetGC()->BarrierEnd();
        return this->cb->Call(ls,{dict});
    }

    TObject TDynamicDictionary::CallMethod(GCList& ls, std::string name, std::vector<TObject> args)
    {
        auto dict = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();
        dict->SetValue("Type", "CallMethod");
        dict->SetValue("Name", name);
        auto argVal = TList::Create(ls);
        argVal->items = args;
        dict->SetValue("Arguments", argVal);
        ls.GetGC()->BarrierEnd();
        return this->cb->Call(ls,{dict});
    }

    TEnumerator* TDynamicDictionary::GetEnumerator(GCList& ls)
    {
        auto dict = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();
        dict->SetValue("Type", "GetEnumerator");

        ls.GetGC()->BarrierEnd();

        return TEnumerator::CreateFromObject(ls,this->cb->Call(ls,{dict}));
    }
    bool TDictionary::MethodExists(GCList& ls,std::string method)
    {
        ls.GetGC()->BarrierBegin();
        auto r = this->GetValue(method);
        TCallable* callable;
        bool res = GetObjectHeap(r,callable);
        ls.GetGC()->BarrierEnd();
        return res;
    }
    bool TDynamicDictionary::MethodExists(GCList& ls,std::string name)
    {
        auto dict = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();
        dict->SetValue("Type", "MethodExists");
        dict->SetValue("Name", name);

        ls.GetGC()->BarrierEnd();

        auto res = this->cb->Call(ls,{dict});
        bool r2;
        if(GetObject(res,r2)) return r2;
        return false;
    }

    TDynamicDictionary::~TDynamicDictionary()
    {

    }
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