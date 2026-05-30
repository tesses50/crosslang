#include "CrossLang.hpp"
using namespace Tesses::Framework;
namespace Tesses::CrossLang::Programs {
TObject CrossLangVM(GCList &ls, TRootEnvironment *env,
                    std::vector<std::string> &argv) {
    if (argv.size() < 2) {
        std::cout << "USAGE: "
                  << (argv.empty() ? (std::string) "crossvm" : argv[0])
                  << " <filename.crvm> <args...>" << std::endl;
        return 1;
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
            return 1;
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
        return 0;
    } else {
        TList *args = TList::Create(ls);
        for (size_t arg = 1; arg < argv.size(); arg++)
            args->Add(std::string(argv[arg]));

        return env->CallFunctionWithFatalError(ls, "main", {args});
    }
}
} // namespace Tesses::CrossLang::Programs