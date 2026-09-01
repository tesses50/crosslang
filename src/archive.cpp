#include "CrossLang.hpp"
#include <iostream>
namespace Tesses::CrossLang {

void CrossArchiveCreate(
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs,
    std::shared_ptr<Tesses::Framework::Streams::Stream> strm, std::string name,
    TVMVersion version, std::string info, std::string icon) {
    std::vector<std::string> ignored_files;
    std::string file = "/.crossarchiveignore";
    if (vfs->FileExists(file)) {
        auto strm = vfs->OpenFile(file, "rb");
        if (strm != nullptr) {
            Tesses::Framework::TextStreams::StreamReader reader(strm);
            std::string ignores;
            while (reader.ReadLine(ignores)) {
                if (!ignores.empty() && ignores[0] != '#') {
                    ignored_files.push_back(ignores);
                }
                ignores.clear();
            }
        }
    }
    static std::vector<uint8_t> error_message_byte_code = {
        PUSHSTRING, 0,          0, 0, 2, GETVARIABLE, PUSHSTRING, 0, 0, 0,
        3,          PUSHSTRING, 0, 0, 0, 4,           PUSHLONG,   0, 0, 0,
        0,          0,          0, 0, 1, CALLMETHOD,  RET};
    auto writeInt = [](std::shared_ptr<Tesses::Framework::Streams::Stream> strm,
                       uint32_t number) -> void {
        uint8_t buff[4];
        BitConverter::FromUint32BE(buff[0], number);
        strm->WriteBlock(buff, 4);
    };
    auto writeStr =
        [&writeInt](std::shared_ptr<Tesses::Framework::Streams::Stream> strm,
                    std::string text) -> void {
        writeInt(strm, (uint32_t)text.size());
        strm->WriteBlock((const uint8_t *)text.c_str(), text.size());
    };

    std::vector<std::string> strs;
    strs.push_back(name);
    strs.push_back(info);
    strs.push_back("Console");
    strs.push_back("WriteLine");
    strs.push_back("You are trying to run a crvm archive, stop it you won't "
                   "get anywhere!\nUse crosslang archiveextract instead.");

    std::vector<std::string> resources;

    auto ensureResource = [&resources](std::string path) -> uint32_t {
        for (uint32_t i = 0; i < (uint32_t)resources.size(); i++) {
            if (resources[i] == path)
                return i;
        }
        uint32_t index = (uint32_t)resources.size();
        resources.push_back(path);
        return index;
    };
    auto ensureString = [&strs](std::string str) -> uint32_t {
        for (uint32_t i = 0; i < (uint32_t)strs.size(); i++) {
            if (strs[i] == str)
                return i;
        }
        uint32_t index = (uint32_t)strs.size();
        strs.push_back(str);
        return index;
    };
    auto ms = std::make_shared<Tesses::Framework::Streams::MemoryStream>(true);

    std::function<void(Tesses::Framework::Filesystem::VFSPath)> walkFS =
        [&ignored_files, vfs, &ensureString, &ensureResource, &ms, &walkFS,
         &writeInt](Tesses::Framework::Filesystem::VFSPath path) -> void {
        if (vfs->DirectoryExists(path)) {

            ms->WriteByte(1);
            std::vector<std::string> paths;
            for (auto item : vfs->EnumeratePaths(path)) {
                if (!item.relative && item.path.size() == 1 &&
                    item.path[0] == "__resdir_tmp")
                    continue;
                if (!item.relative && item.path.size() == 1 &&
                    item.path[0] == ".crossarchiveignore")
                    continue;
                std::string filename = item.GetFileName();
                bool shallIgnore = false;
                for (auto ign : ignored_files)
                    if (ign == filename) {
                        shallIgnore = true;
                        break;
                    }
                if (shallIgnore)
                    continue;
                if (vfs->DirectoryExists(item) || vfs->RegularFileExists(item))
                    paths.push_back(item.GetFileName());
            }
            writeInt(ms, (uint32_t)paths.size());
            for (auto item : paths) {
                writeInt(ms, ensureString(item));
                walkFS(path / item);
            }
        } else if (vfs->RegularFileExists(path)) {
            ms->WriteByte(0);
            writeInt(ms, ensureResource(path.ToString()));
        }
    };

    walkFS(std::string("/"));
    if (!icon.empty())
        ensureResource(icon);

    uint8_t main_header[18];
    memcpy(main_header, "TCROSSVM", 8);
    TVMVersion rtVersion(CROSSLANG_BYTECODE_MAJOR, CROSSLANG_BYTECODE_MINOR,
                         CROSSLANG_BYTECODE_PATCH, CROSSLANG_BYTECODE_BUILD,
                         CROSSLANG_BYTECODE_VERSIONSTAGE);
    rtVersion.ToArray(main_header + 8);
    version.ToArray(main_header + 13);
    strm->WriteBlock(main_header, sizeof(main_header));
    writeInt(strm, (uint32_t)((icon.empty() ? 5 : 6) + resources.size()));
    strm->WriteBlock((const uint8_t *)"STRS", 4);
    uint32_t sz = 4;
    for (auto str : strs)
        sz += (uint32_t)(4 + str.size());
    writeInt(strm, sz);
    writeInt(strm, (uint32_t)strs.size());
    for (auto str : strs) {
        writeStr(strm, str);
    }
    strm->WriteBlock((const uint8_t *)"NAME", 4);
    writeInt(strm, 4);
    writeInt(strm, 0);
    strm->WriteBlock((const uint8_t *)"INFO", 4);
    writeInt(strm, 4);
    writeInt(strm, 1);
    if (!icon.empty()) {
        strm->WriteBlock((const uint8_t *)"ICON", 4);
        writeInt(strm, 4);
        writeInt(strm, ensureResource(icon));
    }
    strm->WriteBlock((const uint8_t *)"CHKS", 4);
    writeInt(strm, (uint32_t)(12 + error_message_byte_code.size()));
    writeInt(strm, 1);
    writeInt(strm, 0);
    writeInt(strm, (uint32_t)error_message_byte_code.size());
    strm->WriteBlock(error_message_byte_code.data(),
                     error_message_byte_code.size());

    for (auto res : resources) {
        strm->WriteBlock((const uint8_t *)"RESO", 4);
        auto strm2 = vfs->OpenFile(res, "rb");
        writeInt(strm, (uint32_t)strm2->GetLength());
        strm2->CopyTo(strm);
    }
    strm->WriteBlock((const uint8_t *)"ARCV", 4);
    writeInt(strm, (uint32_t)ms->GetLength());
    ms->Seek(0, Tesses::Framework::Streams::SeekOrigin::Begin);
    ms->CopyTo(strm);
}
std::pair<std::pair<std::string, TVMVersion>, std::string>
CrossArchiveExtract(std::shared_ptr<Tesses::Framework::Streams::Stream> strm,
                    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs) {
    auto ensure = [strm](uint8_t *buffer, size_t count) -> void {
        size_t read = strm->ReadBlock(buffer, count);
        if (read < count)
            throw VMException(
                "End of file, could not read " +
                std::to_string((int64_t)count) +
                " byte(s)., offset=" + std::to_string(strm->GetLength()));
    };
    auto ensureInt = [&ensure]() -> uint32_t {
        uint8_t buffer[4];
        ensure(buffer, 4);
        return BitConverter::ToUint32BE(buffer[0]);
    };
    auto ensureStr = [&ensure, &ensureInt]() -> std::string {
        auto len = ensureInt();
        if (len == 0)
            return {};
        std::string str = {};
        str.resize((size_t)len);
        ensure((uint8_t *)str.data(), str.size());
        return str;
    };
    std::vector<std::string> strs;
    auto getStr = [&ensure, &ensureInt, &strs]() -> std::string {
        auto index = ensureInt();
        if (index > strs.size())
            throw VMException("String does not exist in TCrossVM file, "
                              "expected string index: " +
                              std::to_string(index) + ", total strings: " +
                              std::to_string(strs.size()));
        return strs[index];
    };
    uint8_t main_header[18];
    ensure(main_header, sizeof(main_header));
    if (strncmp((const char *)main_header, "TCROSSVM", 8) != 0)
        throw VMException("Invalid TCrossVM image.");
    TVMVersion version(main_header + 8);
    if (version.CompareToRuntime() == 1) {
        throw VMException("Runtime is too old.");
    }
    TVMVersion v2(main_header + 13);
    std::string name;
    std::string info;

    size_t _len = (size_t)ensureInt();

    char table_name[4];

    uint32_t resource_id = 0;

    Tesses::Framework::Filesystem::VFSPath tmpDir("/__resdir_tmp");

    vfs->CreateDirectory(tmpDir);
    for (size_t i = 0; i < _len; i++) {
        ensure((uint8_t *)table_name, sizeof(table_name));
        size_t tableLen = (size_t)ensureInt();
        if (strncmp(table_name, "NAME", 4) == 0) {
            name = getStr();
        } else if (strncmp(table_name, "INFO", 4) == 0) {
            info = getStr();

        }

        else if (strncmp(table_name, "RESO", 4) == 0) // resources (using embed)
        {

            auto path = tmpDir / std::to_string(resource_id);

            auto strm2 = vfs->OpenFile(path, "wb");
            size_t read = 0;
            size_t offset = 0;
            uint8_t buff[1024];
            do {

                read = std::min(std::min(tableLen - offset, tableLen),
                                sizeof(buff));
                read = strm->Read(buff, read);
                if (read > 0)
                    strm2->WriteBlock(buff, read);
                offset += read;
            } while (read > 0);

            resource_id++;
        }

        else if (strncmp(table_name, "STRS", 4) == 0) // strings
        {
            size_t strsLen = (size_t)ensureInt();
            for (size_t j = 0; j < strsLen; j++) {
                strs.push_back(ensureStr());
            }
        } else if (strncmp(table_name, "ARCV", 4) == 0) {
            size_t offset = 0;

            std::function<void(Tesses::Framework::Filesystem::VFSPath)>
                walkEntry =
                    [strm, vfs, &getStr, &ensureInt, &tmpDir, &tableLen,
                     &offset, &walkEntry](
                        Tesses::Framework::Filesystem::VFSPath path) -> void {
                if (offset + 1 > tableLen)
                    return;
                uint8_t type = strm->ReadByte();
                offset++;
                if (type == 1) {
                    // ISDIR
                    vfs->CreateDirectory(path);
                    if (offset + 4 > tableLen)
                        return;
                    uint32_t count = ensureInt();
                    offset += 4;
                    for (uint32_t i = 0; i < count; i++) {
                        if (offset + 4 > tableLen)
                            return;
                        std::string name = getStr();
                        offset += 4;
                        walkEntry(path / name);
                    }
                } else if (type == 0) {
                    if (offset + 4 > tableLen)
                        return;
                    uint32_t index = ensureInt();
                    auto fSrc = tmpDir / std::to_string(index);
                    vfs->MoveFile(fSrc, path);
                }
            };

            walkEntry(std::string("/"));
        } else {
            if (strm->CanSeek()) {
                strm->Seek((int64_t)tableLen,
                           Tesses::Framework::Streams::SeekOrigin::Current);
            } else {
                uint8_t *buffer = new uint8_t[tableLen];
                ensure(buffer, tableLen);
                delete[] buffer;
            }
        }
    }
    vfs->DeleteDirectoryRecurse(tmpDir);

    return std::pair<std::pair<std::string, TVMVersion>, std::string>(
        std::pair<std::string, TVMVersion>(name, v2), info);
}
} // namespace Tesses::CrossLang
