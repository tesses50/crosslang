#include "CrossLang.hpp"

namespace Tesses::CrossLang {

#define CROSSLANG_TSTRING_NAMES                                                \
    CROSSLANG_TSTRING_NAME_ENT(ToLower)                                        \
    CROSSLANG_TSTRING_NAME_ENT(ToUpper)                                        \
    CROSSLANG_TSTRING_NAME_ENT(PadLeft)                                        \
    CROSSLANG_TSTRING_NAME_ENT(PadRight)                                       \
    CROSSLANG_TSTRING_NAME_ENT(Contains)                                       \
    CROSSLANG_TSTRING_NAME_ENT(IndexOf)                                        \
    CROSSLANG_TSTRING_NAME_ENT(LastIndexOf)                                    \
    CROSSLANG_TSTRING_NAME_ENT(GetEnumerator)                                  \
    CROSSLANG_TSTRING_NAME_ENT(Substring)                                      \
    CROSSLANG_TSTRING_NAME_ENT(Remove)                                         \
    CROSSLANG_TSTRING_NAME_ENT(TrimStart)                                      \
    CROSSLANG_TSTRING_NAME_ENT(TrimEnd)                                        \
    CROSSLANG_TSTRING_NAME_ENT(StartsWith)                                     \
    CROSSLANG_TSTRING_NAME_ENT(EndsWith)

enum TStringFuncNameEnum {
#define CROSSLANG_TSTRING_NAME_ENT(name) TSF_##name,
    CROSSLANG_TSTRING_NAMES
#undef CROSSLANG_TSTRING_NAME_ENT
}; // namespace Tesses::CrossLang

static std::unordered_map<std::string_view, TStringFuncNameEnum>
    string_func_names = {
#define CROSSLANG_TSTRING_NAME_ENT(name) {#name, TSTF_##name},
        CROSSLANG_TSTRING_NAMES
#undef CROSSLANG_TSTRING_NAME_ENT
};

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
bool TString::opExecuteMethod(InterperterThread *thrd, std::shared_ptr<GC> gc,
                              const std::string &name,
                              const std::vector<TObject> &args) {
    auto result = string_func_names.find(name);

    if (result == string_func_names.end())
        return TByteView::opExecuteMethod(thrd, gc, name, args);

    switch (result->second) {
    case TSF_ToLower: {
        std::string str = this->text;
        for (size_t i = 0; i < str.size(); i++) {
            if (str[i] >= 'A' && str[i] <= 'Z') {
                str[i] = 'a' + (str[i] - 'A');
            }
        }
        GCList ls(gc);

        thrd->call_stack_entries.back()->Push(gc, ls.Create<TString>(str));
        return false;
    }
    case TSF_ToUpper: {
        std::string str = this->text;
        for (size_t i = 0; i < str.size(); i++) {
            if (str[i] >= 'a' && str[i] <= 'z') {
                str[i] = 'A' + (str[i] - 'a');
            }
        }
        GCList ls(gc);

        thrd->call_stack_entries.back()->Push(gc, ls.Create<TString>(str));
        return false;
    }
    case TSF_PadLeft: {
        std::string str = this->text;
        int64_t pad;
        char padChar = ' ';

        if (args.size() == 0 || args.size() > 2) {
            throw VMException(
                "String.PadLeft must only accept 1 or 2 arguments");
        }

        if (GetArgument(args, 0, pad)) {
            if ((size_t)pad < str.size()) {
                thrd->call_stack_entries.back()->Push(gc, str);
                return false;
            }
            GetArgument(args, 1, padChar);
            size_t diff = (size_t)pad - str.size();
            str.insert(str.begin(), diff, padChar);
            thrd->call_stack_entries.back()->Push(gc, str);
            return false;
        } else
            throw VMException("String.PadLeft must have a long for width");
    }
    case TSF_PadRight: {

        std::string str = this->text;
        int64_t pad;
        char padChar = ' ';

        if (args.size() == 0 || args.size() > 2) {
            throw VMException(
                "String.PadRight must only accept 1 or 2 arguments");
        }

        if (GetArgument(args, 0, pad)) {
            if ((size_t)pad < str.size()) {
                thrd->call_stack_entries.back()->Push(gc, str);
                return false;
            }
            GetArgument(args, 1, padChar);
            size_t diff = (size_t)pad - str.size();
            str.insert(str.end(), diff, padChar);
            thrd->call_stack_entries.back()->Push(gc, str);
            return false;
        } else
            throw VMException("String.PadRight must have a long for width");
    }
    case TSF_Contains: {
        std::string str = this->text;
        TString *str2;
        char c;
        if (GetArgumentHeap(args, 0, str2)) {

            int64_t index = 0;
            GetArgument(args, 1, index);

            auto res = str.find(str2->text, (std::size_t)index);

            thrd->call_stack_entries.back()->Push(gc, res != std::string::npos);
            return false;
        }

        else if (GetArgument<char>(args, 0, c)) {
            int64_t index = 0;
            GetArgument(args, 1, index);

            auto res = str.find_first_of(c, (std::size_t)index);

            thrd->call_stack_entries.back()->Push(gc, res != std::string::npos);
            return false;
        }
        thrd->call_stack_entries.back()->Push(gc, nullptr);
        return false;
    }
    case TSF_IndexOf: {
        std::string str = this->text;
        TString *str2;
        char c;
        if (GetArgumentHeap(args, 0, str2)) {

            int64_t index = 0;
            GetArgument(args, 1, index);

            auto res = str.find(str2->text, (std::size_t)index);
            if (res == std::string::npos)
                thrd->call_stack_entries.back()->Push(gc, (int64_t)-1);
            else
                thrd->call_stack_entries.back()->Push(gc, (int64_t)res);
            return false;
        }

        else if (GetArgument<char>(args, 0, c)) {
            int64_t index = 0;
            GetArgument(args, 1, index);

            auto res = str.find_first_of(c, (std::size_t)index);
            if (res == std::string::npos)
                thrd->call_stack_entries.back()->Push(gc, (int64_t)-1);
            else
                thrd->call_stack_entries.back()->Push(gc, (int64_t)res);
            return false;
        }
        thrd->call_stack_entries.back()->Push(gc, nullptr);
        return false;
    }
    case TSF_LastIndexOf: {
        std::string str = this->text;
        TString *str2;
        char c;
        if (GetArgument(args, 0, str2)) {

            int64_t index = str.size();
            GetArgument(args, 1, index);

            auto res = str.rfind(str2->text, (std::size_t)index);
            if (res == std::string::npos)
                thrd->call_stack_entries.back()->Push(gc, (int64_t)-1);
            else
                thrd->call_stack_entries.back()->Push(gc, (int64_t)res);
            return false;
        } else if (GetArgument<char>(args, 0, c)) {
            int64_t index = str.size();
            GetArgument(args, 1, index);

            auto res = str.find_last_of(c, (std::size_t)index);
            if (res == std::string::npos)
                thrd->call_stack_entries.back()->Push(gc, (int64_t)-1);
            else
                thrd->call_stack_entries.back()->Push(gc, (int64_t)res);
            return false;
        }
        thrd->call_stack_entries.back()->Push(gc, nullptr);
        return false;
    }
    case TSF_GetEnumerator: {
        GCList ls(gc);
        thrd->call_stack_entries.back()->Push(
            gc, ls.Create<TStringEnumerator>(this));
        return false;
    }
    case TSF_Substring: {

        std::string str = this->text;
        if (args.size() >= 1 && std::holds_alternative<int64_t>(args[0])) {
            size_t offset = (size_t)std::get<int64_t>(args[0]);
            size_t count = std::string::npos;
            if (args.size() == 2 && std::holds_alternative<int64_t>(args[1])) {
                count = (size_t)std::get<int64_t>(args[1]);
            }
            GCList ls(gc);
            thrd->call_stack_entries.back()->Push(
                gc, ls.Create<TString>(str.substr(offset, count)));
            return false;
        }

        thrd->call_stack_entries.back()->Push(gc, nullptr);
        return false;
    }
    case TSF_Remove: {

        std::string str = this->text;
        if (args.size() >= 1 && std::holds_alternative<int64_t>(args[0])) {
            size_t offset = (size_t)std::get<int64_t>(args[0]);
            size_t count = std::string::npos;
            if (args.size() == 2 && std::holds_alternative<int64_t>(args[1])) {
                count = (size_t)std::get<int64_t>(args[1]);
            }
            GCList ls(gc);
            thrd->call_stack_entries.back()->Push(
                gc, ls.Create<TString>(str.erase(offset, count)));
            return false;
        }
        thrd->call_stack_entries.back()->Push(gc, nullptr);
        return false;
    }
    case TSF_TrimStart: {

        std::string str = this->text;
        if (args.size() >= 0) {
            char c = (args.size() == 1 && std::holds_alternative<char>(args[0]))
                         ? std::get<char>(args[0])
                         : ' ';

            size_t i = 0;
            for (; i < str.size(); i++) {
                if (str[i] != c)
                    break;
            }
            GCList ls(gc);
            thrd->call_stack_entries.back()->Push(
                gc, ls.Create<TString>(
                        (!str.empty() && i < str.size()) ? str.substr(i) : ""));
            return false;
        }
        thrd->call_stack_entries.back()->Push(gc, nullptr);
        return false;
    }
    case TSF_TrimEnd: {

        std::string str = this->text;
        if (args.size() >= 0) {
            char c = (args.size() == 1 && std::holds_alternative<char>(args[0]))
                         ? std::get<char>(args[0])
                         : ' ';
            size_t i = str.size() - 1;
            for (; i >= 0; i--) {
                if (str[i] != c)
                    break;
            }
            GCList ls(gc);
            thrd->call_stack_entries.back()->Push(
                gc, ls.Create<TString>((!str.empty() && i < str.size())
                                           ? str.substr(0, i + 1)
                                           : ""));
            return false;
        }
        thrd->call_stack_entries.back()->Push(gc, nullptr);
        return false;
    }
    case TSF_StartsWith: {
        std::string str = this->text;
        TString *v;
        if (GetArgumentHeap(args, 0, v)) {
            if (str.size() < v->text.size()) {
                thrd->call_stack_entries.back()->Push(gc, false);
                return false;
            }
            for (size_t i = 0; i < v->text.size(); i++) {
                if (v->text[i] != str[i]) {
                    thrd->call_stack_entries.back()->Push(gc, false);
                    return false;
                }
            }
            thrd->call_stack_entries.back()->Push(gc, true);
            return false;
        }
        thrd->call_stack_entries.back()->Push(gc, false);
        return false;
    }
    case TSF_EndsWith: {
        std::string str = this->text;
        TString *v;
        if (GetArgumentHeap(args, 0, v)) {
            if (str.size() < v->text.size()) {
                thrd->call_stack_entries.back()->Push(gc, false);
                return false;
            }

            size_t _end = str.size() - v->text.size();
            for (size_t i = 0; i < v->text.size(); i++) {
                if (v->text[i] != str[i + _end]) {
                    thrd->call_stack_entries.back()->Push(gc, false);
                    return false;
                }
            }
            thrd->call_stack_entries.back()->Push(gc, true);
            return false;
        }
        thrd->call_stack_entries.back()->Push(gc, false);
        return false;
    }
    }
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