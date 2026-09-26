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
#include <iostream>

namespace Tesses::CrossLang::Programs {
using namespace Tesses::Framework::Filesystem;
using namespace Tesses::Framework::Streams;

int64_t CrossArchiveExtract(std::vector<std::string> &argv) {
    Tesses::Framework::TF_Init();
    if (argv.size() < 3) {
        std::cout << "USAGE: " << argv[0] << " <archive.crvm> <dirasroot>"
                  << std::endl;
        return 1;
    }

    auto sdfs = std::make_shared<SubdirFilesystem>(
        Tesses::Framework::Filesystem::LocalFS, std::string(argv[2]));
    auto strm = LocalFS->OpenFile(argv[1], "rb");
    if (!strm->CanRead()) {
        std::cout << "ERROR: could not open " << argv[1] << std::endl;
        return 1;
    }

    auto res = Tesses::CrossLang::CrossArchiveExtract(strm, sdfs);

    std::cout << "Crvm Name: " << res.first.first << std::endl;
    std::cout << "Crvm Version: " << res.first.second.ToString() << std::endl;
    std::cout << "Crvm Info: " << std::endl << res.second << std::endl;

    return 0;
}
} // namespace Tesses::CrossLang::Programs
