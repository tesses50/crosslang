#include "CrossLang.hpp"
#include <chrono>
#include <iostream>
#include <time.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

using namespace Tesses::Framework::Threading;
using namespace std::chrono;
namespace Tesses::CrossLang {
#if defined(GEKKO) || defined(_EE)

struct PthreadCmp {
    bool operator()(const pthread_t &a, const pthread_t &b) const {
        return pthread_equal(a, b) != 0;
    }
};
static std::map<pthread_t, CallStackEntry *, PthreadCmp> current_funcs;
static Tesses::Framework::Threading::Mutex current_funcs_mtx;
#else
thread_local CallStackEntry *current_function;
#endif
bool GC::IsRunning() {

    bool run = this->running;

    return run;
}
GC::GC() {}
TDictionary *CreateThread(GCList &ls, TCallable *callable, bool detached) {

    TDictionary *dict = TDictionary::Create(ls);

    ThreadHandle *th = new ThreadHandle();
    th->gc = ls.GetGC();
    th->callable = callable;
    th->hasInit = false;
    th->hasReturned = false;
    th->detached = detached;
    ls.Add(th);
    ls.GetGC()->Watch(th);

    ls.GetGC()->BarrierBegin();
    dict->SetValue("_internal", th);

    dict->DeclareFunction(
        ls.GetGC(), "Join", "Join thread", {},
        [th](GCList &_ls, std::vector<TObject> _args) -> TObject {
            th->thrd->Join();
            delete th->thrd;

            if (th->hasReturned) {
                _ls.GetGC()->BarrierBegin();
                auto v = th->returnValue;
                _ls.Add(v);
                _ls.GetGC()->BarrierEnd();
                return v;
            }
            return Undefined();
        });
    dict->DeclareFunction(
        ls.GetGC(), "Detach", "Detach thread", {},
        [th](GCList &_ls, std::vector<TObject> _args) -> TObject {
            _ls.GetGC()->BarrierBegin();
            th->detached = true;
            _ls.GetGC()->BarrierEnd();
            return Undefined();
        });

    dict->DeclareFunction(
        ls.GetGC(), "getFinished", "Get whether thread has finished", {},
        [th](GCList &_ls, std::vector<TObject> _args) -> TObject {
            return (bool)(th->hasReturned == true);
        });

    ls.GetGC()->BarrierEnd();
    th->thrd = new Thread([th]() -> void {
        std::shared_ptr<GC> gc = th->gc;
        GCList ls(gc);
        ls.Add(th);
        th->hasInit = true;
        TObject cb = th->callable->Call(ls, {});
        gc->BarrierBegin();
        th->returnValue = cb;
#if defined(GEKKO) || defined(_EE)
        current_funcs_mtx.Lock();
        current_funcs.erase(pthread_self());
        current_funcs_mtx.Unlock();
#endif
        gc->BarrierEnd();

        th->hasReturned = true;
    });
    while (!th->hasInit)
        ;
    return dict;
}
void GC::Start() {

    this->tpool = new Tesses::Framework::Lazy<
        Tesses::Framework::Threading::ThreadPool *>(
        []() -> Tesses::Framework::Threading::ThreadPool * {
            auto threads =
                Tesses::Framework::Threading::ThreadPool::GetNumberOfCores();
            if (threads < 4)
                threads = 4;
            return new Tesses::Framework::Threading::ThreadPool(threads);
        },
        [](Tesses::Framework::Threading::ThreadPool *p) -> void { delete p; });

    this->running = true;
    this->thrd = new Thread([this]() -> void {
        while (this->IsRunning()) {
            this->BarrierBegin();
            while (this->IsRunning() && (this->allocs < ALLOC_THRESHOLD))
                this->cond.Wait(&this->mtx);
            this->allocs = 0;
            std::vector<THeapObject *> to_delete;
            this->Collect(to_delete);
            this->BarrierEnd();

            for (auto o : to_delete)
                delete o;
        }
        {
            std::vector<THeapObject *> to_delete;
            this->BarrierBegin();
            this->Collect(to_delete);
            this->BarrierEnd();

            for (auto item : to_delete)
                delete item;
        }
    });
}

bool GC::UsingNullThreads() { return false; }

void GC::BarrierBegin() { this->mtx.Lock(); }
void GC::BarrierEnd() { this->mtx.Unlock(); }
void GC::Watch(TObject obj) {
    if (std::holds_alternative<THeapObjectHolder>(obj)) {
        auto _item = std::get<THeapObjectHolder>(obj).obj;
        this->BarrierBegin();
        this->objects.insert(_item);
        auto nowAllocs = ++this->allocs;
        this->BarrierEnd();
        if (nowAllocs >= ALLOC_THRESHOLD)
            this->cond.Signal();
    }
}
void GC::Mark(TObject obj) {
    if (std::holds_alternative<THeapObjectHolder>(obj)) {
        auto _item = std::get<THeapObjectHolder>(obj).obj;
        _item->Mark();
    }
}
void GC::Unwatch(TObject obj) {
    if (std::holds_alternative<THeapObjectHolder>(obj)) {
        auto _item = std::get<THeapObjectHolder>(obj).obj;
        this->BarrierBegin();
        this->objects.erase(_item);
        this->BarrierEnd();
    }
}
void GC::SetRoot(GCList *_item) {
    if (_item == nullptr)
        return;
    this->BarrierBegin();

    this->roots.insert(_item);
    this->BarrierEnd();
}
Tesses::Framework::Threading::ThreadPool *GC::GetPool() {
    return this->tpool->GetValue();
}

void GC::UnsetRoot(GCList *_item) {
    if (_item == nullptr)
        return;
    this->BarrierBegin();
    this->roots.erase(_item);
    this->BarrierEnd();
}

GC::~GC() {
    this->running = false;
    this->cond.Signal();
    this->thrd->Join();
    delete this->thrd;
    delete this->tpool;
}

void GC::RegisterEverythingCallback(
    std::function<void(std::shared_ptr<GC> gc, TRootEnvironment *env)> cb) {
    this->register_everything.push_back(cb);
}
void GC::RegisterEverything(TRootEnvironment *env) {
    for (auto item : this->register_everything)
        item(this->shared_from_this(), env);
}

CallStackEntry *GC::GetCurrentFunction() {
#if defined(GEKKO) || defined(_EE)
    current_funcs_mtx.Lock();
    auto val = current_funcs[pthread_self()];
    current_funcs_mtx.Unlock();

    return val;

#else
    return current_function;
#endif
}
void GC::SetCurrentFunction(CallStackEntry *cse) {
#if defined(GEKKO) || defined(_EE)
    current_funcs_mtx.Lock();
    current_funcs[pthread_self()] = cse;
    current_funcs_mtx.Unlock();
#else
    current_function = cse;
#endif
}

void GC::Collect(std::vector<THeapObject *> &to_delete) {

    for (auto item : this->objects) {
        item->marked = false;
    }
    for (auto item : this->roots) {
        item->Mark();
    }
    for (auto index = this->objects.begin(); index != this->objects.end();) {
        THeapObject *o = *index;
        if (!o->marked) {
            to_delete.push_back(o);
            index = this->objects.erase(index);
        } else {
            ++index;
        }
    }
}
}; // namespace Tesses::CrossLang
