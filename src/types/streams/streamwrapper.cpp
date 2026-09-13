#include "CrossLang.hpp"

namespace Tesses::CrossLang {
TObjectStreamWrapper::TObjectStreamWrapper(std::shared_ptr<TObjectStream> strm)
    : strm(strm) {}
TObject TObjectStreamWrapper::CallMethod(InterperterThread *thrd, GCList &ls,
                                         const std::string &name,
                                         const std::vector<TObject> &args) {
    if (Exists(name)) {
        return ITStream::CallMethod(thrd, ls, name, args);
    }

    TDictionary *dict;

    if (GetObjectHeap(strm->obj, dict)) {
        auto val = dict->GetValue(name);

        TCallable *call;
        if (GetObjectHeap(val, call)) {

            auto closure = dynamic_cast<TClosure *>(call);

            if (closure && !closure->closure->args.empty() &&
                closure->closure->args[0]->GetString() == "this") {
                std::vector<TObject> args_new;
                args_new.reserve(args.size() + 1);
                args_new.push_back(dict);
                args_new.insert(args_new.cend(), args.begin(), args.end());

                return closure->Call(ls, args_new);
            } else {
                return call->Call(ls, args);
            }
        }
        if (name.size() > 3 && name[1] == 'e' && name[2] == 't') {
            char c = name[0];
            if (c == 'g') {
                auto fieldName = name.substr(3);
                return dict->GetValue(fieldName);
            } else if (c == 's' && !args.empty()) {
                auto fieldName = name.substr(3);
                dict->SetValue(fieldName, args[0]);
                ls.Add(args[0]);
                return args[0];
            }
        }
    }

    return Undefined();
}
std::shared_ptr<Tesses::Framework::Streams::Stream>
TObjectStreamWrapper::GetStream() {
    return this->strm;
}
std::string TObjectStreamWrapper::TypeName() { return "CustomStream"; }
} // namespace Tesses::CrossLang