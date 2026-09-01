#include "CrossLang.hpp"
namespace Tesses::CrossLang {
TArgWrapper::TArgWrapper(TCallable *callable) : callable(callable) {}
TArgWrapper *TArgWrapper::Create(GCList &ls, TCallable *callable) {
    return ls.Create<TArgWrapper>(callable);
}
TArgWrapper *TArgWrapper::Create(GCList *ls, TCallable *callable) {
    return ls->Create<TArgWrapper>(callable);
}
TObject TArgWrapper::Call(GCList &ls, std::vector<TObject> args) {
    auto cse = GC::GetCurrentFunction();
    TList *argList = TList::Create(ls);
    argList->items = args;
    TObject v = this->callable->Call(ls, {argList});
    GC::SetCurrentFunction(cse);
    return v;
}
void TArgWrapper::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    this->callable->Mark();
    GC::Mark(this->tag);
}
void TCallable::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    GC::Mark(this->tag);
}
void TClosure::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    this->file->Mark();
    this->env->Mark();
    this->closure->Mark();
    GC::Mark(this->tag);
}
TClosure::TClosure(TEnvironment *env, TFile *file, uint32_t chunkId,
                   bool ownScope) {
    this->className = "";
    this->ownScope = ownScope;

    this->chunkId = chunkId;

    if (chunkId < file->chunks.size())
        this->closure = file->chunks[chunkId];
    else
        throw VMException("ChunkId out of bounds.");
    this->env = env;
    this->file = file;
}
TClosure *TClosure::Create(GCList &ls, TEnvironment *env, TFile *file,
                           uint32_t chunkId, bool ownScope) {
    return ls.Create<TClosure>(env, file, chunkId, ownScope);
}
TClosure *TClosure::Create(GCList *ls, TEnvironment *env, TFile *file,
                           uint32_t chunkId, bool ownScope) {
    return ls->Create<TClosure>(env, file, chunkId, ownScope);
}

TObject TClosure::Call(GCList &ls, std::vector<TObject> args) {
    auto cse = GC::GetCurrentFunction();
    InterperterThread *thrd = InterperterThread::Create(ls);
    thrd->AddCallStackEntry(ls, this, args);

    thrd->Execute(ls.GetGC());

    TObject v = thrd->call_stack_entries[0]->Pop(ls);
    GC::SetCurrentFunction(cse);
    // current_function = cse;
    return v;
}
} // namespace Tesses::CrossLang