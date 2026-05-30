#include "CrossLang.hpp"

namespace Tesses::CrossLang {
GCList::GCList(std::shared_ptr<GC> gc) {
    gc->BarrierBegin();
    this->gc = gc;
    gc->SetRoot(this);
    gc->BarrierEnd();
}

std::shared_ptr<GC> GCList::GetGC() { return this->gc; }
void GCList::Remove(TObject obj) {
    if (std::holds_alternative<THeapObjectHolder>(obj)) {
        auto _item = std::get<THeapObjectHolder>(obj).obj;
        this->gc->BarrierBegin();
        for (auto index = this->items.begin(); index != this->items.end();
             index++) {
            if (*index == _item) {
                this->items.erase(index);
                continue;
            }
        }
        this->gc->BarrierEnd();
    }
}
void GCList::Add(TObject obj) {

    if (std::holds_alternative<THeapObjectHolder>(obj)) {
        auto _item = std::get<THeapObjectHolder>(obj).obj;
        this->gc->BarrierBegin();

        for (auto item : this->items) {
            if (item == _item) {
                this->gc->BarrierEnd();
                return;
            }
        }
        this->items.push_back(_item);
        this->gc->BarrierEnd();
    }
}
void GCList::Mark() {
    this->marked = true;
    for (auto item : this->items) {
        item->Mark();
    }
}
GCList::~GCList() {
    gc->BarrierBegin();
    this->gc->UnsetRoot(this);
    gc->BarrierEnd();
}
} // namespace Tesses::CrossLang
