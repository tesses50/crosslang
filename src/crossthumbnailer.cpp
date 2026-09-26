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
#include <TessesFramework/Common.hpp>
#include <TessesFramework/Filesystem/LocalFS.hpp>
#include <TessesFramework/Filesystem/VFS.hpp>
#include <TessesFramework/Streams/Stream.hpp>
#include <fstream>
#include <ios>
#include <iostream>
int main(int argc, char **argv) {
    Tesses::Framework::TF_Init();
    std::string p = argv[0];
    auto emptyThumb =
        Tesses::Framework::Platform::Environment::GetRealExecutablePath(p)
            .GetParent()
            .GetParent() /
        "share" / "icons" / "crosslang.png";

    if (argc < 3) {
        std::cout << "USAGE: " << argv[0] << " CRVMFILE NEWPNG" << std::endl;
        return 1;
    }
    std::string crvm = argv[1];
    std::string png = argv[2];

    if (Tesses::Framework::Filesystem::LocalFS->FileExists(crvm)) {

        Tesses::CrossLang::TFile file;
        auto f = Tesses::Framework::Filesystem::LocalFS->OpenFile(crvm, "rb");

        file.Load(nullptr, f);

        if (file.icon >= 0 && file.icon < file.resources.size()) {
            auto f2 =
                Tesses::Framework::Filesystem::LocalFS->OpenFile(png, "wb");
            if (f2 != nullptr) {
                auto &icon = file.resources[file.icon];
                f2->WriteBlock(icon.data(), icon.size());
            }
            return 0;
        }
    }
    if (Tesses::Framework::Filesystem::LocalFS->FileExists(emptyThumb)) {
        auto src =
            Tesses::Framework::Filesystem::LocalFS->OpenFile(emptyThumb, "rb");
        auto dest = Tesses::Framework::Filesystem::LocalFS->OpenFile(png, "wb");
        if (src != nullptr && dest != nullptr) {
            src->CopyTo(dest);
        }
    }
    return 0;
}