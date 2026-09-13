#include "CrossLang.hpp"

namespace Tesses::CrossLang {

TString::TString() {}
TString::TString(std::string_view str) : text(str) {}
TString::TString(const char *text, size_t len) : text(text, len) {}
TString::TString(std::string_view left, std::string_view right) {
    text.reserve(left.size() + right.size());
    text.append(left.begin(), left.end());
    text.append(right.begin(), right.end());
}
TString::TString(std::string &&str) : text(std::move(str)) {}

const std::string &TString::GetString() const { return this->text; }

std::pair<const uint8_t *, size_t> TString::GetBounds() const {
    return std::pair<const uint8_t *, size_t>(
        reinterpret_cast<const uint8_t *>(text.data()), text.size());
}

std::string TString::TypeName() { return "String"; }

TString *TString::ToString(GCList &ls) {
    ls.Add(this);
    return this;
}

bool TString::opAdd(InterperterThread *thrd, std::shared_ptr<GC> gc,
                    TObject rhs) {
    TString *rStr;
    char rChr;
    if (GetObjectHeap(rhs, rStr)) {
        GCList ls(gc);
        thrd->call_stack_entries.back()->Push(
            gc, ls.Create<TString>(this->GetString(), rStr->GetString()));
        return false;
    }
    if (GetObject(rhs, rChr)) {
        GCList ls(gc);
        thrd->call_stack_entries.back()->Push(
            gc,
            ls.Create<TString>(this->GetString(), std::string_view(&rChr, 1)));
        return false;
    }

    thrd->call_stack_entries.back()->Push(gc, Undefined());
    return false;
}
bool TString::opLessThan(InterperterThread *thrd, std::shared_ptr<GC> gc,
                         TObject rhs) {

    TString *rStr;
    if (GetObjectHeap(rhs, rStr)) {
        thrd->call_stack_entries.back()->Push(gc, this->GetString() <
                                                      rStr->GetString());
        return false;
    }

    thrd->call_stack_entries.back()->Push(gc, Undefined());
    return false;
}
bool TString::opGreaterThan(InterperterThread *thrd, std::shared_ptr<GC> gc,
                            TObject rhs) {

    TString *rStr;
    if (GetObjectHeap(rhs, rStr)) {
        thrd->call_stack_entries.back()->Push(gc, this->GetString() >
                                                      rStr->GetString());
        return false;
    }

    thrd->call_stack_entries.back()->Push(gc, Undefined());
    return false;
}
bool TString::opLessThanEqual(InterperterThread *thrd, std::shared_ptr<GC> gc,
                              TObject rhs) {

    TString *rStr;
    if (GetObjectHeap(rhs, rStr)) {

        thrd->call_stack_entries.back()->Push(gc, this->GetString() <=
                                                      rStr->GetString());
        return false;
    }

    thrd->call_stack_entries.back()->Push(gc, Undefined());
    return false;
}
bool TString::opGreaterThanEqual(InterperterThread *thrd,
                                 std::shared_ptr<GC> gc, TObject rhs) {

    TString *rStr;
    if (GetObjectHeap(rhs, rStr)) {

        thrd->call_stack_entries.back()->Push(gc, this->GetString() >=
                                                      rStr->GetString());
        return false;
    }

    thrd->call_stack_entries.back()->Push(gc, Undefined());
    return false;
}
bool TString::IsEqualTo(std::shared_ptr<GC> gc, TObject rhs) {
    TString *rStr;
    if (GetObjectHeap(rhs, rStr)) {
        if (this == rStr)
            return true;
        return this->GetString() == rStr->GetString();
    }
    return false;
}
bool TString::IsNotEqualTo(std::shared_ptr<GC> gc, TObject rhs) {
    TString *rStr;
    if (GetObjectHeap(rhs, rStr)) {
        if (this == rStr)
            return false;
        return this->GetString() != rStr->GetString();
    }
    return true;
}

} // namespace Tesses::CrossLang