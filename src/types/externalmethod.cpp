#include "CrossLang.hpp"
namespace Tesses::CrossLang {
TExternalMethod::TExternalMethod(
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
    std::string documentation, std::vector<std::string> argNames,
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
    auto gc = ls.GetGC();
    TExternalMethod *method =
        new TExternalMethod(cb, documentation, argNames, destroy);
    ls.Add(method);
    gc->Watch(method);
    return method;
}
TExternalMethod *TExternalMethod::Create(
    GCList *ls, std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb,
    std::function<void()> destroy) {
    auto gc = ls->GetGC();
    TExternalMethod *method =
        new TExternalMethod(cb, documentation, argNames, destroy);
    ls->Add(method);
    gc->Watch(method);
    return method;
}
TExternalMethod *TExternalMethod::Create(
    GCList &ls, std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb) {
    auto gc = ls.GetGC();
    TExternalMethod *method =
        new TExternalMethod(cb, documentation, argNames, []() -> void {});
    ls.Add(method);
    gc->Watch(method);
    return method;
}
TExternalMethod *TExternalMethod::Create(
    GCList *ls, std::string documentation, std::vector<std::string> argNames,
    std::function<TObject(GCList &ls, std::vector<TObject> args)> cb) {
    auto gc = ls->GetGC();
    TExternalMethod *method =
        new TExternalMethod(cb, documentation, argNames, []() -> void {});
    ls->Add(method);
    gc->Watch(method);
    return method;
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