#include "CrossLang.hpp"

namespace Tesses::CrossLang  {
    TDictionary* TSubEnvironment::GetDictionary()
    {
        return this->dict;
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
