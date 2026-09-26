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
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
using namespace Tesses::Framework;
using namespace Tesses::Framework::Filesystem;

namespace Tesses::CrossLang::Programs {
TObject CrossLangInterperter(GCList &ls, TRootEnvironment *env,
                             std::vector<std::string> &argv) {
    std::shared_ptr<GC> gc = ls.GetGC();
    if (argv.size() > 1) {
        std::ifstream strm(argv[1], std::ios_base::in | std::ios_base::binary);
        std::vector<LexToken> tokens;
        Lex(argv[1], strm, tokens);

        Parser parser(tokens);

        CodeGen gen;

        auto sfs = std::make_shared<SubdirFilesystem>(LocalFS, VFSPath("."));
        gen.embedFS = sfs;
        gen.GenRoot(parser.ParseRoot());
        std::vector<uint8_t> data;
        {
            auto strm2 =
                std::make_shared<Tesses::Framework::Streams::MemoryStream>(
                    true);
            gen.Save(strm2);

            {
                TFile *file = TFile::Create(ls);

                strm2->Seek(0, Tesses::Framework::Streams::SeekOrigin::Begin);
                file->Load(gc, strm2);

                env->LoadFile(gc, file);
            }
        }

        TList *args = TList::Create(ls);
        for (int arg = 1; arg < argv.size(); arg++)
            args->Add(std::string(argv[arg]));

        return env->CallFunctionWithFatalError(ls, "main", {args});

    } else {

        while (true) {
            std::cout << "> ";
            std::string source;
            std::getline(std::cin, source);

            auto strm2 =
                std::make_shared<Tesses::Framework::Streams::MemoryStream>(
                    true);

            if (source.find("loadfile ") == 0) {
                std::string filename = source.substr(9);

                std::ifstream strm(filename,
                                   std::ios_base::in | std::ios_base::binary);
                std::vector<LexToken> tokens;
                Lex(filename, strm, tokens);

                Parser parser(tokens);

                CodeGen gen;

                auto sfs =
                    std::make_shared<SubdirFilesystem>(LocalFS, VFSPath("."));
                gen.embedFS = sfs;
                gen.GenRoot(parser.ParseRoot());

                gen.Save(strm2);

            } else if (source == "exit") {
                return (int64_t)0;
            } else {
                std::vector<LexToken> tokens;
                std::stringstream strm(source, std::ios_base::in |
                                                   std::ios_base::binary);
                Lex("lexed.tcross", strm, tokens);

                Parser parser(tokens);

                CodeGen gen;

                auto sfs =
                    std::make_shared<SubdirFilesystem>(LocalFS, VFSPath("."));
                gen.embedFS = sfs;

                gen.GenRoot(parser.ParseRoot());

                gen.Save(strm2);
            }

            {

                TFile *file = TFile::Create(ls);

                strm2->Seek(0, Tesses::Framework::Streams::SeekOrigin::Begin);
                file->Load(gc, strm2);

                env->LoadFile(gc, file);
            }
        }
    }

    return (int64_t)0;
}

} // namespace Tesses::CrossLang::Programs