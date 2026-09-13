#include "CrossLang.hpp"

namespace Tesses::CrossLang {
TMemoryStream::TMemoryStream(
    std::shared_ptr<Tesses::Framework::Streams::MemoryStream> strm)
    : strm(strm) {}
std::shared_ptr<Tesses::Framework::Streams::Stream> TMemoryStream::GetStream() {
    return this->strm;
}
TObject TMemoryStream::CallMethod(InterperterThread *thrd, GCList &ls,
                                  const std::string &name,
                                  const std::vector<TObject> &args) {
    if (name == "GetBytes") {
        return ls.Create<TMemoryStreamMutByteView>(strm);
    }
    return ITStream::CallMethod(thrd, ls, name, args);
}
TMemoryStreamMutByteView::TMemoryStreamMutByteView(
    std::shared_ptr<Tesses::Framework::Streams::MemoryStream> strm)
    : strm(strm) {}
std::pair<uint8_t *, size_t> TMemoryStreamMutByteView::GetMutableBounds() {
    if (!strm)
        return std::pair<uint8_t *, size_t>(nullptr, 0);
    auto &buff = strm->GetBuffer();
    return std::pair<uint8_t *, size_t>(buff.data(), buff.size());
}

std::string TMemoryStream::TypeName() { return "MemoryStream"; }

} // namespace Tesses::CrossLang