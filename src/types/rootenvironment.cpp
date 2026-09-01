#include "CrossLang.hpp"
#include <iostream>
#include <sstream>
namespace Tesses::CrossLang {
void ThrowConstError(std::string key) {
    throw std::runtime_error("Cannot set \"" + key +
                             "\" because it is a const");
}

void TEnvironment::DeclareConstVariable(std::string key, TObject value) {
    this->DeclareVariable(key, value);
    this->decls[key] = TEnvironmentDeclType::TEDT_CONST;
}
bool TEnvironment::HasConstForDeclare(std::string key) {
    if (this->decls.count(key) > 0)
        return this->decls[key] == TEnvironmentDeclType::TEDT_CONST;
    return false;
}
bool TEnvironment::HasConstForSet(std::string key) {
    return HasConstForDeclare(key);
}
bool TRootEnvironment::TryFindClass(std::vector<std::string> &name,
                                    size_t &index) {
    for (size_t i = 0; i < this->classes.size(); i++) {
        bool conUp = false;
        if (classes[i].first->classes.at(classes[i].second).name.size() !=
            name.size())
            continue;
        for (size_t j = 0; j < name.size(); j++)
            if (classes[i].first->classes.at(classes[i].second).name[j] !=
                name[j]) {
                conUp = true;
                break;
            }
        if (conUp)
            continue;
        index = i;
        return true;
    }
    return false;
}
TDictionary *TRootEnvironment::GetPrivateFromFile(std::shared_ptr<GC> gc,
                                                  TFile *file) {
    auto index = reinterpret_cast<uint64_t>(file);
    gc->BarrierBegin();
    TDictionary *obj;
    if (this->private_file_data.count(index) > 0) {
        obj = this->private_file_data[index];
    } else {
        GCList ls(gc);
        obj = TDictionary::Create(ls);
        this->private_file_data[index] = obj;
    }
    gc->BarrierEnd();

    return obj;
}
bool TRootEnvironment::HasVariableOrFieldRecurse(std::string key,
                                                 bool setting) {
    std::string property = (setting ? "set" : "get") + key;
    if (this->HasVariable(property)) {
        auto res = this->GetVariable(property);
        TCallable *callable;
        if (GetObjectHeap(res, callable))
            return true;
    }

    return this->HasVariable(key);
}
TObject TRootEnvironment::GetVariable(GCList &ls, std::string key) {
    ls.GetGC()->BarrierBegin();
    if (this->HasVariable("get" + key)) {
        auto item = this->GetVariable("get" + key);
        TCallable *callable;
        if (GetObjectHeap(item, callable)) {
            ls.GetGC()->BarrierEnd();
            return callable->Call(ls, {});
        }
    }

    auto item = this->GetVariable(key);
    ls.GetGC()->BarrierEnd();

    return item;
}
TObject TRootEnvironment::SetVariable(GCList &ls, std::string key,
                                      TObject value) {
    ls.GetGC()->BarrierBegin();
    if (this->HasVariable("set" + key)) {
        auto item = this->GetVariable("set" + key);
        TCallable *callable;
        if (GetObjectHeap(item, callable)) {
            ls.GetGC()->BarrierEnd();
            return callable->Call(ls, {value});
        }
    }

    this->SetVariable(key, value);
    ls.GetGC()->BarrierEnd();

    return value;
}

void TRootEnvironment::LoadDependency(
    std::shared_ptr<GC> gc,
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs,
    std::pair<std::string, TVMVersion> dep) {
    for (auto item : this->dependencies)
        if (item.first == dep.first && item.second.CompareTo(dep.second) >= 0)
            return;
    std::string name = {};
    name.append(dep.first);
    name.push_back('-');
    name.append(dep.second.ToString());
    name.append(".crvm");
    std::string filename = "/" + name;

    if (vfs->RegularFileExists(filename)) {
        auto file = vfs->OpenFile(filename, "rb");
        GCList ls(gc);
        TFile *f = TFile::Create(ls);
        f->Load(gc, file);

        LoadFileWithDependencies(gc, vfs, f);
    } else
        throw VMException("Could not open file: \"" + name + "\".");
}
TObject TEnvironment::Eval(GCList &ls, std::string code) {
    std::stringstream strm(code);
    std::vector<LexToken> tokens;
    int res = Lex("eval.tcross", strm, tokens);
    if (res != 0) {
        throw VMException("Lex error at line: " + std::to_string(res));
    }
    Parser parser(tokens);

    SyntaxNode n = parser.ParseRoot();
    CodeGen gen;
    gen.GenRoot(n);
    auto ms = std::make_shared<Tesses::Framework::Streams::MemoryStream>(true);
    gen.Save(ms);
    ms->Seek(0, Tesses::Framework::Streams::SeekOrigin::Begin);
    TFile *f = TFile::Create(ls);
    f->Load(ls.GetGC(), ms);
    return this->LoadFile(ls.GetGC(), f);
}
TDictionary *TEnvironment::EnsureDictionary(std::shared_ptr<GC> gc,
                                            std::string key) {
    TObject item = this->GetVariable(key);
    TDictionary *dict;
    if (GetObjectHeap(item, dict))
        return dict;
    GCList ls(gc);
    dict = TDictionary::Create(ls);
    this->DeclareVariable(key, dict);
    return dict;
}
void TEnvironment::DeclareVariable(std::shared_ptr<GC> gc,
                                   std::vector<std::string> name, TObject o) {
    if (name.size() == 0)
        throw VMException("name can't be empty.");

    else if (name.size() == 1) {
        GCList ls(gc);

        gc->BarrierBegin();
        this->DeclareVariable(name[0], o);
        gc->BarrierEnd();
    } else {
        GCList ls(gc);

        TObject v = this->GetVariable(name[0]);
        TDictionary *dict = nullptr;
        if (std::holds_alternative<THeapObjectHolder>(v)) {
            dict =
                dynamic_cast<TDictionary *>(std::get<THeapObjectHolder>(v).obj);
            if (dict == nullptr) {
                dict = TDictionary::Create(ls);
                gc->BarrierBegin();
                this->SetVariable(name[0], dict);
                gc->BarrierEnd();
            }
        } else {
            dict = TDictionary::Create(ls);
            gc->BarrierBegin();
            this->DeclareVariable(name[0], dict);
            gc->BarrierEnd();
        }

        for (size_t i = 1; i < name.size() - 1; i++) {
            gc->BarrierBegin();
            auto v = dict->GetValue(name[i]);
            gc->BarrierEnd();
            if (std::holds_alternative<THeapObjectHolder>(v)) {
                auto dict2 = dynamic_cast<TDictionary *>(
                    std::get<THeapObjectHolder>(v).obj);
                if (dict2 == nullptr) {
                    dict2 = TDictionary::Create(ls);
                    gc->BarrierBegin();
                    dict->SetValue(name[i], dict2);
                    gc->BarrierEnd();
                }
                dict = dict2;
            } else {
                auto dict2 = TDictionary::Create(ls);
                gc->BarrierBegin();
                dict->SetValue(name[i], dict2);
                gc->BarrierEnd();
                dict = dict2;
            }
        }
        gc->BarrierBegin();
        dict->SetValue(name[name.size() - 1], o);
        gc->BarrierEnd();
    }
}

TObject TEnvironment::LoadFile(std::shared_ptr<GC> gc, TFile *file) {
    file->EnsureCanRunInCrossLang();
    for (size_t i = 0; i < file->classes.size(); i++) {
        this->GetRootEnvironment()->classes.push_back(
            std::pair<TFile *, uint32_t>(file, (uint32_t)i));
        std::vector<std::string> clsPart = {"New"};
        clsPart.insert(clsPart.end(), file->classes[i].name.begin(),
                       file->classes[i].name.end());
        GCList ls(gc);
        std::vector<std::string> name = file->classes[i].name;
        auto rootEnv = this->GetRootEnvironment();
        this->DeclareVariable(
            gc, clsPart,
            TExternalMethod::Create(
                ls, "Create instance of the class", {"$$args"},
                [rootEnv, file, i](GCList &ls,
                                   std::vector<TObject> args) -> TObject {
                    return TClassObject::Create(ls, file, i, rootEnv, args);
                }));
        for (auto meth : file->classes[i].entry) {

            if (meth.isFunction && meth.modifier == TClassModifier::Static) {
                std::vector<std::string> method = file->classes[i].name;
                method.push_back(meth.name);
                auto clo = TClosure::Create(ls, this, file, meth.chunkId);
                clo->closure->name = JoinPeriod(method);

                clo->documentation = meth.documentation;
                this->DeclareVariable(gc, method, clo);
            }
        }
    }
    for (auto fn : file->functions) {

        if (fn.first.size() < 2)
            throw VMException("No function name.");

        std::vector<std::string> items(fn.first.begin() + 1, fn.first.end());

        if (fn.second >= file->chunks.size())
            throw VMException("ChunkId out of bounds.");
        TFileChunk *chunk = file->chunks[fn.second];
        chunk->name = JoinPeriod(items);
        GCList ls(gc);
        TClosure *closure = TClosure::Create(ls, this, file, fn.second);
        closure->documentation = fn.first[0];
        this->DeclareVariable(gc, items, closure);
    }
    if (!file->chunks.empty()) {
        GCList ls(gc);
        TClosure *closure = TClosure::Create(ls, this, file, 0);
        return closure->Call(ls, {});
    }
    return nullptr;
}
void TRootEnvironment::LoadFileWithDependencies(
    std::shared_ptr<GC> gc,
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs, TFile *file) {
    this->dependencies.push_back(
        std::pair<std::string, TVMVersion>(file->name, file->version));
    for (auto item : file->dependencies) {
        LoadDependency(gc, vfs, item);
    }
    LoadFile(gc, file);
}
void TRootEnvironment::LoadFileWithDependencies(
    std::shared_ptr<GC> gc,
    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs,
    Tesses::Framework::Filesystem::VFSPath path) {

    if (vfs->RegularFileExists(path)) {
        auto file = vfs->OpenFile(path, "rb");
        GCList ls(gc);
        TFile *f = TFile::Create(ls);
        f->Load(gc, file);

        auto dir =
            std::make_shared<Tesses::Framework::Filesystem::SubdirFilesystem>(
                vfs, path.GetParent());
        LoadFileWithDependencies(gc, dir, f);
    } else
        throw VMException("Could not open file: \"" + path.GetFileName() +
                          "\".");
}
TDictionary *TRootEnvironment::GetDictionary() { return this->dict; }
TObject TRootEnvironment::GetVariable(std::string key) {
    return this->dict->GetValue(key);
}
void TRootEnvironment::SetVariable(std::string key, TObject value) {
    this->dict->SetValue(key, value);
}
void TRootEnvironment::DeclareVariable(std::string key, TObject value) {
    if (this->decls.count(key) == 0)
        this->decls[key] = TEnvironmentDeclType::TEDT_VAR;
    return this->dict->SetValue(key, value);
}
bool TRootEnvironment::HasVariable(std::string key) {
    return this->dict->HasValue(key) || this->decls.count(key) > 0;
}
bool TRootEnvironment::HasVariableRecurse(std::string key) {
    return this->dict->HasValue(key) || this->decls.count(key) > 0;
}
TEnvironment *TRootEnvironment::GetParentEnvironment() { return this; }
TRootEnvironment *TRootEnvironment::GetRootEnvironment() { return this; }

TRootEnvironment::TRootEnvironment(TDictionary *dict) { this->dict = dict; }

void TRootEnvironment::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    this->dict->Mark();
    if (this->permissions.customConsole != nullptr)
        this->permissions.customConsole->Mark();
    for (auto priv : this->private_file_data)
        priv.second->Mark();
    for (auto defer : this->defers)
        defer->Mark();
    if (this->error != nullptr)
        this->error->Mark();
    for (auto cls : this->classes)
        cls.first->Mark();
}
TRootEnvironment *TRootEnvironment::Create(GCList *gc, TDictionary *dict) {
    return gc->Create<TRootEnvironment>(dict);
}
TRootEnvironment *TRootEnvironment::Create(GCList &gc, TDictionary *dict) {
    return gc.Create<TRootEnvironment>(dict);
}

bool TRootEnvironment::HandleException(std::shared_ptr<GC> gc,
                                       TEnvironment *env, TObject err) {
    if (error != nullptr) {
        GCList ls(gc);
        return ToBool(error->Call(
            ls, {TDictionary::Create(ls, {TDItem("IsBreakpoint", false),
                                          TDItem("Exception", err),
                                          TDItem("Environment", env)})}));
    }
    return false;
}
bool TRootEnvironment::HandleBreakpoint(std::shared_ptr<GC> gc,
                                        TEnvironment *env, TObject err) {
    if (error != nullptr) {
        GCList ls(gc);
        return ToBool(error->Call(
            ls, {TDictionary::Create(ls, {TDItem("IsBreakpoint", true),
                                          TDItem("Breakpoint", err),
                                          TDItem("Environment", env)})}));
    }
    return true;
}
void TRootEnvironment::RegisterOnError(TCallable *call) { this->error = call; }
}; // namespace Tesses::CrossLang
