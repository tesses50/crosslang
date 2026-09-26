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
bool GetObjectAsPath(TObject &obj, Tesses::Framework::Filesystem::VFSPath &path,
                     bool allowString) {
    if (GetObject(obj, path))
        return true;
    std::string str;
    if (allowString && GetObject<std::string>(obj, str)) {
        path = Tesses::Framework::Filesystem::VFSPath(str);
        return true;
    }
    return false;
}
bool GetArgumentAsPath(std::vector<TObject> &args, size_t index,
                       Tesses::Framework::Filesystem::VFSPath &path,
                       bool allowString) {
    if (GetArgument(args, index, path))
        return true;
    std::string str;
    if (allowString && GetArgument<std::string>(args, index, str)) {
        path = Tesses::Framework::Filesystem::VFSPath(str);
        return true;
    }
    return false;
}

static TObject Path_Root(GCList &ls, std::vector<TObject> args) {
    auto res = Tesses::Framework::Filesystem::VFSPath();
    res.relative = false;
    return res;
}
static TObject Path_FromString(GCList &ls, std::vector<TObject> args) {
    std::string str;
    if (GetArgument(args, 0, str)) {
        return Tesses::Framework::Filesystem::VFSPath(str);
    }
    return nullptr;
}
static TObject Path_Create(GCList &ls, std::vector<TObject> args) {
    TList *myls;
    bool relative;
    if (GetArgument(args, 0, relative) && GetArgumentHeap(args, 1, myls)) {
        std::vector<std::string> items;
        for (int64_t i = 0; i < myls->Count(); i++) {
            std::string str;
            TObject o = myls->Get(i);
            if (GetObject<std::string>(o, str)) {
                items.push_back(str);
            }
        }
        auto res = Tesses::Framework::Filesystem::VFSPath(items);
        res.relative = relative;
        return res;
    }

    return nullptr;
}
void TStd::RegisterPath(std::shared_ptr<GC> gc, TRootEnvironment *env) {

    env->permissions.canRegisterPath = true;
    GCList ls(gc);
    TDictionary *dict = TDictionary::Create(ls);

    gc->BarrierBegin();
    dict->DeclareFunction(gc, "FromString", "Create a Path from string",
                          {"path"}, Path_FromString);

    dict->DeclareFunction(gc, "Create", "Create a Path from parts",
                          {"relative", "parts"}, Path_Create);
    dict->DeclareFunction(gc, "Root", "Create Absolute Root Path", {},
                          Path_Root);
    env->DeclareVariable("Path", dict);
    gc->BarrierEnd();
}
} // namespace Tesses::CrossLang