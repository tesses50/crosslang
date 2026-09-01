#include "CrossLang.hpp"

namespace Tesses::CrossLang {
TAny *TAny::Create(GCList &ls) { return ls.Create<TAny>(); }
TAny *TAny::Create(GCList *ls) { return ls->Create<TAny>(); }
void TAny::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    GC::Mark(this->other);
}
} // namespace Tesses::CrossLang