#include "CrossLang.hpp"
#include "TessesFramework/Serialization/BitConverter.hpp"
#include "TessesFramework/Streams/ByteReader.hpp"
#include "TessesFramework/Uuid.hpp"
#include <cmath>
#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <variant>
namespace Tesses::CrossLang {
std::string ToString(std::shared_ptr<GC> gc, TObject o) {
    if (std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(o)) {
        return std::get<Tesses::Framework::Filesystem::VFSPath>(o).ToString();
    }
    if (std::holds_alternative<std::string>(o)) {
        return std::get<std::string>(o);
    }
    if (std::holds_alternative<TVMVersion>(o)) {
        return std::get<TVMVersion>(o).ToString();
    }
    if (std::holds_alternative<int64_t>(o)) {
        return std::to_string(std::get<int64_t>(o));
    }

    if (std::holds_alternative<double>(o)) {
        return std::to_string(std::get<double>(o));
    }

    if (std::holds_alternative<char>(o)) {
        return std::string{std::get<char>(o)};
    }
    if (std::holds_alternative<std::nullptr_t>(o)) {
        return "null";
    }
    if (std::holds_alternative<Undefined>(o)) {
        return "undefined";
    }
    if (std::holds_alternative<bool>(o)) {
        return std::get<bool>(o) ? "true" : "false";
    }
    if (std::holds_alternative<
            std::shared_ptr<Tesses::Framework::Date::DateTime>>(o)) {
        return std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(o)
            ->ToString();
    }
    if (std::holds_alternative<
            std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(o)) {
        return std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(o)
            ->ToString(false);
    }
    if (std::holds_alternative<Tesses::Framework::Uuid>(o)) {
        return std::get<Tesses::Framework::Uuid>(o).ToString(
            Framework::UuidStringifyConfig::LowercaseNoCurly);
    }
    if (std::holds_alternative<THeapObjectHolder>(o)) {
        auto obj = std::get<THeapObjectHolder>(o).obj;
        auto dict = dynamic_cast<TDictionary *>(obj);
        auto list = dynamic_cast<TList *>(obj);
        auto bArray = dynamic_cast<TByteArray *>(obj);
        auto natObj = dynamic_cast<TNativeObject *>(obj);
        auto cls = dynamic_cast<TClassObject *>(obj);
        auto aArray = dynamic_cast<TAssociativeArray *>(obj);
        if (aArray != nullptr) {

            std::string str = {};

            gc->BarrierBegin();
            bool first = true;
            for (auto item : aArray->items) {
                if (!first)
                    str.push_back('\n');
                first = false;
                str.push_back('[');
                str.append(Json_Encode(item.first));
                str.append("] = ");
                str.append(Json_Encode(item.second));
                str.append(";");
            }
            gc->BarrierEnd();
            return str;
        }
        if (cls != nullptr) {
            auto res = cls->GetValue("", "ToString");
            TCallable *call;
            GCList ls(gc);
            if (GetObjectHeap(res, call))
                return ToString(gc, call->Call(ls, {}));
            return cls->TypeName();
        }
        if (natObj != nullptr) {
            GCList ls(gc);
            TObject o = natObj->CallMethod(ls, "ToString", {});

            return ToString(gc, o);
        }

        if (dict != nullptr) {
            GCList ls(gc);
            if (dict->MethodExists(ls, "ToString"))
                return ToString(gc, dict->CallMethod(ls, "ToString", {}));
            else {
                return Json_Encode(dict);
            }
        } else if (bArray != nullptr) {
            return std::string(bArray->data.begin(), bArray->data.end());
        } else if (list != nullptr) {
            return Json_Encode(list);
        }
    }

    return "";
}
} // namespace Tesses::CrossLang