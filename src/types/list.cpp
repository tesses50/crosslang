#include "CrossLang.hpp"
namespace Tesses::CrossLang {
    TByteArray* TByteArray::Create(GCList& ls)
    {
         TByteArray* arr=new TByteArray();
        GC* _gc = ls.GetGC();
        ls.Add(arr);
        _gc->Watch(arr);
        return arr;
    }

    TByteArray* TByteArray::Create(GCList* ls)
    {
         TByteArray* arr=new TByteArray();
        GC* _gc = ls->GetGC();
        ls->Add(arr);
        _gc->Watch(arr);
        return arr;
    }
    TList* TList::Create(GCList* gc)
    {
        TList* list=new TList();
        GC* _gc = gc->GetGC();
        gc->Add(list);
        _gc->Watch(list);
        return list;
    }
    TList* TList::Create(GCList& gc)
    {
        TList* list=new TList();
        GC* _gc = gc.GetGC();
        gc.Add(list);
        _gc->Watch(list);
        return list;
    }
    void TList::Add(TObject value)
    {
        this->items.push_back(value);
    }
    void TList::Set(int64_t index, TObject value)
    {
        if(index >= 0 && index < this->Count())
        {
            this->items[index] = value;
        }
    }
    TObject TList::Get(int64_t index)
    {
        if(index >= 0 && index < this->Count())
        {
            return this->items[index];
        }
        return Undefined();
    }
    int64_t TList::Count()
    {
        return (int64_t)this->items.size();
    }
    void TList::Insert(int64_t index, TObject value)
    {
        if(index >= 0 && index <= this->Count())
        {
            this->items.insert(this->items.begin()+index,value);
        }
    }
    void TList::RemoveAt(int64_t index)
    {
        if(index >= 0 && index < this->Count())
        {
            this->items.erase(this->items.begin()+index);
        }
    }
    void TList::Clear()
    {
        this->items.clear();
    }
    void TList::Mark()
    {
        if(this->marked) return;
        this->marked = true;
        for(auto item : this->items)
        {
            GC::Mark(item);
        }
    }
};