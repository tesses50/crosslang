#include "CrossLang.hpp"
#include "TessesFramework/Serialization/BitConverter.hpp"
#include "TessesFramework/Streams/ByteReader.hpp"
#include "TessesFramework/Streams/ByteWriter.hpp"
#include "TessesFramework/Uuid.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <variant>
namespace Tesses::CrossLang {
extern bool
IHttpServer_Handle(std::shared_ptr<Tesses::Framework::Http::IHttpServer> svr,
                   std::vector<TObject> &args);

bool InterperterThread::ExecuteMethod2(std::shared_ptr<GC> gc, TObject instance,
                                       std::string key,
                                       std::vector<TObject> args) {
    std::vector<CallStackEntry *> &cse = this->call_stack_entries;
    if (!cse.empty()) {
        GCList ls(gc);
        std::regex regex;
        TVMVersion version;
        if (std::holds_alternative<std::nullptr_t>(instance)) {
            if (key == "ToString") {
                cse.back()->Push(gc, "null");
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<Undefined>(instance)) {
            if (key == "ToString") {
                cse.back()->Push(gc, "undefined");
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (GetObject(instance, version)) {
            if (key == "ToString") {
                cse.back()->Push(gc, version.ToString());
                return false;
            }
            if (key == "ToByteArray") {
                TByteArray *ba = TByteArray::Create(ls);
                ba->data.resize(5);
                version.ToArray(ba->data.data());
                cse.back()->Push(gc, ba);
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (GetObject(instance, regex)) {
            if (key == "Search") {
                std::string str;
                if (GetArgument(args, 0, str)) {
                    std::smatch m;
                    if (std::regex_search(str, m, regex)) {
                        auto myLs = TList::Create(ls);
                        gc->BarrierBegin();
                        for (auto item : m) {
                            auto itm = TDictionary::Create(ls);
                            itm->SetValue("Offset",
                                          (int64_t)(item.first - str.begin()));
                            itm->SetValue("Length", (int64_t)item.length());
                            itm->SetValue("Matched", item.matched);
                            itm->SetValue("Text", item.str());
                            myLs->Add(itm);
                        }

                        cse.back()->Push(gc, myLs);
                        gc->BarrierEnd();
                        return false;
                    }
                }
            }

            cse.back()->Push(gc, nullptr);
            return false;
        } else if (std::holds_alternative<bool>(instance)) {
            bool flag = std::get<bool>(instance);
            if (key == "ToString") {
                cse.back()->Push(gc, flag ? "true" : "false");
                return false;
            } else if (key == "ToLong") {
                cse.back()->Push(gc, flag ? (int64_t)1 : (int64_t)0);
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<char>(instance)) {
            char c = std::get<char>(instance);
            if (key == "ToString") {
                cse.back()->Push(gc, std::string{c});
                return false;
            } else if (key == "ToLong") {
                uint8_t uc = (uint8_t)c;
                cse.back()->Push(gc, (int64_t)uc);
                return false;
            } else if (key == "IsAscii") {
                bool isAscii = c >= 0;
                cse.back()->Push(gc, isAscii);
                return false;
            }
            if (key == "IsLetter") {
                bool isLetter =
                    (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
                cse.back()->Push(gc, isLetter);
                return false;
            }
            if (key == "IsDigit") {
                bool isDigit = (c >= '0' && c <= '9');
                cse.back()->Push(gc, isDigit);
                return false;
            }
            if (key == "IsLetterOrDigit") {
                bool isDigit = (c >= 'A' && c <= 'Z') ||
                               (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
                cse.back()->Push(gc, isDigit);
                return false;
            }
            if (key == "ToLower") {
                if (c >= 'A' && c <= 'Z')
                    cse.back()->Push(gc, (char)tolower(c));
                else
                    cse.back()->Push(gc, c);

                return false;
            }
            if (key == "ToUpper") {
                if (c >= 'a' && c <= 'z')
                    cse.back()->Push(gc, (char)toupper(c));
                else
                    cse.back()->Push(gc, c);

                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<double>(instance)) {
            double number = std::get<double>(instance);
            if (key == "ToString") {
                cse.back()->Push(gc, std::to_string(number));
                return false;
            }
            if (key == "ToLong") {
                cse.back()->Push(gc, (int64_t)number);
                return false;
            }
            if (key == "ToLongBits") {
                cse.back()->Push(gc, *(int64_t *)&number);
                return false;
            }
            if (key == "Floor") {
                cse.back()->Push(gc, floor(number));
                return false;
            }
            if (key == "Ceiling") {
                cse.back()->Push(gc, ceil(number));
                return false;
            }
            if (key == "Abs") {
                cse.back()->Push(gc, fabs(number));
                return false;
            }
            if (key == "Pow") {
                if (args.size() != 1) {
                    throw VMException(
                        "Double.Pow must only accept one argument");
                }
                if (!std::holds_alternative<double>(args[0])) {
                    throw VMException("Double.Pow must only accept a double");
                }
                cse.back()->Push(gc, pow(number, std::get<double>(args[0])));
                return false;
            }
            if (key == "Atan2") {
                if (args.size() != 1) {
                    throw VMException(
                        "Double.Atan2 must only accept one argument");
                }
                if (!std::holds_alternative<double>(args[0])) {
                    throw VMException("Double.Atan2 must only accept a double");
                }
                cse.back()->Push(gc, atan2(number, std::get<double>(args[0])));
                return false;
            }
            if (key == "Atan") {
                cse.back()->Push(gc, tan(number));
                return false;
            }
            if (key == "Tan") {
                cse.back()->Push(gc, tan(number));
                return false;
            }
            if (key == "Tanh") {
                cse.back()->Push(gc, tanh(number));
                return false;
            }
            if (key == "Asin") {
                cse.back()->Push(gc, asin(number));
                return false;
            }
            if (key == "Sin") {
                cse.back()->Push(gc, sin(number));
                return false;
            }
            if (key == "Sinh") {
                cse.back()->Push(gc, sinh(number));
                return false;
            }
            if (key == "Acos") {
                cse.back()->Push(gc, acos(number));
                return false;
            }
            if (key == "Cos") {
                cse.back()->Push(gc, cos(number));
                return false;
            }
            if (key == "Cosh") {
                cse.back()->Push(gc, cosh(number));
                return false;
            }
            if (key == "Log") {
                cse.back()->Push(gc, log(number));
                return false;
            }
            if (key == "Log10") {
                cse.back()->Push(gc, log10(number));
                return false;
            }
            if (key == "Sqrt") {
                cse.back()->Push(gc, sqrt(number));
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<int64_t>(instance)) {
            int64_t number = std::get<int64_t>(instance);
            if (key == "ToHexString") {
                int64_t width = 1;

                if (!GetArgument(args, 0, width))
                    width = 1;
                size_t _width = (size_t)width;

                std::stringstream strm;
                if (_width > 1)
                    strm << std::setfill('0') << std::setw((int)_width)
                         << std::hex << number;
                else
                    strm << std::hex << number;
                cse.back()->Push(gc, strm.str());
                return false;
            }
            if (key == "ToString") {
                cse.back()->Push(gc, std::to_string(number));
                return false;
            }
            if (key == "ToChar") {
                uint8_t c = (uint8_t)number;
                cse.back()->Push(gc, (char)c);
                return false;
            }
            if (key == "Abs") {
                if (number < 0)
                    cse.back()->Push(gc, -number);
                else
                    cse.back()->Push(gc, number);
                return false;
            }
            if (key == "ToDouble") {
                cse.back()->Push(gc, (double)number);
                return false;
            }
            if (key == "ToDoubleBits") {
                cse.back()->Push(gc, *(double *)&number);
                return false;
            }

            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<
                       Tesses::Framework::Filesystem::VFSPath>(instance)) {
            auto path =
                std::get<Tesses::Framework::Filesystem::VFSPath>(instance);
            if (key == "GetEnumerator") {

                TList *_ls = TList::Create(ls);
                for (auto item : path.path) {
                    _ls->Add(item);
                }

                cse.back()->Push(gc, TListEnumerator::Create(ls, _ls));
                return false;
            }
            if (key == "ToString") {
                cse.back()->Push(gc, path.ToString());
                return false;
            }
            if (key == "GetAt") {
                int64_t index;
                if (GetArgument(args, 0, index)) {
                    size_t idx = (size_t)idx;
                    if (idx < path.path.size())
                        cse.back()->Push(gc, path.path[idx]);
                    else
                        cse.back()->Push(gc, nullptr);
                    return false;
                }
            }
            if (key == "Count" || key == "Length") {
                cse.back()->Push(gc, (int64_t)path.path.size());
                return false;
            }
            if (key == "GetParent") {
                cse.back()->Push(gc, path.GetParent());
                return false;
            }
            if (key == "GetFileName") {
                cse.back()->Push(gc, path.GetFileName());
                return false;
            }
            if (key == "GetExtension") {
                cse.back()->Push(gc, path.GetExtension());
                return false;
            }
            if (cse.back()->env->GetRootEnvironment()->permissions.localfs &&
                key == "MakeAbsolute") {
                Tesses::Framework::Filesystem::VFSPath p;
                if (GetArgumentAsPath(args, 0, p)) {
                    cse.back()->Push(gc, path.MakeAbsolute(p));
                    return false;
                } else {
                    cse.back()->Push(
                        gc, path.MakeAbsolute(
                                cse.back()
                                    ->env->GetRootEnvironment()
                                    ->permissions.localfs->GetWorking()));
                    return false;
                }
            }
            if (cse.back()->env->GetRootEnvironment()->permissions.localfs &&
                key == "MakeRelative") {
                Tesses::Framework::Filesystem::VFSPath p;
                if (GetArgumentAsPath(args, 0, p)) {
                    cse.back()->Push(gc, path.MakeRelative(p));
                    return false;
                } else {
                    cse.back()->Push(
                        gc, path.MakeRelative(
                                cse.back()
                                    ->env->GetRootEnvironment()
                                    ->permissions.localfs->GetWorking()));
                    return false;
                }
            }
            if (key == "ChangeExtension") {
                Tesses::Framework::Filesystem::VFSPath newPath = path;
                std::string ext;
                if (GetArgument(args, 0, ext)) {
                    newPath.ChangeExtension(ext);
                } else {
                    newPath.RemoveExtension();
                }
                cse.back()->Push(gc, newPath);
                return false;
            }
            if (key == "CollapseRelativeParents") {
                cse.back()->Push(gc, path.CollapseRelativeParents());
                return false;
            }
            if (key == "IsRelative") {
                cse.back()->Push(gc, path.relative);
                return false;
                // Path.FromString();
            }
        } else if (std::holds_alternative<std::string>(instance)) {
            std::string str = std::get<std::string>(instance);
            if (key == "ToLower") {
                for (size_t i = 0; i < str.size(); i++) {
                    if (str[i] >= 'A' && str[i] <= 'Z') {
                        str[i] = 'a' + (str[i] - 'A');
                    }
                }
                cse.back()->Push(gc, str);
                return false;
            }
            if (key == "ToUpper") {

                for (size_t i = 0; i < str.size(); i++) {
                    if (str[i] >= 'a' && str[i] <= 'z') {
                        str[i] = 'A' + (str[i] - 'a');
                    }
                }
                cse.back()->Push(gc, str);
                return false;
            }
            if (key == "PadLeft") {
                int64_t pad;
                char padChar = ' ';

                if (args.size() == 0 || args.size() > 2) {
                    throw VMException(
                        "String.PadLeft must only accept 1 or 2 arguments");
                }

                if (GetArgument(args, 0, pad)) {
                    if ((size_t)pad < str.size()) {
                        cse.back()->Push(gc, str);
                        return false;
                    }
                    GetArgument(args, 1, padChar);
                    size_t diff = (size_t)pad - str.size();
                    str.insert(str.begin(), diff, padChar);
                    cse.back()->Push(gc, str);
                    return false;
                } else
                    throw VMException(
                        "String.PadLeft must have a long for width");
            }
            if (key == "PadRight") {
                int64_t pad;
                char padChar = ' ';

                if (args.size() == 0 || args.size() > 2) {
                    throw VMException(
                        "String.PadRight must only accept 1 or 2 arguments");
                }

                if (GetArgument(args, 0, pad)) {
                    if ((size_t)pad < str.size()) {
                        cse.back()->Push(gc, str);
                        return false;
                    }
                    GetArgument(args, 1, padChar);
                    size_t diff = (size_t)pad - str.size();
                    str.insert(str.end(), diff, padChar);
                    cse.back()->Push(gc, str);
                    return false;
                } else
                    throw VMException(
                        "String.PadRight must have a long for width");
            }
            if (key == "Contains") {
                std::string str2;
                char c;
                if (GetArgument(args, 0, str2)) {

                    int64_t index = 0;
                    GetArgument(args, 1, index);

                    auto res = str.find(str2, (std::size_t)index);

                    cse.back()->Push(gc, res != std::string::npos);
                    return false;
                }

                else if (GetArgument<char>(args, 0, c)) {
                    int64_t index = 0;
                    GetArgument(args, 1, index);

                    auto res = str.find_first_of(c, (std::size_t)index);

                    cse.back()->Push(gc, res != std::string::npos);
                    return false;
                }
                cse.back()->Push(gc, nullptr);
                return false;
            }
            if (key == "IndexOf") {
                std::string str2;
                char c;
                if (GetArgument(args, 0, str2)) {

                    int64_t index = 0;
                    GetArgument(args, 1, index);

                    auto res = str.find(str2, (std::size_t)index);
                    if (res == std::string::npos)
                        cse.back()->Push(gc, (int64_t)-1);
                    else
                        cse.back()->Push(gc, (int64_t)res);
                    return false;
                }

                else if (GetArgument<char>(args, 0, c)) {
                    int64_t index = 0;
                    GetArgument(args, 1, index);

                    auto res = str.find_first_of(c, (std::size_t)index);
                    if (res == std::string::npos)
                        cse.back()->Push(gc, (int64_t)-1);
                    else
                        cse.back()->Push(gc, (int64_t)res);
                    return false;
                }
                cse.back()->Push(gc, nullptr);
                return false;
            }
            if (key == "LastIndexOf") {
                std::string str2;
                char c;
                if (GetArgument(args, 0, str2)) {

                    int64_t index = str.size();
                    GetArgument(args, 1, index);

                    auto res = str.rfind(str2, (std::size_t)index);
                    if (res == std::string::npos)
                        cse.back()->Push(gc, (int64_t)-1);
                    else
                        cse.back()->Push(gc, (int64_t)res);
                    return false;
                } else if (GetArgument<char>(args, 0, c)) {
                    int64_t index = str.size();
                    GetArgument(args, 1, index);

                    auto res = str.find_last_of(c, (std::size_t)index);
                    if (res == std::string::npos)
                        cse.back()->Push(gc, (int64_t)-1);
                    else
                        cse.back()->Push(gc, (int64_t)res);
                    return false;
                }
                cse.back()->Push(gc, nullptr);
                return false;
            }
            if (key == "GetEnumerator") {
                cse.back()->Push(gc, TStringEnumerator::Create(ls, str));
                return false;
            }
            if (key == "Substring") {
                if (args.size() >= 1 &&
                    std::holds_alternative<int64_t>(args[0])) {
                    size_t offset = (size_t)std::get<int64_t>(args[0]);
                    size_t count = std::string::npos;
                    if (args.size() == 2 &&
                        std::holds_alternative<int64_t>(args[1])) {
                        count = (size_t)std::get<int64_t>(args[1]);
                    }
                    cse.back()->Push(gc, str.substr(offset, count));
                    return false;
                }
            }
            if (key == "Remove") {
                if (args.size() >= 1 &&
                    std::holds_alternative<int64_t>(args[0])) {
                    size_t offset = (size_t)std::get<int64_t>(args[0]);
                    size_t count = std::string::npos;
                    if (args.size() == 2 &&
                        std::holds_alternative<int64_t>(args[1])) {
                        count = (size_t)std::get<int64_t>(args[1]);
                    }
                    cse.back()->Push(gc, str.erase(offset, count));
                    return false;
                }
            }
            if (key == "TrimStart") {
                if (args.size() >= 0) {
                    char c = (args.size() == 1 &&
                              std::holds_alternative<char>(args[0]))
                                 ? std::get<char>(args[0])
                                 : ' ';

                    size_t i = 0;
                    for (; i < str.size(); i++) {
                        if (str[i] != c)
                            break;
                    }

                    cse.back()->Push(gc, (!str.empty() && i < str.size())
                                             ? str.substr(i)
                                             : "");
                    return false;
                }
            }
            if (key == "TrimEnd") {
                if (args.size() >= 0) {
                    char c = (args.size() == 1 &&
                              std::holds_alternative<char>(args[0]))
                                 ? std::get<char>(args[0])
                                 : ' ';
                    size_t i = str.size() - 1;
                    for (; i >= 0; i--) {
                        if (str[i] != c)
                            break;
                    }

                    cse.back()->Push(gc, (!str.empty() && i < str.size())
                                             ? str.substr(0, i + 1)
                                             : "");
                    return false;
                }
            }
            if (key == "EndsWith") {
                std::string v;
                if (GetArgument(args, 0, v)) {
                    if (str.size() < v.size()) {
                        cse.back()->Push(gc, false);
                        return false;
                    }

                    size_t _end = str.size() - v.size();
                    for (size_t i = 0; i < v.size(); i++) {
                        if (v[i] != str[i + _end]) {
                            cse.back()->Push(gc, false);
                            return false;
                        }
                    }
                    cse.back()->Push(gc, true);
                    return false;
                }
                cse.back()->Push(gc, false);
                return false;
            }
            if (key == "StartsWith") {
                std::string v;
                if (GetArgument(args, 0, v)) {
                    if (str.size() < v.size()) {
                        cse.back()->Push(gc, false);
                        return false;
                    }
                    for (size_t i = 0; i < v.size(); i++) {
                        if (v[i] != str[i]) {
                            cse.back()->Push(gc, false);
                            return false;
                        }
                    }
                    cse.back()->Push(gc, true);
                    return false;
                }
                cse.back()->Push(gc, false);
                return false;
            }
            if (key == "Escape") {
                bool quote;
                if (!GetArgument(args, 0, quote))
                    quote = false;
                cse.back()->Push(gc, EscapeString(str, quote));
                return false;
            }
            if (key == "Replace") {
                std::string oldStr;
                std::string newStr;

                std::string _str = {};
                if (GetArgument(args, 0, oldStr) &&
                    GetArgument(args, 1, newStr)) {
                    _str = Tesses::Framework::Http::HttpUtils::Replace(
                        str, oldStr, newStr);
                }
                cse.back()->Push(gc, _str);

                return false;
            }
            if (key == "Split") {
                std::string delimiter;
                bool removeEmpty = false;
                size_t count = std::string::npos;

                if (args.size() < 1 || args.size() > 3)
                    throw VMException(
                        "String.Split must only accept 1-3 arguments");
                if (!std::holds_alternative<std::string>(args[0]))
                    throw VMException(
                        "String.Split first arg must be a string");
                else
                    delimiter = std::get<std::string>(args[0]);

                if (args.size() > 1 && !std::holds_alternative<bool>(args[1]))
                    throw VMException("String.Split second arg must be a bool");
                else if (args.size() > 1)
                    removeEmpty = std::get<bool>(args[1]);
                if (args.size() > 2 &&
                    !std::holds_alternative<int64_t>(args[2]))
                    throw VMException(
                        "String.Split third arg must be a int64_t");
                else if (args.size() > 2)
                    count = (size_t)std::get<int64_t>(args[2]);

                auto res = Tesses::Framework::Http::HttpUtils::SplitString(
                    str, delimiter, count);
                TList *mls = TList::Create(ls);
                for (auto item : res) {
                    if (!removeEmpty || !item.empty())
                        mls->Add(item);
                }
                cse.back()->Push(gc, mls);
                return false;
                // SplitString()
            }
            if (key == "GetAt") {
                if (args.size() != 1) {
                    throw VMException(
                        "String.GetAt must only accept one argument");
                }

                if (!std::holds_alternative<int64_t>(args[0])) {
                    throw VMException("String.GetAt must only accept a long");
                }

                size_t index = (size_t)std::get<int64_t>(args[0]);
                size_t sz = str.size();
                if (index >= 0 && index < sz) {
                    cse.back()->Push(gc, str[index]);
                    return false;
                }
            }
            if (key == "Count" || key == "Length") {
                int64_t len = (int64_t)str.size();
                if (len < 0)
                    len = 0;

                cse.back()->Push(gc, len);
                return false;
            }
            if (key == "ToString") {
                cse.back()->Push(gc, str);
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<
                       std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(
                       instance)) {
            auto &time =
                std::get<std::shared_ptr<Tesses::Framework::Date::TimeSpan>>(
                    instance);
            if (key == "ToString") {
                bool slim = false;
                GetArgument(args, 0, slim);

                cse.back()->Push(gc, time->ToString(slim));

                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<
                       std::shared_ptr<Tesses::Framework::Date::DateTime>>(
                       instance)) {
            auto &date =
                std::get<std::shared_ptr<Tesses::Framework::Date::DateTime>>(
                    instance);

            if (key == "ToString") {
                std::string fmt;
                if (GetArgument(args, 0, fmt)) {
                    cse.back()->Push(gc, date->ToString(fmt));
                } else {
                    cse.back()->Push(gc, date->ToString());
                }
                return false;
            }
            if (key == "ToHttpDate") {
                cse.back()->Push(gc, date->ToHttpDate());
                return false;
            }
            if (key == "ToEpoch") {
                cse.back()->Push(gc, date->ToEpoch());
                return false;
            }
            if (key == "ToLocal") {
                cse.back()->Push(
                    gc, std::make_shared<Tesses::Framework::Date::DateTime>(
                            date->ToLocal()));
                return false;
            }
            if (key == "ToUTC") {
                cse.back()->Push(
                    gc, std::make_shared<Tesses::Framework::Date::DateTime>(
                            date->ToUTC()));
                return false;
            }
            cse.back()->Push(gc, nullptr);
            return false;

        } else if (std::holds_alternative<std::shared_ptr<
                       Tesses::Framework::TextStreams::TextReader>>(instance)) {
            auto textReader = std::get<
                std::shared_ptr<Tesses::Framework::TextStreams::TextReader>>(
                instance);
            if (key == "Rewind") {
                cse.back()->Push(gc, textReader->Rewind());
                return false;
            }
            if (key == "ReadBlock") {
                int64_t sz;
                if (GetArgument(args, 0, sz)) {
                    std::string block;
                    if (textReader->ReadBlock(block, (size_t)sz)) {
                        cse.back()->Push(gc, block);
                        return false;
                    }
                }
                cse.back()->Push(gc, nullptr);
                return false;
            }
            if (key == "ReadChar") {
                cse.back()->Push(gc, (int64_t)textReader->ReadChar());
                return false;
            }
            if (key == "ReadLine") {
                std::string line;
                if (textReader->ReadLine(line)) {
                    cse.back()->Push(gc, line);
                    return false;
                }
                cse.back()->Push(gc, nullptr);
                return false;
            }
            if (key == "ReadLineHttp") {
                std::string line;
                if (textReader->ReadLineHttp(line)) {
                    cse.back()->Push(gc, line);
                    return false;
                }
                cse.back()->Push(gc, nullptr);
                return false;
            }
            if (key == "ReadAllLines") {
                std::vector<std::string> lines;
                textReader->ReadAllLines(lines);
                gc->BarrierBegin();
                TList *list = TList::Create(ls);
                for (auto &item : lines)
                    list->Add(item);
                gc->BarrierEnd();
                return list;
            }
            if (key == "ReadToEnd") {
                std::string text;
                textReader->ReadToEnd(text);
                cse.back()->Push(gc, text);
                return false;
            }
            if (key == "CopyTo") {
                std::shared_ptr<Tesses::Framework::TextStreams::TextWriter>
                    writer;
                if (GetArgument(args, 0, writer)) {
                    textReader->CopyTo(*writer);
                }
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<std::shared_ptr<
                       Tesses::Framework::TextStreams::TextWriter>>(instance)) {
            auto textWriter = std::get<
                std::shared_ptr<Tesses::Framework::TextStreams::TextWriter>>(
                instance);
            if (key == "Write") {
                if (args.size() > 0)
                    textWriter->Write(ToString(gc, args[0]));
            }
            if (key == "WriteLine") {
                if (args.size() > 0)
                    textWriter->WriteLine(ToString(gc, args[0]));
            }
            if (key == "WriteData") {
                if (args.size() > 0) {
                    auto s = ToString(gc, args[0]);
                    textWriter->WriteData(s.c_str(), s.size());
                }
            }
            cse.back()->Push(gc, nullptr);
            return false;
        } else if (std::holds_alternative<
                       std::shared_ptr<Tesses::Framework::Streams::ByteReader>>(
                       instance)) {
            auto &br = std::get<
                std::shared_ptr<Tesses::Framework::Streams::ByteReader>>(
                instance);
            if (key == "ReadS8") {
                cse.back()->Push(gc, (int64_t)br->ReadI8());
                return false;
            }
            if (key == "ReadU8") {
                cse.back()->Push(gc, (int64_t)br->ReadU8());
                return false;
            }
            if (key == "ReadS16BE") {
                cse.back()->Push(gc, (int64_t)br->ReadI16BE());
                return false;
            }
            if (key == "ReadS16LE") {
                cse.back()->Push(gc, (int64_t)br->ReadI16LE());
                return false;
            }
            if (key == "ReadU16BE") {
                cse.back()->Push(gc, (int64_t)br->ReadU16BE());
                return false;
            }
            if (key == "ReadU16LE") {
                cse.back()->Push(gc, (int64_t)br->ReadU16LE());
                return false;
            }
            if (key == "ReadS32BE") {
                cse.back()->Push(gc, (int64_t)br->ReadI32BE());
                return false;
            }
            if (key == "ReadS32LE") {
                cse.back()->Push(gc, (int64_t)br->ReadI32LE());
                return false;
            }
            if (key == "ReadU32BE") {
                cse.back()->Push(gc, (int64_t)br->ReadU32BE());
                return false;
            }
            if (key == "ReadU32LE") {
                cse.back()->Push(gc, (int64_t)br->ReadU32LE());
                return false;
            }
            if (key == "ReadS64BE") {
                cse.back()->Push(gc, br->ReadI64BE());
                return false;
            }
            if (key == "ReadS64LE") {
                cse.back()->Push(gc, br->ReadI64LE());
                return false;
            }
            if (key == "ReadU16BE") {
                uint64_t value = br->ReadU64BE();
                int64_t value2 = 0;
                memcpy(&value2, &value, sizeof(uint64_t));
                cse.back()->Push(gc, value2);
                return false;
            }
            if (key == "ReadU64LE") {
                uint64_t value = br->ReadU64LE();
                int64_t value2 = 0;
                memcpy(&value2, &value, sizeof(uint64_t));
                cse.back()->Push(gc, value2);
                return false;
            }

            if (key == "ReadF32BE") {
                cse.back()->Push(gc, (double)br->ReadF32BE());
                return false;
            }
            if (key == "ReadF32LE") {
                cse.back()->Push(gc, (double)br->ReadF32LE());
                return false;
            }

            if (key == "ReadF64BE") {
                cse.back()->Push(gc, br->ReadF64BE());
                return false;
            }
            if (key == "ReadF64LE") {
                cse.back()->Push(gc, br->ReadF64LE());
                return false;
            }

            if (key == "ReadUuid") {
                cse.back()->Push(gc, br->ReadUuid());
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<
                       std::shared_ptr<Tesses::Framework::Streams::ByteWriter>>(
                       instance)) {
            auto &bw = std::get<
                std::shared_ptr<Tesses::Framework::Streams::ByteWriter>>(
                instance);
            int64_t number;

            double numberDbl;

            Tesses::Framework::Uuid uuid;
            if (GetArgument(args, 0, number)) {
                if (key == "WriteS8") {
                    bw->WriteI8((int8_t)number);
                } else if (key == "WriteU8") {
                    bw->WriteI8((uint8_t)number);
                }

                else if (key == "WriteS16BE") {
                    bw->WriteI16BE((int16_t)number);
                } else if (key == "WriteS16LE") {
                    bw->WriteI16LE((int16_t)number);
                } else if (key == "WriteU16BE") {
                    bw->WriteU16BE((uint16_t)number);
                } else if (key == "WriteU16LE") {
                    bw->WriteU16LE((uint16_t)number);
                }

                else if (key == "WriteS32BE") {
                    bw->WriteI32BE((int32_t)number);
                } else if (key == "WriteS32LE") {
                    bw->WriteI32LE((int32_t)number);
                } else if (key == "WriteU32BE") {
                    bw->WriteU32BE((uint32_t)number);
                } else if (key == "WriteU32LE") {
                    bw->WriteU32LE((uint32_t)number);
                }

                else if (key == "WriteS64BE") {
                    bw->WriteI64BE(number);
                } else if (key == "WriteS64LE") {
                    bw->WriteI64LE(number);
                } else if (key == "WriteU64BE") {
                    uint64_t number2 = 0;
                    memcpy(&number2, &number, sizeof(uint64_t));
                    bw->WriteU64BE(number2);
                } else if (key == "WriteU64LE") {

                    uint64_t number2 = 0;
                    memcpy(&number2, &number, sizeof(uint64_t));
                    bw->WriteU64LE(number2);
                }

            } else if (GetArgument(args, 0, numberDbl)) {
                if (key == "WriteF32BE") {
                    bw->WriteF32BE((float)numberDbl);
                } else if (key == "WriteF32LE") {
                    bw->WriteF32LE((float)numberDbl);
                } else if (key == "WriteF64BE") {
                    bw->WriteF64BE(numberDbl);
                } else if (key == "WriteF64LE") {
                    bw->WriteF64LE(numberDbl);
                }
            } else if (GetArgument(args, 0, uuid)) {
                if (key == "WriteUuid") {
                    bw->WriteUuid(uuid);
                }
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<std::shared_ptr<
                       Tesses::Framework::Http::ServerSentEvents>>(instance)) {
            auto &sse = std::get<
                std::shared_ptr<Tesses::Framework::Http::ServerSentEvents>>(
                instance);
            if (sse != nullptr) {
                std::string text;
                std::string text2;
                if (key == "SendComment" && GetArgument(args, 0, text)) {
                    sse->SendComment(text);
                } else if (key == "SendCustomEvent" &&
                           GetArgument(args, 0, text) &&
                           GetArgument(args, 1, text2)) {
                    sse->SendCustomEvent(text, text2);
                } else if (key == "SendCustomEvent" &&
                           GetArgument(args, 0, text)) {
                    if (GetArgument(args, 1, text2))
                        sse->SendData(text, text2);
                    else
                        sse->SendData(text);
                } else if (key == "SendId" && GetArgument(args, 0, text)) {
                    sse->SendId(text);
                } else if (key == "SendRetry") {
                    std::shared_ptr<Tesses::Framework::Date::TimeSpan> ts;
                    int64_t num;
                    if (GetArgument(args, 0, ts) && ts) {
                        sse->SendRetry(*ts);
                    } else if (GetArgument(args, 0, num)) {
                        sse->SendRetry((uint32_t)num);
                    }
                }
            }

            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<
                       std::shared_ptr<Tesses::Framework::Streams::Stream>>(
                       instance)) {
            auto &strm =
                std::get<std::shared_ptr<Tesses::Framework::Streams::Stream>>(
                    instance);
            if (strm != nullptr) {
                auto memStrm = std::dynamic_pointer_cast<
                    Tesses::Framework::Streams::MemoryStream>(strm);
                auto netStrm = std::dynamic_pointer_cast<
                    Tesses::Framework::Streams::NetworkStream>(strm);

                auto mystrm = std::dynamic_pointer_cast<TObjectStream>(strm);

                if (mystrm != nullptr) {
                    TDictionary *dict2;
                    if (GetObjectHeap(mystrm->obj, dict2)) {

                        gc->BarrierBegin();
                        auto o = dict2->GetValue(key);
                        gc->BarrierEnd();

                        return InvokeMethod(ls, o, dict2, args);
                    }
                }
                if (memStrm != nullptr) {
                    if (key == "GetBytes") {
                        auto res = TByteArray::Create(ls);
                        res->data = memStrm->GetBuffer();
                        cse.back()->Push(gc, res);
                        return false;
                    }
                }
                if (netStrm != nullptr) {
                    if (key == "SetMulticastMembership") {
                        std::string ma;
                        std::string ifaceIP = "0.0.0.0";
                        if (GetArgument(args, 0, ma)) {
                            GetArgument(args, 1, ifaceIP);
                            netStrm->SetMulticastMembership(ma, ifaceIP);
                        }
                        cse.back()->Push(gc, Undefined());
                        return false;
                    }
                    if (key == "GetPort") {
                        cse.back()->Push(gc, (int64_t)netStrm->GetPort());
                        return false;
                    }
                    if (key == "Bind") {
                        std::string ip;
                        int64_t port;
                        if (GetArgument(args, 0, ip) &&
                            GetArgument(args, 1, port))
                            netStrm->Bind(ip, (uint16_t)port);

                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if (key == "Accept") {
                        std::string ip;
                        uint16_t port;
                        auto strm = netStrm->Accept(ip, port);
                        TDictionary *dict = TDictionary::Create(ls);
                        gc->BarrierBegin();
                        dict->SetValue("IP", ip);
                        dict->SetValue("Port", (int64_t)port);
                        dict->SetValue("Stream", strm);

                        gc->BarrierEnd();
                        cse.back()->Push(gc, dict);
                        return false;
                    }
                    if (key == "Listen") {
                        int64_t backlog;
                        if (GetArgument(args, 0, backlog)) {
                            netStrm->Listen((int32_t)backlog);
                        } else {
                            netStrm->Listen(10);
                        }

                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if (key == "ReadFrom") {
                        TByteArray *data;
                        int64_t offset;
                        int64_t length;
                        if (GetArgumentHeap<TByteArray *>(args, 0, data) &&
                            GetArgument<int64_t>(args, 1, offset) &&
                            GetArgument<int64_t>(args, 2, length)) {
                            size_t off = (size_t)offset;
                            size_t len = (size_t)length;
                            std::string ip = {};
                            uint16_t port = 0;

                            if (off < len)

                                len = netStrm->ReadFrom(
                                    data->data.data() + off,
                                    std::min(len,
                                             std::min(data->data.size() - off,
                                                      data->data.size())),
                                    ip, port);

                            else
                                len = 0;

                            TDictionary *dict = TDictionary::Create(ls);
                            gc->BarrierBegin();
                            dict->SetValue("IP", ip);
                            dict->SetValue("Port", (int64_t)port);
                            dict->SetValue("Read", (int64_t)len);

                            gc->BarrierEnd();
                            cse.back()->Push(gc, dict);

                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if (key == "WriteTo") {
                        TByteArray *data;
                        int64_t offset;
                        int64_t length;
                        std::string ip;
                        int64_t port;
                        if (GetArgumentHeap<TByteArray *>(args, 0, data) &&
                            GetArgument<int64_t>(args, 1, offset) &&
                            GetArgument<int64_t>(args, 2, length) &&
                            GetArgument(args, 3, ip) &&
                            GetArgument(args, 4, port)) {
                            size_t off = (size_t)offset;
                            size_t len = (size_t)length;

                            if (off < len)

                                len = netStrm->WriteTo(
                                    data->data.data() + off,
                                    std::min(len,
                                             std::min(data->data.size() - off,
                                                      data->data.size())),
                                    ip, (int64_t)port);

                            else
                                len = 0;

                            cse.back()->Push(gc, (int64_t)len);
                            return false;
                        }
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                }

                if (key == "Read") {
                    TByteArray *data;
                    int64_t offset;
                    int64_t length;
                    if (GetArgumentHeap<TByteArray *>(args, 0, data) &&
                        GetArgument<int64_t>(args, 1, offset) &&
                        GetArgument<int64_t>(args, 2, length)) {
                        size_t off = (size_t)offset;
                        size_t len = (size_t)length;

                        if (off < len)

                            len = strm->Read(
                                data->data.data() + off,
                                std::min(len, std::min(data->data.size() - off,
                                                       data->data.size())));

                        else
                            len = 0;

                        cse.back()->Push(gc, (int64_t)len);
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "Write") {
                    TByteArray *data;
                    int64_t offset;
                    int64_t length;
                    if (GetArgumentHeap<TByteArray *>(args, 0, data) &&
                        GetArgument<int64_t>(args, 1, offset) &&
                        GetArgument<int64_t>(args, 2, length)) {
                        size_t off = (size_t)offset;
                        size_t len = (size_t)length;

                        if (off < len)

                            len = strm->Write(
                                data->data.data() + off,
                                std::min(len, std::min(data->data.size() - off,
                                                       data->data.size())));

                        else
                            len = 0;

                        cse.back()->Push(gc, (int64_t)len);
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "ReadBlock") {
                    TByteArray *data;
                    int64_t offset;
                    int64_t length;
                    if (GetArgumentHeap<TByteArray *>(args, 0, data) &&
                        GetArgument<int64_t>(args, 1, offset) &&
                        GetArgument<int64_t>(args, 2, length)) {
                        size_t off = (size_t)offset;
                        size_t len = (size_t)length;

                        if (off < len)

                            len = strm->ReadBlock(
                                data->data.data() + off,
                                std::min(len, std::min(data->data.size() - off,
                                                       data->data.size())));

                        else
                            len = 0;

                        cse.back()->Push(gc, (int64_t)len);
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "WriteText") {
                    std::string text;

                    if (GetArgument(args, 0, text)) {
                        strm->WriteBlock((const uint8_t *)text.data(),
                                         text.size());
                    }

                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "WriteBlock") {
                    TByteArray *data;
                    int64_t offset;
                    int64_t length;
                    if (GetArgumentHeap<TByteArray *>(args, 0, data) &&
                        GetArgument<int64_t>(args, 1, offset) &&
                        GetArgument<int64_t>(args, 2, length)) {
                        size_t off = (size_t)offset;
                        size_t len = (size_t)length;

                        if (off < len)

                            strm->WriteBlock(
                                data->data.data() + off,
                                std::min(len, std::min(data->data.size() - off,
                                                       data->data.size())));
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "CopyTo") {
                    std::shared_ptr<Tesses::Framework::Streams::Stream> data;
                    int64_t buffSize;
                    if (GetArgument(args, 0, data)) {
                        if (!GetArgument<int64_t>(args, 1, buffSize))
                            buffSize = 1024;
                        strm->CopyTo(data, (size_t)buffSize);
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "CopyToLimit") {
                    std::shared_ptr<Tesses::Framework::Streams::Stream> data;
                    int64_t cnt;
                    int64_t buffSize;
                    if (GetArgument(args, 0, data) &&
                        GetArgument(args, 1, cnt)) {
                        if (!GetArgument<int64_t>(args, 2, buffSize))
                            buffSize = 1024;
                        strm->CopyToLimit(data, (uint64_t)cnt,
                                          (size_t)buffSize);
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "Flush") {
                    strm->Flush();
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "Close") {
                    strm->Close();
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "Seek") {
                    int64_t pos, whence;
                    if (!GetArgument<int64_t>(args, 0, pos))
                        pos = 0;

                    if (!GetArgument<int64_t>(args, 1, whence))
                        whence = 0;

                    strm->Seek(
                        pos,
                        whence == 0
                            ? Tesses::Framework::Streams::SeekOrigin::Begin
                        : whence == 1
                            ? Tesses::Framework::Streams::SeekOrigin::Current
                            : Tesses::Framework::Streams::SeekOrigin::End);

                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                cse.back()->Push(gc, nullptr);
                return false;
            }
        } else if (std::holds_alternative<
                       std::shared_ptr<Tesses::Framework::Http::IHttpServer>>(
                       instance)) {
            auto svr =
                std::get<std::shared_ptr<Tesses::Framework::Http::IHttpServer>>(
                    instance);
            if (svr != nullptr) {
                auto mountable = std::dynamic_pointer_cast<
                    Tesses::Framework::Http::MountableServer>(svr);
                auto routable = std::dynamic_pointer_cast<
                    Tesses::Framework::Http::RouteServer>(svr);

                if (mountable != nullptr) {
                    if (key == "Mount") {
                        Tesses::Framework::Filesystem::VFSPath p;

                        if (args.size() > 1 && GetArgumentAsPath(args, 0, p)) {
                            std::shared_ptr<
                                Tesses::Framework::Http::IHttpServer>
                                svr2 = ToHttpServer(gc, args[1]);

                            if (svr2)
                                mountable->Mount(p.ToString(), svr2);

                            cse.back()->Push(gc, nullptr);
                            return false;
                        }
                    }
                    if (key == "Unmount") {
                        Tesses::Framework::Filesystem::VFSPath p;

                        if (GetArgumentAsPath(args, 0, p)) {
                            mountable->Unmount(p.ToString());
                            cse.back()->Push(gc, nullptr);
                            return false;
                        }
                    }
                }
                if (routable != nullptr) {
                    if (key == "Add") {
                        std::string method;
                        std::string pattern;
                        TCallable *callable;
                        if (GetArgument(args, 0, method) &&
                            GetArgument(args, 1, pattern) &&
                            GetArgumentHeap(args, 2, callable)) {
                            routable->Add(
                                method, pattern,
                                callable->ToRouteServerRequestHandler(gc));
                        }
                    } else if (key == "Delete") {
                        std::string pattern;
                        TCallable *callable;
                        if (GetArgument(args, 0, pattern) &&
                            GetArgumentHeap(args, 1, callable)) {
                            routable->Delete(
                                pattern,
                                callable->ToRouteServerRequestHandler(gc));
                        }
                    } else if (key == "Get") {
                        std::string pattern;
                        TCallable *callable;
                        if (GetArgument(args, 0, pattern) &&
                            GetArgumentHeap(args, 1, callable)) {
                            routable->Get(
                                pattern,
                                callable->ToRouteServerRequestHandler(gc));
                        }
                    } else if (key == "Options") {
                        std::string pattern;
                        TCallable *callable;
                        if (GetArgument(args, 0, pattern) &&
                            GetArgumentHeap(args, 1, callable)) {
                            routable->Options(
                                pattern,
                                callable->ToRouteServerRequestHandler(gc));
                        }
                    } else if (key == "Patch") {
                        std::string pattern;
                        TCallable *callable;
                        if (GetArgument(args, 0, pattern) &&
                            GetArgumentHeap(args, 1, callable)) {
                            routable->Patch(
                                pattern,
                                callable->ToRouteServerRequestHandler(gc));
                        }
                    } else if (key == "Post") {
                        std::string pattern;
                        TCallable *callable;
                        if (GetArgument(args, 0, pattern) &&
                            GetArgumentHeap(args, 1, callable)) {
                            routable->Post(
                                pattern,
                                callable->ToRouteServerRequestHandler(gc));
                        }
                    } else if (key == "Put") {
                        std::string pattern;
                        TCallable *callable;
                        if (GetArgument(args, 0, pattern) &&
                            GetArgumentHeap(args, 1, callable)) {
                            routable->Put(
                                pattern,
                                callable->ToRouteServerRequestHandler(gc));
                        }
                    } else if (key == "Trace") {
                        std::string pattern;
                        TCallable *callable;
                        if (GetArgument(args, 0, pattern) &&
                            GetArgumentHeap(args, 1, callable)) {
                            routable->Trace(
                                pattern,
                                callable->ToRouteServerRequestHandler(gc));
                        }
                    }
                }
                if (key == "Handle") {
                    cse.back()->Push(gc, IHttpServer_Handle(svr, args));
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
        } else if (std::holds_alternative<
                       std::shared_ptr<Tesses::Framework::Filesystem::VFS>>(
                       instance)) {
            auto vfs =
                std::get<std::shared_ptr<Tesses::Framework::Filesystem::VFS>>(
                    instance);
            if (vfs != nullptr) {
                auto myvfs = std::dynamic_pointer_cast<TObjectVFS>(vfs);

                auto mountable = std::dynamic_pointer_cast<
                    Tesses::Framework::Filesystem::MountableFilesystem>(vfs);

                if (myvfs != nullptr) {
                    TDictionary *dict2;
                    if (GetObjectHeap(myvfs->obj, dict2)) {

                        gc->BarrierBegin();
                        auto o = dict2->GetValue(key);
                        gc->BarrierEnd();

                        return InvokeMethod(ls, o, dict2, args);
                    }
                }
                if (mountable != nullptr) {
                    if (key == "Unmount") {
                        Tesses::Framework::Filesystem::VFSPath path;

                        if (GetArgumentAsPath(args, 0, path)) {
                            mountable->Unmount(path);
                        }

                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                    if (key == "Mount") {
                        Tesses::Framework::Filesystem::VFSPath path;
                        std::shared_ptr<Tesses::Framework::Filesystem::VFS>
                            vfs2;
                        if (GetArgumentAsPath(args, 0, path) &&
                            GetArgument(args, 1, vfs2)) {

                            // mountable->Mount(path, , true);
                            mountable->Mount(path, vfs2);
                        }

                        cse.back()->Push(gc, nullptr);
                        return false;
                    }
                }
                if (key == "Close") {
                    vfs->Close();
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "EnumeratePaths") {
                    Tesses::Framework::Filesystem::VFSPath dir;
                    if (GetArgumentAsPath(args, 0, dir)) {
                        auto tem = TExternalMethod::Create(
                            ls, "Get the enumerator", {},
                            [vfs, dir](GCList &ls,
                                       std::vector<TObject> args) -> TObject {
                                return TVFSPathEnumerator::Create(
                                    ls, vfs->EnumeratePaths(dir));
                            });
                        auto d1 = TDictionary::Create(ls);
                        gc->BarrierBegin();
                        tem->watch.push_back(vfs);
                        d1->SetValue("GetEnumerator", tem);
                        gc->BarrierEnd();

                        cse.back()->Push(gc, d1);
                        return false;
                    }

                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "MoveDirectory") {
                    Tesses::Framework::Filesystem::VFSPath existingFile;

                    Tesses::Framework::Filesystem::VFSPath symlinkFile;
                    if (GetArgumentAsPath(args, 0, existingFile) &&
                        GetArgumentAsPath(args, 1, symlinkFile)) {
                        vfs->MoveDirectory(existingFile, symlinkFile);
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "SetDate") {
                    Tesses::Framework::Filesystem::VFSPath path;
                    std::shared_ptr<Tesses::Framework::Date::DateTime>
                        lastWrite;
                    std::shared_ptr<Tesses::Framework::Date::DateTime>
                        lastAccess;
                    if (GetArgumentAsPath(args, 0, path) &&
                        GetArgument(args, 1, lastWrite) &&
                        GetArgument(args, 2, lastAccess)) {
                        vfs->SetDate(path, *lastWrite, *lastAccess);
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "GetDate") {
                    Tesses::Framework::Filesystem::VFSPath path;
                    if (GetArgumentAsPath(args, 0, path)) {
                        Tesses::Framework::Date::DateTime lastWrite;
                        Tesses::Framework::Date::DateTime lastAccess;
                        vfs->GetDate(path, lastWrite, lastAccess);

                        auto dict = TDictionary::Create(ls);
                        ls.GetGC()->BarrierBegin();

                        dict->SetValue(
                            "LastWrite",
                            std::make_shared<Tesses::Framework::Date::DateTime>(
                                lastWrite));
                        dict->SetValue(
                            "LastAccess",
                            std::make_shared<Tesses::Framework::Date::DateTime>(
                                lastAccess));
                        ls.GetGC()->BarrierEnd();

                        cse.back()->Push(gc, dict);
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "MoveFile") {
                    Tesses::Framework::Filesystem::VFSPath existingFile;

                    Tesses::Framework::Filesystem::VFSPath symlinkFile;
                    if (GetArgumentAsPath(args, 0, existingFile) &&
                        GetArgumentAsPath(args, 1, symlinkFile)) {
                        vfs->MoveFile(existingFile, symlinkFile);
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "CreateHardlink") {
                    Tesses::Framework::Filesystem::VFSPath existingFile;

                    Tesses::Framework::Filesystem::VFSPath symlinkFile;
                    if (GetArgumentAsPath(args, 0, existingFile) &&
                        GetArgumentAsPath(args, 1, symlinkFile)) {
                        vfs->CreateHardlink(existingFile, symlinkFile);
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "CreateSymlink") {
                    Tesses::Framework::Filesystem::VFSPath existingFile;

                    Tesses::Framework::Filesystem::VFSPath symlinkFile;
                    if (GetArgumentAsPath(args, 0, existingFile) &&
                        GetArgumentAsPath(args, 1, symlinkFile)) {
                        vfs->CreateSymlink(existingFile, symlinkFile);
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegularFileExists") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        cse.back()->Push(gc, vfs->RegularFileExists(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "SpecialFileExists") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        cse.back()->Push(gc, vfs->SpecialFileExists(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "FIFOFileExists") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        cse.back()->Push(gc, vfs->FIFOFileExists(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "SocketFileExists") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        cse.back()->Push(gc, vfs->SocketFileExists(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "BlockDeviceExists") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        cse.back()->Push(gc, vfs->BlockDeviceExists(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "CharacterDeviceExists") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        cse.back()->Push(gc,
                                         vfs->CharacterDeviceExists(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "SymlinkExists") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        cse.back()->Push(gc, vfs->SymlinkExists(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "DirectoryExists") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        cse.back()->Push(gc, vfs->DirectoryExists(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "FileExists") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        cse.back()->Push(gc, vfs->FileExists(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "ReadLink") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        cse.back()->Push(gc, vfs->ReadLink(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "VFSPathToSystem") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        cse.back()->Push(gc, vfs->VFSPathToSystem(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "SystemToVFSPath") {
                    std::string filename;
                    if (GetArgument(args, 0, filename)) {
                        cse.back()->Push(gc, vfs->SystemToVFSPath(filename));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "DeleteFile") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        vfs->DeleteFile(filename);
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "Lock") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        vfs->Lock(filename);
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "Unlock") {
                    Tesses::Framework::Filesystem::VFSPath filename;
                    if (GetArgumentAsPath(args, 0, filename)) {
                        vfs->Unlock(filename);
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "DeleteDirectoryRecurse") {
                    Tesses::Framework::Filesystem::VFSPath dirname;
                    if (GetArgumentAsPath(args, 0, dirname)) {
                        vfs->DeleteDirectoryRecurse(dirname);
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "DeleteDirectory") {
                    Tesses::Framework::Filesystem::VFSPath dirname;
                    if (GetArgumentAsPath(args, 0, dirname)) {
                        vfs->DeleteDirectory(dirname);
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "CreateDirectory") {

                    Tesses::Framework::Filesystem::VFSPath dirname;
                    if (GetArgumentAsPath(args, 0, dirname)) {
                        vfs->CreateDirectory(dirname);
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "OpenFile") {

                    Tesses::Framework::Filesystem::VFSPath path;
                    std::string mode;
                    if (GetArgumentAsPath(args, 0, path) &&
                        GetArgument(args, 1, mode)) {
                        auto res = vfs->OpenFile(path, mode);
                        cse.back()->Push(gc, res);
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "Chmod") {
                    Tesses::Framework::Filesystem::VFSPath path;
                    int64_t mode;
                    if (GetArgumentAsPath(args, 0, path) &&
                        GetArgument(args, 1, mode)) {
                        vfs->Chmod(path, (uint32_t)mode);
                    }

                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "Stat") {
                    Tesses::Framework::Filesystem::VFSPath path;

                    if (GetArgumentAsPath(args, 0, path)) {
                        Tesses::Framework::Filesystem::StatData data;
                        if (vfs->Stat(path, data)) {
                            cse.back()->Push(
                                gc,
                                TDictionary::Create(
                                    ls,
                                    {TDItem("BlockCount",
                                            (int64_t)data.BlockCount),
                                     TDItem("BlockSize",
                                            (int64_t)data.BlockSize),
                                     TDItem("Device", (int64_t)data.Device),
                                     TDItem("DeviceId", (int64_t)data.DeviceId),
                                     TDItem("GroupId", (int64_t)data.GroupId),
                                     TDItem("HardLinks",
                                            (int64_t)data.HardLinks),
                                     TDItem("Inode", (int64_t)data.Inode),
                                     TDItem(
                                         "LastAccess",
                                         std::make_shared<
                                             Tesses::Framework::Date::DateTime>(
                                             data.LastAccess)),
                                     TDItem(
                                         "LastModified",
                                         std::make_shared<
                                             Tesses::Framework::Date::DateTime>(
                                             data.LastModified)),
                                     TDItem(
                                         "LastStatus",
                                         std::make_shared<
                                             Tesses::Framework::Date::DateTime>(
                                             data.LastStatus)),
                                     TDItem("Mode", (int64_t)data.Mode),
                                     TDItem("Size", (int64_t)data.Size),
                                     TDItem("UserId", (int64_t)data.UserId)}));
                            return false;
                        }
                    }

                    cse.back()->Push(gc, nullptr);
                    return false;
                }

                if (key == "StatVFS") {
                    Tesses::Framework::Filesystem::VFSPath path;

                    if (GetArgumentAsPath(args, 0, path)) {
                        Tesses::Framework::Filesystem::StatVFSData data;
                        if (vfs->StatVFS(path, data)) {
                            cse.back()->Push(
                                gc,
                                TDictionary::Create(
                                    ls, {TDItem("BlockSize",
                                                (int64_t)data.BlockSize),
                                         TDItem("FragmentSize",
                                                (int64_t)data.FragmentSize),
                                         TDItem("Blocks", (int64_t)data.Blocks),
                                         TDItem("BlocksFree",
                                                (int64_t)data.BlocksFree),
                                         TDItem("BlocksAvailable",
                                                (int64_t)data.BlocksAvailable),
                                         TDItem("TotalInodes",
                                                (int64_t)data.TotalInodes),
                                         TDItem("FreeInodes",
                                                (int64_t)data.FreeInodes),
                                         TDItem("AvailableInodes",
                                                (int64_t)data.AvailableInodes),
                                         TDItem("Id", (int64_t)data.Id),
                                         TDItem("Flags", (int64_t)data.Flags),
                                         TDItem("MaxNameLength",
                                                (int64_t)data.MaxNameLength)}));
                            return false;
                        }
                    }

                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                cse.back()->Push(gc, nullptr);
                return false;
            }

        } else if (std::holds_alternative<Tesses::Framework::Uuid>(instance)) {
            auto &obj = std::get<Tesses::Framework::Uuid>(instance);
            if (key == "ToBytes") {
                TByteArray *ba = TByteArray::Create(ls);
                ba->data.resize(16);
                Tesses::Framework::Serialization::BitConverter::FromUuid(
                    ba->data[0], obj);
                cse.back()->Push(gc, ba);
                return false;
            }
            if (key == "ToString") {
                int64_t arg;
                if (GetArgument(args, 0, arg)) {
                    cse.back()->Push(
                        gc, obj.ToString((Framework::UuidStringifyConfig)arg));
                    return false;
                }
                cse.back()->Push(
                    gc, obj.ToString(
                            Framework::UuidStringifyConfig::LowercaseNoCurly));
                return false;
            }
            cse.back()->Push(gc, Undefined());
            return false;
        } else if (std::holds_alternative<THeapObjectHolder>(instance)) {
            auto obj = std::get<THeapObjectHolder>(instance).obj;
            auto list = dynamic_cast<TList *>(obj);
            auto dynList = dynamic_cast<TDynamicList *>(obj);
            auto bArray = dynamic_cast<TByteArray *>(obj);
            auto dict = dynamic_cast<TDictionary *>(obj);
            auto dynDict = dynamic_cast<TDynamicDictionary *>(obj);
            auto ittr = dynamic_cast<TEnumerator *>(obj);

            auto env = dynamic_cast<TEnvironment *>(obj);

            auto subEnv = dynamic_cast<TSubEnvironment *>(obj);
            auto rootEnv = dynamic_cast<TRootEnvironment *>(obj);
            auto callable = dynamic_cast<TCallable *>(obj);
            auto callstackEntry = dynamic_cast<CallStackEntry *>(obj);

            auto natObj = dynamic_cast<TNativeObject *>(obj);
            auto cls = dynamic_cast<TClassObject *>(obj);
            auto aArray = dynamic_cast<TAssociativeArray *>(obj);
            auto ttask = dynamic_cast<TTask *>(obj);
            auto file = dynamic_cast<TFile *>(obj);

            auto queryable = dynamic_cast<TQueryable *>(obj);

            if (queryable != nullptr) {
                if (key == "Skip") {
                    int64_t count;
                    if (GetArgument(args, 0, count)) {
                        cse.back()->Push(gc, queryable->Skip(ls, count));
                        return false;
                    }
                }
                if (key == "Take") {
                    int64_t count;
                    if (GetArgument(args, 0, count)) {
                        cse.back()->Push(gc, queryable->Take(ls, count));
                        return false;
                    }
                }
                if (key == "SkipWhile") {
                    TCallable *callable;
                    if (GetArgumentHeap(args, 0, callable)) {
                        cse.back()->Push(gc,
                                         queryable->SkipWhile(ls, callable));
                        return false;
                    }
                }
                if (key == "TakeWhile") {
                    TCallable *callable;
                    if (GetArgumentHeap(args, 0, callable)) {
                        cse.back()->Push(gc,
                                         queryable->TakeWhile(ls, callable));
                        return false;
                    }
                }
                if (key == "Select") {
                    TCallable *callable;
                    if (GetArgumentHeap(args, 0, callable)) {
                        cse.back()->Push(gc, queryable->Select(ls, callable));
                        return false;
                    }
                }
                if (key == "Where") {
                    TCallable *callable;
                    if (GetArgumentHeap(args, 0, callable)) {
                        cse.back()->Push(gc, queryable->Where(ls, callable));
                        return false;
                    }
                }
                if (key == "ToList") {
                    cse.back()->Push(gc, queryable->ToList(ls));
                    return false;
                }

                if (key == "ForEach") {
                    TCallable *call;
                    if (GetArgumentHeap(args, 0, call)) {
                        queryable->ForEach(gc, call);
                    }

                    cse.back()->Push(gc, Undefined());
                    return false;
                }

                if (key == "Count") {
                    TCallable *call;
                    if (GetArgumentHeap(args, 0, call)) {
                        cse.back()->Push(gc, queryable->Count(gc, call));
                        return false;
                    } else {
                        cse.back()->Push(gc, queryable->Count(gc));
                        return false;
                    }
                }
                if (key == "Contains") {
                    if (!args.empty()) {
                        cse.back()->Push(gc, queryable->Contains(gc, args[0]));
                        return false;
                    }
                }
                if (key == "All") {
                    TCallable *call;
                    if (GetArgumentHeap(args, 0, call)) {
                        cse.back()->Push(gc, queryable->All(gc, call));
                        return false;
                    }
                }
                if (key == "Any") {
                    TCallable *call;
                    if (GetArgumentHeap(args, 0, call)) {
                        cse.back()->Push(gc, queryable->Any(gc, call));
                        return false;
                    }
                }

                if (key == "GetEnumerator") {
                    cse.back()->Push(gc, queryable->GetEnumerator(ls));
                    return false;
                }

                cse.back()->Push(gc, Undefined());
                return false;
            }

            if (file != nullptr) {
                if (key == "MetadataDecode") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < file->metadata.size()) {
                        cse.back()->Push(
                            gc, file->MetadataDecode(ls, (size_t)index));
                        return false;
                    }
                }
                if (key == "MetadataName") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < file->metadata.size()) {
                        cse.back()->Push(
                            gc, file->metadata.at((size_t)index).first);
                        return false;
                    }
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }
            if (ttask != nullptr) {
                if (key == "ContinueWith") {
                    TCallable *callable2;
                    if (GetArgumentHeap(args, 0, callable2)) {
                        cse.back()->Push(gc,
                                         ttask->ContinueWith(ls, callable2));
                        return false;
                    }
                }
                if (key == "Wait") {

                    cse.back()->Push(gc, ttask->Wait());
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            }

            if (natObj != nullptr) {
                cse.back()->Push(gc, natObj->CallMethod(ls, key, args));
                return false;
            }

            if (callstackEntry != nullptr) {
                if (key == "Resume") {
                    gc->BarrierBegin();
                    cse.push_back(callstackEntry);
                    gc->BarrierEnd();
                    return true;
                }
                if (key == "Push") {
                    if (!args.empty())
                        callstackEntry->Push(gc, args[0]);
                    else
                        callstackEntry->Push(gc, Undefined());
                    return false;
                }
                if (key == "Pop") {
                    cse.back()->Push(gc, callstackEntry->Pop(ls));
                    return false;
                }
            }

            if (rootEnv != nullptr) {
                // TStd::RegisterCrypto
                // TStd::RegisterDictionary
                // TStd::RegisterEnv
                // TStd::RegisterIO
                // TStd::RegisterJson
                // TStd::RegisterNet
                // TStd::RegisterOGC
                // TStd::RegisterPath
                // TStd::RegisterRoot
                // TStd::RegisterSqlite
                // TStd::RegisterVM
                auto myEnv = cse.back()->env->GetRootEnvironment();

                if (key == "RegisterOnError") {
                    TCallable *callable;
                    if (!rootEnv->permissions.locked &&
                        GetArgumentHeap(args, 0, callable)) {
                        gc->BarrierBegin();
                        rootEnv->RegisterOnError(callable);
                        gc->BarrierEnd();
                    }
                }
                if (key == "SetCustomConsole") {
                    TDictionary *dict;
                    if (!rootEnv->permissions.locked &&
                        GetArgumentHeap(args, 0, dict)) {
                        gc->BarrierBegin();
                        rootEnv->permissions.customConsole = dict;
                        gc->BarrierEnd();
                    } else if (!rootEnv->permissions.locked &&
                               myEnv->permissions.customConsole == nullptr) {
                        gc->BarrierBegin();
                        rootEnv->permissions.customConsole = nullptr;
                        gc->BarrierEnd();
                    }
                }
                if (key == "RegisterEverything") {
                    if (rootEnv->permissions.locked) {
                        cse.back()->Push(gc, nullptr);
                        return false;
                    }

                    if (myEnv->permissions.canRegisterEverything) {
                        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
                        if (GetArgument(args, 0, vfs)) {

                            auto rfs = std::dynamic_pointer_cast<
                                Tesses::Framework::Filesystem::
                                    RelativeFilesystem>(vfs);
                            if (rfs) {
                                TStd::RegisterStd(gc, rootEnv, rfs);
                                cse.back()->Push(gc, nullptr);
                                return false;
                            }
                        }

                        if (myEnv->permissions.localfs)
                            TStd::RegisterStd(
                                gc, rootEnv,
                                std::make_shared<Tesses::Framework::Filesystem::
                                                     RelativeFilesystem>(
                                    myEnv->permissions.localfs->GetVFS(),
                                    myEnv->permissions.localfs->GetWorking()));
                        else
                            TStd::RegisterStd(gc, rootEnv, nullptr);

                        cse.back()->Push(gc, nullptr);
                        return false;
                    } else {
                        if (myEnv->permissions.canRegisterConsole &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterConsole(gc, rootEnv);

                        if (myEnv->permissions.canRegisterCrypto &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterCrypto(gc, rootEnv);

                        if (myEnv->permissions.canRegisterDictionary &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterDictionary(gc, rootEnv);

                        if (myEnv->permissions.canRegisterEnv &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterEnv(gc, rootEnv);

                        if (myEnv->permissions.canRegisterIO &&
                            !rootEnv->permissions.locked) {
                            if (myEnv->permissions.localfs) {
                                TStd::RegisterIO(
                                    gc, rootEnv,
                                    std::make_shared<
                                        Tesses::Framework::Filesystem::
                                            RelativeFilesystem>(
                                        myEnv->permissions.localfs->GetVFS(),
                                        myEnv->permissions.localfs
                                            ->GetWorking()));
                            } else {
                                TStd::RegisterIO(gc, rootEnv, nullptr);
                            }
                        }

                        if (myEnv->permissions.canRegisterJSON &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterJson(gc, rootEnv);

                        if (myEnv->permissions.canRegisterNet &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterNet(gc, rootEnv);

                        if (myEnv->permissions.canRegisterOGC &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterOGC(gc, rootEnv);

                        if (myEnv->permissions.canRegisterPath &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterPath(gc, rootEnv);

                        if (myEnv->permissions.canRegisterRoot &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterRoot(gc, rootEnv);

                        if (myEnv->permissions.canRegisterSqlite &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterSqlite(gc, rootEnv);

                        if (myEnv->permissions.canRegisterVM &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterVM(gc, rootEnv);
                        if (myEnv->permissions.canRegisterProcess &&
                            !rootEnv->permissions.locked)
                            TStd::RegisterProcess(gc, rootEnv);
                    }

                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterClass") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterClass) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterClass(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }

                if (key == "RegisterConsole") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterConsole) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterConsole(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterProcess") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterProcess) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterProcess(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterCrypto") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterCrypto) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterCrypto(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterDictionary") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterDictionary) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterDictionary(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterEnv") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterEnv) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterDictionary(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterIO") {
                    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;

                    bool r;
                    if (GetArgument(args, 0, r)) {
                        if ((myEnv->permissions.canRegisterEverything ||
                             myEnv->permissions.canRegisterIO) &&
                            !rootEnv->permissions.locked) {
                            if (myEnv->permissions.localfs) {
                                TStd::RegisterIO(
                                    gc, rootEnv,
                                    std::make_shared<
                                        Tesses::Framework::Filesystem::
                                            RelativeFilesystem>(
                                        myEnv->permissions.localfs->GetVFS(),
                                        myEnv->permissions.localfs
                                            ->GetWorking()));
                            } else {
                                TStd::RegisterIO(gc, rootEnv, nullptr);
                            }
                        }
                    }
                    if (GetArgument(args, 0, vfs)) {
                        auto rfs = std::dynamic_pointer_cast<
                            Tesses::Framework::Filesystem::RelativeFilesystem>(
                            vfs);
                        if (rfs &&
                            (myEnv->permissions.canRegisterEverything ||
                             myEnv->permissions.canRegisterIO) &&
                            !rootEnv->permissions.locked) {
                            TStd::RegisterIO(gc, rootEnv, rfs);
                        }
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterJson") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterJSON) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterJson(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterNet") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterNet) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterNet(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterOGC") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterOGC) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterOGC(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterPath") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterPath) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterPath(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterRoot") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterRoot) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterRoot(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }

                if (key == "RegisterSqlite") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterSqlite) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterSqlite(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }

                if (key == "SetSqliteRoot") {
                    Tesses::Framework::Filesystem::VFSPath p;
                    if (!rootEnv->permissions.locked) {
                        if (GetArgumentAsPath(args, 0, p)) {
                            if (myEnv->permissions.sqlite3Scoped) {
                                rootEnv->permissions.sqliteOffsetPath =
                                    myEnv->permissions.sqliteOffsetPath /
                                    p.CollapseRelativeParents();
                                rootEnv->permissions.sqlite3Scoped = true;
                            } else {
                                rootEnv->permissions.sqliteOffsetPath = p;
                                rootEnv->permissions.sqlite3Scoped = true;
                            }
                        } else {
                            if (myEnv->permissions.sqlite3Scoped) {
                                rootEnv->permissions.sqliteOffsetPath =
                                    myEnv->permissions.sqliteOffsetPath;
                                rootEnv->permissions.sqlite3Scoped = true;
                            } else {
                                rootEnv->permissions.sqlite3Scoped = false;
                            }
                        }
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "RegisterVM") {
                    if ((myEnv->permissions.canRegisterEverything ||
                         myEnv->permissions.canRegisterVM) &&
                        !rootEnv->permissions.locked)
                        TStd::RegisterVM(gc, rootEnv);
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "LockRegister") {
                    if (!rootEnv->permissions.locked) {
                        rootEnv->permissions.locked = true;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "GetDictionary") {

                    cse.back()->Push(gc, rootEnv->GetDictionary());
                    return false;
                }
                if (key == "LoadFileWithDependencies") {
                    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs0;
                    TFile *f;
                    Tesses::Framework::Filesystem::VFSPath p;
                    if (GetArgument(args, 0, vfs0)) {
                        if (GetArgumentHeap(args, 1, f)) {
                            rootEnv->LoadFileWithDependencies(gc, vfs0, f);
                        } else if (GetArgumentAsPath(args, 1, p)) {
                            rootEnv->LoadFileWithDependencies(gc, vfs0, p);
                        }
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
            }
            if (subEnv != nullptr) {
                if (key == "GetDictionary") {
                    cse.back()->Push(gc, subEnv->GetDictionary());
                    return false;
                }
            }

            if (env != nullptr) {
                if (key == "Eval") {
                    std::string str;
                    if (GetArgument(args, 0, str)) {

                        cse.back()->Push(gc, env->Eval(ls, str));
                        return false;
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "GetRootEnvironment") {
                    cse.back()->Push(gc, env->GetRootEnvironment());
                    return false;
                }

                if (key == "GetSubEnvironment") {
                    cse.back()->Push(gc, env->GetSubEnvironment(ls));
                    return false;
                }
                if (key == "GetParentEnvironment") {
                    cse.back()->Push(gc, env->GetParentEnvironment());
                    return false;
                }
                if (key == "GetVariable") {
                    std::string key;
                    gc->BarrierBegin();
                    if (GetArgument(args, 0, key))
                        cse.back()->Push(gc, env->GetVariable(key));
                    gc->BarrierEnd();
                    return false;
                }
                if (key == "SetVariable") {
                    std::string key;

                    gc->BarrierBegin();
                    if (args.size() > 1 && GetArgument(args, 0, key)) {
                        if (env->HasConstForSet(key)) {
                            gc->BarrierEnd();
                            ThrowConstError(key);
                        }

                        env->SetVariable(key, args[1]);
                    }
                    gc->BarrierEnd();
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "DeclareVariable") {
                    std::string key;

                    gc->BarrierBegin();
                    if (args.size() > 1 && GetArgument(args, 0, key)) {
                        if (env->HasConstForDeclare(key)) {
                            gc->BarrierEnd();
                            ThrowConstError(key);
                        }
                        env->DeclareVariable(key, args[1]);
                    }
                    gc->BarrierEnd();
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "DeclareConstVariable") {
                    std::string key;

                    gc->BarrierBegin();
                    if (args.size() > 1 && GetArgument(args, 0, key)) {
                        if (env->HasConstForDeclare(key)) {
                            gc->BarrierEnd();
                            ThrowConstError(key);
                        }
                        env->DeclareConstVariable(key, args[1]);
                    }
                    gc->BarrierEnd();
                    cse.back()->Push(gc, nullptr);
                    return false;
                }
                if (key == "LoadFile") {
                    TFile *f;
                    if (GetArgumentHeap(args, 0, f)) {
                        env->LoadFile(gc, f);
                    }
                    cse.back()->Push(gc, nullptr);
                    return false;
                }

                cse.back()->Push(gc, nullptr);
                return false;
            }

            if (ittr != nullptr) {
                if (key == "MoveNext") {
                    cse.back()->Push(gc, ittr->MoveNext(gc));
                    return false;
                }
                cse.back()->Push(gc, nullptr);
                return false;
            }

            else if (bArray != nullptr) {
                if (key == "Count" || key == "Length") {
                    int64_t len = (int64_t)bArray->data.size();
                    if (len < 0)
                        len = 0;

                    cse.back()->Push(gc, len);
                    return false;
                }
                if (key == "ToString") {
                    cse.back()->Push(gc, std::string(bArray->data.begin(),
                                                     bArray->data.end()));
                    return false;
                }
                if (key == "GetAt") {
                    if (args.size() != 1) {
                        throw VMException(
                            "ByteArray.GetAt must only accept one argument");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException(
                            "ByteArray.GetAt must only accept a long");
                    }

                    size_t index = (size_t)std::get<int64_t>(args[0]);
                    size_t sz = bArray->data.size();
                    if (index >= 0 && index < sz) {
                        cse.back()->Push(gc, (int64_t)bArray->data[index]);
                        return false;
                    }
                }

                if (key == "SetAt") {
                    if (args.size() != 2) {
                        throw VMException(
                            "ByteArray.SetAt must only accept two arguments");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException("ByteArray.SetAt first argument must "
                                          "only accept a long");
                    }

                    uint8_t v = 0;

                    if (std::holds_alternative<int64_t>(args[1])) {
                        v = (uint8_t)(std::get<int64_t>(args[1]) & 0xFF);
                    }

                    size_t index = (size_t)std::get<int64_t>(args[0]);
                    size_t sz = bArray->data.size();
                    if (index >= 0 && index < sz) {
                        bArray->data[index] = v;
                        return false;
                    }
                }
                if (key == "ToCHeaderFile") {
                    std::string name = "NONAME";
                    GetArgument(args, 0, name);

                    cse.back()->Push(
                        gc, Tesses::Framework::Text::GenerateCHeaderFile(
                                bArray->data, name));
                    return false;
                }
                if (key == "CopyTo") {
                    // CopyTo(destBuff, offsetSrc, offsetDest, length)

                    if (args.size() != 4) {
                        throw VMException(
                            "ByteArray.CopyTo must only accept 4 arguments");
                    }

                    TByteArray *bArray2;
                    int64_t offsetSrc;
                    int64_t offsetDest;
                    int64_t count;
                    if (!GetArgumentHeap<TByteArray *>(args, 0, bArray2)) {
                        throw VMException("ByteArray.CopyTo first argument "
                                          "must be a ByteArray");
                    }
                    if (!GetArgument<int64_t>(args, 1, offsetSrc)) {
                        throw VMException("ByteArray.CopyTo second argument "
                                          "must be a Long (offsetSrc)");
                    }
                    if (!GetArgument<int64_t>(args, 2, offsetDest)) {
                        throw VMException("ByteArray.CopyTo third argument "
                                          "must be a Long (offsetDest)");
                    }
                    if (!GetArgument<int64_t>(args, 3, count)) {
                        throw VMException("ByteArray.CopyTo fourth argument "
                                          "must be a Long (length)");
                    }
                    size_t offsrc = (size_t)offsetSrc;
                    size_t offdest = (size_t)offsetDest;
                    size_t len = (size_t)count;

                    if (offsrc > bArray->data.size()) {
                        offsrc = bArray->data.size();
                    }

                    if (offdest > bArray2->data.size()) {
                        offdest = bArray2->data.size();
                    }

                    len = std::min<size_t>(
                        std::min<size_t>(bArray->data.size() - offsrc,
                                         bArray2->data.size() - offdest),
                        len);

                    if (len > 0)
                        memcpy(bArray2->data.data() + offdest,
                               bArray->data.data() + offsrc, len);
                    cse.back()->Push(gc, bArray2);
                    return false;
                }
                if (key == "Insert") {
                    if (args.size() != 4) {
                        throw VMException(
                            "ByteArray.Insert must only accept 4 arguments");
                    }
                    // Insert(offsetDest, buffer, offsetSrc, length)
                    TByteArray *bArray2;
                    int64_t offsetSrc;
                    int64_t offsetDest;
                    int64_t count;
                    if (!GetArgumentHeap<TByteArray *>(args, 1, bArray2)) {
                        throw VMException("ByteArray.Insert second argument "
                                          "must be a ByteArray");
                    }
                    if (!GetArgument<int64_t>(args, 0, offsetSrc)) {
                        throw VMException("ByteArray.Insert first argument "
                                          "must be a Long (offsetSrc)");
                    }
                    if (!GetArgument<int64_t>(args, 2, offsetDest)) {
                        throw VMException("ByteArray.Insert third argument "
                                          "must be a Long (offsetDest)");
                    }
                    if (!GetArgument<int64_t>(args, 3, count)) {
                        throw VMException("ByteArray.Insert fourth argument "
                                          "must be a Long (length)");
                    }

                    size_t offsrc = (size_t)offsetSrc;
                    size_t offdest = (size_t)offsetDest;
                    size_t len = (size_t)count;

                    if (offsrc > bArray->data.size()) {
                        offsrc = bArray->data.size();
                    }
                    if (offdest > bArray->data.size()) {
                        offdest = bArray->data.size();
                    }

                    len = std::min(len, bArray2->data.size() - offsrc);

                    bArray->data.insert(bArray->data.begin() + offdest,
                                        bArray2->data.begin() + offsrc,
                                        bArray2->data.begin() + offsrc + len);
                    cse.back()->Push(gc, bArray);
                    return false;
                }
                if (key == "Append") {
                    if (args.size() != 3) {
                        throw VMException(
                            "ByteArray.Append must only accept 3 arguments");
                    }
                    TByteArray *bArray2;
                    int64_t offset;
                    int64_t count;
                    if (!GetArgumentHeap<TByteArray *>(args, 0, bArray2)) {
                        throw VMException("ByteArray.Append first argument "
                                          "must be a ByteArray");
                    }
                    if (!GetArgument<int64_t>(args, 1, offset)) {
                        throw VMException("ByteArray.Append second argument "
                                          "must be a Long (offset)");
                    }
                    if (!GetArgument<int64_t>(args, 2, count)) {
                        throw VMException("ByteArray.Append third argument "
                                          "must be a Long (length)");
                    }
                    size_t off = (size_t)offset;
                    size_t len = (size_t)count;

                    if (off > bArray->data.size()) {
                        off = bArray->data.size();
                    }

                    len = std::min<size_t>(bArray->data.size() - off, len);
                    bArray->data.insert(bArray->data.end(),
                                        bArray2->data.begin() + off,
                                        bArray2->data.begin() + off + len);
                    cse.back()->Push(gc, bArray);
                    return false;
                }

                if (key == "Resize") {
                    if (args.size() != 1) {
                        throw VMException(
                            "ByteArray.Resize must only accept one argument");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException(
                            "ByteArray.Resize must only accept a long");
                    }

                    size_t len = (size_t)std::get<int64_t>(args[0]);
                    if (len >= 0) {
                        bArray->data.resize(len);
                    }
                    cse.back()->Push(gc, bArray);
                    return false;
                }
                if (key == "SetU16BE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 2 <= bArray->data.size()) {
                        uint16_t number = (uint16_t)value;
                        BitConverter::FromUint16BE(bArray->data[(size_t)index],
                                                   number);
                        cse.back()->Push(gc, (int64_t)value);
                        return false;
                    }
                }
                if (key == "SetU32BE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        uint32_t number = (uint32_t)value;
                        BitConverter::FromUint32BE(bArray->data[(size_t)index],
                                                   number);
                        cse.back()->Push(gc, (int64_t)value);
                        return false;
                    }
                }
                if (key == "SetU64BE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        uint64_t number = 0;
                        memcpy(&number, &value, sizeof(int64_t));
                        BitConverter::FromUint64BE(bArray->data[(size_t)index],
                                                   number);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "SetU16LE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 2 <= bArray->data.size()) {
                        uint16_t number = (uint16_t)value;
                        BitConverter::FromUint16LE(bArray->data[(size_t)index],
                                                   number);
                        cse.back()->Push(gc, (int64_t)value);
                        return false;
                    }
                }
                if (key == "SetU32LE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        uint32_t number = (uint32_t)value;
                        BitConverter::FromUint32LE(bArray->data[(size_t)index],
                                                   number);
                        cse.back()->Push(gc, (int64_t)value);
                        return false;
                    }
                }
                if (key == "SetU64LE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        uint64_t number = 0;
                        memcpy(&number, &value, sizeof(int64_t));
                        BitConverter::FromUint64LE(bArray->data[(size_t)index],
                                                   number);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "SetU16BE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 2 <= bArray->data.size()) {
                        BitConverter::FromSint16BE(bArray->data[(size_t)index],
                                                   (int16_t)value);
                        cse.back()->Push(gc, (int64_t)(int16_t)value);
                        return false;
                    }
                }
                if (key == "SetS32BE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        BitConverter::FromSint32BE(bArray->data[(size_t)index],
                                                   (int32_t)value);
                        cse.back()->Push(gc, (int64_t)(int32_t)value);
                        return false;
                    }
                }
                if (key == "SetS64BE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        BitConverter::FromSint64BE(bArray->data[(size_t)index],
                                                   value);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "SetS16LE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 2 <= bArray->data.size()) {
                        BitConverter::FromSint16LE(bArray->data[(size_t)index],
                                                   (int16_t)value);
                        cse.back()->Push(gc, (int64_t)(int16_t)value);
                        return false;
                    }
                }
                if (key == "SetS32LE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        BitConverter::FromSint32LE(bArray->data[(size_t)index],
                                                   (int32_t)value);
                        cse.back()->Push(gc, (int64_t)(int32_t)value);
                        return false;
                    }
                }
                if (key == "SetS64LE") {
                    int64_t index;
                    int64_t value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        BitConverter::FromSint64LE(bArray->data[(size_t)index],
                                                   value);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "SetF32LE") {
                    int64_t index;
                    double value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        BitConverter::FromFloatLE(bArray->data[(size_t)index],
                                                  (float)value);
                        cse.back()->Push(gc, (double)(float)value);
                        return false;
                    }
                }
                if (key == "SetF64LE") {
                    int64_t index;
                    double value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        BitConverter::FromDoubleLE(bArray->data[(size_t)index],
                                                   value);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "SetF32BE") {
                    int64_t index;
                    double value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        BitConverter::FromFloatBE(bArray->data[(size_t)index],
                                                  (float)value);
                        cse.back()->Push(gc, (double)(float)value);
                        return false;
                    }
                }
                if (key == "SetF64BE") {
                    int64_t index;
                    double value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        BitConverter::FromDoubleBE(bArray->data[(size_t)index],
                                                   value);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "SetUuid") {
                    int64_t index;
                    Tesses::Framework::Uuid value;
                    if (GetArgument(args, 0, index) &&
                        GetArgument(args, 1, value) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 16 <= bArray->data.size()) {
                        BitConverter::FromUuid(bArray->data[(size_t)index],
                                               value);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }

                if (key == "GetU16BE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 2 <= bArray->data.size()) {
                        int64_t value = (int64_t)BitConverter::ToUint16BE(
                            bArray->data[(size_t)index]);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetU32BE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        int64_t value = (int64_t)BitConverter::ToUint32BE(
                            bArray->data[(size_t)index]);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetU64BE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        uint64_t value = BitConverter::ToUint64BE(
                            bArray->data[(size_t)index]);
                        int64_t value2 = 0;
                        memcpy(&value2, &value, sizeof(uint64_t));
                        cse.back()->Push(gc, value2);
                        return false;
                    }
                }
                if (key == "GetU16LE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 2 <= bArray->data.size()) {
                        int64_t value = (int64_t)BitConverter::ToUint16LE(
                            bArray->data[(size_t)index]);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetU32LE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        int64_t value = (int64_t)BitConverter::ToUint32LE(
                            bArray->data[(size_t)index]);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetU64LE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        uint64_t value = BitConverter::ToUint64LE(
                            bArray->data[(size_t)index]);
                        int64_t value2 = 0;
                        memcpy(&value2, &value, sizeof(uint64_t));
                        cse.back()->Push(gc, value2);
                        return false;
                    }
                }

                if (key == "GetS16BE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 2 <= bArray->data.size()) {
                        int64_t value = (int64_t)BitConverter::ToSint16BE(
                            bArray->data[(size_t)index]);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetS32BE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        int64_t value = (int64_t)BitConverter::ToSint32BE(
                            bArray->data[(size_t)index]);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetS64BE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        int64_t value = BitConverter::ToSint64BE(
                            bArray->data[(size_t)index]);

                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetS16LE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 2 <= bArray->data.size()) {
                        int64_t value = (int64_t)BitConverter::ToSint16LE(
                            bArray->data[(size_t)index]);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetS32LE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        int64_t value = (int64_t)BitConverter::ToSint32LE(
                            bArray->data[(size_t)index]);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetS64LE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        int64_t value = BitConverter::ToSint64LE(
                            bArray->data[(size_t)index]);

                        cse.back()->Push(gc, value);
                        return false;
                    }
                }

                if (key == "GetF32LE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        double value = (double)BitConverter::ToFloatLE(
                            bArray->data[(size_t)index]);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetF64LE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        double value = BitConverter::ToDoubleLE(
                            bArray->data[(size_t)index]);

                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetF32BE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 4 <= bArray->data.size()) {
                        double value = (double)BitConverter::ToFloatBE(
                            bArray->data[(size_t)index]);
                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                if (key == "GetF64BE") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 8 <= bArray->data.size()) {
                        double value = BitConverter::ToDoubleBE(
                            bArray->data[(size_t)index]);

                        cse.back()->Push(gc, value);
                        return false;
                    }
                }

                if (key == "GetUuid") {
                    int64_t index;
                    if (GetArgument(args, 0, index) &&
                        (size_t)index < bArray->data.size() &&
                        (size_t)index + 16 <= bArray->data.size()) {
                        auto value =
                            BitConverter::ToUuid(bArray->data[(size_t)index]);

                        cse.back()->Push(gc, value);
                        return false;
                    }
                }
                cse.back()->Push(gc, Undefined());
                return false;
            } else if (aArray != nullptr) {
                if (key == "ToString") {
                    cse.back()->Push(gc, ToString(gc, aArray));
                    return false;
                } else if (key == "GetAt") {
                    if (args.size() != 1) {
                        throw VMException(
                            "AArray.GetAt must only accept one argument");
                    }
                    cse.back()->Push(gc, aArray->Get(gc, args[0]));
                    return false;
                } else if (key == "SetAt") {
                    if (args.size() != 2) {
                        throw VMException(
                            "AArray.SetAt must only accept two arguments");
                    }
                    aArray->Set(gc, args[0], args[1]);
                    cse.back()->Push(gc, args[1]);
                    return false;
                } else if (key == "GetKey") {
                    if (args.size() != 1) {
                        throw VMException(
                            "AArray.GetKey must only accept one argument");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException(
                            "AArray.GetKey must only accept a long");
                    }

                    gc->BarrierBegin();
                    auto res = aArray->GetKey(std::get<int64_t>(args[0]));
                    gc->BarrierEnd();
                    cse.back()->Push(gc, res);
                    return false;
                } else if (key == "GetValue") {
                    if (args.size() != 1) {
                        throw VMException(
                            "AArray.GetValue must only accept one argument");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException(
                            "AArray.GetValue must only accept a long");
                    }

                    gc->BarrierBegin();
                    auto res = aArray->GetValue(std::get<int64_t>(args[0]));
                    gc->BarrierEnd();
                    cse.back()->Push(gc, res);
                    return false;
                } else if (key == "SetKey") {
                    if (args.size() != 2) {
                        throw VMException(
                            "AArray.SetKey must only accept two arguments");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException("AArray.SetKey first argument must "
                                          "only accept a long");
                    }

                    gc->BarrierBegin();
                    aArray->SetKey(std::get<int64_t>(args[0]), args[1]);
                    gc->BarrierEnd();
                    cse.back()->Push(gc, args[1]);
                    return false;
                } else if (key == "SetValue") {
                    if (args.size() != 2) {
                        throw VMException(
                            "AArray.SetValue must only accept two arguments");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException("AArray.SetValue first argument must "
                                          "only accept a long");
                    }

                    gc->BarrierBegin();
                    aArray->SetValue(std::get<int64_t>(args[0]), args[1]);
                    gc->BarrierEnd();
                    cse.back()->Push(gc, args[1]);
                    return false;
                } else if (key == "Count" || key == "Length") {
                    gc->BarrierBegin();
                    cse.back()->Push(gc, aArray->Count());
                    gc->BarrierEnd();
                    return false;
                } else if (key == "GetEnumerator") {

                    cse.back()->Push(
                        gc, TAssociativeArrayEnumerator::Create(ls, aArray));
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            } else if (list != nullptr) {

                if (key == "GetEnumerator") {
                    cse.back()->Push(gc, TListEnumerator::Create(ls, list));
                    return false;
                } else if (key == "ToString") {

                    cse.back()->Push(gc, Json_Encode(list));
                    return false;

                } else if (key == "Insert") {
                    if (args.size() != 2) {
                        throw VMException(
                            "List.Insert must only accept two arguments");
                    }
                    int64_t index;

                    if (!GetArgument(args, 0, index)) {
                        throw VMException(
                            "List.Insert first argument must be Long");
                    }

                    gc->BarrierBegin();
                    list->Insert(index, args[1]);
                    gc->BarrierEnd();
                    cse.back()->Push(gc, Undefined());
                    return false;
                } else if (key == "Add") {
                    if (args.size() != 1) {
                        throw VMException(
                            "List.Add must only accept one argument");
                    }
                    gc->BarrierBegin();
                    list->Add(args[0]);
                    gc->BarrierEnd();
                    cse.back()->Push(gc, Undefined());
                    return false;
                } else if (key == "Contains") {
                    if (args.size() != 1) {
                        throw VMException(
                            "List.Contains must only accept one argument");
                    }
                    gc->BarrierBegin();
                    for (int64_t i = 0; i < list->Count(); i++) {
                        auto item = list->Get(i);
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
                } else if (key == "IndexOf") {
                    // IndexOf(obj, $idx)
                    if (args.size() < 1 || args.size() > 2) {
                        throw VMException("List.IndexOf must either have one "
                                          "or two arguments");
                    }

                    int64_t i = 0;

                    GetArgument(args, 1, i);
                    gc->BarrierBegin();
                    for (; i < list->Count(); i++) {
                        auto item = list->Get(i);
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
                } else if (key == "RemoveAllEqual") {
                    if (args.size() != 1) {
                        throw VMException("List.RemoveAllEqual must only "
                                          "accept one argument");
                    }

                    gc->BarrierBegin();
                    for (int64_t i = 0; i < list->Count(); i++) {
                        auto item = list->Get(i);
                        gc->BarrierEnd();
                        if (Equals(gc, args[0], item)) {
                            gc->BarrierBegin();
                            list->RemoveAt(i);
                            i--;
                        } else
                            gc->BarrierBegin();
                    }
                    gc->BarrierEnd();
                    cse.back()->Push(gc, Undefined());
                    return false;
                } else if (key == "Remove") {
                    if (args.size() != 1) {
                        throw VMException(
                            "List.Remove must only accept one argument");
                    }

                    gc->BarrierBegin();
                    for (int64_t i = 0; i < list->Count(); i++) {
                        auto item = list->Get(i);
                        gc->BarrierEnd();
                        if (Equals(gc, args[0], item)) {
                            gc->BarrierBegin();
                            list->RemoveAt(i);
                            gc->BarrierEnd();
                            break;
                        }
                        gc->BarrierBegin();
                    }
                    gc->BarrierEnd();
                    cse.back()->Push(gc, Undefined());
                    return false;
                } else if (key == "RemoveAt") {
                    if (args.size() != 1) {
                        throw VMException(
                            "List.RemoveAt must only accept one argument");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException(
                            "List.RemoveAt must only accept a long");
                    }
                    gc->BarrierBegin();
                    list->RemoveAt(std::get<int64_t>(args[0]));
                    gc->BarrierEnd();
                    cse.back()->Push(gc, Undefined());
                    return false;
                } else if (key == "Clear") {
                    gc->BarrierBegin();
                    list->Clear();
                    gc->BarrierEnd();
                    cse.back()->Push(gc, Undefined());
                    return false;
                } else if (key == "GetAt") {
                    if (args.size() != 1) {
                        throw VMException(
                            "List.GetAt must only accept one argument");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException("List.GetAt must only accept a long");
                    }

                    int64_t index = std::get<int64_t>(args[0]);
                    if (index >= 0 && index < list->Count()) {
                        cse.back()->Push(gc, list->Get(index));
                        return false;
                    }

                } else if (key == "SetAt") {
                    if (args.size() != 2) {
                        throw VMException(
                            "List.SetAt must only accept two arguments");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException("List.SetAt first argument must only "
                                          "accept a long");
                    }

                    int64_t index = std::get<int64_t>(args[0]);
                    if (index >= 0 && index < list->Count()) {
                        list->Set(index, args[1]);
                        return false;
                    }

                }

                else if (key == "Count" || key == "Length") {
                    gc->BarrierBegin();
                    cse.back()->Push(gc, list->Count());
                    gc->BarrierEnd();
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            } else if (dynList != nullptr) {

                if (key == "GetEnumerator") {
                    cse.back()->Push(
                        gc, TDynamicListEnumerator::Create(ls, dynList));
                    return false;
                } else if (key == "ToString") {
                    cse.back()->Push(gc, dynList->ToString(ls));
                    return false;
                } else if (key == "Insert") {
                    if (args.size() != 2) {
                        throw VMException(
                            "List.Insert must only accept two arguments");
                    }
                    int64_t index;

                    if (!GetArgument(args, 0, index)) {
                        throw VMException(
                            "List.Insert first argument must be Long");
                    }

                    cse.back()->Push(gc, dynList->Insert(ls, index, args[0]));
                    return false;
                }
                if (key == "Add") {
                    if (args.size() != 1) {
                        throw VMException(
                            "List.Add must only accept one argument");
                    }

                    cse.back()->Push(gc, dynList->Add(ls, args[0]));
                    return false;
                }
                if (key == "RemoveAllEqual") {
                    if (args.size() != 1) {
                        throw VMException("List.RemoveAllEqual must only "
                                          "accept one argument");
                    }

                    cse.back()->Push(gc, dynList->RemoveAllEqual(ls, args[0]));
                    return false;
                }
                if (key == "Remove") {
                    if (args.size() != 1) {
                        throw VMException(
                            "List.Remove must only accept one argument");
                    }

                    cse.back()->Push(gc, dynList->Remove(ls, args[0]));
                    return false;
                }
                if (key == "RemoveAt") {
                    if (args.size() != 1) {
                        throw VMException(
                            "List.RemoveAt must only accept one argument");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException(
                            "List.RemoveAt must only accept a long");
                    }
                    cse.back()->Push(
                        gc, dynList->RemoveAt(ls, std::get<int64_t>(args[0])));

                    return false;
                }
                if (key == "Clear") {

                    cse.back()->Push(gc, dynList->Clear(ls));
                    return false;
                }
                if (key == "GetAt") {
                    if (args.size() != 1) {
                        throw VMException(
                            "List.GetAt must only accept one argument");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException("List.GetAt must only accept a long");
                    }

                    int64_t index = std::get<int64_t>(args[0]);
                    if (index >= 0) {
                        cse.back()->Push(gc, dynList->GetAt(ls, index));
                        return false;
                    }
                }
                if (key == "SetAt") {
                    if (args.size() != 2) {
                        throw VMException(
                            "List.SetAt must only accept two arguments");
                    }

                    if (!std::holds_alternative<int64_t>(args[0])) {
                        throw VMException("List.SetAt first argument must only "
                                          "accept a long");
                    }

                    int64_t index = std::get<int64_t>(args[0]);
                    if (index >= 0) {
                        cse.back()->Push(gc,
                                         dynList->SetAt(ls, index, args[1]));
                        return false;
                    }
                }

                if (key == "Count" || key == "Length") {
                    cse.back()->Push(gc, dynList->Count(ls));

                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            } else if (cls != nullptr) {
                gc->BarrierBegin();
                auto obj = cls->GetValue(cse.back()->callable->className, key);
                gc->BarrierEnd();
                TClosure *clos;
                TCallable *callable;
                if (GetObjectHeap(obj, clos)) {
                    this->AddCallStackEntry(ls, clos, args);
                    return true;
                } else if (GetObjectHeap(obj, callable)) {
                    cse.back()->Push(gc, callable->Call(ls, args));
                    return false;
                }
                cse.back()->Push(gc, Undefined());
                return false;
            } else if (dict != nullptr) {
                if (key == "ToString" && !dict->MethodExists(ls, key) &&
                    args.empty()) {
                    cse.back()->Push(gc, Json_Encode(dict));
                    return false;
                }
                gc->BarrierBegin();
                auto o = dict->GetValue(key);
                gc->BarrierEnd();

                return InvokeMethod(ls, o, dict, args);
            } else if (dynDict != nullptr) {

                cse.back()->Push(gc, dynDict->CallMethod(ls, key, args));

                return false;
            } else if (callable != nullptr) {
                if (key == "Call") {
                    TList *argls;
                    if (GetArgumentHeap(args, 0, argls)) {
                        TClosure *clo = dynamic_cast<TClosure *>(callable);
                        if (clo != nullptr) {
                            AddCallStackEntry(ls, clo, argls->items);
                            return true;
                        } else {
                            cse.back()->Push(gc,
                                             callable->Call(ls, argls->items));
                            return false;
                        }
                    }
                }
                cse.back()->Push(gc, nullptr);
                return false;
            } else {
                cse.back()->Push(gc, Undefined());
            }
        } else {
            cse.back()->Push(gc, Undefined());
        }
    }

    return false;
}
} // namespace Tesses::CrossLang