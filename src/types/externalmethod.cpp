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
TExternalMethod::TExternalMethod(
    std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
    std::function<void()> destroy) {

    this->cb = cb;
    this->args = argNames;
    this->documentation = documentation;
    this->destroy = destroy;
}
TExternalMethod *TExternalMethod::Create(
    GCList &ls, std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
    std::function<void()> destroy) {
    return ls.Create<TExternalMethod>(documentation, argNames, cb, destroy);
}
TExternalMethod *TExternalMethod::Create(
    GCList *ls, std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
    std::function<void()> destroy) {
    return ls->Create<TExternalMethod>(documentation, argNames, cb, destroy);
}
TExternalMethod *TExternalMethod::Create(
    GCList &ls, std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb) {
    return ls.Create<TExternalMethod>(documentation, argNames, cb);
}
TExternalMethod *TExternalMethod::Create(
    GCList *ls, std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb) {

    return ls->Create<TExternalMethod>(documentation, argNames, cb);
}
TObject TExternalMethod::Call(GCList &ls, std::vector<TObject> args) {
    if (cb == nullptr)
        return Undefined();
    return this->cb(ls, args);
}
TExternalMethod::~TExternalMethod() {
    if (this->destroy != nullptr)
        this->destroy();
}
} // namespace Tesses::CrossLang