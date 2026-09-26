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