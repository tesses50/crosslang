#include "CrossLang.hpp"

namespace Tesses::CrossLang {
std::string TByteView::TypeName() { return "ByteView"; }
TString *TByteView::ToString(GCList &ls) {
    auto bounds = GetBounds();
    if (bounds.first == nullptr || bounds.second == 0)
        return ls.FromString("");

    return ls.Create<TString>(reinterpret_cast<const char *>(bounds.first),
                              bounds.second);
}
bool TByteView::opExecuteMethod(InterperterThread *thrd, std::shared_ptr<GC> gc,
                                const std::string &name,
                                const std::vector<TObject> &args) {
    if (name == "Slice") {
        int64_t offset = 0;
        int64_t length = -1;

        GetArgument(args, 0, offset);
        GetArgument(args, 1, length);

        GCList ls(gc);

        thrd->call_stack_entries.back()->Push(
            gc, ls.Create<TSpan>(this, offset, length));
        return false;
    }
    if (name == "Count" || name == "Length") {
        thrd->call_stack_entries.back()->Push(
            gc, static_cast<int64_t>(this->GetBounds().second));
        return false;
    }
    if (name == "GetAt") {
        int64_t i64;
        if (GetArgument(args, 0, i64)) {
            auto res = this->GetAt(static_cast<size_t>(i64));

            thrd->call_stack_entries.back()->Push(
                gc, res == -1 ? static_cast<TObject>(nullptr)
                              : static_cast<TObject>(res));
            return false;
        }
    }

    thrd->call_stack_entries.back()->Push(gc, Undefined());
    return false;
}
bool TByteView::opGetField(InterperterThread *thrd, std::shared_ptr<GC> gc,
                           const std::string &name) {
    if (name == "Count" || name == "Length") {
        thrd->call_stack_entries.back()->Push(
            gc, static_cast<int64_t>(this->GetBounds().second));
        return false;
    }
    thrd->call_stack_entries.back()->Push(gc, Undefined());
    return false;
}
void TByteView::CopyTo(TMutByteView *view) {
    auto myBounds = this->GetBounds();
    if (myBounds.first == nullptr || myBounds.second == 0)
        return;
    auto theirBounds = view->GetMutableBoundsConstrained(0, myBounds.second);

    if (theirBounds.first == nullptr || theirBounds.second == 0)
        return;

    memcpy(theirBounds.first, myBounds.first, theirBounds.second);
}
void TByteView::CopyTo(TMutByteView *view, size_t srcOffset, size_t destOffset,
                       size_t length) {
    auto myBounds = this->GetBoundsConstrained(srcOffset, length);
    if (myBounds.first == nullptr || myBounds.second == 0)
        return;
    auto theirBounds =
        view->GetMutableBoundsConstrained(destOffset, myBounds.second);

    if (theirBounds.first == nullptr || theirBounds.second == 0)
        return;

    memcpy(theirBounds.first, myBounds.first, theirBounds.second);
}
} // namespace Tesses::CrossLang