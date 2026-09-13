#include "CrossLang.hpp"

namespace Tesses::CrossLang {
TResource::TResource(std::vector<uint8_t> &&bytes) : bytes(std::move(bytes)) {}
std::pair<const uint8_t *, size_t> TResource::GetBounds() const {
    return std::pair<const uint8_t *, size_t>(bytes.data(), bytes.size());
}

std::string TResource::TypeName() { return "Resource"; }
} // namespace Tesses::CrossLang