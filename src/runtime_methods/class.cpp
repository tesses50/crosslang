#include "CrossLang.hpp"

namespace Tesses::CrossLang {
static TList *VectorOfStringToList(GCList &ls, std::vector<std::string> &strs) {
    TList *list = TList::Create(ls);
    ls.GetGC()->BarrierBegin();
    for (auto &item : strs)
        list->Add(item);
    ls.GetGC()->BarrierEnd();
    return list;
}
static TList *EntriesToList(GCList &ls, std::vector<TClassEntry> &ents) {
    TList *list = TList::Create(ls);
    ls.GetGC()->BarrierBegin();
    for (auto &item : ents) {
        std::string modifier = "public";
        switch (item.modifier) {
        case TClassModifier::Public:
            modifier = "public";
            break;
        case TClassModifier::Private:
            modifier = "private";
            break;
        case TClassModifier::Protected:
            modifier = "protected";
            break;
        case TClassModifier::Static:
            modifier = "static";
            break;
        }

        list->Add(TDictionary::Create(
            ls,
            {TDItem("Name", item.name), TDItem("IsAbstract", item.isAbstract),
             TDItem("IsFunction", item.isFunction),
             TDItem("Documentation", item.documentation),
             TDItem("ChunkId", (int64_t)item.chunkId),
             TDItem("Arguments", VectorOfStringToList(ls, item.args)),
             TDItem("Modifier", modifier)}));
    }
    ls.GetGC()->BarrierEnd();
    return list;
}
static TList *ClassInstanceToList(GCList &ls, TClassObject *co) {
    TList *list = TList::Create(ls);
    ls.GetGC()->BarrierBegin();
    for (auto &item : co->entries) {
        if (item.modifier == TClassModifier::Public) {
            list->Add(TDictionary::Create(
                ls,
                {TDItem("Name", item.name), TDItem("IsFunction", !item.canSet),
                 TDItem("Owner", item.owner), TDItem("Value", item.value)}));
        }
    }
    ls.GetGC()->BarrierEnd();
    return list;
}
TObject GetClassInfo(GCList &ls, TFile *f, uint32_t index) {
    return TDictionary::Create(
        ls, {TDItem("Name", JoinPeriod(f->classes.at(index).name)),
             TDItem("NameParts",
                    VectorOfStringToList(ls, f->classes.at(index).name)),
             TDItem("Inherits", JoinPeriod(f->classes.at(index).inherits)),
             TDItem("InheritsParts",
                    VectorOfStringToList(ls, f->classes.at(index).inherits)),
             TDItem("Documentation", f->classes.at(index).documentation),
             TDItem("Entries", EntriesToList(ls, f->classes.at(index).entry))});
}
static TObject Class_CreateInstance(TRootEnvironment *env, GCList &ls,
                                    std::vector<TObject> args) {
    TList *args_ls;
    if (!GetArgumentHeap(args, 1, args_ls))
        return nullptr;
    TList *list;
    TClassObject *obj;
    std::string str;
    if (GetArgumentHeap(args, 0, list)) {
        std::vector<std::string> clsName;
        for (int64_t i = 0; i < list->Count(); i++) {
            auto o = list->Get(i);
            if (GetObject(o, str))
                clsName.push_back(str);
        }
        for (auto &item : env->classes) {
            auto &f = item.first->classes.at(item.second);

            if (f.name.size() != clsName.size())
                continue;

            bool found = true;
            for (size_t i = 0; i < f.name.size(); i++)
                if (f.name[i] != clsName[i]) {
                    found = false;
                    break;
                }
            if (found) {
                return TClassObject::Create(ls, item.first, item.second, env,
                                            args_ls->items);
            } else
                continue;
        }
    } else if (GetArgument(args, 0, str)) {
        std::vector<std::string> clsName =
            Tesses::Framework::Http::HttpUtils::SplitString(str, ".");
        for (auto &item : env->classes) {
            auto &f = item.first->classes.at(item.second);

            if (f.name.size() != clsName.size())
                continue;

            bool found = true;
            for (size_t i = 0; i < f.name.size(); i++)
                if (f.name[i] != clsName[i]) {
                    found = false;
                    break;
                }
            if (found) {
                return TClassObject::Create(ls, item.first, item.second, env,
                                            args_ls->items);

            } else
                continue;
        }
    }
    return nullptr;
}
static TObject Class_GetClassNames(TRootEnvironment *env, GCList &ls,
                                   std::vector<TObject> args) {
    TList *list = TList::Create(ls);
    ls.GetGC()->BarrierBegin();
    for (auto &item : env->classes) {
        list->Add(JoinPeriod(item.first->classes.at(item.second).name));
    }
    ls.GetGC()->BarrierEnd();
    return list;
}
static TObject Class_GetInfo(TRootEnvironment *env, GCList &ls,
                             std::vector<TObject> args) {

    TList *list;
    TClassObject *obj;
    std::string str;
    if (GetArgumentHeap(args, 0, list)) {
        std::vector<std::string> clsName;
        for (int64_t i = 0; i < list->Count(); i++) {
            auto o = list->Get(i);
            if (GetObject(o, str))
                clsName.push_back(str);
        }
        for (auto &item : env->classes) {
            auto &f = item.first->classes.at(item.second);

            if (f.name.size() != clsName.size())
                continue;

            bool found = true;
            for (size_t i = 0; i < f.name.size(); i++)
                if (f.name[i] != clsName[i]) {
                    found = false;
                    break;
                }
            if (found) {
                return GetClassInfo(ls, item.first, item.second);
            } else
                continue;
        }
    } else if (GetArgument(args, 0, str)) {
        std::vector<std::string> clsName =
            Tesses::Framework::Http::HttpUtils::SplitString(str, ".");
        for (auto &item : env->classes) {
            auto &f = item.first->classes.at(item.second);

            if (f.name.size() != clsName.size())
                continue;

            bool found = true;
            for (size_t i = 0; i < f.name.size(); i++)
                if (f.name[i] != clsName[i]) {
                    found = false;
                    break;
                }
            if (found) {
                return GetClassInfo(ls, item.first, item.second);
            } else
                continue;
        }
    } else if (GetArgumentHeap(args, 0, obj)) {
        return GetClassInfo(ls, obj->file, obj->classIndex);
    }

    return nullptr;
}
void TStd::RegisterClass(std::shared_ptr<GC> gc, TRootEnvironment *env) {
    GCList ls(gc);
    env->permissions.canRegisterClass = true;
    TDictionary *cls = env->EnsureDictionary(gc, "Class");
    gc->BarrierBegin();

    TExternalMethod *ext = TExternalMethod::Create(
        ls, "Get the class info", {"classInstanceOrClassName"},
        [env](GCList &ls, std::vector<TObject> args) -> TObject {
            return Class_GetInfo(env, ls, args);
        });
    ext->watch.push_back(env);

    cls->SetValue("GetInfo", ext);

    ext = TExternalMethod::Create(
        ls, "Get the class names", {},
        [env](GCList &ls, std::vector<TObject> args) -> TObject {
            return Class_GetClassNames(env, ls, args);
        });
    ext->watch.push_back(env);

    cls->SetValue("GetClassNames", ext);
    ext = TExternalMethod::Create(
        ls, "Create an instance of class", {"name", "args"},
        [env](GCList &ls, std::vector<TObject> args) -> TObject {
            return Class_CreateInstance(env, ls, args);
        });
    ext->watch.push_back(env);

    cls->SetValue("CreateInstance", ext);
    cls->DeclareFunction(gc, "Name", "Get class name via instance",
                         {"instance"},
                         [](GCList &ls, std::vector<TObject> args) -> TObject {
                             TClassObject *cls;
                             if (GetArgumentHeap(args, 0, cls)) {
                                 return cls->name;
                             }
                             return "";
                         });

    cls->DeclareFunction(
        gc, "GetInstanceInfo",
        "Get the instance specific info, including current values", {},
        [](GCList &ls, std::vector<TObject> args) -> TObject {
            TClassObject *co;
            if (GetArgumentHeap(args, 0, co)) {
                ls.GetGC()->BarrierBegin();
                auto res = TDictionary::Create(
                    ls, {TDItem("Name", co->name), TDItem("File", co->file),
                         TDItem("ClassIndex", (int64_t)co->classIndex),
                         TDItem("InheritList",
                                VectorOfStringToList(ls, co->inherit_tree)),
                         TDItem("Entries", ClassInstanceToList(ls, co))});
                ls.GetGC()->BarrierEnd();
                return res;
            }
            return nullptr;
        });

    gc->BarrierEnd();
}
} // namespace Tesses::CrossLang