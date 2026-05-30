#include "CrossLang.hpp"
#include <iostream>
namespace Tesses::CrossLang::Programs {
static void Ensure(std::shared_ptr<Tesses::Framework::Streams::Stream> strm,
                   uint8_t *buffer, size_t len) {
    if (strm->ReadBlock(buffer, len) != len) {
        throw VMException("Could not read " + std::to_string(len) +
                          " byte(s).");
    }
}
static uint32_t
EnsureInt(std::shared_ptr<Tesses::Framework::Streams::Stream> strm) {
    uint8_t buff[4];
    Ensure(strm, buff, sizeof(buff));
    return BitConverter::ToUint32BE(buff[0]);
}
static std::string
EnsureString(std::shared_ptr<Tesses::Framework::Streams::Stream> strm) {
    size_t len = (size_t)EnsureInt(strm);
    std::string myStr = {};
    myStr.resize(len);
    Ensure(strm, (uint8_t *)myStr.data(), len);
    return myStr;
}
void CrossLangDump(std::shared_ptr<Tesses::Framework::Streams::Stream> strm) {
    uint8_t main_header[18];
    Ensure(strm, main_header, sizeof(main_header));
    if (strncmp((const char *)main_header, "TCROSSVM", 8) != 0)
        throw VMException("Invalid TCrossVM image.");
    TVMVersion version(main_header + 8);
    if (version.CompareToRuntime() == 1) {
        throw VMException("Runtime is too old.");
    }
    TVMVersion v2(main_header + 13);
    std::cout << "Version: " << v2.ToString() << std::endl;

    size_t _len = (size_t)EnsureInt(strm);

    std::cout << "SectionCount: " << _len << std::endl;

    std::vector<std::string> strs;

    std::unordered_map<uint32_t, std::vector<std::string>> funs;
    std::vector<std::vector<std::string>> closures;

    char table_name[4];
    bool hasIcon = false;

    for (size_t i = 0; i < _len; i++) {
        Ensure(strm, (uint8_t *)table_name, sizeof(table_name));
        size_t tableLen = (size_t)EnsureInt(strm);
        std::string tableName(table_name, 4);
        if (tableName == "ICON") {
            hasIcon = true;
        } else if (tableName == "STRS") {
            size_t strsLen = (size_t)EnsureInt(strm);
            for (size_t j = 0; j < strsLen; j++) {
                strs.push_back(EnsureString(strm));
            }
        } else if (tableName == "DEPS") {

            std::string name = strs.at((size_t)EnsureInt(strm));

            uint8_t version_bytes[5];
            Ensure(strm, version_bytes, sizeof(version_bytes));
            TVMVersion depVersion(version_bytes);
            std::cout << "Dependency: " << name << "-" << depVersion.ToString()
                      << std::endl;
        } else if (tableName == "NAME") {
            std::cout << "Name: " << strs.at((size_t)EnsureInt(strm))
                      << std::endl;
        } else if (tableName == "CLSS") {
            std::cout << "Classes:\n";
            uint32_t clss_cnt = EnsureInt(strm);
            for (uint32_t j = 0; j < clss_cnt; j++) {
                std::cout << "\t/^" << strs.at(EnsureInt(strm)) << "^/"
                          << std::endl;
                uint32_t fnPartsC = EnsureInt(strm);
                std::cout << "\tName: ";
                for (uint32_t k = 0; k < fnPartsC; k++) {
                    if (k > 0)
                        std::cout << ".";
                    std::cout << strs.at(EnsureInt(strm));
                }
                std::cout << std::endl;
                fnPartsC = EnsureInt(strm);
                std::cout << "\tInherits: ";
                for (uint32_t k = 0; k < fnPartsC; k++) {
                    if (k > 0)
                        std::cout << ".";
                    std::cout << strs.at(EnsureInt(strm));
                }
                std::cout << std::endl;

                uint32_t ents = EnsureInt(strm);

                for (uint8_t k = 0; k < ents; k++) {
                    Ensure(strm, main_header, 1);
                    uint8_t flags = main_header[0];
                    std::cout << "\t\t/^" << strs.at(EnsureInt(strm)) << "^/"
                              << std::endl;
                    std::string fnname = strs.at(EnsureInt(strm));
                    std::string fnargs;
                    uint32_t argParts = EnsureInt(strm);

                    for (uint32_t l = 0; l < argParts; l++) {
                        if (l > 0)
                            fnargs += ", ";
                        fnargs += strs.at(EnsureInt(strm));
                    }
                    uint32_t fnchunk = EnsureInt(strm);
                    switch (flags & 3) {
                    case 0:
                        std::cout << "\t\tprivate ";
                        break;
                    case 1:
                        std::cout << "\t\tprotected ";
                        break;
                    case 2:
                        std::cout << "\t\tpublic ";
                        break;
                    case 3:
                        std::cout << "\t\tstatic ";
                        break;
                    }

                    switch ((flags >> 2) & 3) {
                    case 0:
                        std::cout << "func " << fnname << "(" << fnargs
                                  << "), chunk = " << fnchunk << std::endl;

                        break;
                    case 1:
                        std::cout << "field " << fnname
                                  << ", chunk = " << fnchunk << std::endl;
                        break;
                    case 2:
                        std::cout << "abstract " << fnname << "(" << fnargs
                                  << ")" << std::endl;
                        break;
                    case 3:
                        std::cout << "unset_field " << fnname << std::endl;
                        break;
                    }
                    std::cout << std::endl;
                }
            }
        } else if (tableName == "CHKS") {
            size_t chunkCount = (size_t)EnsureInt(strm);

            for (size_t j = 0; j < chunkCount; j++) {
                std::vector<std::string> args;
                size_t argCount = (size_t)EnsureInt(strm);
                for (size_t k = 0; k < argCount; k++) {
                    args.push_back(strs.at(EnsureInt(strm)));
                }
                auto len = EnsureInt(strm);

                strm->Seek(len,
                           Tesses::Framework::Streams::SeekOrigin::Current);

                closures.push_back(args);
            }
        } else if (tableName == "FUNS") {
            size_t funLength = (size_t)EnsureInt(strm);

            for (size_t j = 0; j < funLength; j++) {
                std::vector<std::string> fnParts;
                uint32_t fnPartsC = EnsureInt(strm);
                for (uint32_t k = 0; k < fnPartsC; k++) {
                    fnParts.push_back(strs.at(EnsureInt(strm)));
                }

                uint32_t fnNumber = EnsureInt(strm);
                funs[fnNumber] = fnParts;
            }
        } else if (tableName == "INFO") {

            std::cout << "Info: " << strs.at((size_t)EnsureInt(strm))
                      << std::endl;

        } else {
            strm->Seek((int64_t)tableLen,
                       Tesses::Framework::Streams::SeekOrigin::Current);
        }
    }
    if (hasIcon)
        std::cout << "Has Icon: yes" << std::endl;
    else
        std::cout << "Has Icon: no" << std::endl;

    for (size_t i = 1; i < closures.size(); i++) {
        if (funs.count((uint32_t)i) > 0) {
            std::cout << "Func: ";
            auto res = funs[(uint32_t)i];
            if (!res.empty()) {
                std::cout << "/^" << res[0] << "^/ ";
            }
            for (size_t i = 1; i < res.size(); i++) {
                if (i > 1)
                    std::cout << ".";
                std::cout << res[i];
            }
        } else {
            std::cout << "Closure: ";
        }
        std::cout << "(";
        bool first = true;
        for (auto arg : closures[i]) {
            if (!first)
                std::cout << ", ";
            std::cout << arg;

            if (first)
                first = false;
        }
        std::cout << ")" << std::endl;
    }
    std::cout << std::endl;
    std::cout << "String Table:" << std::endl;

    for (auto str : strs) {
        std::cout << EscapeString(str, true) << std::endl;
    }
}

} // namespace Tesses::CrossLang::Programs