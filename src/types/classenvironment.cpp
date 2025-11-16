#include "CrossLang.hpp"
namespace Tesses::CrossLang {
    bool TClassEnvironment::HasConstForSet(std::string key)
    {
        if(this->env->HasVariableRecurse(key))
        {
            return this->env->HasConstForSet(key);
        }
        return false;
    }
    TClassEnvironment* TClassEnvironment::Create(GCList* gc,TEnvironment* env,TClassObject* obj)
    {
        
        TClassEnvironment* env2=new TClassEnvironment(env,obj);
        
        GC* _gc = gc->GetGC();
        gc->Add(env2);
        _gc->Watch(env2);
        return env2;
    }
    TClassEnvironment* TClassEnvironment::Create(GCList& gc,TEnvironment* env,TClassObject* obj)
    {

        TClassEnvironment* env2=new TClassEnvironment(env,obj);
        
        GC* _gc = gc.GetGC();
        gc.Add(env2);
        _gc->Watch(env2);
        return env2;
    }
    TClassEnvironment::TClassEnvironment(TEnvironment* env,TClassObject* obj)
    {
        this->env = env;
        this->clsObj=obj;
    }
    bool TClassEnvironment::HasVariable(std::string key)
    {
        if(key == "this") return true;
        if(this->clsObj->HasValue(current_function == nullptr ? "" : current_function->callable->className,key)) return true;
        return false;
    }
    bool TClassEnvironment::HasVariableRecurse(std::string key)
    {
        if(HasVariable(key)) return true;
        return this->env->HasVariableRecurse(key);
    }
    bool TClassEnvironment::HasVariableOrFieldRecurse(std::string key, bool setting)
    {
        if(key == "this") return true;
        std::string clsName = current_function == nullptr ? "" : current_function->callable->className;
        if(clsObj->HasMethod(clsName,(setting ? "set" : "get")+key)) return true;
        if(clsObj->HasValue(clsName,key)) return true;
        return env->HasVariableOrFieldRecurse(key,setting);
    }
            
    TObject TClassEnvironment::GetVariable(std::string key)
    {
        if(key == "this") return this->clsObj;
        
        std::string clsName = current_function == nullptr ? "" : current_function->callable->className;
       
        if(clsObj->HasValue(clsName,key)) return this->clsObj->GetValue(clsName,key);
        return env->GetVariable(key);
    }
    void TClassEnvironment::SetVariable(std::string key, TObject value)
    {
        if(key == "this") return;
        
        std::string clsName = current_function == nullptr ? "" : current_function->callable->className;
       
        if(clsObj->HasValue(clsName,key)) 
        {
            this->clsObj->SetValue(clsName,key,value);
            return;
        }
        this->env->SetVariable(key,value);
        return;
    }
    TObject TClassEnvironment::GetVariable(GCList& ls, std::string key)
    {
        if(key == "this") return this->clsObj;
        
        std::string clsName = current_function == nullptr ? "" : current_function->callable->className;
        if(this->clsObj->HasMethod(clsName,"get"+key)) 
        {
            auto res=this->clsObj->GetValue(clsName,"get"+key);
            TCallable* call;
            if(GetObjectHeap(res,call)) return call->Call(ls,{});
        }
        if(this->clsObj->HasValue(clsName,key)) return this->clsObj->GetValue(clsName,key);
        return this->env->GetVariable(ls,key);
    }
    TObject TClassEnvironment::SetVariable(GCList& ls, std::string key, TObject v)
    {
        if(key == "this") return this->clsObj;
        
        std::string clsName = current_function == nullptr ? "" : current_function->callable->className;
        if(this->clsObj->HasMethod(clsName,"set"+key)) 
        {
            auto res=this->clsObj->GetValue(clsName,"set"+key);
            TCallable* call;
            if(GetObjectHeap(res,call)) return call->Call(ls,{v});
        }
        if(this->clsObj->HasValue(clsName,key)) { this->clsObj->SetValue(clsName,key,v); return v;}
        
        return this->env->SetVariable(ls,key,v);
    }
            
    void TClassEnvironment::DeclareVariable(std::string key, TObject value)
    {

    }
    TRootEnvironment* TClassEnvironment::GetRootEnvironment()
    {
        return this->env->GetRootEnvironment();
    }
    TEnvironment* TClassEnvironment::GetParentEnvironment()
    {
        return this->env;
    }
            
    void TClassEnvironment::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        this->clsObj->Mark();
        this->env->Mark();
        for(auto item : this->defers) item->Mark();
    }
}