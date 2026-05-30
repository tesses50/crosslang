#include "CrossLang.hpp"

namespace Tesses::CrossLang {
TRandom::TRandom() : random() {}
TRandom::TRandom(uint64_t seed) : random(seed) {}
std::string TRandom::TypeName() { return "Random"; }
TObject TRandom::CallMethod(GCList &ls, std::string name,
                            std::vector<TObject> args) {
    if (name == "Next") {
        int64_t first;
        int64_t second;
        if (GetArgument(args, 0, first)) {
            if (GetArgument(args, 1, second)) {
                return (int64_t)random.Next((int32_t)first, (int32_t)second);
            }

            return (int64_t)random.Next((uint32_t)first);
        }

        return random.Next();
    }

    if (name == "NextByte") {
        return (int64_t)random.NextByte();
    }

    if (name == "ToString") {
        return "";
    }
    return Undefined();
}
} // namespace Tesses::CrossLang