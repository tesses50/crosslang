#include "CrossLang.hpp"
#if defined(CROSSLANG_ENABLE_SDL2)
#include <SDL2/SDL.h>
#endif
namespace Tesses::CrossLang
{
    #if defined(CROSSLANG_ENABLE_SDL2)
    static void TObjectToRect(TDictionary* dict, SDL_Rect& rect)
    {
       
            int64_t number;
            auto obj = dict->GetValue("x");
            if(GetObject(obj,number)) rect.x = (int)number;
            obj = dict->GetValue("y");
            if(GetObject(obj,number)) rect.y = (int)number;
            obj = dict->GetValue("w");
            if(GetObject(obj,number)) rect.w = (int)number;
            obj = dict->GetValue("h");
            if(GetObject(obj,number)) rect.h = (int)number;
        
    }

    static TObject SDL2_RenderDrawRect(GCList& ls, std::vector<TObject> args)
    {
        TNative* nat;
        TDictionary* dict;
        if(GetArgumentHeap(args,0, nat)  && GetArgumentHeap(args,1,dict))
        {
            SDL_Rect rect;
            TObjectToRect(dict,rect);
            return (int64_t)SDL_RenderDrawRect((SDL_Renderer*)nat->GetPointer(), &rect);
        }
        return nullptr;
    }
    static TObject SDL2_RenderFillRect(GCList& ls, std::vector<TObject> args)
    {
        TNative* nat;
        TDictionary* dict;
        if(GetArgumentHeap(args,0, nat)  && GetArgumentHeap(args,1,dict))
        {
            SDL_Rect rect;
            TObjectToRect(dict,rect);
            return (int64_t)SDL_RenderFillRect((SDL_Renderer*)nat->GetPointer(), &rect);
        }
        return nullptr;
    }
    static TObject SDL2_PollEvent(GCList& ls, std::vector<TObject> args)
    {
        SDL_Event event;
        if(SDL_PollEvent(&event))
        {
            TDictionary* dict = TDictionary::Create(ls);
            ls.GetGC()->BarrierBegin();
            dict->SetValue("Type",(int64_t)event.common.type);
            dict->SetValue("Timestamp",(int64_t)event.common.timestamp);
            switch(event.type)
            {
                case SDL_DROPBEGIN:
                case SDL_DROPCOMPLETE:

                    dict->SetValue("WindowId",(int64_t)event.drop.windowID);
                    break;
                case SDL_DROPFILE:
                    dict->SetValue("File", Tesses::Framework::Filesystem::VFSPath(std::string(event.drop.file)));
                    SDL_free(event.drop.file);
                    break;
                case SDL_DROPTEXT:
                    dict->SetValue("WindowId",(int64_t)event.drop.windowID);
                    dict->SetValue("Text", std::string(event.drop.file));
                    SDL_free(event.drop.file);
                    break;
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                    dict->SetValue("WindowId",(int64_t)event.button.windowID);
                    dict->SetValue("Which",(int64_t)event.button.which);
                    dict->SetValue("Button",(int64_t)event.button.button);
                    dict->SetValue("State",(int64_t)event.button.state);
                    dict->SetValue("Clicks",(int64_t)event.button.clicks);
                    dict->SetValue("X",(int64_t)event.button.x);
                    dict->SetValue("Y",(int64_t)event.button.y);
                    break;
                case SDL_MOUSEMOTION:
                    dict->SetValue("WindowId",(int64_t)event.motion.windowID);
                    dict->SetValue("Which",(int64_t)event.motion.which);
                    dict->SetValue("State",(int64_t)event.motion.state);
                    dict->SetValue("X",(int64_t)event.motion.x);
                    dict->SetValue("Y",(int64_t)event.motion.y);
                    dict->SetValue("XRel",(int64_t)event.motion.xrel);
                    dict->SetValue("YRel",(int64_t)event.motion.yrel);
                    break;
                case SDL_MOUSEWHEEL:
                    dict->SetValue("Direction", (int64_t)event.wheel.direction);
                    dict->SetValue("MouseX",(int64_t)event.wheel.mouseX);
                    dict->SetValue("MouseY",(int64_t)event.wheel.mouseY);
                    dict->SetValue("PreciseX",(double)event.wheel.preciseX);
                    dict->SetValue("PreciseY",(double)event.wheel.preciseY);
                    dict->SetValue("Which",(int64_t)event.wheel.which);
                    dict->SetValue("WindowId",(int64_t)event.wheel.windowID);
                    dict->SetValue("X",(int64_t)event.wheel.x);
                    dict->SetValue("Y",(int64_t)event.wheel.y);
                    break;
                case SDL_KEYUP:
                case SDL_KEYDOWN:
                    dict->SetValue("WindowId",(int64_t)event.key.windowID);
                    dict->SetValue("Repeat",(int64_t)event.key.repeat);
                    dict->SetValue("State",(int64_t)event.key.state);
                    {
                        TDictionary* dict2=TDictionary::Create(ls);
                        dict2->SetValue("Mod",(int64_t)event.key.keysym.mod);
                        dict2->SetValue("Scancode",(int64_t)event.key.keysym.scancode);
                        dict2->SetValue("Sym",(int64_t)event.key.keysym.sym);
                        dict->SetValue("Keysym",dict2);
                    }
                    break;
                case SDL_EventType::SDL_FINGERMOTION:
                    dict->SetValue("Dx",event.tfinger.dx);
                    dict->SetValue("Dy",event.tfinger.dy);
                    //falls into SDL_FINGERUP/DOWN due to having same props
                case SDL_EventType::SDL_FINGERUP:
                case SDL_EventType::SDL_FINGERDOWN:
                    dict->SetValue("X",event.tfinger.x);
                    dict->SetValue("Y",event.tfinger.y);

                    dict->SetValue("FingerId",(int64_t)event.tfinger.fingerId);

                    dict->SetValue("Pressure",event.tfinger.pressure);

                    dict->SetValue("TouchId",(int64_t)event.tfinger.touchId);

                    dict->SetValue("WindowId",(int64_t)event.tfinger.windowID);
                    break;
                case SDL_EventType::SDL_TEXTINPUT:
                    dict->SetValue("Text",event.text.text);
                    dict->SetValue("WindowId",(int64_t)event.text.windowID);
                    break;
                case SDL_EventType::SDL_TEXTEDITING:
                    dict->SetValue("Text",event.edit.text);
                    dict->SetValue("Length",(int64_t)event.edit.length);
                    dict->SetValue("Start",(int64_t)event.edit.start);
                    dict->SetValue("WindowId",(int64_t)event.edit.windowID);
                    break;
                case SDL_EventType::SDL_TEXTEDITING_EXT:
                {
                    dict->SetValue("Text",event.editExt.text);
                    dict->SetValue("Length",(int64_t)event.editExt.length);
                    dict->SetValue("Start",(int64_t)event.editExt.start);
                    dict->SetValue("WindowId",(int64_t)event.editExt.windowID);
                    SDL_free(event.editExt.text);
                }
                    break;
                case SDL_EventType::SDL_WINDOWEVENT:
                {
                    dict->SetValue("WindowId",(int64_t)event.window.windowID);
                    dict->SetValue("Event",(int64_t)event.window.event);
                    dict->SetValue("Data1",(int64_t)event.window.data1);
                    dict->SetValue("Data2",(int64_t)event.window.data2);
                    
                }
                    break;
                
            }
            ls.GetGC()->BarrierEnd();
            return dict;
        }
        return nullptr;
    }
    static TObject SDL2_RenderPresent(GCList& ls, std::vector<TObject> args)
    {
        TNative* renderer;
        if(GetArgumentHeap<TNative*>(args,0,renderer) && !renderer->GetDestroyed())
        {
            SDL_RenderPresent((SDL_Renderer*)renderer->GetPointer());
        }
        return Undefined();
    }
    static TObject SDL2_RenderClear(GCList& ls, std::vector<TObject> args)
    {
        TNative* renderer;
        if(GetArgumentHeap<TNative*>(args,0,renderer) && !renderer->GetDestroyed())
        {
            return SDL_RenderClear((SDL_Renderer*)renderer->GetPointer())==0;
        }
        return Undefined();
    }
    static TObject SDL2_SetRenderDrawColor(GCList& ls, std::vector<TObject> args)
    {
        TNative* renderer;
        int64_t r;
        int64_t g;
        int64_t b;
        int64_t a;
        if(GetArgumentHeap<TNative*>(args,0,renderer) && !renderer->GetDestroyed() && GetArgument<int64_t>(args,1,r) && GetArgument<int64_t>(args,2,g) && GetArgument<int64_t>(args,3,b) && GetArgument<int64_t>(args,4,a))
        {
            return SDL_SetRenderDrawColor((SDL_Renderer*)renderer->GetPointer(),(Uint8)r,(Uint8)g,(Uint8)b,(Uint8)a) == 0;
        }  
        return Undefined();
    }

    static TObject SDL2_Init(GCList& ls, std::vector<TObject> args)
    {
        return (int64_t)SDL_Init(SDL_INIT_EVERYTHING);

    }
    

    static TObject SDL2_CreateRenderer(GCList& ls, std::vector<TObject> args)
    {
        TNative* window;
        int64_t index;
        int64_t flags;
        if(GetArgumentHeap<TNative*>(args,0,window) && !window->GetDestroyed() && GetArgument<int64_t>(args,1,index) && GetArgument<int64_t>(args,2,flags))
        {
            SDL_Renderer* _renderer= SDL_CreateRenderer((SDL_Window*)window->GetPointer(),(int)index, (Uint32)flags);
            if(_renderer == nullptr) return nullptr;
            return TNative::Create(ls,_renderer,[](void* _ptr)-> void{
                SDL_DestroyRenderer((SDL_Renderer*)_ptr);
            });
        }
        return Undefined();
    }
    static TObject SDL2_CreateWindow(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 6 && std::holds_alternative<std::string>(args[0]) && std::holds_alternative<int64_t>(args[1]) && std::holds_alternative<int64_t>(args[2]) && std::holds_alternative<int64_t>(args[3]) && std::holds_alternative<int64_t>(args[4]) && std::holds_alternative<int64_t>(args[5]))
        {
            SDL_Window* window = SDL_CreateWindow(std::get<std::string>(args[0]).c_str(),(int)std::get<int64_t>(args[1]),(int)std::get<int64_t>(args[2]),(int)std::get<int64_t>(args[3]),(int)std::get<int64_t>(args[4]),(uint32_t)std::get<int64_t>(args[5]));
            if(window == nullptr) return nullptr;
            
            return TNative::Create(ls,window,[](void* _ptr)->void {
                SDL_Window* win = (SDL_Window*)_ptr;
                if(win != nullptr) SDL_DestroyWindow(win);
            });
        }
        return Undefined();
    }
    #endif
    void TStd::RegisterSDL2(GC* gc, TRootEnvironment* env)
    {

        env->permissions.canRegisterSDL2=true;
        #if defined(CROSSLANG_ENABLE_SDL2)
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc, "RenderDrawRect","Draw a rectangle using SDL",{"renderer","dictionary_with_x_y_w_h"}, SDL2_RenderDrawRect);
        dict->DeclareFunction(gc, "RenderFillRect","Fill a rectangle using SDL",{"renderer","dictionary_with_x_y_w_h"}, SDL2_RenderFillRect);
        dict->DeclareFunction(gc, "RenderPresent","Present frame (you are finished with the frame)",{"renderer"},SDL2_RenderPresent);   
        dict->DeclareFunction(gc, "RenderClear","Clear renderer with renderer draw color",{"renderer"},SDL2_RenderClear);   
        dict->DeclareFunction(gc, "SetRenderDrawColor","Set SDL2 Renderer Draw Color",{"renderer","r","g","b","a"},SDL2_SetRenderDrawColor);   
        dict->DeclareFunction(gc, "CreateWindow","Create a SDL2 Window",{"title","x","y","w","h","flags"},SDL2_CreateWindow);   
        dict->DeclareFunction(gc, "CreateRenderer","Create a SDL2 Renderer",{"window",""},SDL2_CreateRenderer);  
        dict->DeclareFunction(gc, "PollEvent", "Get events",{},SDL2_PollEvent);
        dict->DeclareFunction(gc, "Init", "Init SDL2",{},SDL2_Init);

        
        gc->BarrierBegin();
        dict->SetValue("DROPBEGIN",(int64_t)SDL_DROPBEGIN);
        dict->SetValue("DROPCOMPLETE",(int64_t)SDL_DROPCOMPLETE);
        dict->SetValue("DROPFILE",(int64_t)SDL_DROPFILE);
        dict->SetValue("DROPTEXT",(int64_t)SDL_DROPTEXT);
        dict->SetValue("MOUSEBUTTONUP",(int64_t)SDL_MOUSEBUTTONUP);
        dict->SetValue("MOUSEBUTTONDOWN",(int64_t)SDL_MOUSEBUTTONDOWN);
        dict->SetValue("MOUSEMOTION",(int64_t)SDL_MOUSEMOTION);
        dict->SetValue("KEYUP",(int64_t)SDL_KEYUP);
        dict->SetValue("KEYDOWN",(int64_t)SDL_KEYDOWN);
        dict->SetValue("FINGERMOTION",(int64_t)SDL_FINGERMOTION);
        dict->SetValue("FINGERUP",(int64_t)SDL_FINGERUP);
        dict->SetValue("FINGERDOWN",(int64_t)SDL_FINGERDOWN);
        dict->SetValue("TEXTINPUT",(int64_t)SDL_TEXTINPUT);
        dict->SetValue("TEXTEDITING",(int64_t)SDL_TEXTEDITING);
        dict->SetValue("TEXTEDITING_EXT",(int64_t)SDL_TEXTEDITING_EXT);
        dict->SetValue("WINDOWEVENT",(int64_t)SDL_WINDOWEVENT);
        dict->SetValue("QUIT",(int64_t)SDL_QUIT);

        dict->SetValue("WINDOW_RESIZABLE",(int64_t)SDL_WINDOW_RESIZABLE);
        dict->SetValue("WINDOW_BORDERLESS",(int64_t)SDL_WINDOW_BORDERLESS);
        dict->SetValue("WINDOW_FULLSCREEN",(int64_t)SDL_WINDOW_FULLSCREEN);
        dict->SetValue("WINDOW_MAXIMIZED",(int64_t)SDL_WINDOW_MAXIMIZED);
        env->DeclareVariable("SDL2", dict);
        gc->BarrierEnd();
        #endif
    }
}
