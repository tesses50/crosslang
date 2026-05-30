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
bool InterperterThread::InvokeTwo(GCList &ls, TObject fn, TObject left,
                                  TObject right) {
    if (std::holds_alternative<THeapObjectHolder>(fn)) {

        auto obj =
            dynamic_cast<TCallable *>(std::get<THeapObjectHolder>(fn).obj);
        if (obj != nullptr) {
            auto closure = dynamic_cast<TClosure *>(obj);
            if (closure != nullptr) {

                if (!closure->closure->args.empty() &&
                    closure->closure->args[0] == "this") {
                    this->AddCallStackEntry(ls, closure, {left, right});
                } else {
                    this->AddCallStackEntry(ls, closure, {right});
                }

            } else {
                this->call_stack_entries[this->call_stack_entries.size() - 1]
                    ->Push(ls.GetGC(), obj->Call(ls, {right}));

                return false;
            }
            return true;
        }
    }
    return false;
}
} // namespace Tesses::CrossLang