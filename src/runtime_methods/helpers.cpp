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
static TObject Helpers_CopyToProgress(GCList &ls, std::vector<TObject> args) {
    std::shared_ptr<Tesses::Framework::Streams::Stream> src;
    std::shared_ptr<Tesses::Framework::Streams::Stream> dest;
    double precision = 1000.0;
    TCallable *callable;
    if (GetArgument(args, 0, src) && GetArgument(args, 1, dest) &&
        GetArgumentHeap(args, 2, callable)) {
        GetArgument(args, 3, precision);
        auto len = src->GetLength();
        callable->Call(ls, {0.0});
        if (len > 0) {
            std::vector<uint8_t> buff(1024);
            int64_t pos = 0;
            int curPercent = 0;
            int lastPercent = 0;
            size_t read = 0;
            do {
                read = src->ReadBlock(buff.data(), buff.size());
                dest->WriteBlock(buff.data(), read);

                if (read == 0)
                    break;
                pos += (int64_t)read;

                double percent = ((double)pos / len);
                percent *= precision;

                curPercent = (int)percent;

                if (curPercent > lastPercent) {
                    lastPercent = curPercent;
                    callable->Call(ls, {curPercent / precision});
                }

            } while (read != 0);
        } else {
            src->CopyTo(dest);
        }
        callable->Call(ls, {1.0});
    }
    return Undefined();
}
void TStd::RegisterHelpers(std::shared_ptr<GC> gc, TRootEnvironment *env) {
    auto helpers = env->EnsureDictionary(gc, "Helpers");
    helpers->DeclareFunction(gc, "CopyToProgress",
                             "Copy Stream to another (but with progress event)",
                             {"src", "dest", "progressCB", "$precision"},
                             Helpers_CopyToProgress);
}
} // namespace Tesses::CrossLang