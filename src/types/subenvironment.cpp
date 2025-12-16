#include "CrossLang.hpp"

namespace Tesses::CrossLang  {
    TDictionary* TSubEnvironment::GetDictionary()
    {
        return this->dict;
    }
    bool TSubEnvironment::HasVariableOrFieldRecurse(std::string key,bool setting)
    {
        std::string property=(setting? "set":"get") + key;
        if(this->HasVariable(property))
        {
            auto res = this->GetVariable(property);
            TCallable* callable;
            if(GetObjectHeap(res,callable)) return true;
        }

        if(this->HasVariable(key)) return true;

        return this->env->HasVariableOrFieldRecurse(key,setting);
    }
    TObject TSubEnvironment::GetVariable(GCList& ls, std::string key)
    {
        ls.GetGC()->BarrierBegin();
        if(this->HasVariable("get" + key))
        {
            auto item = this->GetVariable("get"+key);
            TCallable* callable;
            if(GetObjectHeap(item,callable))
            {
                ls.GetGC()->BarrierEnd();
                return callable->Call(ls,{});
            }
        }
        
        if(this->HasVariable(key))
        {
            auto item = this->GetVariable(key);
            ls.GetGC()->BarrierEnd();
            return item;
        }
        if(this->env->HasVariableOrFieldRecurse(key))
        {
            ls.GetGC()->BarrierEnd();
            return this->env->GetVariable(ls,key);
        }
        ls.GetGC()->BarrierEnd();
        
        
        
        return Undefined();
    }
    bool TSubEnvironment::HasConstForSet(std::string key)
    {
        if(this->dict->HasValue(key))
        {
            return this->HasConstForDeclare(key);
        }
        if(this->env->HasVariableRecurse(key))
        {
            return this->env->HasConstForSet(key);
        }
        return false;
    }
    TObject TSubEnvironment::SetVariable(GCList& ls, std::string key, TObject value)
    {
        ls.GetGC()->BarrierBegin();
        if(this->HasVariable("set" + key))
        {
            auto item = this->GetVariable("set"+key);
            TCallable* callable;
            if(GetObjectHeap(item,callable))
            {
                ls.GetGC()->BarrierEnd();
                return callable->Call(ls,{value});
            }
        }
        
        if(this->HasVariable(key))
        {
            this->SetVariable(key,value);
            ls.GetGC()->BarrierEnd();
            return value;
        }
        if(this->env->HasVariableOrFieldRecurse(key,true))
        {
            ls.GetGC()->BarrierEnd();
            return this->env->SetVariable(ls,key,value);
        }
        else
        {
            this->env->SetVariable(key,value);

            ls.GetGC()->BarrierEnd();
            return value;
        }
        ls.GetGC()->BarrierEnd();
        
        
    }
            
    TObject TSubEnvironment::GetVariable(std::string key)
    {
        if(this->dict->HasValue(key))
        {
            return this->dict->GetValue(key);
        }
        if(this->env->HasVariableRecurse(key))
        {
            return this->env->GetVariable(key);
        }
        return Undefined();
    }
    void TSubEnvironment::DeclareVariable(std::string key,TObject value)
    {
        this->dict->SetValue(key,value);
    }
    void TSubEnvironment::SetVariable(std::string key, TObject value)
    {
        if(this->dict->HasValue(key))
        {
            this->dict->SetValue(key,value);
            return;
        }
        if(this->env->HasVariableRecurse(key))
        {
            this->env->SetVariable(key,value);
        }
        else
        {
            this->dict->SetValue(key,value);
        }
    }
    bool TSubEnvironment::HasVariable(std::string key)
    {
        return this->dict->HasValue(key);
    }
    bool TSubEnvironment::HasVariableRecurse(std::string key)
    {
        if(this->dict->HasValue(key)) return true;
        return this->env->HasVariableRecurse(key);
    }
    TSubEnvironment::TSubEnvironment(TEnvironment* env,TDictionary* dict)
    {
        this->env = env;
        this->dict = dict;
    }
    TSubEnvironment* TEnvironment::GetSubEnvironment(TDictionary* dict)
    {
        TSubEnvironment* subEnv = new TSubEnvironment(this,dict);
        return subEnv;
    }
    TObject TEnvironment::CallFunction(GCList& ls, std::string key, std::vector<TObject> args)
    {
        ls.GetGC()->BarrierBegin();
        auto res = this->GetVariable(key);
        ls.GetGC()->BarrierEnd();
        TCallable* callable;

        if(GetObjectHeap(res,callable))
        {
            return callable->Call(ls,args);
        }
        return Undefined();
    }
    TObject TEnvironment::CallFunctionWithFatalError(GCList& ls, std::string key, std::vector<TObject> args)
    {
        ls.GetGC()->BarrierBegin();
        auto res = this->GetVariable(key);
        ls.GetGC()->BarrierEnd();
        TCallable* callable;

        if(GetObjectHeap(res,callable))
        {
            return callable->CallWithFatalError(ls,args);
        }
        return Undefined();
    }
    TSubEnvironment* TEnvironment::GetSubEnvironment(GCList* gc)
    {
        auto dict=TDictionary::Create(gc);
        TSubEnvironment* sEnv = this->GetSubEnvironment(dict);
        GC* _gc = gc->GetGC();
        gc->Add(sEnv);
        _gc->Watch(sEnv);
        return sEnv;
    }
    TSubEnvironment* TEnvironment::GetSubEnvironment(GCList& gc)
    {
        auto dict=TDictionary::Create(gc);
        TSubEnvironment* sEnv = this->GetSubEnvironment(dict);
        GC* _gc = gc.GetGC();
        gc.Add(sEnv);
        _gc->Watch(sEnv);
        return sEnv;
    }
    TSubEnvironment* TSubEnvironment::Create(GCList* gc, TEnvironment* env, TDictionary* dict)
    {
        TSubEnvironment* senv = new TSubEnvironment(env,dict);
        GC* _gc = gc->GetGC();
        gc->Add(senv);
        _gc->Watch(senv);
        return senv;
    }
    TSubEnvironment* TSubEnvironment::Create(GCList& gc, TEnvironment* env, TDictionary* dict)
    {
        TSubEnvironment* senv = new TSubEnvironment(env,dict);
        GC* _gc = gc.GetGC();
        gc.Add(senv);
        _gc->Watch(senv);
        return senv;
    }
    TEnvironment* TSubEnvironment::GetParentEnvironment()
    {
        return this->env;
    }
    TRootEnvironment* TSubEnvironment::GetRootEnvironment()
    {
        return this->env->GetRootEnvironment();
    }
    
    void TSubEnvironment::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        this->dict->Mark();
        this->env->Mark();
        for(auto defer : defers) defer->Mark();
    }
    void TEnvironment::DeclareFunction(GC* gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb)
    {
        gc->BarrierBegin();
        GCList ls(gc);
        this->DeclareVariable(key, TExternalMethod::Create(ls,documentation,argNames,cb));
        gc->BarrierEnd();
    }
    void TEnvironment::DeclareFunction(GC& gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb)
    {
        gc.BarrierBegin();
        GCList ls(gc);
        this->DeclareVariable(key, TExternalMethod::Create(ls,documentation,argNames,cb));
        gc.BarrierEnd();
    }
    void TEnvironment::DeclareFunction(GC* gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb,std::function<void()> destroy)
    {
        gc->BarrierBegin();
        GCList ls(gc);
        this->DeclareVariable(key, TExternalMethod::Create(ls,documentation,argNames,cb,destroy));
        gc->BarrierEnd();
    }
    void TEnvironment::DeclareFunction(GC& gc,std::string key,std::string documentation, std::vector<std::string> argNames, std::function<TObject(GCList& ls, std::vector<TObject> args)> cb,std::function<void()> destroy)
    {
        gc.BarrierBegin();
        GCList ls(gc);
        this->DeclareVariable(key, TExternalMethod::Create(ls,documentation,argNames,cb,destroy));
        gc.BarrierEnd();
    }
};
