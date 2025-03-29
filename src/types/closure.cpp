#include "CrossLang.hpp"
namespace Tesses::CrossLang {
    TArgWrapper* TArgWrapper::Create(GCList& ls, TCallable* callable)
    {
        TArgWrapper* argWrapper = new TArgWrapper();
        argWrapper->callable = callable;
        GC* gc = ls.GetGC();
        ls.Add(argWrapper);
        gc->Watch(argWrapper);
        return argWrapper;
    }
    TArgWrapper* TArgWrapper::Create(GCList* ls, TCallable* callable)
    {
        TArgWrapper* argWrapper = new TArgWrapper();
        argWrapper->callable = callable;
        GC* gc = ls->GetGC();
        ls->Add(argWrapper);
        gc->Watch(argWrapper);
        return argWrapper;
    }
    TObject TArgWrapper::Call(GCList& ls,std::vector<TObject> args)
    {
        auto cse = current_function;
        TList* argList = TList::Create(ls);
        argList->items = args;
        TObject v=this->callable->Call(ls,{argList});
        current_function = cse;
        return v;
    }
    void TArgWrapper::Mark()
    {
        if(this->marked) return;
        this->marked = true;
        this->callable->Mark();
        GC::Mark(this->tag);
    }
    void TCallable::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        GC::Mark(this->tag);
    }
    void TClosure::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        this->file->Mark();
        this->env->Mark();
        this->closure->Mark();
        GC::Mark(this->tag);
    }
    TClosure* TClosure::Create(GCList& ls,TEnvironment* env,TFile* file,uint32_t chunkId,bool ownScope)
    {
        TClosure* closure = new TClosure();
        closure->ownScope=ownScope;
        GC* _gc = ls.GetGC();
        ls.Add(closure);
        _gc->Watch(closure);
        closure->chunkId = chunkId;
        if(chunkId < file->chunks.size())
        closure->closure = file->chunks[chunkId];
        else throw VMException("ChunkId out of bounds.");
        closure->env = env;
        closure->file = file;
        
        return closure;
    }
    TClosure* TClosure::Create(GCList* ls,TEnvironment* env,TFile* file,uint32_t chunkId,bool ownScope)
    {
        TClosure* closure = new TClosure();
        closure->ownScope=ownScope;
        GC* _gc = ls->GetGC();
        ls->Add(closure);
        _gc->Watch(closure);
        closure->chunkId = chunkId;
        if(chunkId < file->chunks.size())
        closure->closure = file->chunks[chunkId];
        else throw VMException("ChunkId out of bounds.");
        closure->env = env;
        closure->file = file;
        
        return closure;
    }
            
    TObject TClosure::Call(GCList& ls,std::vector<TObject> args)
    {
        auto cse = current_function;
        InterperterThread* thrd=InterperterThread::Create(ls);
        thrd->AddCallStackEntry(ls,this, args);

        thrd->Execute(ls.GetGC());

        TObject v= thrd->call_stack_entries[0]->Pop(ls);
        current_function = cse;
        return v;
    }
}