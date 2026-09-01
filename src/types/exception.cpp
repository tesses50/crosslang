#include "CrossLang.hpp"

namespace Tesses::CrossLang {

TException::TException(std::string message, std::string type,
                       TDictionary *extraFields, TObject innerException)
    : message(message), type(type), extraFields(extraFields),
      innerException(innerException) {}
TObject TException::CallMethod(GCList &ls, std::string key,
                               std::vector<TObject> args) {

    if (key == "getMessage" ||
        key == "getText") // Text is for compatibility reasons
        return this->message;
    if (key == "getInnerException")
        return this->innerException;
    if (key == "getType")
        return this->type;
    if (key == "getLine")
        return static_cast<int64_t>(this->line.value_or(0));
    if (key == "getOffset")
        return static_cast<int64_t>(this->offset.value_or(0));
    if (key == "getColumn")
        return static_cast<int64_t>(this->column.value_or(0));
    if (key == "getFileName")
        return this->filename.value_or("");

    if (key == "setLine") {
        int64_t v;
        if (!this->line && GetArgument(args, 0, v))
            this->line = static_cast<uint32_t>(v);

        return Undefined();
    }
    if (key == "setColumn") {
        int64_t v;
        if (!this->column && GetArgument(args, 0, v))
            this->column = static_cast<uint32_t>(v);

        return Undefined();
    }
    if (key == "setOffset") {
        int64_t v;
        if (!this->offset && GetArgument(args, 0, v))
            this->offset = static_cast<uint32_t>(v);

        return Undefined();
    }
    if (key == "setFileName") {
        std::string v;
        if (!this->filename && GetArgument(args, 0, v))
            this->filename = v;

        return Undefined();
    }

    if (key == "ToString") {
        std::string text;
        text.reserve(this->type.size() + this->message.size() + 3);
        text.append(this->type);
        text.append(": ");
        text.append(this->message);
        text.push_back('\n');
        if (this->filename) {
            auto &filename = *this->filename;
            text.reserve(text.size() + filename.size() + 4);
            text.append("At: ");
            text.append(filename);

            if (this->line) {
                text.push_back(':');
                text.append(std::to_string(*this->line));
                if (this->column) {

                    text.push_back(':');
                    text.append(std::to_string(*this->column));
                }
            }
            text.push_back('\n');

            if (!this->line && this->column) {
                text.append("Column: ");
                text.append(std::to_string(*this->column));
                text.push_back('\n');
            }
        } else {
            if (this->line) {
                text.append("Line: ");
                text.append(std::to_string(*this->line));
                text.push_back('\n');
            }
            if (this->column) {
                text.append("Column: ");
                text.append(std::to_string(*this->column));
                text.push_back('\n');
            }
        }

        if (this->offset) {
            text.append("Offset: ");
            text.append(std::to_string(*this->offset));
            text.push_back('\n');
        }
        if (!std::holds_alternative<Undefined>(this->innerException)) {
            text.append("InnerException:");

            std::string textInner = ToString(ls.GetGC(), this->innerException);

            for (auto item : Tesses::Framework::Http::HttpUtils::SplitString(
                     textInner, "\n")) {
                if (item.empty()) {
                    text.push_back('\n');
                    continue;
                }
                text.append("\n\t");
                text.append(item);
            }
        }
        return text;
    }
    if (extraFields != nullptr) {
        GCList ls2(ls.GetGC());
        if (extraFields->MethodExists(ls2, key)) {
            return extraFields->CallMethod(ls, key, args);
        }

        if (key.size() > 3 && key[0] == 'g' && key[1] == 'e' && key[2] == 't') {
            ls.GetGC()->BarrierBegin();
            auto val = extraFields->GetValue(key.substr(3));
            ls.GetGC()->BarrierEnd();
            return val;
        }
    }

    return Undefined();
}
std::string TException::TypeName() { return this->type; }
bool TException::ToBool() { return true; }
void TException::Mark() {
    if (marked)
        return;
    marked = true;

    if (this->extraFields)
        this->extraFields->Mark();

    GC::Mark(this->innerException);
}

TNativeException::TNativeException(const char *what, std::exception_ptr ptr)
    : what(what != nullptr ? what : ""), ptr(ptr) {}
TObject TNativeException::CallMethod(GCList &ls, std::string key,
                                     std::vector<TObject> args) {
    if (key == "getMessage" || key == "getText")
        return this->what;
    if (key == "getType")
        return "NativeException";

    if (key == "ToString") {
        return "NativeException: " + this->what;
    }
    return Undefined();
}
std::string TNativeException::TypeName() { return "NativeException"; }
std::string TNativeException::What() { return this->what; }
std::exception_ptr TNativeException::GetPointer() { return this->ptr; }
void TNativeException::ThrowIt() { std::rethrow_exception(this->ptr); }
} // namespace Tesses::CrossLang