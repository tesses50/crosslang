#include "CrossLang.hpp"
#include <chrono>
#include <iostream>
#include <time.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif


using namespace Tesses::Framework::Threading;
using namespace std::chrono;
namespace Tesses::CrossLang
{
   
    bool GC::IsRunning()
    {
        
        bool run = this->running;
       
        return run;
    }
    GC::GC()
    {

        this->tpool=new Tesses::Framework::Lazy<Tesses::Framework::Threading::ThreadPool*>([]()->Tesses::Framework::Threading::ThreadPool*{
            auto threads = Tesses::Framework::Threading::ThreadPool::GetNumberOfCores();
            if(threads < 4) threads=4;
            return new Tesses::Framework::Threading::ThreadPool(threads);
        },[](Tesses::Framework::Threading::ThreadPool* p)->void{delete p;});
    }
    TDictionary* CreateThread(GCList& ls, TCallable* callable,bool detached)
    {

        TDictionary* dict = TDictionary::Create(ls);
                        
        ThreadHandle* th = new ThreadHandle();
        th->gc = ls.GetGC();
        th->callable = callable;
        th->hasInit=false;
        th->hasReturned = false;
        th->detached=detached;
        ls.Add(th);
        ls.GetGC()->Watch(th);
                        
        ls.GetGC()->BarrierBegin();
        dict->SetValue("_internal", th);
                        
        dict->DeclareFunction(ls.GetGC(),"Join","Join thread",{},[th](GCList& _ls, std::vector<TObject> _args)-> TObject{
                            
            
            th->thrd->Join();
            delete th->thrd;
                                            
            if(th->hasReturned)
            {
                _ls.GetGC()->BarrierBegin();
                auto v = th->returnValue;
                _ls.Add(v);
                _ls.GetGC()->BarrierEnd();
                 return v;
            }
            return Undefined();
        });
        dict->DeclareFunction(ls.GetGC(), "Detach","Detach thread",{},[th](GCList& _ls, std::vector<TObject> _args)-> TObject{
            _ls.GetGC()->BarrierBegin();
            th->detached=true;
            _ls.GetGC()->BarrierEnd();
            return Undefined();
        });
                        
        dict->DeclareFunction(ls.GetGC(),"getFinished","Get whether thread has finished",{},[th](GCList& _ls, std::vector<TObject> _args)-> TObject{
            return (bool)(th->hasReturned==true);
        });
                        
        ls.GetGC()->BarrierEnd();  
        th->thrd =new Thread([th]()->void {
            GC* gc=th->gc;
            GCList ls(gc);
            ls.Add(th);
            th->hasInit=true;
            TObject cb = th->callable->Call(ls,{});
            gc->BarrierBegin();
            th->returnValue=cb;
            gc->BarrierEnd();   
            th->hasReturned=true;
        });
        while(!th->hasInit);
        return dict;
    }
    void GC::Start()
    {
        this->mtx=new Mutex();
        this->running = true;
        this->thrd = new Thread([this]()->void {
              std::chrono::time_point<std::chrono::system_clock> last_frame, this_frame;
        
        this_frame = system_clock::now();
        last_frame = this_frame;
        
        while(this->IsRunning())
        {
            this_frame = system_clock::now();
            if((this_frame - last_frame) > 10s)
            {


                last_frame = this_frame;
                this->Collect();
                

            }

            #if defined(_WIN32)
            Sleep(100);
            #else
            usleep(100000);
            #endif
        }
        GC::Collect();
        });
    }

    bool GC::UsingNullThreads()
    {
        return false;
    }

    void GC::BarrierBegin()
    {
        
        this->mtx->Lock();
    }
    void GC::BarrierEnd()
    {  
        
        this->mtx->Unlock();
        
    }
    void GC::Watch(TObject obj)
    {
        if(std::holds_alternative<THeapObjectHolder>(obj))
        {
            auto _item=std::get<THeapObjectHolder>(obj).obj;
            this->BarrierBegin();
            
            for(auto item : this->objects)
            {
                if(item == _item) {
                    this->BarrierEnd();
                    return;
                }
            }
            this->objects.push_back(_item);
            this->BarrierEnd();
        }
    }
    void GC::Mark(TObject obj)
    {
        if(std::holds_alternative<THeapObjectHolder>(obj))
        {
            auto _item=std::get<THeapObjectHolder>(obj).obj;
            _item->Mark();
        }
    }
    void GC::Unwatch(TObject obj)
    {
        if(std::holds_alternative<THeapObjectHolder>(obj))
        {
            auto _item=std::get<THeapObjectHolder>(obj).obj;
            this->BarrierBegin();
            for(auto index = this->objects.begin();index<this->objects.end();index++)
            {
                if(*index == _item)
                {
                    this->objects.erase(index);
                    continue;
                }
            }
            this->BarrierEnd();
        }
    }
    void GC::SetRoot(TObject obj)
    {
        if(std::holds_alternative<THeapObjectHolder>(obj))
        {
            auto _item=std::get<THeapObjectHolder>(obj).obj;
            this->BarrierBegin();
            
            for(auto item : this->roots)
            {
                if(item == _item) {
                    this->BarrierEnd();
                    return;
                }
            }
            this->roots.push_back(_item);
            this->BarrierEnd();
        }
    }
    Tesses::Framework::Threading::ThreadPool* GC::GetPool()
    {
        return this->tpool->GetValue();
    }
    void GC::UnsetRoot(TObject obj)
    {
        if(std::holds_alternative<THeapObjectHolder>(obj))
        {
            auto _item=std::get<THeapObjectHolder>(obj).obj;
            this->BarrierBegin();
            for(auto index = this->roots.begin();index<this->roots.end();index++)
            {
                if(*index == _item)
                {
                    this->roots.erase(index);
                    continue;
                }
            }
            this->BarrierEnd();
        }
    }

    GC::~GC()
    {
        GC::BarrierBegin();
        this->roots.clear();
        GC::BarrierEnd();
        this->running=false;
        this->thrd->Join();
        delete this->thrd;
        for(auto item : objects) delete item;
        delete this->mtx;
        delete this->tpool;
    }

    void GC::RegisterEverythingCallback(std::function<void(GC* gc, TRootEnvironment* env)> cb)
    {
        this->register_everything.push_back(cb);
    }
    void GC::RegisterEverything(TRootEnvironment* env)
    {
        for(auto item : this->register_everything)
            item(this,env);
    }
    void GC::Collect()
    {
        this->BarrierBegin();
        for(auto item : this->objects)
        {
            item->marked=false;
        }
        for(auto item : this->roots)
        {
            item->Mark();
        }
        for(auto index = this->objects.begin();index < this->objects.end();index++)
        {
            THeapObject* o = *index;
            if(!o->marked)
            {
                delete o;
                this->objects.erase(index);
                index--;
            }
        }
        this->BarrierEnd();
       
    }
};
