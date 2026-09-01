#include "CrossLang.hpp"

namespace Tesses::CrossLang {

TAssociativeArray *TAssociativeArray::Create(GCList &ls) {
    return ls.Create<TAssociativeArray>();
}
TAssociativeArray *TAssociativeArray::Create(GCList *ls) {
    return ls->Create<TAssociativeArray>();
}
void TAssociativeArray::Set(std::shared_ptr<GC> gc, TObject key,
                            TObject value) {
    if (std::holds_alternative<Undefined>(key))
        return;
    gc->BarrierBegin();
    for (auto index = this->items.begin(); index < this->items.end(); index++) {
        auto first = index->first;
        gc->BarrierEnd();
        auto eq = Equals(gc, key, first);
        gc->BarrierBegin();
        if (eq) {
            if (std::holds_alternative<Undefined>(value)) {
                this->items.erase(index);
            } else {
                index->second = value;
            }
            gc->BarrierEnd();
            return;
        }
    }
    this->items.push_back(std::pair<TObject, TObject>(key, value));
    gc->BarrierEnd();
}
TObject TAssociativeArray::Get(std::shared_ptr<GC> gc, TObject key) {
    if (std::holds_alternative<Undefined>(key))
        return Undefined();
    gc->BarrierBegin();
    for (auto &item : this->items) {
        auto first = item.first;
        gc->BarrierEnd();
        auto eq = Equals(gc, key, first);
        gc->BarrierBegin();
        if (eq) {
            gc->BarrierEnd();
            return item.second;
        }
    }
    gc->BarrierEnd();
    return Undefined();
}
TObject TAssociativeArray::GetKey(int64_t index) {
    if (index >= 0 && index < (int64_t)this->items.size()) {
        return this->items[index].first;
    }
    return Undefined();
}
TObject TAssociativeArray::GetValue(int64_t index) {
    if (index >= 0 && index < (int64_t)this->items.size()) {
        return this->items[index].second;
    }
    return Undefined();
}
void TAssociativeArray::SetKey(int64_t index, TObject key) {

    if (std::holds_alternative<Undefined>(key))
        return;
    if (index >= 0 && index < (int64_t)this->items.size()) {
        this->items[index].first = key;
    }
}
void TAssociativeArray::SetValue(int64_t index, TObject value) {
    if (std::holds_alternative<Undefined>(value))
        return;
    if (index >= 0 && index < (int64_t)this->items.size()) {
        this->items[index].first = value;
    }
}
int64_t TAssociativeArray::Count() { return (int64_t)this->items.size(); }
void TAssociativeArray::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    for (auto &item : this->items) {
        GC::Mark(item.first);
        GC::Mark(item.second);
    }
}
} // namespace Tesses::CrossLang