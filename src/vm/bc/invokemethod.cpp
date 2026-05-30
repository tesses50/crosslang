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

bool InterperterThread::InvokeMethod(GCList &ls, TObject fn, TObject instance,
                                     std::vector<TObject> args) {

    if (std::holds_alternative<THeapObjectHolder>(fn)) {

        auto obj =
            dynamic_cast<TCallable *>(std::get<THeapObjectHolder>(fn).obj);
        if (obj != nullptr) {
            auto closure = dynamic_cast<TClosure *>(obj);
            if (closure != nullptr) {

                if (!closure->closure->args.empty() &&
                    closure->closure->args[0] == "this") {
                    std::vector<TObject> args2;
                    args2.push_back(instance);
                    args2.insert(args2.end(), args.begin(), args.end());
                    this->AddCallStackEntry(ls, closure, args2);
                } else {
                    this->AddCallStackEntry(ls, closure, args);
                }

            } else {
                auto val = obj->Call(ls, args);
                this->call_stack_entries.back()->Push(ls.GetGC(), val);
                return false;
            }
            return true;
        }
    }
    this->call_stack_entries.back()->Push(ls.GetGC(), Undefined());
    return false;
}
} // namespace Tesses::CrossLang