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
static void Help(std::string &filename) {
    std::cout << "USAGE: " << filename
              << " [OPTIONS] <dirasroot> <archive.crvm>" << std::endl;
    printf("OPTIONS:\n");
    printf("  -i:  Set info (ex {\"maintainer\": \"Mike Nolan\", \"repo\": "
           "\"https://example.com/\", \"homepage\": "
           "\"https://example.com/\",\"license\":\"MIT\"})\n");
    printf("  -I:  Set icon name (relative to dirasroot), should be a 128x128 "
           "png\n");
    printf("  -v:  Set version (1.0.0.0-prod defaults to 1.0.0.0-dev)\n");
    printf("  -n:  Set name (MyAppOrLibName defaults to out)\n");
    printf("  -h, --help:  Prints help\n");
    printf("Options except for help have flag with arg like this: -F ARG\n");
    std::exit(1);
}

int64_t CrossArchiveCreate(std::vector<std::string> &argv) {
    Tesses::Framework::TF_Init();
    std::string name = "out";
    std::string info = "{}";
    TVMVersion version;
    std::string icon = "";
    std::vector<std::string> args;
    for (int i = 1; i < argv.size(); i++) {
        if (argv[i] == "--help" || argv[i] == "-h") {
            Help(argv[0]);
        } else if (argv[i] == "-i") {
            i++;
            if (i < argv.size()) {
                info = argv[i];
            }
        } else if (argv[i] == "-I") {
            i++;
            if (i < argv.size()) {
                icon = argv[i];
            }
        } else if (argv[i] == "-n") {
            i++;
            if (i < argv.size()) {
                name = argv[i];
            }
        } else if (argv[i] == "-v") {
            i++;
            if (i < argv.size()) {

                if (!TVMVersion::TryParse(argv[i], version)) {
                    printf("ERROR: Invalid syntax for version\n");
                    printf("Expected "
                           "MAJOR[.MINOR[.PATCH[.BUILD[-dev,-alpha,-beta,-prod]"
                           "]]]\n");
                    std::exit(1);
                }
            }
        } else {
            args.push_back(argv[i]);
        }
    }

    if (args.size() < 2)
        Help(argv[0]);

    auto path =
        Tesses::Framework::Filesystem::LocalFS->SystemToVFSPath(args[0]);
    Tesses::Framework::Filesystem::LocalFS->CreateDirectory(path);
    auto sdfs = std::make_shared<SubdirFilesystem>(
        Tesses::Framework::Filesystem::LocalFS, path);

    FILE *f = fopen(args[1].c_str(), "wb");
    if (f == NULL) {
        printf("ERROR: could not open %s\n", args[1].c_str());
        return 1;
    }

    auto strm = std::make_shared<FileStream>(f, true, "wb", true);
    CrossArchiveCreate(sdfs, strm, name, version, info, icon);

    return 0;
}
} // namespace Tesses::CrossLang::Programs