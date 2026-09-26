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
TObject Dictionary_FindByKey(GCList &ls, std::vector<TObject> args) {
    TList *dest = TList::Create(ls);
    ls.GetGC()->BarrierBegin();
    std::string key;
    if (GetArgument(args, 1, key)) {

        std::function<void(TObject)> crawl;
        crawl = [&](TObject o) -> void {
            TDictionary *dict;
            TList *list;
            TAssociativeArray *aa;
            if (GetObjectHeap(o, aa)) {
                std::string k0;
                for (auto &item : aa->items) {

                    if (GetObject(item.first, k0) && k0 == key)
                        dest->Add(item.second);
                    crawl(item.second);
                }
            }
            if (GetObjectHeap(o, dict)) {
                for (auto &item : dict->items) {
                    if (item.first == key)
                        dest->Add(item.second);
                    crawl(item.second);
                }
            }
            if (GetObjectHeap(o, list)) {
                for (auto &item : list->items) {
                    crawl(item);
                }
            }
        };
        crawl(args[0]);
    }
    ls.GetGC()->BarrierEnd();
    return dest;
}
TObject Dictionary_Items(GCList &ls, std::vector<TObject> args) {

    TDictionary *dict;
    TDynamicDictionary *dynDict;
    if (GetArgumentHeap(args, 0, dynDict)) {
        TDictionary *enumerableItem = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();

        auto fn = TExternalMethod::Create(
            ls, "Get Enumerator for Dictionary", {},
            [dynDict](GCList &ls2, std::vector<TObject> args) -> TObject {
                return dynDict->GetEnumerator(ls2);
            });
        fn->watch.push_back(dynDict);

        enumerableItem->SetValue("GetEnumerator", fn);

        ls.GetGC()->BarrierEnd();

        return enumerableItem;
    }
    if (GetArgumentHeap(args, 0, dict)) {
        TDictionary *enumerableItem = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();

        auto fn = TExternalMethod::Create(
            ls, "Get Enumerator for Dictionary", {"dict"},
            [dict](GCList &ls2, std::vector<TObject> args) -> TObject {
                return TDictionaryEnumerator::Create(ls2, dict);
            });
        fn->watch.push_back(dict);

        enumerableItem->SetValue("GetEnumerator", fn);

        ls.GetGC()->BarrierEnd();

        return enumerableItem;
    }

    return Undefined();
}
TObject Dictionary_GetField(GCList &ls, std::vector<TObject> args) {
    TDictionary *dict;
    TDynamicDictionary *dynDict;
    std::string key;
    if (GetArgument(args, 1, key)) {
        if (GetArgumentHeap(args, 0, dict)) {
            ls.GetGC()->BarrierBegin();
            auto res = dict->GetValue(key);
            ls.GetGC()->BarrierEnd();
            return res;
        } else if (GetArgumentHeap(args, 0, dynDict)) {
            return dynDict->GetField(ls, key);
        }
    }
    return nullptr;
}
TObject Dictionary_SetField(GCList &ls, std::vector<TObject> args) {
    TDictionary *dict;
    TDynamicDictionary *dynDict;
    std::string key;
    if (args.size() == 3 && GetArgument(args, 1, key)) {
        if (GetArgumentHeap(args, 0, dict)) {
            ls.GetGC()->BarrierBegin();
            dict->SetValue(key, args[2]);
            ls.GetGC()->BarrierEnd();
        } else if (GetArgumentHeap(args, 0, dynDict)) {
            dynDict->SetField(ls, key, args[2]);
        }
    }
    return nullptr;
}
void TStd::RegisterDictionary(std::shared_ptr<GC> gc, TRootEnvironment *env) {

    env->permissions.canRegisterDictionary = true;
    GCList ls(gc);
    TDictionary *dict = TDictionary::Create(ls);

    gc->BarrierBegin();
    dict->DeclareFunction(
        gc, "FindByKey",
        "Scan object recursively, return list of items with key",
        {"obj", "key"}, Dictionary_FindByKey);
    dict->DeclareFunction(gc, "Items",
                          "Get Dictionary Item Enumerable, for the each(item : "
                          "Dictionary.Items(myDict)){item.Key; item.Value;}",
                          {"dictionary"}, Dictionary_Items);
    dict->DeclareFunction(gc, "SetField", "Set a field in dictionary",
                          {"dict", "key", "value"}, Dictionary_SetField);
    dict->DeclareFunction(gc, "GetField", "Get a field in dictionary",
                          {"dict", "key"}, Dictionary_GetField);

    env->DeclareVariable("Dictionary", dict);
    gc->BarrierEnd();
}
} // namespace Tesses::CrossLang