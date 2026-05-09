#include "CrossLang.hpp"

namespace Tesses::CrossLang
{
    bool TYieldEnumerator::MoveNext(std::shared_ptr<GC> ls)
    {
        CallStackEntry* ent;
        GCList ls2(ls);
        if(!this->hasStarted)
        {
            TClosure* clos;
            if(!GetObjectHeap(this->enumerator,clos)) return false;
            auto _enumerator= clos->Call(ls2,{});
            ls->BarrierBegin();
            this->enumerator = _enumerator;
            this->hasStarted=true;
            ls->BarrierEnd();
        }
        else {
            
            if(GetObjectHeap(this->enumerator,ent))
            {
                auto _enumerator= ent->Resume(ls2);
                ls->BarrierBegin();
                this->enumerator = _enumerator;
                ls->BarrierEnd();
                
            } else return false;
        }
        
        if(GetObjectHeap(this->enumerator,ent))
        {
            ls->BarrierBegin();
            this->current = ent->Pop(ls2);
            ls->BarrierEnd();
            return true;
        }
        return false;
    }
    TObject TYieldEnumerator::GetCurrent(GCList& ls)
    {
        ls.Add(this->current);
        return this->current;
    }
    void TYieldEnumerator::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        GC::Mark(this->current);
        GC::Mark(this->enumerator);
    }
    TYieldEnumerator* TYieldEnumerator::Create(GCList& ls,TObject v)
    {
        TYieldEnumerator* yieldEnum = new TYieldEnumerator();
        yieldEnum->current=nullptr;
        yieldEnum->hasStarted=false;
        yieldEnum->enumerator = v;
        
        std::shared_ptr<GC> _gc = ls.GetGC();
        ls.Add(yieldEnum);
        _gc->Watch(yieldEnum);
        return yieldEnum;
    }
    TYieldEnumerator* TYieldEnumerator::Create(GCList* ls,TObject v)
    {

        TYieldEnumerator* yieldEnum = new TYieldEnumerator();
        yieldEnum->current=nullptr;
        yieldEnum->hasStarted=false;
        yieldEnum->enumerator = v;
        
        std::shared_ptr<GC> _gc = ls->GetGC();
        ls->Add(yieldEnum);
        _gc->Watch(yieldEnum);
        return yieldEnum;
    }
    
    bool TCustomEnumerator::MoveNext(std::shared_ptr<GC> ls)
    {   
        GCList ls2(ls);
        auto res = this->dict->CallMethod(ls2,"MoveNext",{});
        bool out;
        if(GetObject(res,out)) return out;
        return false;
    }
    TObject TCustomEnumerator::GetCurrent(GCList& ls)
    {
        TObject res=Undefined();
        ls.GetGC()->BarrierBegin();
        auto getCurrent = this->dict->GetValue("getCurrent");
        TCallable* call;
        if(GetObjectHeap(getCurrent,call))
        {
            ls.GetGC()->BarrierEnd();
            res=call->Call(ls,{});
            ls.GetGC()->BarrierBegin();
        }else{

            res=this->dict->GetValue("Current");
        }
        ls.GetGC()->BarrierEnd();
        return res;
    }
    void TCustomEnumerator::Mark()
    {
        if(this->marked) return;
        this->dict->Mark();
    }
    TCustomEnumerator* TCustomEnumerator::Create(GCList* ls, TDictionary* dict)
    {
        TCustomEnumerator* customEnum = new TCustomEnumerator();
        customEnum->dict = dict;
        std::shared_ptr<GC> _gc = ls->GetGC();
        ls->Add(customEnum);
        _gc->Watch(customEnum);
        return customEnum;
    }
    TCustomEnumerator* TCustomEnumerator::Create(GCList& ls, TDictionary* dict)
    {
        TCustomEnumerator* customEnum = new TCustomEnumerator();
        customEnum->dict = dict;
        std::shared_ptr<GC> _gc = ls.GetGC();
        ls.Add(customEnum);
        _gc->Watch(customEnum);
        return customEnum;
    }
    TEnumerator* TEnumerator::CreateFromObject(GCList& ls, TObject obj)
    {
        std::string str;
        TList* mls;
        TDynamicList* dynList;
        TDynamicDictionary* dynDict;
        TDictionary* dict;
        TEnumerator* enumerator;
        TQueryable* q;
        if(GetObject(obj,str))
        {
            return TStringEnumerator::Create(ls, str);
        }
        else if(GetObjectHeap(obj,mls))
        {
            return TListEnumerator::Create(ls,mls);
        }
        else if(GetObjectHeap(obj,dynList))
        {
            return TDynamicListEnumerator::Create(ls,dynList);
        }
        else if(GetObjectHeap(obj,dict))
        {
            auto res=dict->CallMethod(ls,"GetEnumerator",{});
            if(GetObjectHeap(res,dict))
            {
                return TCustomEnumerator::Create(ls,dict);
            }
            else if(GetObjectHeap(res,enumerator))
            {
                return enumerator;
            }
        }
        else if(GetObjectHeap(obj, q))
        {
            return q->GetEnumerator(ls);
        }
        else if(GetObjectHeap(obj, enumerator))
        {
            return enumerator;
        }
        return nullptr;
    }
    TVFSPathEnumerator* TVFSPathEnumerator::Create(GCList& ls, Tesses::Framework::Filesystem::VFSPathEnumerator enumerator)
    {
        TVFSPathEnumerator* vfspathe = new TVFSPathEnumerator();
        vfspathe->enumerator = enumerator;
        std::shared_ptr<GC> _gc = ls.GetGC();
        ls.Add(vfspathe);
        _gc->Watch(vfspathe);
        return vfspathe;
    }
    TVFSPathEnumerator* TVFSPathEnumerator::Create(GCList* ls, Tesses::Framework::Filesystem::VFSPathEnumerator enumerator)
    {
        TVFSPathEnumerator* vfspathe = new TVFSPathEnumerator();
        vfspathe->enumerator = enumerator;
        std::shared_ptr<GC> _gc = ls->GetGC();
        ls->Add(vfspathe);
        _gc->Watch(vfspathe);
        return vfspathe;
    }
    bool TVFSPathEnumerator::MoveNext(std::shared_ptr<GC> ls)
    {
        return enumerator.MoveNext();
    }
    TObject TVFSPathEnumerator::GetCurrent(GCList& ls)
    {
        return enumerator.Current;
    }
    TDictionaryEnumerator* TDictionaryEnumerator::Create(GCList& ls, TDictionary* dict)
    {
        TDictionaryEnumerator* dicte=new TDictionaryEnumerator();
        dicte->dict = dict;
        dicte->hasStarted=false;
        std::shared_ptr<GC> _gc = ls.GetGC();
        ls.Add(dicte);
        _gc->Watch(dicte);
        return dicte;
    }
    TDictionaryEnumerator* TDictionaryEnumerator::Create(GCList* ls, TDictionary* dict)
    {
        TDictionaryEnumerator* dicte=new TDictionaryEnumerator();
        dicte->dict = dict;
        dicte->hasStarted=false;
        std::shared_ptr<GC> _gc = ls->GetGC();
        ls->Add(dicte);
        _gc->Watch(dicte);
        return dicte;
    }
    
    bool TDictionaryEnumerator::MoveNext(std::shared_ptr<GC> ls)
    {
        if(!this->hasStarted)
        {
            this->hasStarted=true;
            this->ittr = this->dict->items.begin();
            return !this->dict->items.empty();
        }
        else
        {
            this->ittr++;
            return this->ittr != this->dict->items.end();
        }
    }
    TObject TDictionaryEnumerator::GetCurrent(GCList& ls)
    {
        if(!this->hasStarted) return Undefined();
        if(this->ittr != this->dict->items.end())
        {
            ls.GetGC()->BarrierBegin();
            std::string key = this->ittr->first;
            TObject value = this->ittr->second;
            auto kvp = TDictionary::Create(ls);
            kvp->SetValue("Key",key);
            kvp->SetValue("Value",value);
            ls.GetGC()->BarrierEnd();
            return kvp;
        }
        return Undefined();
    }
    void TDictionaryEnumerator::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        this->dict->Mark();
    }


    TListEnumerator* TListEnumerator::Create(GCList& ls, TList* list)
    {
        TListEnumerator* liste=new TListEnumerator();
        liste->ls = list;
        liste->index = -1;
        std::shared_ptr<GC> _gc = ls.GetGC();
        ls.Add(liste);
        _gc->Watch(liste);
        return liste;
    }
    TListEnumerator* TListEnumerator::Create(GCList* ls, TList* list)
    {
        TListEnumerator* liste=new TListEnumerator();
        liste->ls = list;
        liste->index = -1;
        std::shared_ptr<GC> _gc = ls->GetGC();
        ls->Add(liste);
        _gc->Watch(liste);
        return liste;
    }
    bool TListEnumerator::MoveNext(std::shared_ptr<GC> ls)
    {
        this->index++;
        return this->index >= 0 && this->index < this->ls->Count();
    }
    TObject TListEnumerator::GetCurrent(GCList& ls)
    {
        
        if(this->index < -1) return nullptr;
        if(this->ls->Count() == 0) return nullptr;
        if(this->index >= this->ls->Count()) return nullptr;
        ls.GetGC()->BarrierBegin();
        TObject o = this->ls->Get(index);
        ls.GetGC()->BarrierEnd();
        return o;
    }
    void TListEnumerator::Mark()
    {
        if(this->marked) return;
        this->marked = true;
        this->ls->Mark();
    }

    TAssociativeArrayEnumerator* TAssociativeArrayEnumerator::Create(GCList& ls, TAssociativeArray* list)
    {
        TAssociativeArrayEnumerator* liste=new TAssociativeArrayEnumerator();
        liste->ls = list;
        liste->index = -1;
        std::shared_ptr<GC> _gc = ls.GetGC();
        ls.Add(liste);
        _gc->Watch(liste);
        return liste;
    }
    TAssociativeArrayEnumerator* TAssociativeArrayEnumerator::Create(GCList* ls, TAssociativeArray* list)
    {
        TAssociativeArrayEnumerator* liste=new TAssociativeArrayEnumerator();
        liste->ls = list;
        liste->index = -1;
        std::shared_ptr<GC> _gc = ls->GetGC();
        ls->Add(liste);
        _gc->Watch(liste);
        return liste;
    }
    bool TAssociativeArrayEnumerator::MoveNext(std::shared_ptr<GC> ls)
    {
        this->index++;
        return this->index >= 0 && this->index < this->ls->Count();
    }
    TObject TAssociativeArrayEnumerator::GetCurrent(GCList& ls)
    {
        
        if(this->index < -1) return nullptr;
        if(this->ls->Count() == 0) return nullptr;
        if(this->index >= this->ls->Count()) return nullptr;
        ls.GetGC()->BarrierBegin();
        TDictionary* dict = TDictionary::Create(ls);
        dict->SetValue("Key",this->ls->GetKey(this->index));
        dict->SetValue("Value",this->ls->GetValue(this->index));
        ls.GetGC()->BarrierEnd();
        return dict;
    }
    void TAssociativeArrayEnumerator::Mark()
    {
        if(this->marked) return;
        this->marked = true;
        this->ls->Mark();
    }



    TDynamicListEnumerator* TDynamicListEnumerator::Create(GCList& ls, TDynamicList* list)
    {
        TDynamicListEnumerator* liste=new TDynamicListEnumerator();
        liste->ls = list;
        liste->index = -1;
        std::shared_ptr<GC> _gc = ls.GetGC();
        ls.Add(liste);
        _gc->Watch(liste);
        return liste;
    }
    TDynamicListEnumerator* TDynamicListEnumerator::Create(GCList* ls, TDynamicList* list)
    {
        TDynamicListEnumerator* liste=new TDynamicListEnumerator();
        liste->ls = list;
        liste->index = -1;
        std::shared_ptr<GC> _gc = ls->GetGC();
        ls->Add(liste);
        _gc->Watch(liste);
        return liste;
    }
    bool TDynamicListEnumerator::MoveNext(std::shared_ptr<GC> ls)
    {
        this->index++;
        GCList ls2(ls);
        return this->index >= 0 && this->index < this->ls->Count(ls2);
    }
    TObject TDynamicListEnumerator::GetCurrent(GCList& ls)
    {
        
        if(this->index < -1) return nullptr;
        auto r = this->ls->Count(ls);
        if(r == 0) return nullptr;
        if(this->index >= r) return nullptr;
        ls.GetGC()->BarrierBegin();
        TObject o = this->ls->GetAt(ls,index);
        ls.GetGC()->BarrierEnd();
        return o;
    }
    void TDynamicListEnumerator::Mark()
    {
        if(this->marked) return;
        this->marked = true;
        this->ls->Mark();
    }


    TStringEnumerator* TStringEnumerator::Create(GCList& ls,std::string str)
    {
        TStringEnumerator* stre=new TStringEnumerator();
        stre->str = str;
        stre->hasStarted=false;
        std::shared_ptr<GC> _gc = ls.GetGC();
        ls.Add(stre);
        _gc->Watch(stre);
        return stre;
    }
    TStringEnumerator* TStringEnumerator::Create(GCList* ls,std::string str)
    {
        TStringEnumerator* stre=new TStringEnumerator();
        stre->str = str;
        stre->hasStarted=false;
        std::shared_ptr<GC> _gc = ls->GetGC();
        ls->Add(stre);
        _gc->Watch(stre);
        return stre;
    }
    bool TStringEnumerator::MoveNext(std::shared_ptr<GC> ls)
    {
        if(!this->hasStarted)
        {
            this->hasStarted=true;
            this->index = 0;
            return !this->str.empty();
        }
        else
        {
            if(this->index >= this->str.size()) return false;
            this->index++;
            return this->index < this->str.size();
        }
    }
    TObject TStringEnumerator::GetCurrent(GCList& ls)
    {
        if(!this->hasStarted) return nullptr;
        if(this->index < this->str.size()) return this->str[this->index];
        return nullptr;
    }

};