#include "CrossLang.hpp"

namespace Tesses::CrossLang
{
    TObject Dictionary_Items(GCList& ls, std::vector<TObject> args)
    {
        
        TDictionary* dict;
        TDynamicDictionary* dynDict;
        if(GetArgumentHeap(args,0,dynDict))
        {
            TDictionary* enumerableItem = TDictionary::Create(ls);
            ls.GetGC()->BarrierBegin();

            auto fn = TExternalMethod::Create(ls,"Get Enumerator for Dictionary",{"dict"},[dynDict](GCList& ls2, std::vector<TObject> args)->TObject {
                return dynDict->GetEnumerator(ls2);
            });
            fn->watch.push_back(dynDict);
                
            enumerableItem->SetValue("GetEnumerator", fn);
                
            ls.GetGC()->BarrierEnd();

            return enumerableItem;
            
        }
        if(GetArgumentHeap(args,0,dict))
        {
            TDictionary* enumerableItem = TDictionary::Create(ls);
            ls.GetGC()->BarrierBegin();

            auto fn = TExternalMethod::Create(ls,"Get Enumerator for Dictionary",{"dict"},[dict](GCList& ls2, std::vector<TObject> args)->TObject {
                return TDictionaryEnumerator::Create(ls2,dict);
            });
            fn->watch.push_back(dict);
                
            enumerableItem->SetValue("GetEnumerator", fn);
                
            ls.GetGC()->BarrierEnd();

            return enumerableItem;
            
        }

        return Undefined();
    }
    TObject Dictionary_GetField(GCList& ls, std::vector<TObject> args)
    {
        TDictionary* dict;
        TDynamicDictionary* dynDict;
        std::string key;
        if(GetArgument(args,1,key))
        {
            if(GetArgumentHeap(args,0,dict))
            {
                ls.GetGC()->BarrierBegin();
                auto res = dict->GetValue(key);
                ls.GetGC()->BarrierEnd();
                return res;
            }
            else if(GetArgumentHeap(args,0,dynDict))
            {
                return dynDict->GetField(ls,key);
            }
        }
        return nullptr;
    }
    TObject Dictionary_SetField(GCList& ls, std::vector<TObject> args)
    {
        TDictionary* dict;
        TDynamicDictionary* dynDict;
        std::string key;
        if(args.size() == 3 && GetArgument(args,1,key))
        {
            if(GetArgumentHeap(args,0,dict))
            {
                ls.GetGC()->BarrierBegin();
                dict->SetValue(key,args[2]);
                ls.GetGC()->BarrierEnd();
            }
            else if(GetArgumentHeap(args,0,dynDict))
            {
                dynDict->SetField(ls,key,args[2]);
            }
        }
        return nullptr;
    }
    void TStd::RegisterDictionary(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterDictionary=true;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        

        gc->BarrierBegin();
       
        dict->DeclareFunction(gc, "Items","Get Dictionary Item Enumerable, for the each(item : Dictionary.Items(myDict)){item.Key; item.Value;}",{"dictionary"},Dictionary_Items);
        dict->DeclareFunction(gc, "SetField","Set a field in dictionary",{"dict","key","value"},Dictionary_SetField);
        dict->DeclareFunction(gc, "GetField","Get a field in dictionary",{"dict","key"},Dictionary_GetField);

        env->DeclareVariable("Dictionary", dict);
        gc->BarrierEnd();
    }
}