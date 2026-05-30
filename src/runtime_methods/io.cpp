#include "CrossLang.hpp"

namespace Tesses::CrossLang {
static TObject FS_CreateArchive(GCList &ls, std::vector<TObject> args) {
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;

    std::shared_ptr<Tesses::Framework::Streams::Stream> strm;

    std::string name;
    std::string version;
    std::string info;
    std::string icon = "";
    TVMVersion version2;

    if (GetArgument(args, 0, vfs) && GetArgument(args, 1, strm) &&
        GetArgument(args, 2, name) && GetArgument(args, 4, info) &&
        ((GetArgument(args, 3, version) &&
          TVMVersion::TryParse(version, version2)) ||
         GetArgument(args, 3, version2))) {
        GetArgument(args, 5, icon);
        CrossArchiveCreate(vfs, strm, name, version2, info, icon);
    }
    return nullptr;
}
static TObject FS_ExtractArchive(GCList &ls, std::vector<TObject> args) {
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;

    std::shared_ptr<Tesses::Framework::Streams::Stream> strm;

    if (GetArgument(args, 0, strm) && GetArgument(args, 1, vfs)) {
        auto res = CrossArchiveExtract(strm, vfs);

        TDictionary *dict = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();
        dict->SetValue("Name", res.first.first);
        dict->SetValue("Version", res.first.second.ToString());
        dict->SetValue("Info", res.second);
        ls.GetGC()->BarrierEnd();
        return dict;
    }
    return nullptr;
}

static TObject FS_ReadAllText(GCList &ls, std::vector<TObject> args) {
    Tesses::Framework::Filesystem::VFSPath path;

    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;

    if (GetArgument(args, 0, vfs) && GetArgumentAsPath(args, 1, path)) {
        return Tesses::Framework::Filesystem::Helpers::ReadAllText(vfs, path);
    }
    return "";
}

static TObject FS_ReadAllLines(GCList &ls, std::vector<TObject> args) {
    Tesses::Framework::Filesystem::VFSPath path;

    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;

    if (GetArgument(args, 0, vfs) && GetArgumentAsPath(args, 1, path)) {
        std::vector<std::string> lines;

        Tesses::Framework::Filesystem::Helpers::ReadAllLines(vfs, path, lines);

        ls.GetGC()->BarrierBegin();
        auto items = TList::Create(ls);
        for (auto &l : lines) {
            items->Add(l);
        }
        ls.GetGC()->BarrierEnd();
        return items;
    }
    return nullptr;
}

static TObject FS_ReadAllBytes(GCList &ls, std::vector<TObject> args) {
    Tesses::Framework::Filesystem::VFSPath path;
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;

    if (GetArgument(args, 0, vfs) && GetArgumentAsPath(args, 1, path)) {
        auto res = TByteArray::Create(ls);
        Tesses::Framework::Filesystem::Helpers::ReadAllBytes(vfs, path,
                                                             res->data);

        return res;
    }
    return nullptr;
}

static TObject FS_WriteAllLines(GCList &ls, std::vector<TObject> args) {
    Tesses::Framework::Filesystem::VFSPath path;
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;

    TList *lines;
    if (GetArgument(args, 0, vfs) && GetArgumentAsPath(args, 1, path) &&
        GetArgumentHeap(args, 2, lines)) {
        std::vector<std::string> content;
        ls.GetGC()->BarrierBegin();
        for (auto &item : lines->items) {
            if (std::holds_alternative<std::string>(item))
                content.push_back(std::get<std::string>(item));
        }
        ls.GetGC()->BarrierEnd();
        Tesses::Framework::Filesystem::Helpers::WriteAllLines(vfs, path,
                                                              content);
    }
    return nullptr;
}
static TObject FS_WriteAllText(GCList &ls, std::vector<TObject> args) {
    Tesses::Framework::Filesystem::VFSPath path;
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;

    std::string content;
    if (GetArgument(args, 0, vfs) && GetArgumentAsPath(args, 1, path) &&
        GetArgument(args, 2, content)) {
        Tesses::Framework::Filesystem::Helpers::WriteAllText(vfs, path,
                                                             content);
    }
    return nullptr;
}
static TObject FS_WriteAllBytes(GCList &ls, std::vector<TObject> args) {
    Tesses::Framework::Filesystem::VFSPath path;

    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;

    TByteArray *bArray;
    if (GetArgument(args, 0, vfs) && GetArgumentAsPath(args, 1, path) &&
        GetArgumentHeap(args, 2, bArray)) {
        Tesses::Framework::Filesystem::Helpers::WriteAllBytes(vfs, path,
                                                              bArray->data);
    }
    return nullptr;
}

class FS_Watcher : public TNativeObject {
  public:
    std::shared_ptr<Tesses::Framework::Filesystem::FSWatcher> watcher;

    FS_Watcher(
        std::shared_ptr<Tesses::Framework::Filesystem::FSWatcher> watcher)
        : watcher(watcher) {}

    TObject CallMethod(GCList &ls, std::string name,
                       std::vector<TObject> args) {
        if (name == "getPath") {
            return watcher->GetPath();
        }
        if (name == "getFilesystem") {
            return watcher->GetFilesystem();
        }
        if (name == "setEnabled") {
            bool enabled;
            if (GetArgument(args, 0, enabled)) {
                watcher->SetEnabled(enabled);
                return enabled;
            }
        }
        if (name == "getEnabled") {
            return watcher->GetEnabled();
        }
        if (name == "setEvents") {
            int64_t evts;
            if (GetArgument(args, 0, evts)) {
                watcher->events =
                    (Tesses::Framework::Filesystem::FSWatcherEventType)evts;
                return evts;
            }
        }
        if (name == "getEvents") {
            return (int64_t)watcher->events;
        }
        if (name == "setCallback") {
            TCallable *callable = nullptr;
            if (GetArgumentHeap(args, 0, callable)) {
                auto markedT = CreateMarkedTObject(ls, callable);
                watcher->event =
                    [markedT](
                        Tesses::Framework::Filesystem::FSWatcherEvent &evt)
                    -> void {
                    GCList ls(markedT->GetGC());
                    TObject o = markedT->GetObject();
                    TCallable *callable;
                    if (GetObjectHeap(o, callable)) {
                        auto isEvent = TExternalMethod::Create(
                            ls, "", {},
                            [evt](GCList &ls,
                                  std::vector<TObject> args) -> TObject {
                                int64_t n;
                                if (GetArgument(args, 0, n)) {
                                    auto myevt = evt;
                                    return myevt.IsEvent(
                                        (Tesses::Framework::Filesystem::
                                             FSWatcherEventType)n);
                                }

                                return false;
                            });
                        auto toString = TExternalMethod::Create(
                            ls, "", {},
                            [evt](GCList &ls,
                                  std::vector<TObject> args) -> TObject {
                                auto myevt = evt;
                                return myevt.ToString();
                            });

                        auto dict = TDictionary::Create(
                            ls, {TDItem("IsDirectory", evt.isDir),
                                 TDItem("Type", (int64_t)evt.type),
                                 TDItem("Source", evt.src),
                                 TDItem("Destination", evt.dest),
                                 TDItem("ToString", toString),
                                 TDItem("IsEvent", isEvent)});
                        callable->Call(ls, {dict});
                    }
                };
            } else {
                watcher->event = nullptr;
            }
            return nullptr;
        }
        if (name == "Start") {
            watcher->SetEnabled(true);
        }
        if (name == "Stop") {
            watcher->SetEnabled(false);
        }
        if (name == "ToString") {
            return "FSWatcher";
        }
        return Undefined();
    }

    std::string TypeName() { return "FSWatcher"; }
};

TObject New_FSWatcher(GCList &ls, std::vector<TObject> args) {
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
    Tesses::Framework::Filesystem::VFSPath path;
    if (GetArgument(args, 0, vfs) && GetArgumentAsPath(args, 1, path)) {
        return TNativeObject::Create<FS_Watcher>(
            ls, Tesses::Framework::Filesystem::FSWatcher::Create(vfs, path));
    }
    return nullptr;
}
void TStd::RegisterIO(std::shared_ptr<GC> gc, TRootEnvironment *env,
                      bool enable) {
    if (enable) {
        RegisterIO(
            gc, env,
            std::make_shared<Tesses::Framework::Filesystem::RelativeFilesystem>(
                Tesses::Framework::Filesystem::LocalFS,
                Tesses::Framework::Filesystem::VFSPath::
                    GetAbsoluteCurrentDirectory()));
    } else {
        RegisterIO(gc, env, nullptr);
    }
}
void TStd::RegisterIO(
    std::shared_ptr<GC> gc, TRootEnvironment *env,
    std::shared_ptr<Tesses::Framework::Filesystem::RelativeFilesystem> fs) {

    env->permissions.canRegisterIO = true;
    env->permissions.localfs = fs;
    GCList ls(gc);

    gc->BarrierBegin();
    auto newDict = env->EnsureDictionary(gc, "New");
    newDict->DeclareFunction(gc, "FSWatcher", "Watch a file/directory",
                             {"vfs", "path"}, New_FSWatcher);
    auto dict = env->EnsureDictionary(gc, "FS");
    dict->SetValue("SEEK_SET",
                   (int64_t)Tesses::Framework::Streams::SeekOrigin::Begin);
    dict->SetValue("SEEK_CUR",
                   (int64_t)Tesses::Framework::Streams::SeekOrigin::Current);
    dict->SetValue("SEEK_END",
                   (int64_t)Tesses::Framework::Streams::SeekOrigin::End);
    dict->SetValue(
        "FSWatcherEvents",
        TDictionary::Create(
            ls,
            {TDItem(
                 "None",
                 (int64_t)
                     Tesses::Framework::Filesystem::FSWatcherEventType::None),
             TDItem("Accessed", (int64_t)Tesses::Framework::Filesystem::
                                    FSWatcherEventType::Accessed),
             TDItem("AttributeChanged",
                    (int64_t)Tesses::Framework::Filesystem::FSWatcherEventType::
                        AttributeChanged),
             TDItem(
                 "Writen",
                 (int64_t)
                     Tesses::Framework::Filesystem::FSWatcherEventType::Writen),
             TDItem(
                 "Read",
                 (int64_t)
                     Tesses::Framework::Filesystem::FSWatcherEventType::Read),
             TDItem("Created", (int64_t)Tesses::Framework::Filesystem::
                                   FSWatcherEventType::Created),
             TDItem("Deleted", (int64_t)Tesses::Framework::Filesystem::
                                   FSWatcherEventType::Deleted),
             TDItem("WatchEntryDeleted",
                    (int64_t)Tesses::Framework::Filesystem::FSWatcherEventType::
                        WatchEntryDeleted),
             TDItem("Modified", (int64_t)Tesses::Framework::Filesystem::
                                    FSWatcherEventType::Modified),
             TDItem("WatchEntryMoved", (int64_t)Tesses::Framework::Filesystem::
                                           FSWatcherEventType::WatchEntryMoved),
             TDItem("MoveOld", (int64_t)Tesses::Framework::Filesystem::
                                   FSWatcherEventType::MoveOld),
             TDItem("MoveNew", (int64_t)Tesses::Framework::Filesystem::
                                   FSWatcherEventType::MoveNew),
             TDItem(
                 "Opened",
                 (int64_t)
                     Tesses::Framework::Filesystem::FSWatcherEventType::Opened),
             TDItem(
                 "Closed",
                 (int64_t)
                     Tesses::Framework::Filesystem::FSWatcherEventType::Closed),
             TDItem(
                 "Moved",
                 (int64_t)
                     Tesses::Framework::Filesystem::FSWatcherEventType::Moved),
             TDItem(
                 "All",
                 (int64_t)
                     Tesses::Framework::Filesystem::FSWatcherEventType::All)}));

    if (fs) {

        dict->SetValue("Local", fs);
        dict->DeclareFunction(
            gc, "MakeFull", "Make absolute path from relative path", {"path"},
            [fs](GCList &ls, std::vector<TObject> args) -> TObject {
                Tesses::Framework::Filesystem::VFSPath path;
                if (GetArgumentAsPath(args, 0, path)) {
                    if (path.relative) {
                        auto myPath = fs->GetWorking() / path;
                        myPath = myPath.CollapseRelativeParents();
                        return myPath;
                    }
                    return path.CollapseRelativeParents();
                }
                return nullptr;
            });
        dict->DeclareFunction(
            gc, "getCurrentPath", "Get current path", {},
            [fs](GCList &ls, std::vector<TObject> args) -> TObject {
                return fs->GetWorking();
            });
        dict->DeclareFunction(
            gc, "setCurrentPath", "Set the current path", {"path"},
            [fs](GCList &ls, std::vector<TObject> args) -> TObject {
                Tesses::Framework::Filesystem::VFSPath path;
                if (GetArgumentAsPath(args, 0, path)) {
                    if (path.relative) {
                        fs->SetWorking(path.MakeAbsolute(fs->GetWorking()));
                    } else {
                        fs->SetWorking(path);
                    }
                }
                return path;
            });
    }

    dict->DeclareFunction(gc, "ReadAllText", "Read all text from file",
                          {"fs", "filename"}, FS_ReadAllText);
    dict->DeclareFunction(gc, "WriteAllText", "Write all text to file",
                          {"fs", "filename", "content"}, FS_WriteAllText);

    dict->DeclareFunction(gc, "ReadAllLines", "Read all lines from file",
                          {"fs", "filename"}, FS_ReadAllLines);
    dict->DeclareFunction(gc, "WriteAllLines", "Write all lines to file",
                          {"fs", "filename", "lines"}, FS_WriteAllLines);

    dict->DeclareFunction(gc, "ReadAllBytes", "Read all bytes from file",
                          {"fs", "filename"}, FS_ReadAllBytes);
    dict->DeclareFunction(gc, "WriteAllBytes", "Write all bytes to file",
                          {"fs", "filename", "content"}, FS_WriteAllBytes);

    dict->DeclareFunction(gc, "CreateArchive", "Create a crvm archive",
                          {"fs", "strm", "name", "version", "info"},
                          FS_CreateArchive);
    dict->DeclareFunction(gc, "ExtractArchive", "Extract a crvm archive",
                          {"strm", "vfs"}, FS_ExtractArchive);

    gc->BarrierEnd();
}
} // namespace Tesses::CrossLang