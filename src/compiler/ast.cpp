#include "CrossLang.hpp"
#if defined(CROSSLANG_ENABLE_JSON)
#include <jansson.h>
#endif
namespace Tesses::CrossLang
{
AdvancedSyntaxNode AdvancedSyntaxNode::Create(std::string_view nodeName,bool isExpression, std::vector<SyntaxNode> nodes)
{
    AdvancedSyntaxNode asn;
    asn.nodeName = std::string(nodeName);
    asn.isExpression = isExpression;
    asn.nodes = nodes;
    return asn;
}

#if defined(CROSSLANG_ENABLE_JSON)
static SyntaxNode Deserialize2(json_t* json)
{
    if(json_is_null(json)) return nullptr;
    if(json_is_true(json)) return true;
    if(json_is_false(json)) return false;
    if(json_is_integer(json)) return (int64_t)json_integer_value(json);
    if(json_is_real(json)) return json_real_value(json);
    if(json_is_string(json)) return std::string(json_string_value(json),json_string_length(json));
    if(json_is_object(json))
    {
        json_t* typ = json_object_get(json, "type");
        if(json_is_string(typ))
        {
            std::string type(json_string_value(typ),json_string_length(typ));
            if(type == CharExpression)
            {
                json_t* chrData = json_object_get(json,"value");
                if(json_is_integer(chrData))
                {
                    return (char)(uint8_t)json_integer_value(chrData);
                }
            }
            if(type == UndefinedExpression)
            {
                return Undefined();
            }
            json_t* isExpr = json_object_get(json,"isExpression");
            bool isExprB = json_is_true(isExpr);
             AdvancedSyntaxNode asn;
            asn.nodeName = type;
            asn.isExpression = isExprB;
            json_t* args = json_object_get(json, "args");
            if(json_is_array(args))
            {
                size_t index;
                json_t* value;
                
                json_array_foreach(args, index, value)
                {
                    asn.nodes.push_back(Deserialize2(value));
                }
            }
            return asn;
        }
    }
    return nullptr;
}
#endif


SyntaxNode Deserialize(std::string astData)
{
    #if defined(CROSSLANG_ENABLE_JSON)
    json_t* json = json_loadb(astData.c_str(),astData.size(), 0, NULL);

    auto r = Deserialize2(json);
    json_decref(json);

    return r;
    #endif

    return nullptr;
}
#if defined(CROSSLANG_ENABLE_JSON)
static json_t* Serialize2(SyntaxNode node)
{
    if(std::holds_alternative<std::nullptr_t>(node))
        return json_null();
    if(std::holds_alternative<int64_t>(node))
    {
        return json_integer(std::get<int64_t>(node));
    }
    if(std::holds_alternative<double>(node))
    {
        return json_real(std::get<double>(node));
    }
    if(std::holds_alternative<bool>(node))
    {
        return json_boolean(std::get<bool>(node));
    }
    if(std::holds_alternative<std::string>(node))
    {
        std::string str = std::get<std::string>(node);
        return json_stringn(str.c_str(),str.size());
    }
    if(std::holds_alternative<char>(node))
    {
        char c = std::get<char>(node);
        json_t* json = json_object();
        json_object_set_new(json,"type",json_string((const char*)CharExpression.data()));
        json_object_set_new(json, "value", json_integer((uint8_t)c));
        return json;
    }
    if(std::holds_alternative<Undefined>(node))
    {
        json_t* json = json_object();
        json_object_set_new(json,"type",json_string((const char*)UndefinedExpression.data()));
        return json;
    }


    if(std::holds_alternative<AdvancedSyntaxNode>(node))
    {
        auto asn = std::get<AdvancedSyntaxNode>(node);
        json_t* j = json_object();
        json_object_set_new(j, "type", json_string(asn.nodeName.c_str()));
        json_object_set_new(j, "isExpression",json_boolean(asn.isExpression));
        json_t* arr=json_array();
        for(auto item : asn.nodes)
        {
            json_array_append_new(arr,Serialize2(item));
        }
        json_object_set_new(j, "args", arr);
        return j;
    }
    
    return json_null();
}
#endif

std::string Serialize(SyntaxNode node)
{

#if defined(CROSSLANG_ENABLE_JSON)
json_t* json = Serialize2(node);
char* txt = json_dumps(json,0);
std::string str(txt);
free(json);
json_decref(json);
return str;
#endif
return "";
}

}
