#include <CrossLang.hpp>

namespace Tesses::CrossLang
{
    SharedPtrTObject::SharedPtrTObject(GC* gc, TObject o)
    {
        this->ls = new GCList(gc);
        this->ls->Add(o);
        this->o = o;
    }
    TObject& SharedPtrTObject::GetObject()
    {
        return this->o;
    }
    SharedPtrTObject::~SharedPtrTObject()
    {
        if(this->ls)
        delete this->ls;
    }
    GC* SharedPtrTObject::GetGC()
    {
        return this->ls->GetGC();
    }
    MarkedTObject CreateMarkedTObject(GC* gc, TObject o)
    {
        return std::make_shared<SharedPtrTObject>(gc,o);
    }
    MarkedTObject CreateMarkedTObject(GC& gc, TObject o)
    {
        return CreateMarkedTObject(&gc,o);
    }
    MarkedTObject CreateMarkedTObject(GCList* gc, TObject o)
    {
        return CreateMarkedTObject(gc->GetGC(),o);
    }
    MarkedTObject CreateMarkedTObject(GCList& gc, TObject o)
    {
        return CreateMarkedTObject(gc.GetGC(),o);
    }
}