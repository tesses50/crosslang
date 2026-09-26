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
bool InterperterThread::InvokeTwo(GCList &ls, TObject fn, TObject left,
                                  TObject right) {
    if (std::holds_alternative<THeapObject *>(fn)) {

        auto obj = dynamic_cast<TCallable *>(std::get<THeapObject *>(fn));
        if (obj != nullptr) {
            auto closure = dynamic_cast<TClosure *>(obj);
            if (closure != nullptr) {

                if (!closure->closure->args.empty() &&
                    closure->closure->args[0] == "this") {
                    this->AddCallStackEntry(ls, closure, {left, right});
                } else {
                    this->AddCallStackEntry(ls, closure, {right});
                }

            } else {
                this->call_stack_entries[this->call_stack_entries.size() - 1]
                    ->Push(ls.GetGC(), obj->Call(ls, {right}));

                return false;
            }
            return true;
        }
    }
    return false;
}
} // namespace Tesses::CrossLang