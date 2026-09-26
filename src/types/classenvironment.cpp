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
bool TClassEnvironment::HasConstForSet(std::string key) {
    if (this->env->HasVariableRecurse(key)) {
        return this->env->HasConstForSet(key);
    }
    return false;
}
TClassEnvironment *TClassEnvironment::Create(GCList *gc, TEnvironment *env,
                                             TClassObject *obj) {

    return gc->Create<TClassEnvironment>(env, obj);
}
TClassEnvironment *TClassEnvironment::Create(GCList &gc, TEnvironment *env,
                                             TClassObject *obj) {

    return gc.Create<TClassEnvironment>(env, obj);
}
TClassEnvironment::TClassEnvironment(TEnvironment *env, TClassObject *obj) {
    this->env = env;
    this->clsObj = obj;
}
bool TClassEnvironment::HasVariable(std::string key) {
    if (key == "this")
        return true;
    auto current_function = GC::GetCurrentFunction();
    if (this->clsObj->HasValue(current_function == nullptr
                                   ? ""
                                   : current_function->callable->className,
                               key))
        return true;
    return false;
}
bool TClassEnvironment::HasVariableRecurse(std::string key) {
    if (HasVariable(key))
        return true;
    return this->env->HasVariableRecurse(key);
}
bool TClassEnvironment::HasVariableOrFieldRecurse(std::string key,
                                                  bool setting) {
    if (key == "this")
        return true;

    auto current_function = GC::GetCurrentFunction();
    std::string clsName = current_function == nullptr
                              ? ""
                              : current_function->callable->className;
    if (clsObj->HasMethod(clsName, (setting ? "set" : "get") + key))
        return true;
    if (clsObj->HasValue(clsName, key))
        return true;
    return env->HasVariableOrFieldRecurse(key, setting);
}

TObject TClassEnvironment::GetVariable(std::string key) {
    if (key == "this")
        return this->clsObj;

    auto current_function = GC::GetCurrentFunction();
    std::string clsName = current_function == nullptr
                              ? ""
                              : current_function->callable->className;

    if (clsObj->HasValue(clsName, key))
        return this->clsObj->GetValue(clsName, key);
    return env->GetVariable(key);
}
void TClassEnvironment::SetVariable(std::string key, TObject value) {
    if (key == "this")
        return;

    auto current_function = GC::GetCurrentFunction();
    std::string clsName = current_function == nullptr
                              ? ""
                              : current_function->callable->className;

    if (clsObj->HasValue(clsName, key)) {
        this->clsObj->SetValue(clsName, key, value);
        return;
    }
    this->env->SetVariable(key, value);
    return;
}
TObject TClassEnvironment::GetVariable(GCList &ls, std::string key) {
    if (key == "this")
        return this->clsObj;

    auto current_function = GC::GetCurrentFunction();
    std::string clsName = current_function == nullptr
                              ? ""
                              : current_function->callable->className;
    if (this->clsObj->HasMethod(clsName, "get" + key)) {
        auto res = this->clsObj->GetValue(clsName, "get" + key);
        TCallable *call;
        if (GetObjectHeap(res, call))
            return call->Call(ls, {});
    }
    if (this->clsObj->HasValue(clsName, key))
        return this->clsObj->GetValue(clsName, key);
    return this->env->GetVariable(ls, key);
}
TObject TClassEnvironment::SetVariable(GCList &ls, std::string key, TObject v) {
    if (key == "this")
        return this->clsObj;

    auto current_function = GC::GetCurrentFunction();
    std::string clsName = current_function == nullptr
                              ? ""
                              : current_function->callable->className;
    if (this->clsObj->HasMethod(clsName, "set" + key)) {
        auto res = this->clsObj->GetValue(clsName, "set" + key);
        TCallable *call;
        if (GetObjectHeap(res, call))
            return call->Call(ls, {v});
    }
    if (this->clsObj->HasValue(clsName, key)) {
        this->clsObj->SetValue(clsName, key, v);
        return v;
    }

    return this->env->SetVariable(ls, key, v);
}

void TClassEnvironment::DeclareVariable(std::string key, TObject value) {}
TRootEnvironment *TClassEnvironment::GetRootEnvironment() {
    return this->env->GetRootEnvironment();
}
TEnvironment *TClassEnvironment::GetParentEnvironment() { return this->env; }

void TClassEnvironment::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    this->clsObj->Mark();
    this->env->Mark();
    for (auto item : this->defers)
        item->Mark();
}
} // namespace Tesses::CrossLang