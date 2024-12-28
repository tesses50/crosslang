#include "CrossLang.hpp"

namespace Tesses::CrossLang
{

    TObject Dictionary_Items(GCList& ls, std::vector<TObject> args)
    {
        
        TDictionary* dict;
       
        dict->items.begin();
        if(args.size() == 1 && std::holds_alternative<THeapObjectHolder>(args[0]))
        {
            auto item = dynamic_cast<TDictionary*>(std::get<THeapObjectHolder>(args[0]).obj);
            if(item != nullptr)
            {
                TDictionary* enumerableItem = TDictionary::Create(ls);
                ls.GetGC()->BarrierBegin();

                auto fn = TExternalMethod::Create(ls,"Get Enumerator for Dictionary",{"dict"},[item](GCList& ls2, std::vector<TObject> args)->TObject {
                    return TDictionaryEnumerator::Create(ls2,item);
                });
                fn->watch.push_back(item);
                
                enumerableItem->SetValue("GetEnumerator", fn);
                
                ls.GetGC()->BarrierEnd();

                return enumerableItem;
            }
        }

        return Undefined();
    }
 
    void TStd::RegisterDictionary(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterDictionary=true;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        

        gc->BarrierBegin();
       
        dict->DeclareFunction(gc, "Items","Get Dictionary Item Enumerable, for the each(item : Dictionary.Items(myDict)){item.Key; item.Value;}",{"dictionary"},Dictionary_Items);

        env->DeclareVariable("Dictionary", dict);
        gc->BarrierEnd();
    }
}