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
GCList::GCList(std::shared_ptr<GC> gc) {
    gc->BarrierBegin();
    this->gc = gc;
    gc->SetRoot(this);
    gc->BarrierEnd();
}

std::shared_ptr<GC> GCList::GetGC() const { return this->gc; }
void GCList::Remove(TObject obj) {
    if (std::holds_alternative<THeapObject *>(obj)) {
        auto _item = std::get<THeapObject *>(obj);
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

    if (std::holds_alternative<THeapObject *>(obj)) {
        auto _item = std::get<THeapObject *>(obj);
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
