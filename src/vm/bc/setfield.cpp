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

static void empty() {}
bool InterperterThread::SetField(std::shared_ptr<GC> gc) {
    std::vector<CallStackEntry *> &cse = this->call_stack_entries;

    if (!cse.empty()) {
        auto stk = cse.back();
        GCList ls(gc);
        TObject value = stk->Pop(ls);
        TObject _key = stk->Pop(ls);
        TObject instance = stk->Pop(ls);

        if (!std::holds_alternative<std::string>(_key)) {
            stk->Push(gc, Undefined());
            return false;
        }

        std::string key = std::get<std::string>(_key);
        if (std::holds_alternative<
                std::shared_ptr<Tesses::Framework::TF_Timer_Handle>>(
                instance)) {
            auto handle =
                std::get<std::shared_ptr<Tesses::Framework::TF_Timer_Handle>>(
                    instance);
            if (handle) {
                int64_t ms;
                bool b;
                if (key == "Enabled" && GetObject(value, b)) {

                    handle->SetEnabled(b);

                    cse.back()->Push(gc, b);
                    return false;
                }

                if (key == "Interval" && GetObject(value, ms)) {
                    handle->SetIntervalFromMilliseconds(ms);

                    cse.back()->Push(gc, ms);
                    return false;
                }
                if (key == "Callback") {
                    TCallable *callable;
                    if (GetObjectHeap(value, callable)) {

                        auto obj = CreateMarkedTObject(ls.GetGC(), callable);
                        handle->SetCallback([obj]() -> void {
                            TCallable *callable;
                            if (GetObjectHeap(obj->GetObject(), callable)) {
                                GCList ls(obj->GetGC());
                                callable->Call(ls, {});
                            }
                        });
                        cse.back()->Push(gc, callable);
                        return false;
                    } else {
                        handle->SetCallback(empty);
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
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
                    if (std::holds_alternative<std::string>(value)) {
                        stringWriter->GetString() =
                            std::get<std::string>(value);
                    }
                    cse.back()->Push(gc, stringWriter->GetString());
                    return false;
                }
            }
            if (key == "NewLine") {
                if (std::holds_alternative<std::string>(value)) {
                    writer->newline = std::get<std::string>(value);
                }
                cse.back()->Push(gc, writer->newline);
                return false;
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
            auto relative = std::dynamic_pointer_cast<
                Tesses::Framework::Filesystem::RelativeFilesystem>(vfs);

            if (relative) {
                if (key == "Working") {
                    Tesses::Framework::Filesystem::VFSPath path;
                    if (GetObjectAsPath(value, path)) {
                        if (path.relative) {
                            relative->SetWorking(
                                path.MakeAbsolute(relative->GetWorking()));
                        } else {
                            relative->SetWorking(path);
                        }
                    }
                }
            }

            cse.back()->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<
                std::shared_ptr<Tesses::Framework::Http::IHttpServer>>(
                instance)) {
            auto svr =
                std::get<std::shared_ptr<Tesses::Framework::Http::IHttpServer>>(
                    instance);
            auto bas = std::dynamic_pointer_cast<
                Tesses::Framework::Http::BasicAuthServer>(svr);
            auto cgi =
                std::dynamic_pointer_cast<Tesses::Framework::Http::CGIServer>(
                    svr);
            auto changable = std::dynamic_pointer_cast<
                Tesses::Framework::Http::ChangeableServer>(svr);
            if (changable != nullptr) {
                if (key == "Server") {
                    bas->server = ToHttpServer(gc, value);
                    stk->Push(gc, value);
                    return false;
                }
            }

            if (bas != nullptr) {
                if (key == "Realm") {
                    bool val;
                    if (GetObject(value, val)) {
                        bas->realm = val;
                        stk->Push(gc, val);

                        return false;
                    }
                }
                if (key == "Server") {
                    bas->server = ToHttpServer(gc, value);
                    stk->Push(gc, value);
                    return false;
                }
                if (key == "Authorization") {
                    TCallable *val;
                    if (GetObjectHeap(value, val)) {
                        auto marked = CreateMarkedTObject(ls, val);
                        bas->authorization =
                            [marked](std::string user,
                                     std::string password) -> bool {
                            GCList ls(marked->GetGC());
                            TCallable *callable;
                            if (GetObjectHeap(marked->GetObject(), callable)) {
                                return ToBool(
                                    callable->Call(ls, {user, password}));
                            }
                            return false;
                        };
                        stk->Push(gc, val);

                        return false;
                    }
                }
            }

            if (cgi != nullptr) {
                if (key == "WorkingDirectory") {
                    Tesses::Framework::Filesystem::VFSPath path;
                    if (GetObjectAsPath(value, path)) {
                        cgi->workingDirectory = path;
                        stk->Push(gc, path);
                        return false;
                    } else {
                        cgi->workingDirectory = std::nullopt;
                        stk->Push(gc, nullptr);
                        return false;
                    }
                }
                if (key == "DocumentRoot") {
                    Tesses::Framework::Filesystem::VFSPath path;
                    if (GetObjectAsPath(value, path)) {
                        cgi->document_root = path;
                        stk->Push(gc, path);
                        return false;
                    } else {
                        cgi->document_root = std::nullopt;
                        stk->Push(gc, nullptr);
                        return false;
                    }
                }
                if (key == "AdminEmail") {
                    std::string str;
                    if (GetObject(value, str)) {
                        cgi->adminEmail = str;
                        stk->Push(gc, str);
                        return false;
                    } else {
                        cgi->adminEmail = std::nullopt;
                        stk->Push(gc, nullptr);
                        return false;
                    }
                }
            }

            stk->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<
                std::shared_ptr<Tesses::Framework::Streams::Stream>>(
                instance)) {
            auto strm =
                std::get<std::shared_ptr<Tesses::Framework::Streams::Stream>>(
                    instance);

            auto netStrm = std::dynamic_pointer_cast<
                Tesses::Framework::Streams::NetworkStream>(strm);
            if (netStrm != nullptr) {
                int64_t n0;
                bool bc;
                if (key == "Broadcast" && GetObject(value, bc))
                    netStrm->SetBroadcast(bc);
                if (key == "NoDelay" && GetObject(value, bc))
                    netStrm->SetNoDelay(bc);
                if (key == "ReuseAddress" && GetObject(value, bc))
                    netStrm->SetReuseAddress(bc);
                if (key == "ReusePort" && GetObject(value, bc))
                    netStrm->SetReusePort(bc);
                if (key == "MulticastTTL" && GetObject(value, n0))
                    netStrm->SetMulticastTTL((uint8_t)n0);
            }
            stk->Push(gc, Undefined());
            return false;
        }
        if (std::holds_alternative<THeapObjectHolder>(instance)) {
            auto obj = std::get<THeapObjectHolder>(instance).obj;

            auto dict = dynamic_cast<TDictionary *>(obj);
            auto dynDict = dynamic_cast<TDynamicDictionary *>(obj);
            auto natObj = dynamic_cast<TNativeObject *>(obj);
            auto cls = dynamic_cast<TClassObject *>(obj);
            if (cls != nullptr) {
                gc->BarrierBegin();
                auto obj =
                    cls->GetValue(cse.back()->callable->className, "set" + key);
                gc->BarrierEnd();
                TClosure *clos;
                TCallable *callable;
                if (GetObjectHeap(obj, clos)) {
                    this->AddCallStackEntry(ls, clos, {value});
                    return true;
                } else if (GetObjectHeap(obj, callable)) {
                    cse.back()->Push(gc, callable->Call(ls, {value}));
                    return false;
                }
                gc->BarrierBegin();
                cls->SetValue(cse.back()->callable->className, key, value);
                gc->BarrierEnd();
                cse.back()->Push(gc, value);
                return false;

            } else if (natObj != nullptr) {
                cse.back()->Push(gc,
                                 natObj->CallMethod(ls, "set" + key, {value}));
                return false;
            }
            auto tcallable = dynamic_cast<TCallable *>(obj);
            if (tcallable != nullptr) {
                if (key == "Tag") {
                    gc->BarrierBegin();
                    tcallable->tag = value;
                    gc->BarrierEnd();
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
            }
            if (dynDict != nullptr) {
                if (dynDict->MethodExists(ls, "set" + key)) {
                    cse.back()->Push(
                        gc, dynDict->CallMethod(ls, "set" + key, {value}));
                } else {
                    dynDict->SetField(ls, key, value);
                    cse.back()->Push(gc, value);
                }
                return false;
            }
            if (dict != nullptr) {
                gc->BarrierBegin();
                TObject fn = dict->GetValue("set" + key);
                gc->BarrierEnd();
                if (std::holds_alternative<THeapObjectHolder>(fn) &&
                    dynamic_cast<TCallable *>(
                        std::get<THeapObjectHolder>(fn).obj) != nullptr) {
                    return InvokeTwo(ls, fn, dict, value);
                } else {
                    gc->BarrierBegin();
                    dict->SetValue(key, value);
                    stk->Push(gc, value);
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