#include "CrossLang.hpp"
namespace Tesses::CrossLang {
TDynamicList::TDynamicList(TCallable *callable) : cb(callable) {}
TDynamicList *TDynamicList::Create(GCList &ls, TCallable *callable) {
    return ls.Create<TDynamicList>(callable);
}
TDynamicList *TDynamicList::Create(GCList *ls, TCallable *callable) {
    return ls->Create<TDynamicList>(callable);
}

void TDynamicList::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    this->cb->Mark();
}

int64_t TDynamicList::Count(GCList &ls) {

    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", ls.FromString("Count"));
    ls.GetGC()->BarrierEnd();
    auto res = cb->Call(ls, {dict});
    int64_t n;
    if (GetObject(res, n))
        return n;
    return 0;
}
TObject TDynamicList::Add(GCList &ls, TObject v) {

    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", ls.FromString("Add"));
    dict->SetValue("Value", v);
    ls.GetGC()->BarrierEnd();
    return cb->Call(ls, {dict});
}
TObject TDynamicList::Insert(GCList &ls, int64_t index, TObject v) {

    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", ls.FromString("Insert"));
    dict->SetValue("Index", index);
    dict->SetValue("Value", v);
    ls.GetGC()->BarrierEnd();
    return cb->Call(ls, {dict});
}
TObject TDynamicList::Clear(GCList &ls) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", ls.FromString("Clear"));
    ls.GetGC()->BarrierEnd();
    return cb->Call(ls, {dict});
}
TObject TDynamicList::Remove(GCList &ls, TObject obj) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", ls.FromString("Remove"));
    dict->SetValue("Value", obj);
    ls.GetGC()->BarrierEnd();

    return cb->Call(ls, {dict});
}
TObject TDynamicList::RemoveAllEqual(GCList &ls, TObject obj) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", ls.FromString("RemoveAllEqual"));
    dict->SetValue("Value", obj);
    ls.GetGC()->BarrierEnd();

    return cb->Call(ls, {dict});
}
TObject TDynamicList::RemoveAt(GCList &ls, int64_t index) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", ls.FromString("RemoveAt"));
    dict->SetValue("Index", index);
    ls.GetGC()->BarrierEnd();

    return cb->Call(ls, {dict});
}
TObject TDynamicList::ToString(GCList &ls) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", ls.FromString("ToString"));
    ls.GetGC()->BarrierEnd();

    return cb->Call(ls, {dict});
}

TObject TDynamicList::GetAt(GCList &ls, int64_t index) {

    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", ls.FromString("GetAt"));
    dict->SetValue("Index", index);
    ls.GetGC()->BarrierEnd();
    return cb->Call(ls, {dict});
}

TObject TDynamicList::SetAt(GCList &ls, int64_t index, TObject val) {
    auto dict = TDictionary::Create(ls);
    ls.GetGC()->BarrierBegin();
    dict->SetValue("Type", ls.FromString("SetAt"));
    dict->SetValue("Index", index);
    dict->SetValue("Value", val);
    ls.GetGC()->BarrierEnd();
    return cb->Call(ls, {dict});
}

TDynamicList::~TDynamicList() {}

TByteArray *TByteArray::Create(GCList &ls) { return ls.Create<TByteArray>(); }

TByteArray *TByteArray::Create(GCList *ls) { return ls->Create<TByteArray>(); }

void TList::Add(TObject value) { this->items.push_back(value); }
void TList::Set(int64_t index, TObject value) {
    if (index >= 0 && index < this->Count()) {
        this->items[index] = value;
    }
}
TObject TList::Get(int64_t index) {
    if (index >= 0 && index < this->Count()) {
        return this->items[index];
    }
    return Undefined();
}
int64_t TList::Count() { return this->items.size(); }
void TList::Insert(int64_t index, TObject value) {
    if (index >= 0 && index <= this->Count()) {
        this->items.insert(this->items.begin() + index, value);
    }
}
void TList::RemoveAt(int64_t index) {
    if (index >= 0 && index < this->Count()) {
        this->items.erase(this->items.begin() + index);
    }
}
void TList::Clear() { this->items.clear(); }
void TList::Mark() {
    if (this->marked)
        return;
    this->marked = true;
    for (auto item : this->items) {
        GC::Mark(item);
    }
}
std::string TList::TypeName() { return "List"; }
bool TList::opGetField(InterperterThread *thrd, std::shared_ptr<GC> gc,
                       const std::string &name) {
    auto &cse = thrd->call_stack_entries;
    if (name == "Count" || name == "Length") {
        int64_t len = this->Count();
        if (len < 0)
            len = 0;

        cse.back()->Push(gc, len);
        return false;
    }
    cse.back()->Push(gc, Undefined());
    return false;
}
bool TList::opExecuteMethod(InterperterThread *thrd, std::shared_ptr<GC> gc,
                            const std::string &name,
                            const std::vector<TObject> &args) {

    auto &cse = thrd->call_stack_entries;
    GCList ls(gc);

    if (name == "GetEnumerator") {
        cse.back()->Push(gc, ls.Create<TListEnumerator>(this));
        return false;
    } else if (name == "ToString") {

        cse.back()->Push(gc, Json_Encode(this));
        return false;

    } else if (name == "Insert") {
        if (args.size() != 2) {
            throw VMException("List.Insert must only accept two arguments");
        }
        int64_t index;

        if (!GetArgument(args, 0, index)) {
            throw VMException("List.Insert first argument must be Long");
        }

        gc->BarrierBegin();
        this->Insert(index, args[1]);
        gc->BarrierEnd();
        cse.back()->Push(gc, Undefined());
        return false;
    } else if (name == "Add") {
        if (args.size() != 1) {
            throw VMException("List.Add must only accept one argument");
        }
        gc->BarrierBegin();
        this->Add(args[0]);
        gc->BarrierEnd();
        cse.back()->Push(gc, Undefined());
        return false;
    } else if (name == "Contains") {
        if (args.size() != 1) {
            throw VMException("List.Contains must only accept one argument");
        }
        gc->BarrierBegin();
        for (int64_t i = 0; i < this->Count(); i++) {
            auto item = this->Get(i);
            gc->BarrierEnd();
            if (Equals(gc, args[0], item)) {
                cse.back()->Push(gc, true);
                return false;
            }
            gc->BarrierBegin();
        }
        gc->BarrierEnd();
        cse.back()->Push(gc, false);
        return false;
    } else if (name == "IndexOf") {
        // IndexOf(obj, $idx)
        if (args.size() < 1 || args.size() > 2) {
            throw VMException("List.IndexOf must either have one "
                              "or two arguments");
        }

        int64_t i = 0;

        GetArgument(args, 1, i);
        gc->BarrierBegin();
        for (; i < this->Count(); i++) {
            auto item = this->Get(i);
            gc->BarrierEnd();
            if (Equals(gc, args[0], item)) {

                cse.back()->Push(gc, i);
                return false;
            }
            gc->BarrierBegin();
        }
        gc->BarrierEnd();
        cse.back()->Push(gc, (int64_t)-1);
        return false;
    } else if (name == "RemoveAllEqual") {
        if (args.size() != 1) {
            throw VMException("List.RemoveAllEqual must only "
                              "accept one argument");
        }

        gc->BarrierBegin();
        for (int64_t i = 0; i < this->Count(); i++) {
            auto item = this->Get(i);
            gc->BarrierEnd();
            if (Equals(gc, args[0], item)) {
                gc->BarrierBegin();
                this->RemoveAt(i);
                i--;
            } else
                gc->BarrierBegin();
        }
        gc->BarrierEnd();
        cse.back()->Push(gc, Undefined());
        return false;
    } else if (name == "Remove") {
        if (args.size() != 1) {
            throw VMException("List.Remove must only accept one argument");
        }

        gc->BarrierBegin();
        for (int64_t i = 0; i < this->Count(); i++) {
            auto item = this->Get(i);
            gc->BarrierEnd();
            if (Equals(gc, args[0], item)) {
                gc->BarrierBegin();
                this->RemoveAt(i);
                gc->BarrierEnd();
                break;
            }
            gc->BarrierBegin();
        }
        gc->BarrierEnd();
        cse.back()->Push(gc, Undefined());
        return false;
    } else if (name == "RemoveAt") {
        if (args.size() != 1) {
            throw VMException("List.RemoveAt must only accept one argument");
        }

        if (!std::holds_alternative<int64_t>(args[0])) {
            throw VMException("List.RemoveAt must only accept a long");
        }
        gc->BarrierBegin();
        this->RemoveAt(std::get<int64_t>(args[0]));
        gc->BarrierEnd();
        cse.back()->Push(gc, Undefined());
        return false;
    } else if (name == "Clear") {
        gc->BarrierBegin();
        this->Clear();
        gc->BarrierEnd();
        cse.back()->Push(gc, Undefined());
        return false;
    } else if (name == "GetAt") {
        if (args.size() != 1) {
            throw VMException("List.GetAt must only accept one argument");
        }

        if (!std::holds_alternative<int64_t>(args[0])) {
            throw VMException("List.GetAt must only accept a long");
        }

        int64_t index = std::get<int64_t>(args[0]);
        if (index >= 0 && index < this->Count()) {
            cse.back()->Push(gc, this->Get(index));
            return false;
        }

    } else if (name == "SetAt") {
        if (args.size() != 2) {
            throw VMException("List.SetAt must only accept two arguments");
        }

        if (!std::holds_alternative<int64_t>(args[0])) {
            throw VMException("List.SetAt first argument must only "
                              "accept a long");
        }

        int64_t index = std::get<int64_t>(args[0]);
        if (index >= 0 && index < this->Count()) {
            this->Set(index, args[1]);
            return false;
        }

    }

    else if (name == "Count" || name == "Length") {
        gc->BarrierBegin();
        cse.back()->Push(gc, this->Count());
        gc->BarrierEnd();
        return false;
    }
    cse.back()->Push(gc, Undefined());
    return false;
}

}; // namespace Tesses::CrossLang