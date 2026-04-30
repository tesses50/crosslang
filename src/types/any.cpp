#include "CrossLang.hpp"

namespace Tesses::CrossLang {
    TAny* TAny::Create(GCList& ls)
    {
        TAny* anyObj = new TAny();
        
        std::shared_ptr<GC> gc = ls.GetGC();
        ls.Add(anyObj);
        gc->Watch(anyObj);
        return anyObj;
    }
    TAny* TAny::Create(GCList* ls)
    {
         TAny* anyObj = new TAny();
        
        std::shared_ptr<GC> gc = ls->GetGC();
        ls->Add(anyObj);
        gc->Watch(anyObj);
        return anyObj;
    }
    void TAny::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        GC::Mark(this->other);
    }
}