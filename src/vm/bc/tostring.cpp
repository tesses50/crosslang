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
#include "TessesFramework/Serialization/BitConverter.hpp"
#include "TessesFramework/Streams/ByteReader.hpp"
#include "TessesFramework/Uuid.hpp"
#include <cmath>
#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <variant>
namespace Tesses::CrossLang {
std::string ToString(std::shared_ptr<GC> gc, TObject o) {
    if (std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(o)) {
        return std::get<Tesses::Framework::Filesystem::VFSPath>(o).ToString();
    }
    if (std::holds_alternative<std::string>(o)) {
        return std::get<std::string>(o);
    }
    if (std::holds_alternative<TVMVersion>(o)) {
        return std::get<TVMVersion>(o).ToString();
    }
    if (std::holds_alternative<int64_t>(o)) {
        return std::to_string(std::get<int64_t>(o));
    }

    if (std::holds_alternative<double>(o)) {
        return std::to_string(std::get<double>(o));
    }

    if (std::holds_alternative<char>(o)) {
        return std::string{std::get<char>(o)};
    }
    if (IsNull(o)) {
        return "null";
    }
    if (std::holds_alternative<Undefined>(o)) {
        return "undefined";
    }
    if (std::holds_alternative<bool>(o)) {
        return std::get<bool>(o) ? "true" : "false";
    }
    if (std::holds_alternative<
            std::shared_ptr<Tesses::Framework::Date::DateTime>>(o)) {
        return std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(o)
            ->ToString();
    }
    if (std::holds_alternative<
            std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(o)) {
        return std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(o)
            ->ToString(false);
    }
    if (std::holds_alternative<Tesses::Framework::Uuid>(o)) {
        return std::get<Tesses::Framework::Uuid>(o).ToString(
            Framework::UuidStringifyConfig::LowercaseNoCurly);
    }
    if (std::holds_alternative<THeapObject *>(o)) {
        auto obj = std::get<THeapObject *>(o);
        auto dict = dynamic_cast<TDictionary *>(obj);
        auto list = dynamic_cast<TList *>(obj);
        auto bArray = dynamic_cast<TByteArray *>(obj);
        auto natObj = dynamic_cast<TNativeObject *>(obj);
        auto cls = dynamic_cast<TClassObject *>(obj);
        auto aArray = dynamic_cast<TAssociativeArray *>(obj);
        if (aArray != nullptr) {

            std::string str = {};

            gc->BarrierBegin();
            bool first = true;
            for (auto item : aArray->items) {
                if (!first)
                    str.push_back('\n');
                first = false;
                str.push_back('[');
                str.append(Json_Encode(item.first));
                str.append("] = ");
                str.append(Json_Encode(item.second));
                str.append(";");
            }
            gc->BarrierEnd();
            return str;
        }
        if (cls != nullptr) {
            auto res = cls->GetValue("", "ToString");
            TCallable *call;
            GCList ls(gc);
            if (GetObjectHeap(res, call))
                return ToString(gc, call->Call(ls, {}));
            return cls->TypeName();
        }
        if (natObj != nullptr) {
            GCList ls(gc);
            TObject o = natObj->CallMethod(ls, "ToString", {});

            return ToString(gc, o);
        }

        if (dict != nullptr) {
            GCList ls(gc);
            if (dict->MethodExists(ls, "ToString"))
                return ToString(gc, dict->CallMethod(ls, "ToString", {}));
            else {
                return Json_Encode(dict);
            }
        } else if (bArray != nullptr) {
            return std::string(bArray->data.begin(), bArray->data.end());
        } else if (list != nullptr) {
            return Json_Encode(list);
        }
    }

    return "";
}
} // namespace Tesses::CrossLang