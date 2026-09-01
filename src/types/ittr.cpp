#include "CrossLang.hpp"

namespace Tesses::CrossLang {
bool TYieldEnumerator::MoveNext(std::shared_ptr<GC> ls) {
    CallStackEntry *ent;
    GCList ls2(ls);
    if (!this->hasStarted) {
        TClosure *clos;
        if (!GetObjectHeap(this->enumerator, clos))
            return false;
        auto _enumerator = clos->Call(ls2, {});
        ls->BarrierBegin();
        this->enumerator = _enumerator;
        this->hasStarted = true;
        ls->BarrierEnd();
    } else {

        if (GetObjectHeap(this->enumerator, ent)) {
            auto _enumerator = ent->Resume(ls2);
            ls->BarrierBegin();
            this->enumerator = _enumerator;
            ls->BarrierEnd();

        } else
            return false;
    }

    if (GetObjectHeap(this->enumerator, ent)) {
        ls->BarrierBegin();
        this->current = ent->Pop(ls2);
        ls->BarrierEnd();
        return true;
    }
    return false;
}
TObject TYieldEnumerator::GetCurrent(GCList &ls) {
    ls.Add(this->current);
    return this->current;
}
void TYieldEnumerator::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    GC::Mark(this->current);
    GC::Mark(this->enumerator);
}

TYieldEnumerator::TYieldEnumerator(TObject v)
    : current(nullptr), hasStarted(false), enumerator(v) {}

TYieldEnumerator *TYieldEnumerator::Create(GCList &ls, TObject v) {
    return ls.Create<TYieldEnumerator>(v);
}
TYieldEnumerator *TYieldEnumerator::Create(GCList *ls, TObject v) {
    return ls->Create<TYieldEnumerator>(v);
}

bool TCustomEnumerator::MoveNext(std::shared_ptr<GC> ls) {
    GCList ls2(ls);
    auto res = this->dict->CallMethod(ls2, "MoveNext", {});
    bool out;
    if (GetObject(res, out))
        return out;
    return false;
}
TObject TCustomEnumerator::GetCurrent(GCList &ls) {
    TObject res = Undefined();
    ls.GetGC()->BarrierBegin();
    auto getCurrent = this->dict->GetValue("getCurrent");
    TCallable *call;
    if (GetObjectHeap(getCurrent, call)) {
        ls.GetGC()->BarrierEnd();
        res = call->Call(ls, {});
        ls.GetGC()->BarrierBegin();
    } else {

        res = this->dict->GetValue("Current");
    }
    ls.GetGC()->BarrierEnd();
    return res;
}
void TCustomEnumerator::Mark() {
    if (this->marked)
        return;
    this->dict->Mark();
}
TCustomEnumerator::TCustomEnumerator(TDictionary *dict) : dict(dict) {}
TCustomEnumerator *TCustomEnumerator::Create(GCList *ls, TDictionary *dict) {
    return ls->Create<TCustomEnumerator>(dict);
}
TCustomEnumerator *TCustomEnumerator::Create(GCList &ls, TDictionary *dict) {
    return ls.Create<TCustomEnumerator>(dict);
}
TEnumerator *TEnumerator::CreateFromObject(GCList &ls, TObject obj) {
    std::string str;
    TList *mls;
    TDynamicList *dynList;
    TDynamicDictionary *dynDict;
    TDictionary *dict;
    TEnumerator *enumerator;
    TQueryable *q;
    if (GetObject(obj, str)) {
        return TStringEnumerator::Create(ls, str);
    } else if (GetObjectHeap(obj, mls)) {
        return TListEnumerator::Create(ls, mls);
    } else if (GetObjectHeap(obj, dynList)) {
        return TDynamicListEnumerator::Create(ls, dynList);
    } else if (GetObjectHeap(obj, dict)) {
        auto res = dict->CallMethod(ls, "GetEnumerator", {});
        if (GetObjectHeap(res, dict)) {
            return TCustomEnumerator::Create(ls, dict);
        } else if (GetObjectHeap(res, enumerator)) {
            return enumerator;
        }
    } else if (GetObjectHeap(obj, q)) {
        return q->GetEnumerator(ls);
    } else if (GetObjectHeap(obj, enumerator)) {
        return enumerator;
    }
    return nullptr;
}
TVFSPathEnumerator::TVFSPathEnumerator(
    Tesses::Framework::Filesystem::VFSPathEnumerator enumerator)
    : enumerator(enumerator) {}
TVFSPathEnumerator *TVFSPathEnumerator::Create(
    GCList &ls, Tesses::Framework::Filesystem::VFSPathEnumerator enumerator) {
    return ls.Create<TVFSPathEnumerator>(enumerator);
}
TVFSPathEnumerator *TVFSPathEnumerator::Create(
    GCList *ls, Tesses::Framework::Filesystem::VFSPathEnumerator enumerator) {
    return ls->Create<TVFSPathEnumerator>(enumerator);
}
bool TVFSPathEnumerator::MoveNext(std::shared_ptr<GC> ls) {
    return enumerator.MoveNext();
}
TObject TVFSPathEnumerator::GetCurrent(GCList &ls) {
    return enumerator.Current;
}
TDictionaryEnumerator::TDictionaryEnumerator(TDictionary *dict)
    : dict(dict), hasStarted(false) {}
TDictionaryEnumerator *TDictionaryEnumerator::Create(GCList &ls,
                                                     TDictionary *dict) {
    return ls.Create<TDictionaryEnumerator>(dict);
}
TDictionaryEnumerator *TDictionaryEnumerator::Create(GCList *ls,
                                                     TDictionary *dict) {
    return ls->Create<TDictionaryEnumerator>(dict);
}

bool TDictionaryEnumerator::MoveNext(std::shared_ptr<GC> ls) {
    if (!this->hasStarted) {
        this->hasStarted = true;
        this->ittr = this->dict->items.begin();
        return !this->dict->items.empty();
    } else {
        this->ittr++;
        return this->ittr != this->dict->items.end();
    }
}
TObject TDictionaryEnumerator::GetCurrent(GCList &ls) {
    if (!this->hasStarted)
        return Undefined();
    if (this->ittr != this->dict->items.end()) {
        ls.GetGC()->BarrierBegin();
        std::string key = this->ittr->first;
        TObject value = this->ittr->second;
        auto kvp = TDictionary::Create(ls);
        kvp->SetValue("Key", key);
        kvp->SetValue("Value", value);
        ls.GetGC()->BarrierEnd();
        return kvp;
    }
    return Undefined();
}
void TDictionaryEnumerator::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    this->dict->Mark();
}
TListEnumerator::TListEnumerator(TList *list) : ls(list), index(-1) {}

TListEnumerator *TListEnumerator::Create(GCList &ls, TList *list) {
    return ls.Create<TListEnumerator>(list);
}
TListEnumerator *TListEnumerator::Create(GCList *ls, TList *list) {
    return ls->Create<TListEnumerator>(list);
}
bool TListEnumerator::MoveNext(std::shared_ptr<GC> ls) {
    this->index++;
    return this->index >= 0 && this->index < this->ls->Count();
}
TObject TListEnumerator::GetCurrent(GCList &ls) {

    if (this->index < -1)
        return nullptr;
    if (this->ls->Count() == 0)
        return nullptr;
    if (this->index >= this->ls->Count())
        return nullptr;
    ls.GetGC()->BarrierBegin();
    TObject o = this->ls->Get(index);
    ls.GetGC()->BarrierEnd();
    return o;
}
void TListEnumerator::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    this->ls->Mark();
}

TAssociativeArrayEnumerator::TAssociativeArrayEnumerator(
    TAssociativeArray *list)
    : ls(list), index(-1) {}

TAssociativeArrayEnumerator *
TAssociativeArrayEnumerator::Create(GCList &ls, TAssociativeArray *list) {
    return ls.Create<TAssociativeArrayEnumerator>(list);
}
TAssociativeArrayEnumerator *
TAssociativeArrayEnumerator::Create(GCList *ls, TAssociativeArray *list) {
    return ls->Create<TAssociativeArrayEnumerator>(list);
}
bool TAssociativeArrayEnumerator::MoveNext(std::shared_ptr<GC> ls) {
    this->index++;
    return this->index >= 0 && this->index < this->ls->Count();
}
TObject TAssociativeArrayEnumerator::GetCurrent(GCList &ls) {

    if (this->index < -1)
        return nullptr;
    if (this->ls->Count() == 0)
        return nullptr;
    if (this->index >= this->ls->Count())
        return nullptr;
    ls.GetGC()->BarrierBegin();
    TDictionary *dict = TDictionary::Create(ls);
    dict->SetValue("Key", this->ls->GetKey(this->index));
    dict->SetValue("Value", this->ls->GetValue(this->index));
    ls.GetGC()->BarrierEnd();
    return dict;
}
void TAssociativeArrayEnumerator::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    this->ls->Mark();
}
TDynamicListEnumerator::TDynamicListEnumerator(TDynamicList *list)
    : ls(list), index(-1) {}

TDynamicListEnumerator *TDynamicListEnumerator::Create(GCList &ls,
                                                       TDynamicList *list) {
    return ls.Create<TDynamicListEnumerator>(list);
}
TDynamicListEnumerator *TDynamicListEnumerator::Create(GCList *ls,
                                                       TDynamicList *list) {
    return ls->Create<TDynamicListEnumerator>(list);
}
bool TDynamicListEnumerator::MoveNext(std::shared_ptr<GC> ls) {
    this->index++;
    GCList ls2(ls);
    return this->index >= 0 && this->index < this->ls->Count(ls2);
}
TObject TDynamicListEnumerator::GetCurrent(GCList &ls) {

    if (this->index < -1)
        return nullptr;
    auto r = this->ls->Count(ls);
    if (r == 0)
        return nullptr;
    if (this->index >= r)
        return nullptr;
    ls.GetGC()->BarrierBegin();
    TObject o = this->ls->GetAt(ls, index);
    ls.GetGC()->BarrierEnd();
    return o;
}
void TDynamicListEnumerator::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    this->ls->Mark();
}
TStringEnumerator::TStringEnumerator(std::string str)
    : str(str), hasStarted(false) {}
TStringEnumerator *TStringEnumerator::Create(GCList &ls, std::string str) {
    return ls.Create<TStringEnumerator>(str);
}
TStringEnumerator *TStringEnumerator::Create(GCList *ls, std::string str) {
    return ls->Create<TStringEnumerator>(str);
}
bool TStringEnumerator::MoveNext(std::shared_ptr<GC> ls) {
    if (!this->hasStarted) {
        this->hasStarted = true;
        this->index = 0;
        return !this->str.empty();
    } else {
        if (this->index >= this->str.size())
            return false;
        this->index++;
        return this->index < this->str.size();
    }
}
TObject TStringEnumerator::GetCurrent(GCList &ls) {
    if (!this->hasStarted)
        return nullptr;
    if (this->index < this->str.size())
        return this->str[this->index];
    return nullptr;
}

}; // namespace Tesses::CrossLang