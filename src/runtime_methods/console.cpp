#ifndef CROSSLANG_CUSTOM_CONSOLE
#include "CrossLang.hpp"
#include <iostream>

namespace Tesses::CrossLang {

TObject Console_getIsTTY(GCList &ls, std::vector<TObject> args) {

    return Tesses::Framework::Console::IsTTY();
}
TObject Console_getEcho(GCList &ls, std::vector<TObject> args) {

    return Tesses::Framework::Console::GetEcho();
}
TObject Console_setEcho(GCList &ls, std::vector<TObject> args) {
    if (args.size() == 1 && std::holds_alternative<bool>(args[0])) {
        bool cooked = std::get<bool>(args[0]);
        Tesses::Framework::Console::SetEcho(cooked);
        return cooked;
    }
    return Undefined();
}
TObject Console_getCanonical(GCList &ls, std::vector<TObject> args) {
    return Tesses::Framework::Console::GetCanonical();
}
TObject Console_setCanonical(GCList &ls, std::vector<TObject> args) {
    if (args.size() == 1 && std::holds_alternative<bool>(args[0])) {
        bool cooked = std::get<bool>(args[0]);
        Tesses::Framework::Console::SetCanonical(cooked);
        return cooked;
    }
    return Undefined();
}
TObject Console_getSignals(GCList &ls, std::vector<TObject> args) {
    return Tesses::Framework::Console::GetSignals();
}
TObject Console_setSignals(GCList &ls, std::vector<TObject> args) {
    if (args.size() == 1 && std::holds_alternative<bool>(args[0])) {
        bool cooked = std::get<bool>(args[0]);
        Tesses::Framework::Console::SetSignals(cooked);
        return cooked;
    }
    return Undefined();
}

TObject Console_Read(GCList &ls, std::vector<TObject> args) {
    return (int64_t)Tesses::Framework::Console::Read();
}
TObject Console_ReadLine(GCList &ls, std::vector<TObject> args) {
    return Tesses::Framework::Console::ReadLine();
}
TObject Console_ReadPassword(GCList &ls, std::vector<TObject> args) {
    return Tesses::Framework::Console::ReadPassword();
}
TObject Console_Write(GCList &ls, std::vector<TObject> args) {
    if (args.size() < 1) {
        return Undefined();
    }
    Tesses::Framework::Console::Write(ToString(ls.GetGC(), args[0]));
    return Undefined();
}
TObject Console_Fatal(GCList &ls, std::vector<TObject> args) {
    if (args.size() < 1) {
        Tesses::Framework::Console::ErrorLine("FATAL: <NO MESSAGE>");
        exit(1);
    }
    Tesses::Framework::Console::ErrorLine("FATAL: " +
                                          ToString(ls.GetGC(), args[0]));
    exit(1);
}
TObject Console_WriteLine(GCList &ls, std::vector<TObject> args) {
    if (args.size() < 1) {
        Tesses::Framework::Console::WriteLineView("");
        return Undefined();
    }
    Tesses::Framework::Console::WriteLine(ToString(ls.GetGC(), args[0]));

    return Undefined();
}
TObject Console_Error(GCList &ls, std::vector<TObject> args) {
    if (args.size() < 1) {
        return Undefined();
    }
    Tesses::Framework::Console::Error(ToString(ls.GetGC(), args[0]));
    return Undefined();
}
TObject Console_ErrorLine(GCList &ls, std::vector<TObject> args) {
    if (args.size() < 1) {
        Tesses::Framework::Console::ErrorLine("");
        return Undefined();
    }
    Tesses::Framework::Console::ErrorLine(ToString(ls.GetGC(), args[0]));

    return Undefined();
}
TObject Console_getIn(GCList &ls, std::vector<TObject> args) {
    return std::make_shared<Tesses::Framework::Streams::FileStream>(
        stdin, false, "r", false);
}

TObject Console_getOut(GCList &ls, std::vector<TObject> args) {
    return std::make_shared<Tesses::Framework::Streams::FileStream>(
        stdout, false, "w", false);
}
TObject Console_getError(GCList &ls, std::vector<TObject> args) {
    return std::make_shared<Tesses::Framework::Streams::FileStream>(
        stderr, false, "w", false);
}
TObject Console_Clear(GCList &ls, std::vector<TObject> args) {
    Tesses::Framework::Console::Clear();
    return Undefined();
}

TObject Console_ProgressBar(GCList &ls, std::vector<TObject> args) {
    double dbl;
    int64_t i64;
    if (GetArgument(args, 0, i64)) {
        Tesses::Framework::Console::ProgressBar((int)i64);
    } else if (GetArgument(args, 0, dbl)) {
        Tesses::Framework::Console::ProgressBar(dbl);
    }

    return Undefined();
}
TObject Console_getSize(GCList &ls, std::vector<TObject> args) {
    auto sz = Tesses::Framework::Console::GetSize();
    TDictionary *dict =
        TDictionary::Create(ls, {TDItem("Width", (int64_t)sz.first),
                                 TDItem("Height", (int64_t)sz.second)});

    return dict;
}
TObject Console_List(GCList &ls, std::vector<TObject> args) {
    if (!args.empty()) {
        GCList ls2(ls.GetGC());
        auto enumerator = TEnumerator::CreateFromObject(ls2, args[0]);
        std::vector<std::string> items;

        while (enumerator->MoveNext(ls2.GetGC())) {
            auto item = enumerator->GetCurrent(ls2);
            items.push_back(ToString(ls2.GetGC(), item));
        }

        return (int64_t)Tesses::Framework::Console::List(items);
    }
    return 0;
}
void TStd::RegisterConsole(std::shared_ptr<GC> gc, TRootEnvironment *env) {
    env->permissions.canRegisterConsole = true;
    if (env->permissions.customConsole != nullptr) {
        gc->BarrierBegin();
        env->DeclareVariable("Console", env->permissions.customConsole);
        gc->BarrierEnd();
        return;
    }

    GCList ls(gc);
    TDictionary *dict = TDictionary::Create(ls);
    dict->DeclareFunction(
        gc, "getIsTTY",
        "Get whether terminal is a terminal or just piped to file", {},
        Console_getIsTTY);
    dict->DeclareFunction(gc, "getEcho",
                          "Get whether terminal is echoing characters read", {},
                          Console_getEcho);
    dict->DeclareFunction(gc, "setEcho",
                          "Set whether terminal is echoing characters read",
                          {"flag"}, Console_setEcho);
    dict->DeclareFunction(gc, "getCanonical",
                          "Get whether terminal is buffering line by line "
                          "(true) or byte by byte (false)",
                          {}, Console_getCanonical);
    dict->DeclareFunction(gc, "setCanonical",
                          "Set whether terminal is buffering line by line "
                          "(true) or byte by byte (false)",
                          {"flag"}, Console_setCanonical);
    dict->DeclareFunction(gc, "getSignals",
                          "Get whether terminal is sending signals for CTRL+C "
                          "(true) or via read (false)",
                          {}, Console_getSignals);
    dict->DeclareFunction(gc, "setSignals",
                          "Set whether terminal is sending signals for CTRL+C "
                          "(true) or via read (false)",
                          {"flag"}, Console_setSignals);
    dict->DeclareFunction(gc, "Clear", "Clear the console", {}, Console_Clear);
    dict->DeclareFunction(gc, "Read", "Reads a byte from stdin", {},
                          Console_Read);
    dict->DeclareFunction(gc, "ReadLine", "Reads line from stdin", {},
                          Console_ReadLine);
    dict->DeclareFunction(gc, "ReadPassword", "Reads password from stdin", {},
                          Console_ReadPassword);
    dict->DeclareFunction(gc, "Write", "Write text \"text\" to stdout",
                          {"text"}, Console_Write);
    dict->DeclareFunction(gc, "WriteLine",
                          "Write text \"text\" to stdout with new line",
                          {"$text"}, Console_WriteLine);
    dict->DeclareFunction(gc, "Error", "Write text \"error\" to stderr",
                          {"error"}, Console_Error);
    dict->DeclareFunction(gc, "ErrorLine", "Write text \"error\" to stderr",
                          {"$error"}, Console_ErrorLine);
    dict->DeclareFunction(gc, "ProgressBar", "Draw progressbar", {},
                          Console_ProgressBar);
    dict->DeclareFunction(gc, "List", "Draw list", {"itemsitterator"},
                          Console_List);
    if (env->permissions.canRegisterEverything)
        dict->DeclareFunction(gc, "Fatal",
                              "Stop the program with an optional error message",
                              {"$text"}, Console_Fatal);
    dict->DeclareFunction(gc, "getIn", "Get stdin Stream", {}, Console_getIn);
    dict->DeclareFunction(gc, "getOut", "Get stdout Stream", {},
                          Console_getOut);
    dict->DeclareFunction(gc, "getError", "Get stderr Stream", {},
                          Console_getError);
    dict->DeclareFunction(gc, "getSize", "Get console size", {},
                          Console_getSize);
    gc->BarrierBegin();
    env->DeclareVariable("Console", dict);
    auto _new = env->EnsureDictionary(gc, "New");
    _new->DeclareFunction(
        gc, "ConsoleReader", "Read from console", {},
        [](GCList &ls, std::vector<TObject> args) -> TObject {
            return std::make_shared<
                Tesses::Framework::TextStreams::ConsoleReader>();
        });
    _new->DeclareFunction(
        gc, "ConsoleWriter", "Write to console", {"$isStderr"},
        [](GCList &ls, std::vector<TObject> args) -> TObject {
            bool err;
            if (GetArgument(args, 0, err))
                return std::make_shared<
                    Tesses::Framework::TextStreams::ConsoleWriter>(err);
            return std::make_shared<
                Tesses::Framework::TextStreams::ConsoleWriter>();
        });
    gc->BarrierEnd();
}
} // namespace Tesses::CrossLang
#endif
