#include "CrossLang.hpp"

namespace Tesses::CrossLang 
{
    TQueryable::TQueryable(TObject parent) : TQueryable(parent,TQueryableMode::Passthrough, {})
    {

    }
    TQueryable::TQueryable(TObject parent, TQueryableMode mode, std::vector<TObject> args): parent(parent), mode(mode), args(args)
    {

    }

    TQueryable* TQueryable::Skip(GCList& ls,int64_t no)
    {
        return TQueryable::Create(ls,this,TQueryableMode::Skip, {no});
    }
    TQueryable* TQueryable::SkipWhile(GCList& ls, TCallable* call)
    {
        return TQueryable::Create(ls,this,TQueryableMode::SkipWhile, {call});
    }
    TQueryable* TQueryable::Take(GCList& ls, int64_t no)
    {
        return TQueryable::Create(ls,this,TQueryableMode::Take, {no});
    }
    TQueryable* TQueryable::TakeWhile(GCList& ls, TCallable* call)
    {
        return TQueryable::Create(ls,this,TQueryableMode::TakeWhile, {call});
    }
    TQueryable* TQueryable::Select(GCList& ls, TCallable* call)
    {
        return TQueryable::Create(ls,this,TQueryableMode::Select, {call});
    }
    TQueryable* TQueryable::Where(GCList& ls, TCallable* call)
    {
        return TQueryable::Create(ls,this,TQueryableMode::Where, {call});
    }
    
    TList* TQueryable::ToList(GCList& ls)
    {
        auto gc = ls.GetGC();
        GCList ls2(gc);
        auto enumerator = this->GetEnumerator(ls);
        if(enumerator == nullptr) return nullptr;
        auto list = TList::Create(ls);
        while(enumerator->MoveNext(gc))
        {
            gc->BarrierBegin();
            list->Add(enumerator->GetCurrent(ls));
            gc->BarrierEnd();
        }
        return list;
    }
    TQueryable* TQueryable::Create(GCList& ls, TObject parent)
    {
        TQueryable* queryable = new TQueryable(parent);
        std::shared_ptr<GC> gc = ls.GetGC();
        ls.Add(queryable);
        gc->Watch(queryable);
        return queryable;
    }
    TQueryable* TQueryable::Create(GCList& ls, TObject parent, TQueryableMode mode, std::vector<TObject> args)
    {
        TQueryable* queryable = new TQueryable(parent, mode,args);
        std::shared_ptr<GC> gc = ls.GetGC();
        ls.Add(queryable);
        gc->Watch(queryable);
        return queryable;
    }
    

    void TQueryable::Mark()
    {
        if(this->marked) return;
        this->marked=true;
        
        GC::Mark(this->parent);
        for(auto& item : args)
            GC::Mark(item);
    }

    void TQueryable::ForEach(std::shared_ptr<GC> gc, TCallable* call)
    {
        if(call == nullptr) return;
        GCList ls(gc);
        auto enumerator = this->GetEnumerator(ls);
        if(enumerator == nullptr) return;
        while(enumerator->MoveNext(gc))
        {
            GCList ls2(gc);
            call->Call(ls2,{enumerator->GetCurrent(ls2)});
        }
    }
    int64_t TQueryable::Count(std::shared_ptr<GC> gc, TCallable* call)
    {
        if(call == nullptr) return 0;
        GCList ls(gc);
        auto enumerator = this->GetEnumerator(ls);
        if(enumerator == nullptr) return 0;
        int64_t count=0;
        while(enumerator->MoveNext(gc))
        {
            GCList ls2(gc);
            if(ToBool(call->Call(ls2,{enumerator->GetCurrent(ls2)}))) count++;
        }
        return count;
    }
    int64_t TQueryable::Count(std::shared_ptr<GC> gc)
    {
        GCList ls(gc);
        auto enumerator = this->GetEnumerator(ls);
        if(enumerator == nullptr) return 0;
        int64_t count=0;
        while(enumerator->MoveNext(gc))
        {
            count++;
        }
        return count;
    }
    bool TQueryable::Contains(std::shared_ptr<GC> gc, TObject value)
    {
        GCList ls(gc);
        auto enumerator = this->GetEnumerator(ls);
        if(enumerator == nullptr) return false;
        while(enumerator->MoveNext(gc))
        {
            GCList ls2(gc);
            

            if(Equals(gc,value,enumerator->GetCurrent(ls2)))
                return true;
            
        }
        return false;
    }
    bool TQueryable::Any(std::shared_ptr<GC> gc, TCallable* call)
    {
        if(call == nullptr) return false;
        GCList ls(gc);
        auto enumerator = this->GetEnumerator(ls);
        if(enumerator == nullptr) return false;
       
        while(enumerator->MoveNext(gc))
        {
            GCList ls2(gc);
            if(ToBool(call->Call(ls2,{enumerator->GetCurrent(ls2)}))) return true;
        }
        return false;
    }
    bool TQueryable::All(std::shared_ptr<GC> gc, TCallable* call)
    {
        if(call == nullptr) return true;
        GCList ls(gc);
        auto enumerator = this->GetEnumerator(ls);
        if(enumerator == nullptr) return true;
       
        while(enumerator->MoveNext(gc))
        {
            GCList ls2(gc);
            if(!ToBool(call->Call(ls2,{enumerator->GetCurrent(ls2)}))) return false;
        }
        return true;
    }

    class SkipItterator : public TEnumerator {
        private:
            TEnumerator* parentEnum;
            int64_t skipCount;
            SkipItterator(TEnumerator* parentEnum, int64_t skipCount) : parentEnum(parentEnum), skipCount(skipCount)
            {

            }
        public:
            static SkipItterator* Create(GCList& ls, TEnumerator* parentEnum, int64_t skipCount)
            {
                SkipItterator* queryable = new SkipItterator(parentEnum, skipCount);
                std::shared_ptr<GC> gc = ls.GetGC();
                ls.Add(queryable);
                gc->Watch(queryable);
                return queryable;
            }
            bool MoveNext(std::shared_ptr<GC> ls)
            {
                if(this->parentEnum == nullptr) return false;
                while(skipCount > 0)
                {
                    if(!this->parentEnum->MoveNext(ls)) {
                        skipCount=0;
                        return false;
                    }
                    skipCount--;
                }
                return this->parentEnum->MoveNext(ls);
            }
            TObject GetCurrent(GCList& ls)
            {
                return this->parentEnum->GetCurrent(ls);
            }
            void Mark()
            {
                if(this->marked) return;
                this->marked=true;

                if(parentEnum != nullptr) parentEnum->Mark();
            }
    };
    class TakeItterator : public TEnumerator {
        private:
            TEnumerator* parentEnum;
            int64_t takeCount;
            TakeItterator(TEnumerator* parentEnum, int64_t takeCount) : parentEnum(parentEnum), takeCount(takeCount)
            {

            }
        public:
            static TakeItterator* Create(GCList& ls, TEnumerator* parentEnum, int64_t takeCount)
            {
                TakeItterator* queryable = new TakeItterator(parentEnum, takeCount);
                std::shared_ptr<GC> gc = ls.GetGC();
                ls.Add(queryable);
                gc->Watch(queryable);
                return queryable;
            }
            bool MoveNext(std::shared_ptr<GC> ls)
            {
                if(this->parentEnum == nullptr) return false;
                if(takeCount > 0)
                {
                    takeCount--;
                    return this->parentEnum->MoveNext(ls);
                }
                return false;
            }
            TObject GetCurrent(GCList& ls)
            {
                return this->parentEnum->GetCurrent(ls);
            }
            void Mark()
            {
                if(this->marked) return;
                this->marked=true;

                if(parentEnum != nullptr) parentEnum->Mark();
            }
    };
    class SkipWhileItterator : public TEnumerator {
        private:
            TEnumerator* parentEnum;
            TCallable* callable;
            SkipWhileItterator(TEnumerator* parentEnum, TCallable* callable) : parentEnum(parentEnum), callable(callable)
            {

            }
        public:
            static SkipWhileItterator* Create(GCList& ls, TEnumerator* parentEnum, TCallable* callable)
            {
                SkipWhileItterator* queryable = new SkipWhileItterator(parentEnum, callable);
                std::shared_ptr<GC> gc = ls.GetGC();
                ls.Add(queryable);
                gc->Watch(queryable);
                return queryable;
            }
            bool MoveNext(std::shared_ptr<GC> ls)
            {
                if(this->parentEnum == nullptr) return false;
                ls->BarrierBegin();
                auto callable = this->callable;
                ls->BarrierEnd();
                
                if(callable != nullptr)
                {
                    while(true)
                    {
                        if(this->parentEnum->MoveNext(ls))
                        {
                            GCList ls2(ls);
                            auto result = callable->Call(ls2,{this->parentEnum->GetCurrent(ls2)});
                            if(!ToBool(result))
                            {
                                ls->BarrierBegin();
                                this->callable=nullptr;
                                ls->BarrierEnd();
                                return true;
                            }
                    
                        } 
                        else 
                        { 
                            ls->BarrierBegin();
                            this->callable=nullptr;
                            ls->BarrierEnd();
                            return false;
                        }
                    }
                }
                else {
                    return this->parentEnum->MoveNext(ls);
                }
                return false;
            }
            TObject GetCurrent(GCList& ls)
            {
                return this->parentEnum->GetCurrent(ls);
            }
            void Mark()
            {
                if(this->marked) return;
                this->marked=true;

                if(parentEnum != nullptr) parentEnum->Mark();
                if(callable != nullptr) callable->Mark();
            }
    };
    class TakeWhileItterator : public TEnumerator {
        private:
            TEnumerator* parentEnum;
            TCallable* callable;
            TakeWhileItterator(TEnumerator* parentEnum, TCallable* callable) : parentEnum(parentEnum), callable(callable)
            {

            }
        public:
            static TakeWhileItterator* Create(GCList& ls, TEnumerator* parentEnum, TCallable* callable)
            {
                TakeWhileItterator* queryable = new TakeWhileItterator(parentEnum, callable);
                std::shared_ptr<GC> gc = ls.GetGC();
                ls.Add(queryable);
                gc->Watch(queryable);
                return queryable;
            }
            bool MoveNext(std::shared_ptr<GC> ls)
            {
                if(this->parentEnum == nullptr) return false;
                ls->BarrierBegin();
                auto callable = this->callable;
                ls->BarrierEnd();
                if(callable != nullptr)
                {
                    if(this->parentEnum->MoveNext(ls))
                    {
                        GCList ls2(ls);
                        auto result = callable->Call(ls2,{this->parentEnum->GetCurrent(ls2)});
                        if(!ToBool(result))
                        {
                            ls->BarrierBegin();
                            this->callable=nullptr;
                            ls->BarrierEnd();
                            return false;
                        }
                        return true;
                    }
                }
                return false;
            }
            TObject GetCurrent(GCList& ls)
            {
                return this->parentEnum->GetCurrent(ls);
            }
            void Mark()
            {
                if(this->marked) return;
                this->marked=true;

                if(parentEnum != nullptr) parentEnum->Mark();
                if(callable != nullptr) callable->Mark();
            }
    };
    class WhereItterator : public TEnumerator {
        private:
            TEnumerator* parentEnum;
            TCallable* callable;
            WhereItterator(TEnumerator* parentEnum, TCallable* callable) : parentEnum(parentEnum), callable(callable)
            {

            }
        public:
            static WhereItterator* Create(GCList& ls, TEnumerator* parentEnum, TCallable* callable)
            {
                WhereItterator* queryable = new WhereItterator(parentEnum, callable);
                std::shared_ptr<GC> gc = ls.GetGC();
                ls.Add(queryable);
                gc->Watch(queryable);
                return queryable;
            }
            bool MoveNext(std::shared_ptr<GC> ls)
            {
                if(this->parentEnum == nullptr || this->callable == nullptr) return false;
                while(this->parentEnum->MoveNext(ls)) 
                {
                    GCList ls2(ls);
                    auto cur = this->parentEnum->GetCurrent(ls2);
                    if(ToBool(callable->Call(ls2,{cur})))
                        return true;
                    
                }
                return false;
            }
            TObject GetCurrent(GCList& ls)
            {
                return this->parentEnum->GetCurrent(ls);
            }
            void Mark()
            {
                if(this->marked) return;
                this->marked=true;

                if(parentEnum != nullptr) parentEnum->Mark();
                if(callable != nullptr) callable->Mark();
            }
    };
    class SelectItterator : public TEnumerator {
        private:
            TEnumerator* parentEnum;
            TCallable* callable;
            TObject value;
            SelectItterator(TEnumerator* parentEnum, TCallable* callable) : parentEnum(parentEnum), callable(callable)
            {

            }
        public:
            static SelectItterator* Create(GCList& ls, TEnumerator* parentEnum, TCallable* callable)
            {
                SelectItterator* queryable = new SelectItterator(parentEnum, callable);
                std::shared_ptr<GC> gc = ls.GetGC();
                ls.Add(queryable);
                gc->Watch(queryable);
                return queryable;
            }
            bool MoveNext(std::shared_ptr<GC> ls)
            {
                if(this->parentEnum == nullptr || this->callable == nullptr) return false;
                if(this->parentEnum->MoveNext(ls)) 
                {
                    GCList ls2(ls);
                    auto cur = this->parentEnum->GetCurrent(ls2);
                    auto value = this->callable->Call(ls2,{cur});

                    ls->BarrierBegin();
                    this->value = value;
                    ls->BarrierEnd();

                    return true;
                }
                return false;
            }
            TObject GetCurrent(GCList& ls)
            {
                ls.GetGC()->BarrierBegin();
                auto value=this->value;
                ls.GetGC()->BarrierEnd();
                return value;
            }
            void Mark()
            {
                if(this->marked) return;
                this->marked=true;

                if(parentEnum != nullptr) parentEnum->Mark();
                if(callable != nullptr) callable->Mark();

                GC::Mark(this->value);
            }
    };
    
   
    TEnumerator* TQueryable::GetEnumerator(GCList& ls)
    {
        switch(this->mode)
        {
            case TQueryableMode::Skip:
            {
                int64_t skipCount;
                if(GetArgument(args,0,skipCount))
                    return SkipItterator::Create(ls,  TEnumerator::CreateFromObject(ls, this->parent), skipCount);
            }
                break;
            case TQueryableMode::Take:
            {
                int64_t takeCount;
                if(GetArgument(args,0,takeCount))
                    return TakeItterator::Create(ls,  TEnumerator::CreateFromObject(ls, this->parent), takeCount);
           
            }
                break;
            case TQueryableMode::SkipWhile:
            {
                TCallable* callable;
                if(GetArgumentHeap(args,0,callable))
                    return SkipWhileItterator::Create(ls,  TEnumerator::CreateFromObject(ls, this->parent), callable);
           
            }
                break;
            case TQueryableMode::TakeWhile:
            {
                TCallable* callable;
                if(GetArgumentHeap(args,0,callable))
                    return TakeWhileItterator::Create(ls,  TEnumerator::CreateFromObject(ls, this->parent), callable);
           
            }
                break;
            case TQueryableMode::Select:
            {
                TCallable* callable;
                if(GetArgumentHeap(args,0,callable))
                    return SelectItterator::Create(ls,  TEnumerator::CreateFromObject(ls, this->parent), callable);
           
            }
                break;
            case TQueryableMode::Where:
            {
                TCallable* callable;
                if(GetArgumentHeap(args,0,callable))
                    return WhereItterator::Create(ls,  TEnumerator::CreateFromObject(ls, this->parent), callable);
           
            }
                break;
        }

        return TEnumerator::CreateFromObject(ls, this->parent);
    }

}