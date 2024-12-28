#include "CrossLang.hpp"
#if defined(CROSSLANG_ENABLE_SDL2)
#include <SDL2/SDL.h>
#endif
namespace Tesses::CrossLang
{
    #if defined(CROSSLANG_ENABLE_SDL2)
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
        
        dict->DeclareFunction(gc, "RenderPresent","Present frame (you are finished with the frame)",{"renderer"},SDL2_RenderPresent);   
        dict->DeclareFunction(gc, "RenderClear","Clear renderer with renderer draw color",{"renderer"},SDL2_RenderClear);   
        dict->DeclareFunction(gc, "SetRenderDrawColor","Set SDL2 Renderer Draw Color",{"renderer","r","g","b","a"},SDL2_SetRenderDrawColor);   
        dict->DeclareFunction(gc, "CreateWindow","Create a SDL2 Window",{"title","x","y","w","h","flags"},SDL2_CreateWindow);   
        dict->DeclareFunction(gc, "CreateRenderer","Create a SDL2 Renderer",{"window",""},SDL2_CreateRenderer);   
        
        gc->BarrierBegin();
        env->DeclareVariable("SDL2", dict);
        gc->BarrierEnd();
        #endif
    }
}