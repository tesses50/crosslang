#ifndef CROSSLANG_CUSTOM_CONSOLE
#include "CrossLang.hpp"
#include <iostream>

#if defined(GEKKO) || defined(__SWITCH__) || defined(_WIN32)
#undef CROSSLANG_ENABLE_TERMIOS
#endif

#ifdef CROSSLANG_ENABLE_TERMIOS
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif
namespace Tesses::CrossLang {
    #ifdef CROSSLANG_ENABLE_TERMIOS
    struct termios orig_termios;
    static void disableRawMode()
    {
        tcsetattr(0, TCSAFLUSH, &orig_termios);
    }
    #endif
    TObject Console_getEcho(GCList& ls, std::vector<TObject> args)
    {
        #ifdef CROSSLANG_ENABLE_TERMIOS
            struct termios raw;
            tcgetattr(0, &raw);
            return (raw.c_lflag & ECHO) > 0;
        
        #endif
        return false;
    }
    TObject Console_setEcho(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 1 && std::holds_alternative<bool>(args[0]))
        {
            bool cooked = std::get<bool>(args[0]);
            #ifdef CROSSLANG_ENABLE_TERMIOS
                struct termios raw;
                tcgetattr(0, &raw);
                if(cooked)
                {
                    raw.c_lflag |= ECHO;
                }
                else
                {
                    raw.c_lflag &= ~(ECHO);
                }
        
                  tcsetattr(0, TCSAFLUSH, &raw);

            #endif
            return cooked;
        }
        return Undefined();
    }
    TObject Console_getCanonical(GCList& ls, std::vector<TObject> args)
    {
        #ifdef CROSSLANG_ENABLE_TERMIOS
            struct termios raw;
            tcgetattr(0, &raw);
            return (raw.c_lflag & ICANON) > 0;
        
        #endif
        return false;
    }
    TObject Console_setCanonical(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 1 && std::holds_alternative<bool>(args[0]))
        {
            bool cooked = std::get<bool>(args[0]);
            #ifdef CROSSLANG_ENABLE_TERMIOS
                struct termios raw;
                tcgetattr(0, &raw);
                if(cooked)
                {
                    raw.c_lflag |= ICANON;
                }
                else
                {
                    raw.c_lflag &= ~(ICANON);
                }
        
                  tcsetattr(0, TCSAFLUSH, &raw);

            #endif
            return cooked;
        }
        return Undefined();
    }
    TObject Console_getSignals(GCList& ls, std::vector<TObject> args)
    {
        #ifdef CROSSLANG_ENABLE_TERMIOS
            struct termios raw;
            tcgetattr(0, &raw);
            return (raw.c_lflag & ISIG) > 0;
        
        #endif
        return false;
    }
    TObject Console_setSignals(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 1 && std::holds_alternative<bool>(args[0]))
        {
            bool cooked = std::get<bool>(args[0]);
            #ifdef CROSSLANG_ENABLE_TERMIOS
                struct termios raw;
                tcgetattr(0, &raw);
                if(cooked)
                {
                    raw.c_lflag |= ISIG;
                }
                else
                {
                    raw.c_lflag &= ~(ISIG);
                }
        
                  tcsetattr(0, TCSAFLUSH, &raw);

            #endif
            return cooked;
        }
        return Undefined();
    }
  
    TObject Console_Read(GCList& ls, std::vector<TObject> args)
    {
        uint8_t byte;
        std::cin.read((char*)&byte,1);
        return std::cin.eof() ? (int64_t)-1 : (int64_t)byte;
    }
    TObject Console_ReadLine(GCList& ls, std::vector<TObject> args)
    {
        std::string str;
        std::getline(std::cin,str);
        return str;
    }
    TObject Console_Write(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() < 1)
            {
                return Undefined();
            }
            std::cout << ToString(ls.GetGC(),args[0]);
            return Undefined();
    }
    TObject Console_Fatal(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() < 1)
        {
            std::cout << "FATAL: <NO MESSAGE>" << std::endl;
            exit(1);
        }
        std::cout << "FATAL: " <<  ToString(ls.GetGC(),args[0]) << std::endl;
        exit(1);
    }
    TObject Console_WriteLine(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() < 1)
            {
                std::cout << "\n";
                return Undefined();
            }
            std::cout << ToString(ls.GetGC(),args[0]) << "\n";
            return Undefined();
    }
    TObject Console_Error(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() < 1)
            {
                return Undefined();
            }
            std::cerr << ToString(ls.GetGC(),args[0]);
            return Undefined();
    }
    TObject Console_ErrorLine(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() < 1)
            {
                std::cout << "\n";
                return Undefined();
            }
            std::cerr << ToString(ls.GetGC(),args[0]) << "\n";
            return Undefined();
    }
    TObject Console_getIn(GCList& ls, std::vector<TObject> args)
    {
        return std::make_shared<Tesses::Framework::Streams::FileStream>(stdin,false,"r",false);
    }

    TObject Console_getOut(GCList& ls, std::vector<TObject> args)
    {
        return std::make_shared<Tesses::Framework::Streams::FileStream>(stdout,false,"w",false);
    }
    TObject Console_getError(GCList& ls, std::vector<TObject> args)
    {
        return std::make_shared<Tesses::Framework::Streams::FileStream>(stderr,false,"w",false);
    }
    TObject Console_Clear(GCList& ls, std::vector<TObject> args)
    {
        //because I just really want a clear function
        #if defined(_WIN32)
        system("cls"); 
        #else
        std::cout << "\x1b[2J\x1b[H" << std::flush; //because of wii and stuff (dont want to rely on clear command)
        #endif
        return Undefined();
    }
    static void con_sz(int& w, int& h)
    {
        w = 0;
        h = 0;
        

        #if defined(_WIN32)
            
        #elif defined(CROSSLANG_ENABLE_TERMIOS)
            if(!isatty(STDOUT_FILENO)) 
                return;
            struct winsize ws;
            if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
            {
                w = ws.ws_col;
                h = ws.ws_row;
            }
        #endif
    }

    TObject Console_ProgressBar(GCList& ls, std::vector<TObject> args)
    {
        //[===   ]  50%
        bool showBar = true;
        #if defined(CROSSLANG_ENABLE_TERMIOS)
            if(!isatty(STDOUT_FILENO)) showBar=false;
        #endif  
        int64_t progress = 0;
        double pdbl = 0;
        GetArgument(args,0,pdbl);
        if(GetArgument(args,0,progress)) pdbl = progress / 100.0;
        if(pdbl < 0) pdbl=0;
        if(pdbl > 1) pdbl=1;

        std::cout << "\r";
        if(showBar)
        {
            int w,h;
            con_sz(w,h);

            int totalBlocks = w - 10;
            if(totalBlocks > 0)
            {
                std::cout << "[\033[0;32m";
                int i;
                int off = pdbl * totalBlocks;
                for(int i = 0; i < totalBlocks; i++)
                {
                    if(i < off)
                    std::cout << "=";
                    else 
                    std::cout << " ";
                }

                std::cout << "\033[0m] ";
            }
        }

        std::cout << std::setw(3) << (int)(pdbl*100) << "%" << std::flush;
        return Undefined();
    }
    TObject Console_getSize(GCList& ls, std::vector<TObject> args)
    {
        #if defined(CROSSLANG_ENABLE_TERMIOS)
            if(!isatty(STDOUT_FILENO)) return nullptr;
        #endif  
        int w, h;
        con_sz(w,h);
        TDictionary* dict = TDictionary::Create(ls,{
            TDItem("Width", (int64_t)w),
            TDItem("Height",(int64_t)h)
        });

        return dict;
    }
    void TStd::RegisterConsole(std::shared_ptr<GC> gc,TRootEnvironment* env)
    {
         env->permissions.canRegisterConsole=true;
        if(env->permissions.customConsole != nullptr)
        {
            gc->BarrierBegin();
            env->DeclareVariable("Console", env->permissions.customConsole );
            gc->BarrierEnd();
            return;
        }
        
        #ifdef CROSSLANG_ENABLE_TERMIOS
            tcgetattr(0, &orig_termios);
            atexit(disableRawMode);   
        #endif
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        
        dict->DeclareFunction(gc,"getEcho","Get whether terminal is echoing characters read",{},Console_getEcho);
        dict->DeclareFunction(gc,"setEcho","Set whether terminal is echoing characters read",{"flag"},Console_setEcho);
        dict->DeclareFunction(gc,"getCanonical","Get whether terminal is buffering line by line (true) or byte by byte (false)",{},Console_getCanonical);
        dict->DeclareFunction(gc,"setCanonical","Set whether terminal is buffering line by line (true) or byte by byte (false)",{"flag"},Console_setCanonical);
        dict->DeclareFunction(gc,"getSignals","Get whether terminal is sending signals for CTRL+C (true) or via read (false)",{},Console_getSignals);
        dict->DeclareFunction(gc,"setSignals","Set whether terminal is sending signals for CTRL+C (true) or via read (false)",{"flag"},Console_setSignals);
        dict->DeclareFunction(gc,"Clear", "Clear the console",{},Console_Clear);
        dict->DeclareFunction(gc,"Read", "Reads a byte from stdin",{},Console_Read);
        dict->DeclareFunction(gc,"ReadLine","Reads line from stdin",{},Console_ReadLine);
        dict->DeclareFunction(gc,"Write","Write text \"text\" to stdout",{"text"},Console_Write);
        dict->DeclareFunction(gc,"WriteLine","Write text \"text\" to stdout with new line",{"$text"},Console_WriteLine);
        dict->DeclareFunction(gc,"Error", "Write text \"error\" to stderr",{"error"},Console_Error);
        dict->DeclareFunction(gc,"ErrorLine","Write text \"error\" to stderr",{"$error"},Console_ErrorLine);
        dict->DeclareFunction(gc,"ProgressBar","Draw progressbar", {}, Console_ProgressBar);
        if(env->permissions.canRegisterEverything)
            dict->DeclareFunction(gc,"Fatal","Stop the program with an optional error message",{"$text"},Console_Fatal);
        dict->DeclareFunction(gc,"getIn","Get stdin Stream",{},Console_getIn);
        dict->DeclareFunction(gc,"getOut","Get stdout Stream",{},Console_getOut);
        dict->DeclareFunction(gc,"getError", "Get stderr Stream",{},Console_getError);
        dict->DeclareFunction(gc, "getSize", "Get console size",{},Console_getSize);
        gc->BarrierBegin();
        env->DeclareVariable("Console", dict);
        auto _new = env->EnsureDictionary(gc,"New");
        _new->DeclareFunction(gc,"ConsoleReader","Read from console",{},[](GCList& ls,std::vector<TObject> args)->TObject {
            return std::make_shared<Tesses::Framework::TextStreams::ConsoleReader>();
        });
        _new->DeclareFunction(gc,"ConsoleWriter","Write to console",{"$isStderr"},[](GCList& ls,std::vector<TObject> args)->TObject {
            bool err;
            if(GetArgument(args,0,err)) return std::make_shared<Tesses::Framework::TextStreams::ConsoleWriter>(err);
            return std::make_shared<Tesses::Framework::TextStreams::ConsoleWriter>();
        });
        gc->BarrierEnd();
    }
}
#endif
