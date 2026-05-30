#include "CrossLang.hpp"
#include "TessesFramework/Filesystem/VFS.hpp"
using namespace Tesses::Framework;
using namespace Tesses::CrossLang;
using namespace Tesses::Framework::Filesystem;

int main(int argc, char **argv) {

    TF_InitWithConsole();
    TF_AllowPortable();

    std::vector<std::string> args(argc);

    for (int i = 0; i < argc; i++)
        args[i] = argv[i];

    auto execName = TF_GetExecutableName();

    if (!execName.empty()) {
        Tesses::Framework::Filesystem::VFSPath execPath = execName;
        if (execPath.HasExtension()) {
            execPath.RemoveExtension();
        }

        if (Tesses::Framework::Filesystem::LocalFS->FileExists(execPath +
                                                               ".crvm")) {

            if (args.empty())
                args.push_back(execPath.GetFileName());

            auto crvmFile = execPath + ".crvm";

            args.insert(args.begin() + 1, crvmFile.ToString());
            int64_t myi64 = 0;

            std::shared_ptr<GC> gc = std::make_shared<GC>();
            gc->Start();
            GCList ls(gc);
            TRootEnvironment *env =
                TRootEnvironment::Create(ls, TDictionary::Create(ls));
            TStd::RegisterStd(gc, env);
            auto res = Programs::CrossLangVM(ls, env, args);
            GetObject(res, myi64);
            return (int)myi64;
        }
    }

    if (args.size() > 1) {
        if (args[1] == "--help") {
            if (!Tesses::Framework::Filesystem::LocalFS->FileExists(
                    GetCrossLangConfigDir() / "Shell" / "Shell.crvm")) {
                Console::WriteLine(args[0] + " COMMAND");
                Console::WriteLine("COMMANDS:");
                Console::WriteLine(
                    "int:            interperter, used to be crossint");
                Console::WriteLine(
                    "c:              the compiler, used to be crossc");
                Console::WriteLine("archivecreate:  create archives, used to "
                                   "be crossarchivecreate");
                Console::WriteLine("archiveextract: extract archives, used to "
                                   "be crossarchiveextract");
                Console::WriteLine("thumbnailer:    create thumbnails on "
                                   "freedesktop, used to be crossthumbnailer");
                Console::WriteLine("dump:           dump metadata about crvm "
                                   "file, used to be crossdump");
                Console::WriteLine("vm:             interpert bytecode file, "
                                   "used to be crossvm");
                Console::WriteLine("update-shell:   update the shell");
                Console::WriteLine(
                    "configdir:      print the config directory");

                Console::WriteLine("");
                Console::WriteLine("NOTE: As you don't have the shell, these "
                                   "commands are limited");
                Console::WriteLine(
                    "NOTE: You can put this binary in a folder with a crvm "
                    "file, as long as filename matches (ignoring extension) it "
                    "will directly launch the crvm file");
                return 1;
            }
        }
        if (args[1] == "int") {
            int64_t myi64 = 0;
            args.erase(args.begin());
            std::shared_ptr<GC> gc = std::make_shared<GC>();
            gc->Start();
            GCList ls(gc);
            TRootEnvironment *env =
                TRootEnvironment::Create(ls, TDictionary::Create(ls));
            TStd::RegisterStd(gc, env);
            auto res = Programs::CrossLangInterperter(ls, env, args);

            GetObject(res, myi64);

            return (int)myi64;

        } else if (args[1] == "c") {
            args.erase(args.begin());
            Programs::CrossLangCompiler(args);
            return 0;
        } else if (args[1] == "archivecreate") {
            args.erase(args.begin());
            Programs::CrossArchiveCreate(args);
            return 0;
        } else if (args[1] == "archiveextract") {
            args.erase(args.begin());
            Programs::CrossArchiveExtract(args);
            return 0;
        } else if (args[1] == "thumbnailer") {
            Tesses::Framework::Filesystem::VFSPath execPath =
                TF_GetExecutableName();
            auto emptyThumb = execPath.GetParent().GetParent() / "share" /
                              "icons" / "crosslang.png";
            if (argc < 4) {
                std::cout << "USAGE: " << argv[0]
                          << " thumbnailer CRVMFILE NEWPNG" << std::endl;
                return 1;
            }

            std::string crvm = argv[2];
            std::string png = argv[3];

            if (Tesses::Framework::Filesystem::LocalFS->FileExists(crvm)) {

                Tesses::CrossLang::TFile file;
                auto f = Tesses::Framework::Filesystem::LocalFS->OpenFile(crvm,
                                                                          "rb");

                file.Load(nullptr, f);

                if (file.icon >= 0 && file.icon < file.resources.size()) {
                    auto f2 = Tesses::Framework::Filesystem::LocalFS->OpenFile(
                        png, "wb");
                    if (f2 != nullptr) {
                        auto &icon = file.resources[file.icon];
                        f2->WriteBlock(icon.data(), icon.size());
                    }
                    return 0;
                }
            }
            if (Tesses::Framework::Filesystem::LocalFS->FileExists(
                    emptyThumb)) {
                auto src = Tesses::Framework::Filesystem::LocalFS->OpenFile(
                    emptyThumb, "rb");
                auto dest =
                    Tesses::Framework::Filesystem::LocalFS->OpenFile(png, "wb");
                if (src != nullptr && dest != nullptr) {
                    src->CopyTo(dest);
                }
            }
            return 0;
        } else if (args[1] == "dump") {
            for (size_t i = 2; i < args.size(); i++) {
                VFSPath path = args[i];
                if (LocalFS->FileExists(path)) {
                    std::cout << "File: " << path.ToString() << std::endl;
                    auto strm = LocalFS->OpenFile(path, "rb");
                    Programs::CrossLangDump(strm);
                } else {
                    std::cout << "File: " << path.ToString()
                              << " does not exist." << std::endl;
                }
            }
            return 0;
        } else if (args[1] == "vm") {
            int64_t myi64 = 0;

            args.erase(args.begin());
            std::shared_ptr<GC> gc = std::make_shared<GC>();
            gc->Start();
            GCList ls(gc);
            TRootEnvironment *env =
                TRootEnvironment::Create(ls, TDictionary::Create(ls));
            TStd::RegisterStd(gc, env);
            auto res = Programs::CrossLangVM(ls, env, args);
            GetObject(res, myi64);

            return (int)myi64;
        }
    }
    {
        int64_t myi64 = 0;
        std::shared_ptr<GC> gc = std::make_shared<GC>();
        gc->Start();
        GCList ls(gc);
        auto res = Programs::CrossLangShell(ls, args);

        GetObject(res, myi64);

        return (int)myi64;
    }
}
