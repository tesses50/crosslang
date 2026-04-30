#include "CrossLang.hpp"

namespace Tesses::CrossLang 
{
    TNativeObject::~TNativeObject()
    {

    }

    TNative::TNative(void* ptr,std::function<void(void*)> destroy)
    {
        this->ptr = ptr;
        this->destroyed=false;
        this->destroy = destroy;
        
    }
    bool TNative::GetDestroyed()
    {
        return this->destroyed;
    }
    void* TNative::GetPointer()
    {
        return this->ptr;
    }
    void TNative::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        
            GC::Mark(this->other);
        
    }
    void TNative::Destroy()
    {
        if(this->destroyed) return;
        if(this->destroy != nullptr)
        {
            this->destroyed=true;
            this->destroy(this->ptr);
        }
    }
    bool TNativeObject::ToBool()
    {
        return true;
    }
    bool TNativeObject::Equals(std::shared_ptr<GC> gc, TObject right)
    {
        if(std::holds_alternative<THeapObjectHolder>(right))
        {
            return this == std::get<THeapObjectHolder>(right).obj;
        }
        return false;
    }
    TNative* TNative::Create(GCList& ls, void* ptr,std::function<void(void*)> destroy)
    {
        TNative* native = new TNative(ptr,destroy);
        std::shared_ptr<GC> gc = ls.GetGC();
        ls.Add(native);
        gc->Watch(native);
        return native;
    }
    TNative* TNative::Create(GCList* ls, void* ptr,std::function<void(void*)> destroy)
    {
        TNative* native = new TNative(ptr,destroy);
        std::shared_ptr<GC> gc = ls->GetGC();
        ls->Add(native);
        gc->Watch(native);
        return native;
    }
    TNative::~TNative()
    {
        this->Destroy();
    }
    
}
