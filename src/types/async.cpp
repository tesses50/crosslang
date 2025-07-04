#include "CrossLang.hpp"
//THANKS TO https://www.youtube.com/watch?v=R-z2Hv-7nxk
namespace Tesses::CrossLang {
    TTask::TTask(GC* gc)
    {
        this->gc = gc;
    }
    TTask* TTask::Create(GCList& ls)
    {
        TTask* task = new TTask(ls.GetGC());
        ls.Add(task);
        task->gc->Watch(task);
        return task;
    }
    bool TTask::IsCompleted()
    {
        gc->BarrierBegin();
        bool r = this->isCompleted;
        gc->BarrierEnd();

        return r;
    }
    TTask* TTask::ContinueWith(GCList& ls, TCallable* callable)
    {
        TTask* task = TTask::Create(ls);

        TExternalMethod* em = TExternalMethod::Create(ls,"Internal async thing",{"_asyncObj"},[callable,task](GCList& ls,std::vector<TObject> args)->TObject{
            try {
                task->SetSucceeded(callable->Call(ls,args));
            } catch(...) {
                task->SetFailed(std::current_exception());
            }
            return nullptr;
        });
        em->watch = {callable,task};
        
        this->gc->BarrierBegin();
        if(this->isCompleted)
        {
            this->gc->BarrierEnd();
            std::shared_ptr<GCList> ls=std::make_shared<GCList>(gc);
            auto cobj = this->obj;
            ls->Add(cobj);
            ls->Add(em);
            this->gc->GetPool()->Schedule([ls,em,cobj](size_t s)->void {
                em->Call(*ls,{cobj});
            });
            return task;
        }
        else 
        {
            this->cont = em;
        }

        this->gc->BarrierEnd();
        return task;
    }
    void TTask::ContinueWith(TCallable* callable)
    {
        this->gc->BarrierBegin();
        if(this->isCompleted)
        {
            this->gc->BarrierEnd();
            std::shared_ptr<GCList> ls=std::make_shared<GCList>(gc);
            auto cobj = this->obj;
            ls->Add(cobj);
            ls->Add(callable);
            this->gc->GetPool()->Schedule([ls,callable,cobj](size_t s)->void {
                callable->Call(*ls,{cobj});
                
            });
            return;
        }
        else 
        {
            this->cont = callable;
        }

        this->gc->BarrierEnd();
    }

   


    void TTask::SetFailed(std::exception_ptr ex)
    {
        this->ex = ex;
        this->SetSucceeded(nullptr);
    }
    void TTask::SetSucceeded(TObject v)
    {
        gc->BarrierBegin();
        if(this->isCompleted)
        {
            gc->BarrierEnd();
            return;
        }
        this->isCompleted=true;
        this->obj = v;
        auto cont = this->cont;
        gc->BarrierEnd();
        if(cont != nullptr)
        {
            std::shared_ptr<GCList> ls=std::make_shared<GCList>(gc);
            
            auto callable = cont;
            ls->Add(v);
            ls->Add(callable);
            this->gc->GetPool()->Schedule([ls,callable,v](size_t s)->void {
                callable->Call(*ls,{v});
            });
        }
    }

    TObject TTask::Wait()
    {
        while(true)
        {
            gc->BarrierBegin();
            if(this->isCompleted)
            {
                if(this->ex)
                {
                    auto error = this->ex;
                    gc->BarrierEnd();
                    std::rethrow_exception(error);
                }
                
                auto o = this->obj;
                gc->BarrierEnd();
                return o;
                
            }
            gc->BarrierEnd();
        }
    }
    void TTask::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        GC::Mark(this->obj);
        if(this->cont != nullptr)
            this->cont->Mark();
    }

    TTask* TTask::Run(GCList& ls, TCallable* callable)
    {
        TTask* task = TTask::Create(ls);
        std::shared_ptr<GCList> ls2=std::make_shared<GCList>(ls.GetGC());
        ls2->Add(callable);
        ls2->Add(task);

        ls.GetGC()->GetPool()->Schedule([ls2,callable,task](size_t s)->void {
                try
                {
                    task->SetSucceeded(callable->Call(*ls2,{}));
                } 
                catch(...)
                {
                    task->SetFailed(std::current_exception());
                }
        });

        return task;
    }

    TTask* TTask::FromResult(GCList& ls, TObject v)
    {
        TTask* task = TTask::Create(ls);
        task->SetSucceeded(v);
        return task;
    }

    class TTaskCseObj {
        GC* gc;
        TTask* task;
        public:
            TTaskCseObj(GC* gc, TTask* task)
            {
                this->gc = gc;
                this->task = task;
            }

            void Invoke(std::shared_ptr<TTaskCseObj> shared_o,CallStackEntry* cse, TObject o)
            {
                try {
                GCList ls(gc);
                cse->Push(gc,o);
                auto res = cse->Resume(ls);
                CallStackEntry* cse2;
                if(GetObjectHeap(res,cse2))
                {
                    auto taskPiece=cse2->Pop(ls);
                    TTask* task;
                    if(GetObjectHeap(taskPiece,task))
                    {
                        auto em = TExternalMethod::Create(ls,"",{"obj"},[cse2,task,shared_o](GCList& ls, std::vector<TObject> args)->TObject {
                    
                            shared_o->Invoke(shared_o,cse2,args.empty() ? (TObject)Undefined():args[0]);

                            return nullptr;
                        });
                        em->watch.push_back(task);
                        em->watch.push_back(cse2);
               
                        task->ContinueWith(em);
                    }
                }
                else {
                    this->task->SetSucceeded(res);
                }
                } catch(...) {
                    this->task->SetFailed(std::current_exception());
                }
            }
    };
    TTask* TTask::FromClosure(GCList& ls, TClosure* closure)
    {
        auto res =  closure->Call(ls,{});
        CallStackEntry* ent;
        if(GetObjectHeap(res,ent))
        {
            return FromCallStackEntry(ls,ent);
        }
        return FromResult(ls,res);
    }
    TTask* TTask::FromCallStackEntry(GCList& ls, CallStackEntry* ent)
    {
        TTask* task = TTask::Create(ls);

        //try {
            GCList ls2(ls.GetGC());
            GC* gc = ls.GetGC();
            
            std::shared_ptr<TTaskCseObj> obj = std::make_shared<TTaskCseObj>(gc,task);


            ls2.Add(task);
            auto res = ent->Pop(ls2);
            TTask* task2;
            if(GetObjectHeap(res,task2))
            {
                auto em = TExternalMethod::Create(ls2,"",{},[ent,task,obj](GCList& ls, std::vector<TObject> args)->TObject {
                    
                    obj->Invoke(obj,ent,args.empty() ? (TObject)Undefined():args[0]);

                    return nullptr;
                });
                em->watch.push_back(task);
                em->watch.push_back(ent);
               
                task2->ContinueWith(em);
            }

        /*} catch(...) {
            task->SetFailed(std::current_exception());
        }*/

        return task;
    }
}