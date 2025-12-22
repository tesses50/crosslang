What is TObject
===============

It is a std::variant that can be (not in that order)

- Undefined (std::monostate)
- Null (std::nullptr_t)
- Long (int64_t)
- Double (double)
- Char (char)
- String (std::string)
- Version (Tesses::CrossLang::TVMVersion)
- Regex (std::regex)
- Path (Tesses::Framework::Filesystem::VFSPath)
- MethodInvoker (Tesses::CrossLang::MethodInvoker)
- HeapObjects (Tesses::CrossLang::THeapObjectHolder)