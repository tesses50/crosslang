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

TNative::TNative(void *ptr, std::function<void(void *)> destroy) {
    this->ptr = ptr;
    this->destroyed = false;
    this->destroy = destroy;
}
bool TNative::GetDestroyed() { return this->destroyed; }
void *TNative::GetPointer() { return this->ptr; }
void TNative::Mark() {
    if (this->marked)
        return;
    this->marked = true;

    GC::Mark(this->other);
}
void TNative::Destroy() {
    if (this->destroyed)
        return;
    if (this->destroy != nullptr) {
        this->destroyed = true;
        this->destroy(this->ptr);
    }
}
bool TNativeObject::ToBool() { return true; }
bool TNativeObject::Equals(std::shared_ptr<GC> gc, TObject right) {
    if (std::holds_alternative<THeapObject *>(right)) {
        return this == std::get<THeapObject *>(right);
    }
    return false;
}
TNative *TNative::Create(GCList &ls, void *ptr,
                         std::function<void(void *)> destroy) {
    return ls.Create<TNative>(ptr, destroy);
}
TNative *TNative::Create(GCList *ls, void *ptr,
                         std::function<void(void *)> destroy) {
    return ls->Create<TNative>(ptr, destroy);
}
TNative::~TNative() { this->Destroy(); }

} // namespace Tesses::CrossLang
