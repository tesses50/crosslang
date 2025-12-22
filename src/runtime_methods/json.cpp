
#include "CrossLang.hpp"
using namespace Tesses::Framework::Serialization::Json;

namespace Tesses::CrossLang
{

    static bool IsValidForJson(TObject v)
    {
        if(std::holds_alternative<std::nullptr_t>(v)) return true;

        if(std::holds_alternative<int64_t>(v)) return true;

        if(std::holds_alternative<double>(v)) return true;


        if(std::holds_alternative<bool>(v)) return true;


        if(std::holds_alternative<std::string>(v)) return true;


        if(std::holds_alternative<THeapObjectHolder>(v))
        {
            auto res = std::get<THeapObjectHolder>(v);
            auto ls = dynamic_cast<TList*>(res.obj);
            auto dict = dynamic_cast<TDictionary*>(res.obj);
            if(ls != nullptr) return true;
            if(dict != nullptr) return true;
        }
        return false;
    }
    static JToken JsonSerialize(TObject v)
    {
        if(std::holds_alternative<std::nullptr_t>(v)) return nullptr;
        if(std::holds_alternative<int64_t>(v)) return std::get<int64_t>(v);
        if(std::holds_alternative<double>(v)) return std::get<double>(v);
        if(std::holds_alternative<bool>(v)) return std::get<bool>(v);
        if(std::holds_alternative<std::string>(v)) return std::get<std::string>(v);
        if(std::holds_alternative<THeapObjectHolder>(v))
        {
            auto obj = std::get<THeapObjectHolder>(v).obj;
            auto ls = dynamic_cast<TList*>(obj);
            auto dict = dynamic_cast<TDictionary*>(obj);

            if(ls != nullptr)
            {
                JArray items;
                for(int64_t i = 0; i < ls->Count(); i++)
                {
                    auto val = ls->Get(i);
                    if(IsValidForJson(val))
                        items.Add(JsonSerialize(val));
                }
                return items;
            }

            if(dict != nullptr)
            {
                JObject obj;
                for(auto item : dict->items)
                {
                    if(IsValidForJson(item.second))
                        obj.SetValue(item.first,JsonSerialize(item.second));
                }   
                return obj;
            }
        }

        return nullptr;
    }
    static TObject JsonEncode(GCList& ls2, std::vector<TObject> args)
    {
        if(args.size() >= 1)
        {
            bool indent = (args.size() == 2 && std::holds_alternative<bool>(args[1]) && std::get<bool>(args[1]));
            
            auto json = JsonSerialize(args[0]);

            return Json::Encode(json,indent);
        }
        return "null";
    }
    static TObject JsonDocEncode(GCList& ls2, std::vector<TObject> args)
    {
        if(args.size() >= 1)
        {
            bool indent = (args.size() == 2 && std::holds_alternative<bool>(args[1]) && std::get<bool>(args[1]));
            
            auto json = JsonSerialize(args[0]);
            JArray ar;
            if(TryGetJToken(json,ar))
            return Json::DocEncode(ar,indent);

            
        }
        return "";
    }
    static TObject JsonDeserialize(GCList& ls2,JToken json)
    {
        if(std::holds_alternative<JUndefined>(json)) return nullptr;
        if(std::holds_alternative<std::nullptr_t>(json)) return nullptr;
        bool b;
        int64_t _i64;
        double _f64;
        std::string str;
        JArray arr;
        JObject obj;
        if(TryGetJToken(json,b)) return b;
        if(TryGetJToken(json,_i64)) return _i64;
        if(TryGetJToken(json,_f64)) return _f64;
        if(TryGetJToken(json,str)) return str;
        if(TryGetJToken(json,arr))
        {
            TList* ls = TList::Create(ls2);
           
            for(auto& item : arr)
            {
                auto itemRes = JsonDeserialize(ls2,item);
                ls2.GetGC()->BarrierBegin();
                ls->Add(itemRes);
                ls2.GetGC()->BarrierEnd();
            }
            return ls;
        }
        if(TryGetJToken(json,obj))
        {

            TDictionary* dict = TDictionary::Create(ls2);
      
            
            for(auto& item : obj)
            {
                
                auto itemRes = JsonDeserialize(ls2,item.second);       
                ls2.GetGC()->BarrierBegin();
                dict->SetValue(item.first,itemRes);
                ls2.GetGC()->BarrierEnd();
            }
            return dict;
        }
        return Undefined();
    }
    static TObject JsonDecode(GCList& ls2,std::vector<TObject> args)
    {
        if(args.size() > 0 && std::holds_alternative<std::string>(args[0]))
        {
            
            
           return JsonDeserialize(ls2, Json::Decode(std::get<std::string>(args[0])));
            
            
        }
        return Undefined();
    }
    static TObject JsonDocDecode(GCList& ls2,std::vector<TObject> args)
    {
        if(args.size() > 0 && std::holds_alternative<std::string>(args[0]))
        {
            
            
           return JsonDeserialize(ls2, Json::DocDecode(std::get<std::string>(args[0])));
            
            
        }

        return Undefined();
    }
    std::string Json_Encode(TObject o,bool indent)
    {
        return Json::Encode(JsonSerialize(o),indent);
    }
    TObject Json_Decode(GCList ls,std::string str)
    {
        return JsonDeserialize(ls,Json::Decode(str));
    }
    std::string Json_DocEncode(TObject o,bool indent)
    {
        auto obj = JsonSerialize(o);
        JArray ls;
        if(TryGetJToken(obj,ls))
            return Json::DocEncode(ls,indent);
        return "";
    }
    TObject Json_DocDecode(GCList ls,std::string str)
    {
        return JsonDeserialize(ls,Json::DocDecode(str));
    }
    void TStd::RegisterJson(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterJSON=true;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc, "Decode","Deserialize Json",{"jsonString"},JsonDecode);   
        dict->DeclareFunction(gc, "Encode","Serialize Json",{"any","$indent"},JsonEncode);  
        dict->DeclareFunction(gc, "DocDecode", "Deserialize JsonDoc", {"jsonDocString"},JsonDocDecode);
        dict->DeclareFunction(gc, "DocEncode", "Serialize JsonDoc", {"ls","$indent"},JsonDocEncode);
        
        
        
        gc->BarrierBegin();
        env->DeclareVariable("Json", dict);
        gc->BarrierEnd();
    }
    
}