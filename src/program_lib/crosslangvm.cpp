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
using namespace Tesses::Framework;
namespace Tesses::CrossLang::Programs {
TObject CrossLangVM(GCList &ls, TRootEnvironment *env,
                    std::vector<std::string> &argv) {
    if (argv.size() < 2) {
        std::cout << "USAGE: "
                  << (argv.empty() ? (std::string) "crossvm" : argv[0])
                  << " <filename.crvm> <args...>" << std::endl;
        return (int64_t)1;
    }

    env->LoadFileWithDependencies(
        ls.GetGC(), Tesses::Framework::Filesystem::LocalFS,
        Tesses::Framework::Filesystem::LocalFS->SystemToVFSPath(argv[1]));

    if (env->HasVariable("WebAppMain")) {
        Args args(argv);
        int port = 4206;
        for (auto &item : args.options) {
            if (item.first == "port") {
                port = std::stoi(item.second);
            }
        }

        env->EnsureDictionary(ls.GetGC(), "Net")
            ->SetValue("WebServerPort", (int64_t)port);
        TList *args2 = TList::Create(ls);
        for (auto &item : args.positional) {
            args2->Add(item);
        }

        auto res = env->CallFunctionWithFatalError(ls, "WebAppMain", {args2});
        auto svr2 = Tesses::CrossLang::ToHttpServer(ls.GetGC(), res);
        if (svr2 == nullptr)
            return (int64_t)1;
        Tesses::Framework::Http::HttpServer svr(port, svr2);
        svr.StartAccepting();
        TF_RunEventLoop();
        TDictionary *_dict;
        TClassObject *_co;
        if (GetObjectHeap(res, _dict)) {
            _dict->CallMethod(ls, "Close", {});
        }
        if (GetObjectHeap(res, _co)) {
            _co->CallMethod(ls, "", "Close", {});
        }
        TF_Quit();
        return (int64_t)0;
    } else {
        TList *args = TList::Create(ls);
        for (size_t arg = 1; arg < argv.size(); arg++)
            args->Add(std::string(argv[arg]));

        return env->CallFunctionWithFatalError(ls, "main", {args});
    }
}
} // namespace Tesses::CrossLang::Programs