#include "CrossLang.hpp"
namespace Tesses::CrossLang {
TExternalMethod::TExternalMethod(
    std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
    std::function<void()> destroy) {

    this->cb = cb;
    this->args = argNames;
    this->documentation = documentation;
    this->destroy = destroy;
}
TExternalMethod *TExternalMethod::Create(
    GCList &ls, std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
    std::function<void()> destroy) {
    return ls.Create<TExternalMethod>(documentation, argNames, cb, destroy);
}
TExternalMethod *TExternalMethod::Create(
    GCList *ls, std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
    std::function<void()> destroy) {
    return ls->Create<TExternalMethod>(documentation, argNames, cb, destroy);
}
TExternalMethod *TExternalMethod::Create(
    GCList &ls, std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb) {
    return ls.Create<TExternalMethod>(documentation, argNames, cb);
}
TExternalMethod *TExternalMethod::Create(
    GCList *ls, std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb) {

    return ls->Create<TExternalMethod>(documentation, argNames, cb);
}
TObject TExternalMethod::Call(GCList &ls, std::vector<TObject> args) {
    if (cb == nullptr)
        return Undefined();
    return this->cb(ls, args);
}
TExternalMethod::~TExternalMethod() {
    if (this->destroy != nullptr)
        this->destroy();
}
} // namespace Tesses::CrossLang