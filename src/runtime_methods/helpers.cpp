#include "CrossLang.hpp"

namespace Tesses::CrossLang {
static TObject Helpers_CopyToProgress(GCList &ls, std::vector<TObject> args) {
    std::shared_ptr<Tesses::Framework::Streams::Stream> src;
    std::shared_ptr<Tesses::Framework::Streams::Stream> dest;
    double precision = 1000.0;
    TCallable *callable;
    if (GetArgument(args, 0, src) && GetArgument(args, 1, dest) &&
        GetArgumentHeap(args, 2, callable)) {
        GetArgument(args, 3, precision);
        auto len = src->GetLength();
        callable->Call(ls, {0.0});
        if (len > 0) {
            std::vector<uint8_t> buff(1024);
            int64_t pos = 0;
            int curPercent = 0;
            int lastPercent = 0;
            size_t read = 0;
            do {
                read = src->ReadBlock(buff.data(), buff.size());
                dest->WriteBlock(buff.data(), read);

                if (read == 0)
                    break;
                pos += (int64_t)read;

                double percent = ((double)pos / len);
                percent *= precision;

                curPercent = (int)percent;

                if (curPercent > lastPercent) {
                    lastPercent = curPercent;
                    callable->Call(ls, {curPercent / precision});
                }

            } while (read != 0);
        } else {
            src->CopyTo(dest);
        }
        callable->Call(ls, {1.0});
    }
    return Undefined();
}
void TStd::RegisterHelpers(std::shared_ptr<GC> gc, TRootEnvironment *env) {
    auto helpers = env->EnsureDictionary(gc, "Helpers");
    helpers->DeclareFunction(gc, "CopyToProgress",
                             "Copy Stream to another (but with progress event)",
                             {"src", "dest", "progressCB", "$precision"},
                             Helpers_CopyToProgress);
}
} // namespace Tesses::CrossLang