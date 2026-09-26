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
TRandom::TRandom() : random() {}
TRandom::TRandom(uint64_t seed) : random(seed) {}
std::string TRandom::TypeName() { return "Random"; }
TObject TRandom::CallMethod(GCList &ls, std::string name,
                            std::vector<TObject> args) {
    if (name == "Next") {
        int64_t first;
        int64_t second;
        if (GetArgument(args, 0, first)) {
            if (GetArgument(args, 1, second)) {
                return (int64_t)random.Next((int32_t)first, (int32_t)second);
            }

            return (int64_t)random.Next((uint32_t)first);
        }
        {
            uint64_t num = random.Next();
            int64_t val2 = 0;
            memcpy(&val2, &num, sizeof(int64_t));
            return val2;
        }
    }

    if (name == "NextByte") {
        return (int64_t)random.NextByte();
    }

    if (name == "ToString") {
        return "";
    }
    return Undefined();
}
} // namespace Tesses::CrossLang