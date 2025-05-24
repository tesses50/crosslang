#include "CrossLang.hpp"
namespace Tesses::CrossLang
{
    std::string TClassObject::TypeName()
    {
        std::string type = "class ";
        type += this->name;
        for(auto& item : this->inherit_tree) type += " : " + item;
        return type;
    }
    TClassObjectEntry* TClassObject::GetEntry(std::string classN, std::string key)
    {
        for(auto& item : this->entries)
        {
            if(item.name == key)
            {
                switch(item.modifier)
                {
                    case TClassModifier::Private:
                        if(classN != item.owner) return nullptr;
                        break;
                    case TClassModifier::Protected:
                        if(classN.empty()) return nullptr;
                        if(classN != item.owner)
                        {
                            for(auto inh : this->inherit_tree)
                            {
                               if(inh == classN) {
                                    
                                    return &item;
                                }
                            }
                            return nullptr;
                        }
                            
                        break;
                }
                return &item;
            }
        }
        return nullptr;
    }
    TClassObject* TClassObject::Create(GCList& ls, TFile* f, uint32_t classIndex, TEnvironment* env, std::vector<TObject> args)
    {
        return Create(&ls,f,classIndex,env,args);
    }
    std::string JoinPeriod(std::vector<std::string>& p)
    {
        std::string newStr = "";
        for(size_t i = 0; i < p.size(); i++)
        {
            if(i > 0) newStr.push_back('.');
            newStr += p[i];
        }
        return newStr;
    }
    TClassObject* TClassObject::Create(GCList* ls, TFile* f, uint32_t classIndex, TEnvironment* env, std::vector<TObject> args)
    {
        if(ls == nullptr) return nullptr;
        TClassObject* obj = new TClassObject();
        obj->file = f;
        obj->classIndex = classIndex;
        obj->ogEnv=env;
        obj->env = TClassEnvironment::Create(ls,env,obj);
        obj->name = JoinPeriod(f->classes[classIndex].name);
        bool hasToString=false;
        
        for(auto entry : f->classes[classIndex].entry)
        {
            if(entry.modifier == TClassModifier::Static) continue;
            TClassObjectEntry ent;
            ent.name = entry.name;
            ent.modifier = entry.modifier;
            ent.canSet = !entry.isFunction;
            ent.owner = obj->name;
            
            if(entry.isFunction)
            {
                if(ent.name == "ToString") hasToString=true;
                if(entry.isAbstract) 
                {
                    delete obj;
                    throw VMException("Method " + ent.name +  " in " + ent.owner + " is abstract.");
                }
                else
                {
                   auto clos = TClosure::Create(ls,obj->env,obj->file,entry.chunkId);
                   clos->documentation = entry.documentation;
                   clos->className = ent.owner;
                   ent.value = clos;
                }
            }
            else {
                if(entry.isAbstract) ent.value = Undefined();
                else {
                    auto clos = TClosure::Create(ls,obj->env,obj->file,entry.chunkId);
                    
                    clos->className=ent.owner;
                    ent.value = clos->Call(*ls,{});
                }
            }
            
            obj->entries.push_back(ent);
        }
        
        TClass* clsCur = &f->classes[classIndex];
        TRootEnvironment* rEnv = env->GetRootEnvironment();
        while(!clsCur->inherits.empty() && !(clsCur->inherits.size() == 1 && clsCur->inherits[0] == "ClassObject"))
        {
            obj->inherit_tree.push_back(JoinPeriod(clsCur->inherits));
            size_t idx;
            if(rEnv->TryFindClass(clsCur->name,idx))
            {
                auto file = rEnv->classes[idx].first;
                clsCur = &rEnv->classes[idx].first->classes.at(rEnv->classes[idx].second);
                auto ownerNow = JoinPeriod(clsCur->name);
                for(auto entry : clsCur->entry)
                {
                    if(entry.modifier == TClassModifier::Static) continue;
                    bool cont=false;
                    for(auto e : obj->entries)
                        if(e.name == entry.name)
                        {
                            cont=true;
                            break;
                        }
                    if(cont) continue;

                    TClassObjectEntry ent;
                    ent.name = entry.name;
                    ent.modifier = entry.modifier;
                    ent.canSet = !entry.isFunction;
                    ent.owner = ownerNow;
            
                if(entry.isFunction)
                {

                    if(ent.name == "ToString") hasToString=true;
                    if(entry.isAbstract) 
                    {
                        delete obj;
                        throw VMException("Method " + ent.name +  " in " + ownerNow + " is abstract.");
                    }
                    else
                    {
                        auto clos = TClosure::Create(ls,obj->env,file,entry.chunkId);
                        clos->className = ownerNow;
                        clos->documentation = entry.documentation;
                        ent.value = clos;
                    }
                }
                else {
                    if(entry.isAbstract) ent.value = Undefined();
                    else {
                        auto clos = TClosure::Create(ls,obj->env,file,entry.chunkId);
                        clos->className = ownerNow;
                        ent.value = clos->Call(*ls,{});
                    }
                }   
            
                obj->entries.push_back(ent);
            }
        
            }
        }
        if(!hasToString)
        {
            TClassObjectEntry ent;
            ent.canSet=false;
            ent.modifier = TClassModifier::Public;
            ent.name = "ToString";
            ent.owner = obj->name;
            ent.value = TExternalMethod::Create(ls,"The ToString",{},[obj](GCList& ls,std::vector<TObject> args)->TObject { return obj->TypeName();});
            obj->entries.push_back(ent);
        }
        TCallable* call=nullptr;
        std::string fnName = f->classes[classIndex].name[f->classes[classIndex].name.size()-1];
        for(auto& item : obj->entries)
            if(item.name == fnName)
            {
                GetObjectHeap(item.value,call);
                break;
            }
        ls->Add(obj);
        ls->GetGC()->Watch(obj);
        
        if(call != nullptr) call->Call(*ls,args);
        return obj;
    }

    TObject TClassObject::GetValue(std::string className, std::string key)
    {
        auto ent = GetEntry(className,key);
        if(ent == nullptr) return Undefined();
        return ent->value;
    }
    void TClassObject::SetValue(std::string className, std::string key,TObject value)
    {
        auto ent = GetEntry(className,key);
        if(ent == nullptr) return;
        if(ent->canSet) ent->value = value;
    }
    bool TClassObject::HasValue(std::string className,std::string key)
    {
        auto ent = GetEntry(className,key);
        return ent != nullptr;
        
    }
    bool TClassObject::HasField(std::string className,std::string key)
    {
        auto ent = GetEntry(className,key);
        if(ent == nullptr) return false;
        return ent->canSet;
    }
    bool TClassObject::HasMethod(std::string className,std::string key)
    {
        auto ent = GetEntry(className,key);
        if(ent == nullptr) return false;
        TCallable* call;
        return GetObjectHeap(ent->value,call);
    }
    void TClassObject::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        this->env->Mark();
        this->file->Mark();
        this->ogEnv->Mark();
        for(auto& item : this->entries) GC::Mark(item.value);
        
    }
}