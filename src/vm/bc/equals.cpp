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

bool Equals(std::shared_ptr<GC> gc, TObject left, TObject right) {
    GCList ls(gc);
    if (std::holds_alternative<std::nullptr_t>(left) &&
        std::holds_alternative<std::nullptr_t>(right)) {

        return true;
    }

    else if (std::holds_alternative<Undefined>(left) &&
             std::holds_alternative<Undefined>(right)) {
        return true;
    }

    else if (std::holds_alternative<int64_t>(left) &&
             std::holds_alternative<int64_t>(right)) {
        return std::get<int64_t>(left) == std::get<int64_t>(right);
    }

    else if (std::holds_alternative<double>(left) &&
             std::holds_alternative<double>(right)) {
        return std::get<double>(left) == std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int64_t>(right)) {
        return std::get<double>(left) == std::get<int64_t>(right);
    } else if (std::holds_alternative<int64_t>(left) &&
               std::holds_alternative<double>(right)) {
        return std::get<int64_t>(left) == std::get<double>(right);
    }

    else if (std::holds_alternative<std::string>(left) &&
             std::holds_alternative<std::string>(right)) {
        return std::get<std::string>(left) == std::get<std::string>(right);
    }

    else if (std::holds_alternative<bool>(left) &&
             std::holds_alternative<bool>(right)) {
        return std::get<bool>(left) == std::get<bool>(right);
    }

    else if (std::holds_alternative<char>(left) &&
             std::holds_alternative<char>(right)) {
        return std::get<char>(left) == std::get<char>(right);
    } else if (std::holds_alternative<char>(left) &&
               std::holds_alternative<int64_t>(right)) {
        return std::get<char>(left) == std::get<int64_t>(right);
    } else if (std::holds_alternative<int64_t>(left) &&
               std::holds_alternative<char>(right)) {
        return std::get<int64_t>(left) == std::get<char>(right);
    } else if (std::holds_alternative<
                   std::shared_ptr<Tesses::Framework::Date::DateTime>>(left) &&
               std::holds_alternative<
                   std::shared_ptr<Tesses::Framework::Date::DateTime>>(right)) {
        return std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(
                   left)
                   ->ToEpoch() ==
               std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(
                   right)
                   ->ToEpoch();
    } else if (std::holds_alternative<
                   std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(left) &&
               std::holds_alternative<
                   std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(right)) {
        return std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(
                   left)
                   ->TotalSeconds() ==
               std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(
                   right)
                   ->TotalSeconds();
    } else if (std::holds_alternative<TVMVersion>(left) &&
               std::holds_alternative<TVMVersion>(right)) {
        auto lver = std::get<TVMVersion>(left);
        auto rver = std::get<TVMVersion>(right);
        auto r = lver.CompareTo(rver);
        return r == 0;
    } else if (std::holds_alternative<Tesses::Framework::Uuid>(left) &&
               std::holds_alternative<Tesses::Framework::Uuid>(right)) {
        auto l = std::get<Tesses::Framework::Uuid>(left);
        auto r = std::get<Tesses::Framework::Uuid>(right);

        return l == r;
    } else if (std::holds_alternative<THeapObjectHolder>(left)) {
        auto obj = std::get<THeapObjectHolder>(left).obj;
        auto dict = dynamic_cast<TDictionary *>(obj);

        auto dynDict = dynamic_cast<TDynamicDictionary *>(obj);
        auto native = dynamic_cast<TNative *>(obj);
        auto natObj = dynamic_cast<TNativeObject *>(obj);
        auto cls = dynamic_cast<TClassObject *>(obj);
        if (cls != nullptr) {
            gc->BarrierBegin();
            auto obj = cls->GetValue("", "operator==");
            gc->BarrierEnd();
            TCallable *callable;

            if (GetObjectHeap(obj, callable)) {
                return ToBool(callable->Call(ls, {right}));

            } else if (std::holds_alternative<std::nullptr_t>(right)) {
                return false;
            } else if (std::holds_alternative<Undefined>(right)) {
                return false;
            } else if (std::holds_alternative<THeapObjectHolder>(right)) {
                return cls == std::get<THeapObjectHolder>(right).obj;
            }
        } else if (natObj != nullptr) {
            return natObj->Equals(gc, right);
        }
        if (dict != nullptr) {
            gc->BarrierBegin();
            TObject fn = dict->GetValue("operator==");
            gc->BarrierEnd();
            if (!std::holds_alternative<Undefined>(fn)) {
                if (std::holds_alternative<THeapObjectHolder>(fn)) {

                    auto obj = dynamic_cast<TCallable *>(
                        std::get<THeapObjectHolder>(fn).obj);
                    if (obj != nullptr) {
                        auto closure = dynamic_cast<TClosure *>(obj);
                        if (closure != nullptr) {

                            if (!closure->closure->args.empty() &&
                                closure->closure->args[0] == "this") {
                                return ToBool(obj->Call(ls, {left, right}));
                            } else {
                                return ToBool(obj->Call(ls, {right}));
                            }

                        } else {
                            return ToBool(obj->Call(ls, {right}));
                        }
                    } else {

                        return dict == std::get<THeapObjectHolder>(right).obj;
                    }

                } else {

                    return dict == std::get<THeapObjectHolder>(right).obj;
                }

            } else {

                return dict == std::get<THeapObjectHolder>(right).obj;
            }
            return false;

        }

        else if (dynDict != nullptr) {
            auto res = dynDict->CallMethod(ls, "operator==", {right});
            if (!std::holds_alternative<std::nullptr_t>(res) &&
                std::holds_alternative<Undefined>(res)) {
                return ToBool(res);
            }
        } else if (native != nullptr &&
                   std::holds_alternative<std::nullptr_t>(right)) {
            return native->GetDestroyed();
        }

        if (std::holds_alternative<THeapObjectHolder>(right)) {
            return obj == std::get<THeapObjectHolder>(right).obj;

        }

        else if (std::holds_alternative<std::nullptr_t>(right)) {
            return false;
        } else if (std::holds_alternative<Undefined>(right)) {

            return false;
        } else {
            return false;
        }

    }

    else if (std::holds_alternative<std::nullptr_t>(right)) {
        return false;
    } else if (std::holds_alternative<Undefined>(right)) {
        return false;
    } else {
        return false;
    }

    return false;
}
} // namespace Tesses::CrossLang