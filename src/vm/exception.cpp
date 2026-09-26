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
TObject TCallable::CallWithFatalError(GCList &ls, std::vector<TObject> args) {
    try {
        return Call(ls, args);
    } catch (VMByteCodeException &ex) {
        ThrowFatalError(ex);
    } catch (VMException &ex) {
        ThrowFatalError(ex);
    } catch (SyntaxException &ex) {
        ThrowFatalError(ex);
    } catch (std::runtime_error &ex) {
        ThrowFatalError(ex);
    } catch (std::exception &ex) {
        ThrowFatalError(ex);
    }
    return (int64_t)1;
}
void ThrowFatalError(std::exception &ex) {
    using namespace Tesses::Framework::TextStreams;
    std::exception *ex2 = &ex;
    auto clexcept = dynamic_cast<VMByteCodeException *>(ex2);
    auto clrtexcept = dynamic_cast<VMException *>(ex2);
    auto compiler = dynamic_cast<SyntaxException *>(ex2);
    Tesses::Framework::TextStreams::ConsoleWriter error(true);

    if (clexcept != nullptr) {
        error.WriteLine("CrossLang has encountered a fatal exception");
        error.WriteLine();

        error.WriteLine(
            ToString(clexcept->GetGCList()->GetGC(), clexcept->exception));
        error.WriteLine();
        error.WriteLine("STACKTRACE:");

        for (auto ittr = clexcept->stack_trace.crbegin();
             ittr != clexcept->stack_trace.crend(); ittr++) {
            auto item = *ittr;
            std::string text = "\t";
            text += item->callable->closure->name.value_or(
                (std::string) "<Closure>");
            text += "(";
            bool first = true;
            for (auto &arg : item->callable->closure->args) {
                if (!first)
                    text += ", ";
                text += arg;
                first = false;
            }
            text += ")";
            if (item->srcline >= 1) {
                text += " at ";
                text += item->srcfile;
                text += ":";
                text += std::to_string(item->srcline);
            }
            error.WriteLine(text);
        }

        std::exit(1);
    }
    if (clrtexcept != nullptr) {
        error.WriteLine("CrossLang has encountered a fatal runtime exception");
        error.WriteLine();
        error.WriteLine(clrtexcept->what());
        std::exit(1);
    }
    if (compiler != nullptr) {
        error.WriteLine("CrossLang has encountered a compiler error");
        error.WriteLine();
        error.WriteLine(compiler->Message());
        error.WriteLine();
        auto li = compiler->LineInfo();
        error.Write("in file: ");
        error.Write(li.filename);
        error.Write(":");
        error.Write((int64_t)li.line);
        error.Write(":");
        error.Write((int64_t)li.column);
        error.Write(":");
        error.WriteLine((int64_t)li.offset);

        std::exit(1);
    }

    error.WriteLine("CrossLang has encountered a fatal C++ exception");
    error.WriteLine();
    error.Write("what(): ");
    error.WriteLine(ex2->what());
    std::exit(1);
}
} // namespace Tesses::CrossLang