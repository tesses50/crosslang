/*

*/

#include "CrossLang.hpp"

#if defined(CROSSLANG_ENABLE_JSON)
#include <jansson.h>
#endif

namespace Tesses::CrossLang
{

#if defined(CROSSLANG_ENABLE_JSON)
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
    static json_t* JsonSerialize(TObject v)
    {
        if(std::holds_alternative<std::nullptr_t>(v)) return json_null();
        if(std::holds_alternative<int64_t>(v)) return json_integer(std::get<int64_t>(v));
        if(std::holds_alternative<double>(v)) return json_real(std::get<double>(v));
        if(std::holds_alternative<bool>(v)) return json_boolean(std::get<bool>(v));
        if(std::holds_alternative<std::string>(v)) 
        { 
            std::string txt = std::get<std::string>(v); 
            return json_stringn(txt.c_str(),txt.size());
        }
        if(std::holds_alternative<THeapObjectHolder>(v))
        {
            auto obj = std::get<THeapObjectHolder>(v).obj;
            auto ls = dynamic_cast<TList*>(obj);
            auto dict = dynamic_cast<TDictionary*>(obj);

            if(ls != nullptr)
            {
                json_t* items=json_array();
                for(int64_t i = 0; i < ls->Count(); i++)
                {
                    auto val = ls->Get(i);
                    if(IsValidForJson(val))
                    json_array_append_new(items,JsonSerialize(val));
                }
                return items;
            }

            if(dict != nullptr)
            {
                json_t* obj = json_object();
                for(auto item : dict->items)
                {
                    if(IsValidForJson(item.second))
                    json_object_set_new(obj, item.first.c_str(),JsonSerialize(item.second));
                }   
                return obj;
            }
        }

        return json_null();
    }
    static TObject Json_Encode(GCList& ls2, std::vector<TObject> args)
    {
        if(args.size() >= 1)
        {
            bool indent = (args.size() == 2 && std::holds_alternative<bool>(args[1]) && std::get<bool>(args[1]));
            
            auto json = JsonSerialize(args[0]);
            char* txt = json_dumps(json, indent ? JSON_INDENT(4) : 0);
            std::string str = txt;
            free(txt);
            json_decref(json);
            return str;
        }
        return "null";
    }
    static TObject JsonDeserialize(GCList& ls2,json_t* json)
    {
        if(json == nullptr) return nullptr;
        if(json_is_null(json)) return nullptr;
        if(json_is_true(json)) return true;
        if(json_is_false(json)) return false;
        if(json_is_integer(json)) return (int64_t)json_integer_value(json);
        if(json_is_real(json)) return json_real_value(json);
        if(json_is_string(json))
        {
            return std::string(json_string_value(json),json_string_length(json));
        }
        if(json_is_array(json))
        {
            TList* ls = TList::Create(ls2);
            json_t* item;
            size_t index;
            json_array_foreach(json,index, item)
            {
                auto itemRes = JsonDeserialize(ls2,item);
                ls2.GetGC()->BarrierBegin();
                ls->Add(itemRes);
                ls2.GetGC()->BarrierEnd();
            }
            return ls;
        }
        if(json_is_object(json))
        {

            TDictionary* dict = TDictionary::Create(ls2);
            void* n;
            const char* key;
            size_t len;
            json_t* value;
            
            json_object_foreach_safe(json,n,key,value)
            {
                auto itemRes = JsonDeserialize(ls2,value);       
                ls2.GetGC()->BarrierBegin();
                dict->SetValue(std::string(key),itemRes);
                ls2.GetGC()->BarrierEnd();
            }
            return dict;
        }
        return Undefined();
    }
    static TObject Json_Decode(GCList& ls2,std::vector<TObject> args)
    {
        if(args.size() > 0 && std::holds_alternative<std::string>(args[0]))
        {
            std::string jsonText = std::get<std::string>(args[0]);
            json_t* json = json_loadb(jsonText.c_str(), jsonText.size(),0,NULL);
            
            auto res = JsonDeserialize(ls2, json);
            json_decref(json);
            return res;
        }
        return Undefined();
    }
    #endif
    void TStd::RegisterJson(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterJSON=true;
        #if defined(CROSSLANG_ENABLE_JSON)
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc, "Decode","Deserialize Json",{"Json string"},Json_Decode);   
        dict->DeclareFunction(gc, "Encode","Serialize Json",{"any","$indent"},Json_Encode);   
        
        
        gc->BarrierBegin();
        env->DeclareVariable("Json", dict);
        gc->BarrierEnd();
        #endif
    }
    
}