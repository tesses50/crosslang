#include "CrossLang.hpp"

namespace Tesses::CrossLang {
SharedPtrTObject::SharedPtrTObject(std::shared_ptr<GC> gc, TObject o) {
    this->ls = new GCList(gc);
    this->ls->Add(o);
    this->o = o;
}
TObject &SharedPtrTObject::GetObject() { return this->o; }
SharedPtrTObject::~SharedPtrTObject() {
    if (this->ls)
        delete this->ls;
}
std::shared_ptr<GC> SharedPtrTObject::GetGC() { return this->ls->GetGC(); }
MarkedTObject CreateMarkedTObject(std::shared_ptr<GC> gc, TObject o) {
    return std::make_shared<SharedPtrTObject>(gc, o);
}

MarkedTObject CreateMarkedTObject(GCList *gc, TObject o) {
    return CreateMarkedTObject(gc->GetGC(), o);
}
MarkedTObject CreateMarkedTObject(GCList &gc, TObject o) {
    return CreateMarkedTObject(gc.GetGC(), o);
}
} // namespace Tesses::CrossLang