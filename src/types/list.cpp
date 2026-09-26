/*
    CrossLang is a dynamically-typed scripting language built on
   TessesFramework, named in honor of Jesus's sacrifice.

    Copyright (C) 2026 Mike Nolan
    SPDX-License-Identifier: GPL-3.0-or-later WITH TessesFramework-Exception-1.0

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "CrossLang.hpp"
namespace Tesses::CrossLang {
TDynamicList::TDynamicList(TCallable *callable) : cb(callable) {}
TDynamicList *TDynamicList::Create(GCList &ls, TCallable *callable) {
    return ls.Create<TDynamicList>(callable);
}
TDynamicList *TDynamicList::Create(GCList *ls, TCallable *callable) {
    return ls->Create<TDynamicList>(callable);
}

void TDynamicList::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    this->cb->Mark();
}

int64_t TDynamicList::Count(GCList &ls) {

    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", "Count");
    ls.GetGC()->BarrierEnd();
    auto res = cb->Call(ls, {dict});
    int64_t n;
    if (GetObject(res, n))
        return n;
    return 0;
}
TObject TDynamicList::Add(GCList &ls, TObject v) {

    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", "Add");
    dict->SetValue("Value", v);
    ls.GetGC()->BarrierEnd();
    return cb->Call(ls, {dict});
}
TObject TDynamicList::Insert(GCList &ls, int64_t index, TObject v) {

    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", "Insert");
    dict->SetValue("Index", index);
    dict->SetValue("Value", v);
    ls.GetGC()->BarrierEnd();
    return cb->Call(ls, {dict});
}
TObject TDynamicList::Clear(GCList &ls) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", "Clear");
    ls.GetGC()->BarrierEnd();
    return cb->Call(ls, {dict});
}
TObject TDynamicList::Remove(GCList &ls, TObject obj) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", "Remove");
    dict->SetValue("Value", obj);
    ls.GetGC()->BarrierEnd();

    return cb->Call(ls, {dict});
}
TObject TDynamicList::RemoveAllEqual(GCList &ls, TObject obj) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", "RemoveAllEqual");
    dict->SetValue("Value", obj);
    ls.GetGC()->BarrierEnd();

    return cb->Call(ls, {dict});
}
TObject TDynamicList::RemoveAt(GCList &ls, int64_t index) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", "RemoveAt");
    dict->SetValue("Index", index);
    ls.GetGC()->BarrierEnd();

    return cb->Call(ls, {dict});
}
TObject TDynamicList::ToString(GCList &ls) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", "ToString");
    ls.GetGC()->BarrierEnd();

    return cb->Call(ls, {dict});
}

TObject TDynamicList::GetAt(GCList &ls, int64_t index) {

    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", "GetAt");
    dict->SetValue("Index", index);
    ls.GetGC()->BarrierEnd();
    return cb->Call(ls, {dict});
}

TObject TDynamicList::SetAt(GCList &ls, int64_t index, TObject val) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", "SetAt");
    dict->SetValue("Index", index);
    dict->SetValue("Value", val);
    ls.GetGC()->BarrierEnd();
    return cb->Call(ls, {dict});
}

TDynamicList::~TDynamicList() {}

TByteArray *TByteArray::Create(GCList &ls) { return ls.Create<TByteArray>(); }

TByteArray *TByteArray::Create(GCList *ls) { return ls->Create<TByteArray>(); }
TList *TList::Create(GCList *gc) { return gc->Create<TList>(); }
TList *TList::Create(GCList &gc) { return gc.Create<TList>(); }
void TList::Add(TObject value) { this->items.push_back(value); }
void TList::Set(int64_t index, TObject value) {
    if (index >= 0 && index < this->Count()) {
        this->items[index] = value;
    }
}
TObject TList::Get(int64_t index) {
    if (index >= 0 && index < this->Count()) {
        return this->items[index];
    }
    return Undefined();
}
int64_t TList::Count() { return this->items.size(); }
void TList::Insert(int64_t index, TObject value) {
    if (index >= 0 && index <= this->Count()) {
        this->items.insert(this->items.begin() + index, value);
    }
}
void TList::RemoveAt(int64_t index) {
    if (index >= 0 && index < this->Count()) {
        this->items.erase(this->items.begin() + index);
    }
}
void TList::Clear() { this->items.clear(); }
void TList::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    for (auto item : this->items) {
        GC::Mark(item);
    }
}
}; // namespace Tesses::CrossLang