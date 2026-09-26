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
bool ToBool(TObject obj) {
    if (std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(obj)) {
        return true;
    }
    if (std::holds_alternative<TVMVersion>(obj)) {
        auto v = std::get<TVMVersion>(obj);
        return v.AsLong() != 0;
    }
    if (std::holds_alternative<std::string>(obj)) {
        return !std::get<std::string>(obj).empty();
    } else if (std::holds_alternative<int64_t>(obj)) {
        return std::get<int64_t>(obj) != 0;
    } else if (std::holds_alternative<double>(obj)) {
        return std::get<double>(obj);
    } else if (std::holds_alternative<bool>(obj)) {
        return std::get<bool>(obj);
    } else if (std::holds_alternative<char>(obj)) {
        return std::get<char>(obj) != 0;
    } else if (std::holds_alternative<
                   std::shared_ptr<Tesses::Framework::Date::DateTime>>(obj)) {
        auto &dt =
            std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(obj);
        return !(dt->Year() == 1970 && dt->Month() == 1 && dt->Day() == 1 &&
                 dt->Hour() == 0 && dt->Minute() == 0 && dt->Second() == 0 &&
                 !dt->IsLocal());

    } else if (std::holds_alternative<
                   std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(obj)) {
        return std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(obj)
                   ->TotalSeconds() != 0;
    } else if (std::holds_alternative<Tesses::Framework::Uuid>(obj)) {
        auto &uuid = std::get<Tesses::Framework::Uuid>(obj);
        return !uuid.IsEmpty();
    } else if (std::holds_alternative<THeapObject *>(obj)) {
        auto o = std::get<THeapObject *>(obj);
        auto ls = dynamic_cast<TList *>(o);
        auto aarray = dynamic_cast<TAssociativeArray *>(o);
        auto dict = dynamic_cast<TDictionary *>(o);
        auto ba = dynamic_cast<TByteArray *>(o);
        auto nat = dynamic_cast<TNative *>(o);
        auto thrd = dynamic_cast<ThreadHandle *>(o);
        auto natObj = dynamic_cast<TNativeObject *>(o);

        auto any = dynamic_cast<TAny *>(o);
        auto cls = dynamic_cast<TClassObject *>(o);
        if (cls != nullptr)
            return true;
        if (natObj != nullptr)
            return natObj->ToBool();
        if (any != nullptr)
            return any->any.has_value();

        if (ls != nullptr) {
            return ls->Count() != 0;
        }
        if (aarray != nullptr) {
            return aarray->Count() != 0;
        } else if (dict != nullptr) {
            return !dict->items.empty();
        } else if (ba != nullptr) {
            return !ba->data.empty();
        } else if (nat != nullptr) {
            return !nat->GetDestroyed();
        } else if (thrd != nullptr) {
            return !thrd->hasReturned;
        }
        return true;
    }

    return false;
}
} // namespace Tesses::CrossLang