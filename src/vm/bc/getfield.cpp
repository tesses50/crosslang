#include "CrossLang.hpp"
#include "TessesFramework/Serialization/BitConverter.hpp"
#include "TessesFramework/Streams/ByteReader.hpp"
#include "TessesFramework/Uuid.hpp"
#include <cmath>
#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <variant>
namespace Tesses::CrossLang {
bool InterperterThread::GetField(std::shared_ptr<GC> gc) {
    std::vector<CallStackEntry *> &cse = this->call_stack_entries;

    if (!cse.empty()) {
        auto stk = cse.back();
        GCList ls(gc);
        TObject _key = stk->Pop(ls);

        TObject instance = stk->Pop(ls);

        if (!std::holds_alternative<std::string>(_key)) {
            stk->Push(gc, Undefined());
            return false;
        }

        std::string key = std::get<std::string>(_key);

        if (std::holds_alternative<std::string>(instance)) {
            std::string str = std::get<std::string>(instance);

            if (key == "Count" || key == "Length") {
                int64_t len = (int64_t)str.size();
                if (len < 0)
                    len = 0;

                cse.back()->Push(gc, len);
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<
                std::shared_ptr<Tesses::Framework::TF_Timer_Handle>>(
                instance)) {
            auto timer =
                std::get<std::shared_ptr<Tesses::Framework::TF_Timer_Handle>>(
                    instance);
            if (timer) {
                if (key == "Interval") {
                    cse.back()->Push(gc, timer->GetIntervalMilliseconds());
                    return false;
                }
                if (key == "Enabled") {
                    cse.back()->Push(gc, timer->GetEnabled());
                    return false;
                }
            }
            cse.back()->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<
                std::shared_ptr<Tesses::Framework::Filesystem::VFS>>(
                instance)) {
            auto vfs =
                std::get<std::shared_ptr<Tesses::Framework::Filesystem::VFS>>(
                    instance);
            auto tmpFS = std::dynamic_pointer_cast<
                Tesses::Framework::Filesystem::TempFS>(vfs);
            auto relative = std::dynamic_pointer_cast<
                Tesses::Framework::Filesystem::RelativeFilesystem>(vfs);
            auto myFS = std::dynamic_pointer_cast<TObjectVFS>(vfs);
            if (relative) {
                if (key == "Working") {
                    cse.back()->Push(gc, relative->GetWorking());
                    return false;
                }
                if (key == "Filesystem") {
                    cse.back()->Push(gc, relative->GetVFS());
                    return false;
                }
            }
            if (myFS) {
                if (key == "Inner") {
                    cse.back()->Push(gc, myFS->obj);
                    return false;
                }
            }
            if (tmpFS) {
                if (key == "TempDirectoryName") {

                    cse.back()->Push(gc, tmpFS->TempDirectoryName());
                    return false;
                }
            }
            cse.back()->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<
                std::shared_ptr<Tesses::Framework::TextStreams::TextWriter>>(
                instance)) {
            auto writer = std::get<
                std::shared_ptr<Tesses::Framework::TextStreams::TextWriter>>(
                instance);
            auto stringWriter = std::dynamic_pointer_cast<
                Tesses::Framework::TextStreams::StringWriter>(writer);
            if (stringWriter != nullptr) {
                if (key == "Text") {
                    cse.back()->Push(gc, stringWriter->GetString());
                    return false;
                }
            }
            if (key == "NewLine") {
                cse.back()->Push(gc, writer->newline);
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<
                std::shared_ptr<Tesses::Framework::Streams::ByteReader>>(
                instance)) {
            auto br = std::get<
                std::shared_ptr<Tesses::Framework::Streams::ByteReader>>(
                instance);
            if (key == "Stream") {
                cse.back()->Push(gc, br->GetStream());
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<
                std::shared_ptr<Tesses::Framework::Streams::ByteWriter>>(
                instance)) {
            auto bw = std::get<
                std::shared_ptr<Tesses::Framework::Streams::ByteWriter>>(
                instance);
            if (key == "Stream") {

                cse.back()->Push(gc, bw->GetStream());
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<
                std::shared_ptr<Tesses::Framework::Streams::Stream>>(
                instance)) {
            auto strm =
                std::get<std::shared_ptr<Tesses::Framework::Streams::Stream>>(
                    instance);
            if (strm != nullptr) {
                auto netStrm = std::dynamic_pointer_cast<
                    Tesses::Framework::Streams::NetworkStream>(strm);

                if (key == "CanRead") {

                    cse.back()->Push(gc, strm->CanRead());
                    return false;
                }
                if (key == "CanWrite") {

                    cse.back()->Push(gc, strm->CanWrite());
                    return false;
                }
                if (key == "CanSeek") {

                    cse.back()->Push(gc, strm->CanSeek());
                    return false;
                }
                if (key == "EndOfStream") {

                    cse.back()->Push(gc, strm->EndOfStream());
                    return false;
                }
                if (key == "Length") {

                    cse.back()->Push(gc, strm->GetLength());
                    return false;
                }
                if (key == "Position") {

                    cse.back()->Push(gc, strm->GetPosition());
                    return false;
                }

                if (netStrm != nullptr) {
                    if (key == "Port") {
                        cse.back()->Push(gc, netStrm->GetPort());
                        return false;
                    }
                }

                cse.back()->Push(gc, Undefined());

                return false;
            }
        }
        if (std::holds_alternative<TVMVersion>(instance)) {
            TVMVersion &version = std::get<TVMVersion>(instance);
            if (key == "Major") {
                stk->Push(gc, (int64_t)version.Major());
                return false;
            }
            if (key == "Minor") {
                stk->Push(gc, (int64_t)version.Minor());
                return false;
            }
            if (key == "Patch") {
                stk->Push(gc, (int64_t)version.Patch());
                return false;
            }
            if (key == "Build") {
                stk->Push(gc, (int64_t)version.Build());
                return false;
            }
            if (key == "VersionInt") {
                stk->Push(gc, (int64_t)version.AsLong());
                return false;
            }
            if (key == "Stage") {
                switch (version.VersionStage()) {
                case TVMVersionStage::DevVersion:
                    stk->Push(gc, "dev");
                    break;
                case TVMVersionStage::AlphaVersion:
                    stk->Push(gc, "alpha");
                    break;
                case TVMVersionStage::BetaVersion:
                    stk->Push(gc, "beta");
                    break;
                case TVMVersionStage::ProductionVersion:
                    stk->Push(gc, "prod");
                    break;
                }
                return false;
            }

            stk->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<
                std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(instance)) {
            auto time =
                std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(
                    instance);
            if (key == "Days") {
                stk->Push(gc, (int64_t)time->Days());
                return false;
            }
            if (key == "Hours") {
                stk->Push(gc, (int64_t)time->Hours());
                return false;
            }

            if (key == "Minutes") {
                stk->Push(gc, (int64_t)time->Minutes());
                return false;
            }
            if (key == "Seconds") {
                stk->Push(gc, (int64_t)time->Seconds());
                return false;
            }

            if (key == "TotalHours") {
                stk->Push(gc, time->TotalHours());
                return false;
            }

            if (key == "TotalMinutes") {
                stk->Push(gc, time->TotalMinutes());
                return false;
            }

            if (key == "TotalSeconds") {
                stk->Push(gc, time->TotalSeconds());
                return false;
            }

            stk->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<
                std::shared_ptr<Tesses::Framework::Date::DateTime>>(instance)) {
            auto &date =
                std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(
                    instance);
            if (key == "IsLocal") {
                stk->Push(gc, date->IsLocal());
                return false;
            }
            if (key == "Year") {
                stk->Push(gc, (int64_t)date->Year());
                return false;
            }
            if (key == "Month") {
                stk->Push(gc, (int64_t)date->Month());
                return false;
            }
            if (key == "Day") {
                stk->Push(gc, (int64_t)date->Day());
                return false;
            }
            if (key == "Hour") {
                stk->Push(gc, (int64_t)date->Hour());
                return false;
            }
            if (key == "Minute") {
                stk->Push(gc, (int64_t)date->Minute());
                return false;
            }
            if (key == "Second") {
                stk->Push(gc, (int64_t)date->Second());
                return false;
            }
            if (key == "DayOfWeek") {
                stk->Push(gc, (int64_t)date->DayOfWeek());
                return false;
            }

            stk->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<THeapObjectHolder>(instance)) {
            auto obj = std::get<THeapObjectHolder>(instance).obj;
            auto bA = dynamic_cast<TByteArray *>(obj);
            auto list = dynamic_cast<TList *>(obj);
            auto dict = dynamic_cast<TDictionary *>(obj);
            auto dynDict = dynamic_cast<TDynamicDictionary *>(obj);
            auto dynList = dynamic_cast<TDynamicList *>(obj);
            auto tcallable = dynamic_cast<TCallable *>(obj);
            auto closure = dynamic_cast<TClosure *>(obj);
            auto externalMethod = dynamic_cast<TExternalMethod *>(obj);
            auto ittr = dynamic_cast<TEnumerator *>(obj);

            auto callstackEntry = dynamic_cast<CallStackEntry *>(obj);
            auto file = dynamic_cast<TFile *>(obj);
            auto chunk = dynamic_cast<TFileChunk *>(obj);
            auto natObj = dynamic_cast<TNativeObject *>(obj);
            auto cls = dynamic_cast<TClassObject *>(obj);
            auto aarray = dynamic_cast<TAssociativeArray *>(obj);
            auto task = dynamic_cast<TTask *>(obj);
            if (task != nullptr) {
                if (key == "IsCompleted") {
                    cse.back()->Push(gc, task->IsCompleted());
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
            if (aarray != nullptr) {
                if (key == "Count" || key == "Length") {
                    cse.back()->Push(gc, aarray->Count());
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            } else if (cls != nullptr) {
                gc->BarrierBegin();
                auto obj =
                    cls->GetValue(cse.back()->callable->className, "get" + key);
                gc->BarrierEnd();
                TClosure *clos;
                TCallable *callable;
                if (GetObjectHeap(obj, clos)) {
                    this->AddCallStackEntry(ls, clos, {});
                    return true;
                } else if (GetObjectHeap(obj, callable)) {
                    cse.back()->Push(gc, callable->Call(ls, {}));
                    return false;
                }
                cse.back()->Push(
                    gc, cls->GetValue(cse.back()->callable->className, key));
                return false;

            } else if (natObj != nullptr) {
                cse.back()->Push(gc, natObj->CallMethod(ls, "get" + key, {}));
                return false;
            }

            if (file != nullptr) {
                if (key == "Version") {
                    cse.back()->Push(gc, file->version);
                    return false;
                } else if (key == "Name") {
                    cse.back()->Push(gc, file->name);
                    return false;
                } else if (key == "Info") {
                    cse.back()->Push(gc, file->info);
                    return false;
                } else if (key == "Dependencies") {
                    auto list = TList::Create(ls);
                    gc->BarrierBegin();
                    for (auto item : file->dependencies) {
                        auto res = TDictionary::Create(ls);
                        res->SetValue("Name", item.first);
                        res->SetValue("Version", item.second);
                        list->Add(res);
                    }
                    gc->BarrierEnd();
                    cse.back()->Push(gc, list);
                    return false;
                } else if (key == "Tools") {
                    auto list = TList::Create(ls);
                    gc->BarrierBegin();
                    for (auto item : file->tools) {
                        auto res = TDictionary::Create(ls);
                        res->SetValue("Name", item.first);
                        res->SetValue("Version", item.second);
                        list->Add(res);
                    }
                    gc->BarrierEnd();
                    cse.back()->Push(gc, list);
                    return false;
                } else if (key == "Strings") {
                    auto list = TList::Create(ls);
                    gc->BarrierBegin();
                    for (auto item : file->name) {
                        list->Add(item);
                    }
                    gc->BarrierEnd();

                    cse.back()->Push(gc, list);
                    return false;
                } else if (key == "MetadataCount") {

                    cse.back()->Push(gc, (int64_t)file->metadata.size());
                    return false;
                } else if (key == "Metadata") {
                    TList *meta = TList::Create(ls);
                    gc->BarrierBegin();
                    for (size_t i = 0; i < file->metadata.size(); i++) {
                        meta->Add(TDictionary::Create(
                            ls, {TDItem("Name", file->metadata[i].first),
                                 TDItem("Index", (int64_t)i)}));
                    }
                    gc->BarrierEnd();

                    cse.back()->Push(gc, meta);
                    return false;
                } else if (key == "Sections") {
                    TList *sections = TList::Create(ls);
                    gc->BarrierBegin();
                    for (auto &item : file->sections) {
                        TByteArray *ba = TByteArray::Create(ls);
                        ba->data = item.second;
                        sections->Add(
                            TDictionary::Create(ls, {TDItem("Name", item.first),
                                                     TDItem("Data", ba)}));
                    }
                    gc->BarrierEnd();

                    cse.back()->Push(gc, sections);
                    return false;
                } else if (key == "SupportedVMs") {
                    TList *supported = TList::Create(ls);
                    gc->BarrierBegin();
                    if (file->vms.empty()) {
                        supported->Add(TDictionary::Create(
                            ls, {TDItem("Name", std::string(VMName)),
                                 TDItem("HowToGet", std::string(VMHowToGet))}));
                    } else {
                        for (auto item : file->vms)
                            supported->Add(TDictionary::Create(
                                ls, {TDItem("Name", item.first),
                                     TDItem("HowToGet", item.second)}));
                    }
                    gc->BarrierEnd();
                    cse.back()->Push(gc, supported);
                    return false;
                } else if (key == "Chunks") {
                    auto list = TList::Create(ls);
                    gc->BarrierBegin();
                    for (auto item : file->chunks) {
                        list->Add(item);
                    }

                    gc->BarrierEnd();

                    cse.back()->Push(gc, list);
                    return false;
                } else if (key == "Classes") {
                    auto list = TList::Create(ls);
                    gc->BarrierBegin();

                    for (uint32_t i = 0; i < (uint32_t)file->classes.size();
                         i++) {
                        list->Add(GetClassInfo(ls, file, i));
                    }

                    cse.back()->Push(gc, list);
                    gc->BarrierEnd();
                    return false;
                } else if (key == "Functions") {
                    auto list = TList::Create(ls);
                    gc->BarrierBegin();
                    for (auto &item : file->functions) {
                        TDictionary *dict = TDictionary::Create(ls);
                        if (!item.first.empty())
                            dict->SetValue("Documentation", item.first[0]);
                        TList *nameParts = TList::Create(ls);
                        for (size_t i = 1; i < item.first.size(); i++) {
                            nameParts->Add(item.first[i]);
                        }
                        dict->SetValue("NameParts", nameParts);
                        dict->SetValue("ChunkId", (int64_t)item.second);

                        list->Add(dict);
                    }

                    cse.back()->Push(gc, list);
                    gc->BarrierEnd();
                    return false;
                } else if (key == "Icon") {
                    if (file->icon >= 0 &&
                        file->icon < file->resources.size()) {
                        TByteArray *ba = TByteArray::Create(ls);
                        ba->data = file->resources[file->icon];
                        cse.back()->Push(gc, ba);
                        return false;
                    } else {
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
            if (chunk != nullptr) {
                if (key == "Arguments") {
                    auto myargs = TList::Create(ls);
                    gc->BarrierBegin();
                    for (auto item : chunk->args) {
                        myargs->Add(item);
                    }
                    gc->BarrierEnd();
                    cse.back()->Push(gc, myargs);
                    return false;
                }
                if (key == "Code") {
                    auto ba = TByteArray::Create(ls);
                    ba->data = chunk->code;
                    cse.back()->Push(gc, ba);
                    return false;
                }
                if (key == "File") {
                    cse.back()->Push(gc, chunk->file);
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
            if (callstackEntry != nullptr) {
                if (key == "IP") {
                    cse.back()->Push(gc, (int64_t)callstackEntry->ip);
                    return false;
                }
                if (key == "IsDone") {
                    cse.back()->Push(
                        gc, callstackEntry->ip >=
                                callstackEntry->callable->closure->code.size());
                    return false;
                }
                if (key == "Closure") {
                    cse.back()->Push(gc, callstackEntry->callable);
                    return false;
                }
                if (key == "StackEmpty") {
                    cse.back()->Push(gc, callstackEntry->stack.empty());
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }

            if (ittr != nullptr) {
                if (key == "Current") {
                    cse.back()->Push(gc, ittr->GetCurrent(ls));
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
            if (closure != nullptr) {
                if (key == "Arguments") {
                    GCList ls2(gc);
                    TList *ls = TList::Create(ls2);
                    for (auto arg : closure->closure->args) {
                        ls->Add(arg);
                    }
                    cse.back()->Push(gc, ls);
                    return false;
                }
                if (key == "File") {
                    cse.back()->Push(gc, closure->file);
                    return false;
                }
            }
            if (externalMethod != nullptr) {
                if (key == "Arguments") {
                    GCList ls2(gc);
                    TList *ls = TList::Create(ls2);
                    for (auto arg : externalMethod->args) {
                        ls->Add(arg);
                    }
                    cse.back()->Push(gc, ls);
                    return false;
                }
            }

            if (tcallable != nullptr) {
                if (key == "Documentation") {
                    cse.back()->Push(gc, tcallable->documentation);
                    return false;
                }
                if (key == "Tag") {
                    gc->BarrierBegin();
                    cse.back()->Push(gc, tcallable->tag);
                    gc->BarrierEnd();
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }

            if (bA != nullptr) {
                if (key == "Count" || key == "Length") {
                    int64_t len = (int64_t)bA->data.size();
                    if (len < 0)
                        len = 0;

                    stk->Push(gc, len);
                    return false;
                }
            }
            if (list != nullptr) {
                if (key == "Count" || key == "Length") {
                    int64_t len = list->Count();
                    if (len < 0)
                        len = 0;

                    stk->Push(gc, len);
                    return false;
                }
            }
            if (dynList != nullptr) {
                if (key == "Count" || key == "Length") {
                    int64_t len = dynList->Count(ls);
                    if (len < 0)
                        len = 0;

                    stk->Push(gc, len);
                    return false;
                }
            }
            if (dynDict != nullptr) {
                if (dynDict->MethodExists(ls, "get" + key)) {
                    cse.back()->Push(gc,
                                     dynDict->CallMethod(ls, "get" + key, {}));
                } else {
                    cse.back()->Push(gc, dynDict->GetField(ls, key));
                }
                return false;
            }

            if (dict != nullptr) {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("get" + key);
                gc->BarrierEnd();
                if (std::holds_alternative<THeapObjectHolder>(fn) &&
                    dynamic_cast<TCallable *>(
                        std::get<THeapObjectHolder>(fn).obj) != nullptr) {
                    return InvokeOne(ls, fn, dict);
                } else {
                    gc->BarrierBegin();
                    fn = dict->GetValue(key);
                    stk->Push(gc, fn);
                    gc->BarrierEnd();
                    return false;
                }
            }
        }

        stk->Push(gc, Undefined());
    }

    return false;
}
} // namespace Tesses::CrossLang