
#include <TessesFramework/TessesFrameworkFeatures.h>
#include "CrossLang.hpp"
#include <iostream>
#if defined(TESSESFRAMEWORK_ENABLE_SDL2)
#include <TessesFramework/SDL2/SDL_Headers.hpp>
#include <TessesFramework/SDL2/GUI.hpp>
#include <TessesFramework/SDL2/Stream.hpp>
#include <TessesFramework/SDL2/Views/AbsoluteView.hpp>
#include <TessesFramework/SDL2/Views/ButtonView.hpp>
#include <TessesFramework/SDL2/Views/LabelView.hpp>
#include <TessesFramework/SDL2/Views/ProgressView.hpp>
#include <TessesFramework/SDL2/Views/TextListView.hpp>
#include <TessesFramework/SDL2/Views/ScrollableTextListView.hpp>
#include <TessesFramework/SDL2/Views/DropDownView.hpp>
#include <TessesFramework/SDL2/Views/EditTextView.hpp>
using namespace Tesses::Framework::SDL2;
#endif

namespace Tesses::CrossLang
{
    #if defined(TESSESFRAMEWORK_ENABLE_SDL2)
    TDictionary* SDL_Event2Dict(GCList& ls, SDL_Event& event)
    {
        TDictionary* dict= TDictionary::Create(ls,{
            TDItem("Type",(int64_t)event.common.type),
            TDItem("Timestamp",(int64_t)event.common.timestamp)
        });
        switch(event.type)
        {
            case SDL_DISPLAYEVENT:
            {
                ls.GetGC()->BarrierBegin();
                dict->SetValue("Display",TDictionary::Create(ls,{
                    TDItem("Display",(int64_t)event.display.display),
                    TDItem("Data1",(int64_t)event.display.data1),
                    TDItem("Event",(int64_t)event.display.event)
                }));
                ls.GetGC()->BarrierEnd();
            }
                break;
            case SDL_WINDOWEVENT:
            {
                ls.GetGC()->BarrierBegin();
                dict->SetValue("Window",TDictionary::Create(ls,{
                    TDItem("WindowId",(int64_t)event.window.windowID),
                    TDItem("Data1",(int64_t)event.window.data1),
                    TDItem("Data2",(int64_t)event.window.data2),
                    TDItem("Event",(int64_t)event.window.event)
                }));
                ls.GetGC()->BarrierEnd();
            }
                break;
            case SDL_SYSWMEVENT:
            {
                ls.GetGC()->BarrierBegin();
                dict->SetValue("Message",TNative::Create(ls,event.syswm.msg,[](void*)->void{}));
                ls.GetGC()->BarrierEnd();
            }
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                 ls.GetGC()->BarrierBegin();
                dict->SetValue("Key",TDictionary::Create(ls,{
                    TDItem("WindowId",(int64_t)event.key.windowID),
                    TDItem("State",(int64_t)event.key.state),
                    TDItem("Repeat",(int64_t)event.key.repeat),
                    TDItem("Mod",(int64_t)event.key.keysym.mod),

                    TDItem("Scancode",(int64_t)event.key.keysym.scancode),
                    TDItem("Sym",(int64_t)event.key.keysym.sym),
                    TDItem("Event",(int64_t)event.window.event)
                }));
                ls.GetGC()->BarrierEnd();
            }
                break;
            case SDL_TEXTEDITING:
            {
                ls.GetGC()->BarrierBegin();
                dict->SetValue("Edit",TDictionary::Create(ls,{
                    TDItem("WindowId",(int64_t)event.edit.windowID),
                    TDItem("Start",(int64_t)event.edit.start),
                    TDItem("Length",(int64_t)event.edit.length),
                    TDItem("Text",(std::string)event.edit.text),
                }));
                ls.GetGC()->BarrierEnd();
            }
                break;
            case SDL_TEXTEDITING_EXT:
            {
                std::string text = event.editExt.text;
                SDL_free(event.editExt.text);
                ls.GetGC()->BarrierBegin();
                dict->SetValue("Edit",TDictionary::Create(ls,{
                    TDItem("WindowId",(int64_t)event.editExt.windowID),
                    TDItem("Start",(int64_t)event.editExt.start),
                    TDItem("Length",(int64_t)event.editExt.length),
                    TDItem("Text",text),
                }));
                ls.GetGC()->BarrierEnd();
            }
                break;
            case SDL_TEXTINPUT:
            {
                ls.GetGC()->BarrierBegin();
                dict->SetValue("Text",TDictionary::Create(ls,{
                    TDItem("WindowId",(int64_t)event.text.windowID),
                    TDItem("Text",(std::string)event.text.text),
                }));
                ls.GetGC()->BarrierEnd();
            }
                break;
            case SDL_MOUSEMOTION:
            {
                ls.GetGC()->BarrierBegin();
                dict->SetValue("Motion",TDictionary::Create(ls,{
                    TDItem("WindowId",(int64_t)event.motion.windowID),
                    TDItem("X",(int64_t)event.motion.x),
                    TDItem("Y",(int64_t)event.motion.y),
                    TDItem("XRel",(int64_t)event.motion.xrel),
                    TDItem("YRel",(int64_t)event.motion.yrel),
                    TDItem("State",(int64_t)event.motion.state),
                    TDItem("Which",(int64_t)event.motion.which)
                }));
                ls.GetGC()->BarrierEnd();
            }
            break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                ls.GetGC()->BarrierBegin();
                dict->SetValue("Motion",TDictionary::Create(ls,{
                    TDItem("WindowId",(int64_t)event.button.windowID),
                    TDItem("X",(int64_t)event.button.x),
                    TDItem("Y",(int64_t)event.button.y),
                    TDItem("State",(int64_t)event.button.state),
                    TDItem("Clicks",(int64_t)event.button.clicks),
                    TDItem("Which",(int64_t)event.button.which),
                    TDItem("Button",(int64_t)event.button.button)
                }));
                ls.GetGC()->BarrierEnd();
            }
            break;
            case SDL_MOUSEWHEEL:
            {

                ls.GetGC()->BarrierBegin();
                dict->SetValue("Motion",TDictionary::Create(ls,{
                    TDItem("WindowId",(int64_t)event.wheel.windowID),
                    TDItem("X",(int64_t)event.wheel.x),
                    TDItem("Y",(int64_t)event.wheel.y),
                    TDItem("Direction",(int64_t)event.wheel.direction),
                    TDItem("MouseX",(int64_t)event.wheel.mouseX),
                    TDItem("MouseY",(int64_t)event.wheel.mouseY),
                    TDItem("PreciseX",(double)event.wheel.preciseX),
                    TDItem("PreciseY",(double)event.wheel.preciseY),
                    TDItem("Which",(int64_t)event.wheel.which),
                    
                }));
                ls.GetGC()->BarrierEnd();
            }
            break;
        }
        return dict;
    }
    class SDL2_Window;
    class SDL2_Renderer;
    class SDL2_Color;
    class SDL2_Rectangle;
    class SDL2_Texture;
    class SDL2_FontCache;
    class SDL2_Surface;
    class SDL2_Window : public TNativeObject {
        private:
            SDL_Window* window;
        public:
        SDL2_Window(std::string title, int x, int y, int w, int h,uint32_t flags)
        {
            window = SDL_CreateWindow(title.c_str(),x,y,w,h,flags);
        }
        TObject CallMethod(GCList& ls,std::string name, std::vector<TObject> args)
        {
            if(name == "setTitle")
            {
                std::string title;
                if(GetArgument(args,0,title))
                {
                    SDL_SetWindowTitle(window,title.c_str());
                    return title;
                }
            }
            if(name == "getTitle")
            {
                return std::string(SDL_GetWindowTitle(window));
            }

            return Undefined();
        }
        std::string TypeName()
        {
            return "SDL2.Window";
        }
        ~SDL2_Window()
        {
            SDL_DestroyWindow(window);
        }
        friend class SDL2_Renderer;
        friend class SDL2_Texture;

    };

    class SDL2_Color : public TNativeObject {
        SDL_Color color;
        
        public:
            SDL2_Color()
            {
                color={0};
            }
            SDL2_Color(SDL_Color color)
            {
                this->color = color;
            }
            SDL2_Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
            {
                this->color = {.r=r,.g=g,.b=b,.a=a};
            }
            TObject CallMethod(GCList& ls,std::string name, std::vector<TObject> args)
            {
                if(name == "getR")
                {
                    return (int64_t)this->color.r;
                }
                if(name == "setR")
                {
                    int64_t v;
                    if(GetArgument(args,0,v))
                    {
                        this->color.r = (uint8_t)v;
                        return v & 0xFF;
                    }
                    
                }
                if(name == "getG")
                {
                    return (int64_t)this->color.g;
                }
                if(name == "setG")
                {
                    int64_t v;
                    if(GetArgument(args,0,v))
                    {
                        this->color.g = (uint8_t)v;
                        return v & 0xFF;
                    }
                    
                }
                if(name == "getB")
                {
                    return (int64_t)this->color.b;
                }
                if(name == "setB")
                {
                    int64_t v;
                    if(GetArgument(args,0,v))
                    {
                        this->color.b = (uint8_t)v;
                        return v & 0xFF;
                    }
                    
                }
                if(name == "getA")
                {
                    return (int64_t)this->color.a;
                }
                if(name == "setA")
                {
                    int64_t v;
                    if(GetArgument(args,0,v))
                    {
                        this->color.a = (uint8_t)v;
                        return v & 0xFF;
                    }
                    
                }
                if(name == "ToString")
                {
                    std::string res="Red: ";
                    res.append(std::to_string((int64_t)color.r));
                    res.append(", Green: ");
                    res.append(std::to_string((int64_t)color.g));
                    res.append(", Blue: ");
                    res.append(std::to_string((int64_t)color.b));
                    res.append(", Alpha: ");
                    res.append(std::to_string((int64_t)color.a));
                    return res;
                }
                return Undefined();
            }
            std::string TypeName()
            {
                return "SDL2.Color";
            }
            friend class SDL2_Renderer;
            friend class SDL2_FontCache;
    };
    
    class SDL2_Rectangle : public TNativeObject {
        SDL_Rect rect;
        
        public:
            SDL2_Rectangle()
            {
                rect={0};
            }
            SDL2_Rectangle(SDL_Rect rect)
            {
                this->rect = rect;
            }
            SDL2_Rectangle(int x, int y, int w, int h)
            {
                this->rect = {.x=x,.y=y,.w=w,.h=h};
            }
            TObject CallMethod(GCList& ls,std::string name, std::vector<TObject> args)
            {
                if(name == "getX")
                {
                    return (int64_t)this->rect.x;
                }
                if(name == "setX")
                {
                    int64_t v;
                    if(GetArgument(args,0,v))
                    {
                        this->rect.x = (int)v;
                        return v;
                    }
                    
                }
                if(name == "getY")
                {
                    return (int64_t)this->rect.y;
                }
                if(name == "setY")
                {
                    int64_t v;
                    if(GetArgument(args,0,v))
                    {
                        this->rect.y = (int)v;
                        return v;
                    }
                    
                }
                if(name == "getWidth")
                {
                    return (int64_t)this->rect.w;
                }
                if(name == "setWidth")
                {
                    int64_t v;
                    if(GetArgument(args,0,v))
                    {
                        this->rect.w = (int)v;
                        return v;
                    }
                    
                }
                if(name == "getHeight")
                {
                    return (int64_t)this->rect.h;
                }
                if(name == "setHeight")
                {
                    int64_t v;
                    if(GetArgument(args,0,v))
                    {
                        this->rect.h = (int)v;
                        return v;
                    }
                    
                }
                if(name == "ToString")
                {
                    std::string res="X: ";
                    res.append(std::to_string((int64_t)rect.x));
                    res.append(", Y: ");
                    res.append(std::to_string((int64_t)rect.y));
                    res.append(", Width: ");
                    res.append(std::to_string((int64_t)rect.w));
                    res.append(", Height: ");
                    res.append(std::to_string((int64_t)rect.h));
                    return res;
                }
                return Undefined();
            }
            std::string TypeName()
            {
                return "SDL2.Rectangle";
            }
            friend class SDL2_Renderer;
    };
    class SDL2_Surface : public TNativeObject {
        TObject ToTexture(GCList& ls, std::vector<TObject> args);
        SDL_Surface* surface;
        public:
            SDL2_Surface(SDL_Surface* surf)
            {
                this->surface = surf;
            }
            ~SDL2_Surface()
            {
                SDL_FreeSurface(surface);
            }
            std::string TypeName()
            {
                return "SDL2.Surface";
            }
            TObject CallMethod(GCList& ls, std::string key, std::vector<TObject> args)
            {
                
                if(key == "getWidth" && this->surface != nullptr)
                {
                    return (int64_t)this->surface->w;
                }

                if(key == "getHeight" && this->surface != nullptr)
                {
                    return (int64_t)this->surface->w;
                }
                if(key == "ToTexture")
                {
                    return ToTexture(ls,args);
                }

                if(key == "ToString")
                {
                    return "SDL2.Surface";
                }
                return Undefined();
            }
            friend class SDL2_Texture;
    };
    class SDL2_Texture :  public TNativeObject {
        THeapObject* cache;
        SDL_Texture* tex;
        
        public:
            SDL2_Texture(THeapObject* cache, SDL_Texture* tex)
            {
                this->cache=cache;
                this->tex = tex;
            }
            SDL2_Texture(SDL_Texture* tex)
            {
                this->cache = nullptr;
                this->tex = tex;
            }
            SDL2_Texture(SDL2_Renderer* renderer,SDL2_Surface* surface);
            void Mark()
            {
                if(this->marked) return;
                this->marked=true;
                if(this->cache != nullptr) this->cache->Mark();
            }

            TObject CallMethod(GCList& ls,std::string key,std::vector<TObject> args)
            {
                if(key == "getSize")
                {
                    int w,h;
                    if(SDL_QueryTexture(this->tex,NULL,NULL,&w,&h)==0)
                    return TList::Create(ls,{
                        (int64_t)w,
                        (int64_t)h
                    });
                }
                if(key == "getWidth")
                {
                    int w;
                    if(SDL_QueryTexture(this->tex,NULL,NULL,&w,NULL)==0)
                    return (int64_t)w;
                }
                if(key == "getHeight")
                {
                    int h;
                    if(SDL_QueryTexture(this->tex,NULL,NULL,NULL,&h)==0)
                    return (int64_t)h;
                }
                return Undefined();
            }
            
            std::string TypeName()
            {
                return "SDL2.Texture";
            }
            ~SDL2_Texture()
            {
                if(this->cache == nullptr) SDL_DestroyTexture(this->tex);
            }
            friend class SDL2_FontCache;
            friend class SDL2_Renderer;
    };

   
    class SDL2_Renderer : public TNativeObject {
        SDL_Renderer* renderer;
        public:
            SDL2_Renderer(SDL2_Window* window, int index, uint32_t flags)
            {
                renderer = SDL_CreateRenderer(window->window,index,flags);
            }
            TObject CallMethod(GCList& ls,std::string name, std::vector<TObject> args)
            {
                if(name == "Clear")
                {
                    return (int64_t)SDL_RenderClear(renderer);
                }

                if(name == "getDrawColor")
                {
                    SDL_Color c;
                    if(SDL_GetRenderDrawColor(renderer,&c.r,&c.g,&c.b,&c.a)==0)
                    {
                        return TNativeObject::Create<SDL2_Color>(ls,c);
                    }
                }

                if(name == "setDrawColor")
                {
                    SDL2_Color* c;
                    if(GetArgumentHeap(args,0,c))
                    {
                        SDL_SetRenderDrawColor(renderer,c->color.r,c->color.g,c->color.b,c->color.a);
                        return c;
                    }
                }

                
                if(name == "Copy")
                {
                    SDL2_Texture* tex;
                    SDL2_Rectangle *src=nullptr,*dest=nullptr;
                    if(GetArgumentHeap(args,0,tex))
                    {
                        GetArgumentHeap(args,1,src);

                        GetArgumentHeap(args,2,dest);
                        SDL_RenderCopy(this->renderer,tex->tex,src == nullptr ? (SDL_Rect*)nullptr : &src->rect,dest == nullptr ? (SDL_Rect*)nullptr : &dest->rect);
                    }
                }

                if(name == "Present")
                {
                    SDL_RenderPresent(renderer);
                }
                
                return Undefined();
            }
            
            std::string TypeName()
            {
                return "SDL2.Renderer";
            }

            ~SDL2_Renderer()
            {
                SDL_DestroyRenderer(renderer);
            }
            friend class SDL2_FontCache;
            friend class SDL2_Texture;
    };
    TObject SDL2_Surface::ToTexture(GCList& ls,std::vector<TObject> args)
    {
        SDL2_Renderer* renderer;
        if(GetArgumentHeap(args,0,renderer))
        {
            return TNativeObject::Create<SDL2_Texture>(ls,renderer,this);
        }
        return Undefined();
    }
    SDL2_Texture::SDL2_Texture(SDL2_Renderer* renderer,SDL2_Surface* surface)
    {  
        this->cache = nullptr;
        this->tex = SDL_CreateTextureFromSurface(renderer->renderer,surface->surface);
    }
    class SDL2_FontCache : public TNativeObject {
        Tesses::Framework::SDL2::FontCache* cache;
        std::array<SDL2_Texture*,96> textures;
        SDL2_FontCache(GCList& ls, Tesses::Framework::SDL2::FontCache* fc)
        {
            this->cache = fc;
            for(size_t i = 0; i < 96; i++)        
            {
                textures[i] = TNativeObject::Create<SDL2_Texture>(ls,this, fc->operator[](i+32));
            }
        }
        public:
            SDL2_FontCache(GCList& ls,SDL2_Renderer* renderer, int pt) : SDL2_FontCache(ls,new Tesses::Framework::SDL2::FontCache(renderer->renderer,pt))
            {

            }
            SDL2_FontCache(GCList& ls,SDL2_Renderer* renderer,TByteArray* ba, int pt) : SDL2_FontCache(ls,new Tesses::Framework::SDL2::FontCache(renderer->renderer,ba->data,pt))
            {

            }
            SDL2_FontCache(GCList& ls,SDL2_Renderer* renderer,TTF_Font* font, int pt) : SDL2_FontCache(ls,new Tesses::Framework::SDL2::FontCache(renderer->renderer,font))
            {

            }
            
            TObject CallMethod(GCList& ls,std::string key,std::vector<TObject> args)
            {
                if(key == "GetAt")
                {
                    char c;
                    if(GetArgument(args,0,c))
                    {
                        if(c >= 32 && c <= 126)
                        {
                            return this->textures[c-32];
                        }
                        return this->textures[95];
                    }
                }
                if(key == "getPointSize")
                {
                    return (int64_t)this->cache->PointSize();
                }
                if(key == "getMaxWidth")
                {
                    return (int64_t)this->cache->MaxWidth();
                }
                if(key == "getMaxHeight")
                {
                    return (int64_t)this->cache->MaxHeight();
                }
                if(key == "ToStirng")
                {
                    return "SDL2.FontCache";
                }
                return Undefined();
            }

            std::string TypeName()
            {
                return "SDL2.FontCache";
            }

            void Mark()
            {
                if(this->marked) return;
                this->marked=true;
                for(auto item : this->textures) item->Mark();
            }
            
            ~SDL2_FontCache()
            {
                delete cache;
            }
    };
    
    template<typename T>
    class GUI_EventInstance : public Tesses::Framework::Event<Tesses::Framework::SDL2::View*,T&>
    {
        public:
        GCList* ls;
        TCallable* callable;
        TObject myObj;
        GUI_EventInstance(GC* gc,TObject parent,TCallable* call)
        {
            this->ls = new GCList(gc);
            this->callable = call;
            this->myObj = parent;
        }
        void Invoke(Tesses::Framework::SDL2::View* _v,T& _e)
        {
            GCList ls2(this->ls->GetGC());
            Tesses::Framework::SDL2::GUIEventArgs* e= &_e;

            auto btnE = dynamic_cast<Tesses::Framework::SDL2::GUIMouseButtonEventArgs*>(e);
            if(btnE != nullptr)
            {
                
                callable->Call(ls2,{
                    myObj,TDictionary::Create(ls2,{
                        TDItem("Type",std::string("GUI.MouseButtonEventArgs")),
                        TDItem("Which",(int64_t)btnE->which),
                        TDItem("X",(int64_t)btnE->x),
                        TDItem("Y",(int64_t)btnE->y),
                        TDItem("Button",(int64_t)btnE->button)
                    })
                });
            }
            
            else {
                callable->Call(ls2,{
                    myObj,TDictionary::Create(ls2,{
                        TDItem("Type",std::string("GUI.EventArgs"))
                    })
                });
            }
        }

        ~GUI_EventInstance()
        {
            delete ls;
        }
    };


    class GUI_View : public TNativeObject {
        protected:
            template<typename T>
            void AddEvent(GC* gc,Tesses::Framework::EventList<View*,T&>& el,TCallable* callable)
            {
                el +=std::make_shared<GUI_EventInstance<T>>(gc, this, callable);
            }
            template<typename T>
            void RemoveEvent(Tesses::Framework::EventList<View*,T&>& el,TCallable* callable)
            {
                el.Remove([callable](auto e)->bool{
                    auto res = dynamic_cast<GUI_EventInstance<T>*>(e.get());
                    return res != nullptr && res->callable == callable;
                    
                });
            }
            template<typename T>
            TDictionary* CreateEventDict(GCList& ls,Tesses::Framework::EventList<View*,T&>* el)
            {
                return TDictionary::Create(ls,{
                        TDItem("operator+",TExternalMethod::Create(ls,"",{"right"},[el,this](GCList& ls, std::vector<TObject> args)->TObject {
                            TCallable* callable;
                            if(GetArgumentHeap(args,0,callable))
                            {
                                this->AddEvent(ls.GetGC(),*el,callable);
                            }
                            return nullptr;
                        })),
                        TDItem("operator-",TExternalMethod::Create(ls,"",{"right"},[el,this](GCList& ls, std::vector<TObject> args)->TObject {
                            TCallable* callable;
                            if(GetArgumentHeap(args,0,callable))
                            {
                                this->RemoveEvent(*el,callable);
                            }
                            return nullptr;
                        })),
                });
            }
        public:
            Tesses::Framework::SDL2::View* view;
            GUI_View()
            {
                this->view = nullptr;
            }

            GUI_View(Tesses::Framework::SDL2::View* view)
            {
                this->view = view;
            }
            static TObject FindViewById(GCList& ls,View* v, std::string id);
            TObject FindViewById(GCList& ls,std::string id);

            

            virtual TObject CallMethod(GCList& ls,std::string key, std::vector<TObject> args)
            {
                if(key == "getClick")
                {
                    return CreateEventDict<GUIEventArgs>(ls,&this->view->Click);
                }
                if(key == "FindViewById")
                {
                    std::string id;
                    if(GetArgument(args,0,id))
                    {
                        return FindViewById(ls,id);
                    }
                    return nullptr;
                }
                if(key == "getText")
                {
                    return this->view->GetText();
                }
                if(key == "setText")
                {
                    std::string str;
                    if(GetArgument(args,0,str))
                    {
                        this->view->SetText(str);
                        return str;
                    }
                }
                return Undefined();
            }

           
    };
    class GUI_ButtonView : public GUI_View {
        public:
            GUI_ButtonView(Views::ButtonView* view) : GUI_View(view)
            {

            }
            GUI_ButtonView(std::string text) : GUI_View(new Views::ButtonView(text))
            {

            }
            std::string TypeName()
            {
                return "GUI.ButtonView";
            }
    };
    class GUI_ContainerView : public GUI_View {
        public:
            virtual std::string TypeName()
            {
                return "GUI.ContainerView";
            }
    };

    class GUI_EditTextView : public GUI_View {
        public:
            GUI_EditTextView(Views::EditTextView* view) : GUI_View(view)
            {

            }
            GUI_EditTextView(std::string hint) : GUI_View(new Views::EditTextView(hint))
            {

            }
            std::string TypeName()
            {
                return "GUI.EditTextView";
            }
            virtual TObject CallMethod(GCList& ls, std::string key, std::vector<TObject> args)
            {
                auto edit = dynamic_cast<Views::EditTextView*>(this->view);
                if(edit == nullptr) return GUI_View::CallMethod(ls,key,args);
                if(key == "getHint")
                {
                    return edit->GetHint();
                }
                if(key == "setHint")
                {
                    std::string str;
                    if(GetArgument(args,0,str))
                    {
                        edit->SetHint(str);
                        return str;
                    }
                }
                return GUI_View::CallMethod(ls,key,args);
            }
    };

    class GUI_Window : public TNativeObject {
        Tesses::Framework::SDL2::GUIPalette Default()
        {
            return Tesses::Framework::SDL2::GUIPalette(true,{.r=192,.g=255,.b=0,.a=255});
        }
        public:
            GUIWindow* window;
            //"title","width","height","flags","$pallete"
            GUI_Window(std::string title, int width, int height, Uint32 flags, Tesses::Framework::SDL2::GUIPalette* palette)
            {
                this->window = new Tesses::Framework::SDL2::GUIWindow(title,width,height,flags,palette != nullptr ? *palette : Default());
            }

            std::string TypeName()
            {
                return "GUI.Window";
            }

            TObject CallMethod(GCList& ls, std::string key, std::vector<TObject> args)
            {
                
                if(window)
                {
                    if(key == "setView")
                    {
                        GUI_View* v;
                        if(GetArgumentHeap(args,0,v))
                        {
                            window->SetView(v->view,true);
                        }
                    }
                    if(key == "FindViewById")
                    {
                        std::string id;
                        if(GetArgument(args,0,id))
                        {
                            if(id == window->GetId()) return this;
                            else {
                                auto v=window->GetViewAt(0);
                                if(v != nullptr)
                                    return GUI_View::FindViewById(ls,v,id);
                            }
                        }
                        return nullptr;
                    }
                    if(key == "getText")
                    {
                        return this->window->GetText();
                    }
                    if(key == "setText")
                    {
                        std::string str;
                        if(GetArgument(args,0,str))
                        {
                            this->window->SetText(str);
                            return str;
                        }
                    }
                }
                return Undefined();
            }
    };

    static TObject New_SDL2_Window(GCList& ls, std::vector<TObject> args)
    {
        std::string title;
        int64_t x,y,w,h,flags;

        if(GetArgument(args,0,title) && GetArgument(args,1,x) && GetArgument(args,2,y) && GetArgument(args,3,w) && GetArgument(args,4,h) && GetArgument(args,5,flags))
        {
            return TNativeObject::Create<SDL2_Window>(ls,title,(int)x,(int)y,(int)w,(int)h,(uint32_t)(uint64_t)flags);
        }
        return Undefined();
    }
    static TObject New_SDL2_Renderer(GCList& ls, std::vector<TObject> args)
    {
        SDL2_Window* window;
        int64_t index,flags;

        if(GetArgumentHeap(args,0,window) && GetArgument(args,1,index) && GetArgument(args,2,flags))
        {
            return TNativeObject::Create<SDL2_Renderer>(ls,window,(int)index,(uint32_t)(uint64_t)flags);
        }
        return Undefined();
    }
    static TObject New_SDL2_Color(GCList& ls, std::vector<TObject> args)
    {
        int64_t r=0,g=0,b=0,a=255;
        GetArgument(args,0,r);
        GetArgument(args,1,g);
        GetArgument(args,2,b);
        GetArgument(args,3,a);
        return TNativeObject::Create<SDL2_Color>(ls,r,g,b,a);
        
        
    }
    static TObject New_SDL2_Rectangle(GCList& ls, std::vector<TObject> args)
    {
        int64_t x=0,y=0,w=0,h=0;
        GetArgument(args,0,x);
        GetArgument(args,1,y);
        GetArgument(args,2,w);
        GetArgument(args,3,h);
        return TNativeObject::Create<SDL2_Rectangle>(ls,x,y,w,h);
        
        
    }
    static TObject New_SDL2_Image(GCList& ls, std::vector<TObject> args)
    {
        TVFSHeapObject* vfs;
        Tesses::Framework::Filesystem::VFSPath path;
        TByteArray* ba;
        TStreamHeapObject* mstrm;
       

        if(GetArgumentHeap(args,0,vfs) && GetArgumentAsPath(args,1,path))
        {
            auto strm = vfs->vfs->OpenFile(path,"rb");
            if(strm != nullptr)
            {
                if(strm->CanRead())
                {
                    auto strm2 = Tesses::Framework::SDL2::RwopsFromStream(strm);
                    return TNativeObject::Create<SDL2_Surface>(ls,IMG_Load_RW(strm2,1));
                }
                delete strm;
            }
        }
        else if(GetArgumentHeap(args,0,ba))
        {
            auto ms =new Tesses::Framework::Streams::MemoryStream(false);
            ms->GetBuffer() = ba->data;
            return TNativeObject::Create<SDL2_Surface>(ls,IMG_Load_RW(Tesses::Framework::SDL2::RwopsFromStream(ms),1));
        }
        else if(GetArgumentHeap(args,0,mstrm))
        {
            return TNativeObject::Create<SDL2_Surface>(ls,IMG_Load_RW(Tesses::Framework::SDL2::RwopsFromStream(mstrm->stream,false),1));
        }
        else if(args.size() == 1) {
            return TNativeObject::Create<SDL2_Surface>(ls,IMG_Load_RW(Tesses::Framework::SDL2::RwopsFromStream(new TObjectStream(ls.GetGC(),args[0])),1));
        }
        return Undefined();
    }
    static TObject New_SDL2_FontCache(GCList& ls, std::vector<TObject> args)
    {
        SDL2_Renderer* renderer;
        int64_t pt;
        if(GetArgumentHeap(args,0,renderer) &&  GetArgument(args,2,pt))
        {
            TByteArray* ba;
            
            TStreamHeapObject* strm;
            if(GetArgumentHeap(args,1,ba))
            {
                return TNativeObject::Create<SDL2_FontCache>(ls,ls,renderer,ba,pt);
            }
            else if(GetArgumentHeap(args,1,strm))
            {
                auto res = TTF_OpenFontRW(Tesses::Framework::SDL2::RwopsFromStream(strm->stream,false),1,pt);
                auto res2= TNativeObject::Create<SDL2_FontCache>(ls,ls,renderer,res,pt);
                TTF_CloseFont(res);
                return res2;
            }
            else if(args.size() > 1 && !std::holds_alternative<nullptr_t>(args[1]))
            {
                
                auto res = TTF_OpenFontRW(Tesses::Framework::SDL2::RwopsFromStream(new TObjectStream(ls.GetGC(),args[1])),1,pt);
                auto res2= TNativeObject::Create<SDL2_FontCache>(ls,ls,renderer,res,pt);
                TTF_CloseFont(res);
                return res2;
            }
            return TNativeObject::Create<SDL2_FontCache>(ls,ls,renderer,pt);
        }
        return Undefined();
    }
    static TObject New_GUI_ButtonView(GCList& ls, std::vector<TObject> args)
    {
        std::string text;
        GetArgument(args,0,text);

        return TNativeObject::Create<GUI_ButtonView>(ls,text);
    }
    static TObject New_GUI_Window(GCList& ls, std::vector<TObject> args)
    {
        //"title","width","height","flags"
        std::string title;
        int64_t w;
        int64_t h;
        int64_t flags;
        if(GetArgument(args,0,title) && GetArgument(args,1,w) && GetArgument(args,2,h) && GetArgument(args,3,flags))
        {
            return TNativeObject::Create<GUI_Window>(ls,title,w,h,flags,nullptr);
        }

        return nullptr;
    }
    class SDL2_getEvents_Enumerator : public TEnumerator
    {
        SDL_Event event;
        bool has;
        public:
            static SDL2_getEvents_Enumerator* Create(GCList& ls)
            {
                SDL2_getEvents_Enumerator* e=new SDL2_getEvents_Enumerator();
                auto gc = ls.GetGC();
                ls.Add(e);
                gc->Watch(e);
                return e;
            }
            bool MoveNext(GC* ls)
            {
                has = SDL_PollEvent(&event);
                return has;
            }
            TObject GetCurrent(GCList& ls)
            {
                return SDL_Event2Dict(ls, event);
            }
    };
    TObject GUI_View::FindViewById(GCList& ls,std::string id)
    {
        return FindViewById(ls,view,id);
    }
    TObject GUI_View::FindViewById(GCList& ls,View* curView,std::string id)
    {
        auto view = curView->FindViewById(id);
        auto buttonView = dynamic_cast<Views::ButtonView*>(view);
        auto editTextView = dynamic_cast<Views::EditTextView*>(view);
        if(buttonView != nullptr)
        {
            return TNativeObject::Create<GUI_ButtonView>(ls,buttonView);
        }
        if(editTextView != nullptr)
        {
            return TNativeObject::Create<GUI_EditTextView>(ls,editTextView);
        }
        return nullptr;
    }
    static TObject SDL2_getEvents(GCList& ls, std::vector<TObject> args)
    {
        return TDictionary::Create(ls,{
            TDItem("GetEnumerator", TExternalMethod::Create(ls,"GetEnumerator",{},[](GCList& ls,std::vector<TObject> args)->TObject {
                return SDL2_getEvents_Enumerator::Create(ls);
            }))
        });
    }
    static TObject GUI_Window_FromDictionary(GCList& ls, std::vector<TObject> args)
    {
        int64_t flags;
        TObject o;

        if(args.size() >= 1)
        {
            GetArgument(args,1,flags);
            std::string jsonText = ToString(ls.GetGC(),args[0]);
            GUI_Window* window = TNativeObject::Create<GUI_Window>(ls,std::string("Unnamed Window"),320,240,flags,nullptr);
            
            window->window->SetView(Tesses::Framework::Serialization::Json::Json::Decode(jsonText));
            return window;
        }
        return nullptr;
    }
    
   
    #endif 
    void TStd::RegisterSDL2(GC* gc,TRootEnvironment* env)
    {
        env->permissions.canRegisterSDL2=true;
        #if defined(TESSESFRAMEWORK_ENABLE_SDL2)
        GCList ls(gc);
        
        

        auto newDict=env->EnsureDictionary(gc,"New");

        TDictionary* sdl2_new = TDictionary::Create(ls,{
            TDItem("Image",TExternalMethod::Create(ls, "Load image",{"streamOrBA"},New_SDL2_Image)),
            TDItem("FontCache",TExternalMethod::Create(ls, "Create font cache",{"renderer","font","pt","color"},New_SDL2_FontCache)),
            TDItem("Window",TExternalMethod::Create(ls,"Create window for SDL2",{"title","x","y","w","h","flags"},New_SDL2_Window)),
            TDItem("Renderer",TExternalMethod::Create(ls,"Create renderer for SDL2",{"window","index","flags"},New_SDL2_Renderer)),
            TDItem("Rectangle",TExternalMethod::Create(ls,"Create rectangle for SDL2",{"$x","$y","$w","$h"},New_SDL2_Rectangle)),
            TDItem("Color",TExternalMethod::Create(ls,"Create color for SDL2",{"$r","$g","$b","$a"},New_SDL2_Color))
        });
        TDictionary* gui_new = TDictionary::Create(ls, {
            TDItem("ButtonView",TExternalMethod::Create(ls,"Create a button",{"$text"},New_GUI_ButtonView)),
            TDItem("Window",TExternalMethod::Create(ls,"Create the window",{"title","width","height","flags","$pallete"},New_GUI_Window))
        });
        
        TDictionary* gui_d = TDictionary::Create(ls, {
            TDItem("Window",TDictionary::Create(ls, {
                TDItem("FromJSON",TExternalMethod::Create(ls, "Create window from JSON (anything that returns valid stringifed json via ToString)",{"dictOrStringOrByteArray","$flags"},GUI_Window_FromDictionary))
            }))
        });

        TDictionary* sdl2_d = TDictionary::Create(ls,{
            TDItem("getEvents",TExternalMethod::Create(ls,"Get events",{},SDL2_getEvents)),
            TDItem("WindowPos",TDictionary::Create(ls,{
                TDItem("Undefined",(int64_t)SDL_WINDOWPOS_UNDEFINED),
                TDItem("Centered",(int64_t)SDL_WINDOWPOS_CENTERED)
            })),
            TDItem("Window",TDictionary::Create(ls,{
                TDItem("Fullscreen",(int64_t)SDL_WINDOW_FULLSCREEN),
                TDItem("OpenGL",(int64_t)SDL_WINDOW_OPENGL),
                TDItem("Shown",(int64_t)SDL_WINDOW_SHOWN),
                TDItem("Hidden",(int64_t)SDL_WINDOW_HIDDEN),
                TDItem("Borderless",(int64_t)SDL_WINDOW_BORDERLESS),
                TDItem("Resizable",(int64_t)SDL_WINDOW_RESIZABLE),
                TDItem("Minimized",(int64_t)SDL_WINDOW_MINIMIZED),
                TDItem("Maximized",(int64_t)SDL_WINDOW_MAXIMIZED),
                TDItem("MouseGrabbed",(int64_t)SDL_WINDOW_MOUSE_GRABBED),
                TDItem("InputFocus",(int64_t)SDL_WINDOW_INPUT_FOCUS),
                TDItem("MouseFocus",(int64_t)SDL_WINDOW_MOUSE_FOCUS),
                TDItem("FullscreenDesktop",(int64_t)SDL_WINDOW_FULLSCREEN_DESKTOP),
                TDItem("Foreign",(int64_t)SDL_WINDOW_FOREIGN),
                TDItem("AllowHighDPI",(int64_t)SDL_WINDOW_ALLOW_HIGHDPI),
                TDItem("MouseCapture",(int64_t)SDL_WINDOW_MOUSE_CAPTURE),
                TDItem("AlwaysOnTop",(int64_t)SDL_WINDOW_ALWAYS_ON_TOP),
                TDItem("SkipTaskbar",(int64_t)SDL_WINDOW_SKIP_TASKBAR),
                TDItem("Utility",(int64_t)SDL_WINDOW_UTILITY),
                TDItem("Tooltip",(int64_t)SDL_WINDOW_TOOLTIP),
                TDItem("PopupMenu",(int64_t)SDL_WINDOW_POPUP_MENU),
                TDItem("KeyboardGrabbed",(int64_t)SDL_WINDOW_KEYBOARD_GRABBED),
                TDItem("Vulkan",(int64_t)SDL_WINDOW_VULKAN),
                TDItem("Metal",(int64_t)SDL_WINDOW_METAL),
                TDItem("InputGrabbed",(int64_t)SDL_WINDOW_INPUT_GRABBED)
            })),
            TDItem("Event",TDictionary::Create(ls,{
                TDItem("Quit",(int64_t)SDL_QUIT),
                TDItem("AppTerminating",(int64_t)SDL_APP_TERMINATING),
                TDItem("AppLowMemory",(int64_t)SDL_APP_LOWMEMORY),
                TDItem("AppWillEnterBackground",(int64_t)SDL_APP_WILLENTERBACKGROUND),
                TDItem("AppDidEnterBackground",(int64_t)SDL_APP_DIDENTERBACKGROUND),
                TDItem("AppWillEnterForeground",(int64_t)SDL_APP_WILLENTERFOREGROUND),
                TDItem("AppDidEnterForeground",(int64_t)SDL_APP_DIDENTERFOREGROUND),
                TDItem("LocaleChanged",(int64_t)SDL_LOCALECHANGED),
                TDItem("DisplayEvent",(int64_t)SDL_DISPLAYEVENT),
                TDItem("WindowEvent",(int64_t)SDL_WINDOWEVENT),
                TDItem("SysWMEvent",(int64_t)SDL_SYSWMEVENT),
                TDItem("KeyDown",(int64_t)SDL_KEYDOWN),
                TDItem("KeyUp",(int64_t)SDL_KEYUP),
                TDItem("TextEditing",(int64_t)SDL_TEXTEDITING),
                TDItem("TextInput",(int64_t)SDL_TEXTINPUT),
                TDItem("KeyMapChanged",(int64_t)SDL_KEYMAPCHANGED),
                TDItem("TextEditingExt",(int64_t)SDL_TEXTEDITING_EXT),
                TDItem("MouseMotion",(int64_t)SDL_MOUSEMOTION),
                TDItem("MouseButtonDown",(int64_t)SDL_MOUSEBUTTONDOWN),
                TDItem("MouseButtonUp",(int64_t)SDL_MOUSEBUTTONUP),
                TDItem("MouseWheel",(int64_t)SDL_MOUSEWHEEL),
                TDItem("JoyAxisMotion",(int64_t)SDL_JOYAXISMOTION),
                TDItem("JoyBallMotion",(int64_t)SDL_JOYBALLMOTION),
                TDItem("JoyHatMotion",(int64_t)SDL_JOYHATMOTION),
                TDItem("JoyButtonDown",(int64_t)SDL_JOYBUTTONDOWN),
                TDItem("JoyButtonUp",(int64_t)SDL_JOYBUTTONUP),
                TDItem("JoyButtonDown",(int64_t)SDL_JOYBUTTONDOWN),
                TDItem("JoyDeviceAdded",(int64_t)SDL_JOYDEVICEADDED),
                TDItem("JoyDeviceRemoved",(int64_t)SDL_JOYDEVICEREMOVED),
                TDItem("JoyBatteryUpdated",(int64_t)SDL_JOYBATTERYUPDATED),
                TDItem("ControllerAxisMotion",(int64_t)SDL_CONTROLLERAXISMOTION),
                TDItem("ControllerButtonDown",(int64_t)SDL_CONTROLLERBUTTONDOWN),
                TDItem("ControllerButtonUp",(int64_t)SDL_CONTROLLERBUTTONUP),
                TDItem("ControllerDeviceAdded",(int64_t)SDL_CONTROLLERDEVICEADDED),
                TDItem("ControllerDeviceRemoved",(int64_t)SDL_CONTROLLERDEVICEREMOVED),
                TDItem("ControllerDeviceRemapped",(int64_t)SDL_CONTROLLERDEVICEREMAPPED),
                TDItem("ControllerTouchpadDown",(int64_t)SDL_CONTROLLERTOUCHPADDOWN),
                TDItem("ControllerTouchpadMotion",(int64_t)SDL_CONTROLLERTOUCHPADMOTION),
                TDItem("ControllerTouchPadUp",(int64_t)SDL_CONTROLLERTOUCHPADUP),
                TDItem("ControllerSensorUpdate",(int64_t)SDL_CONTROLLERSENSORUPDATE),
                TDItem("ControllerSteamHandleUpdated",(int64_t)SDL_CONTROLLERSTEAMHANDLEUPDATED),
                TDItem("FingerDown",(int64_t)SDL_FINGERDOWN),
                TDItem("FingerUp",(int64_t)SDL_FINGERUP),
                TDItem("FingerMotion",(int64_t)SDL_FINGERMOTION),
                TDItem("DollarGesture",(int64_t)SDL_DOLLARGESTURE),
                TDItem("DollarRecord",(int64_t)SDL_DOLLARRECORD),
                TDItem("MultiGesture",(int64_t)SDL_MULTIGESTURE),
                TDItem("ClipboardUpdate",(int64_t)SDL_CLIPBOARDUPDATE),
                TDItem("DropFile",(int64_t)SDL_DROPFILE),
                TDItem("DropText",(int64_t)SDL_DROPTEXT),
                TDItem("DropBegin",(int64_t)SDL_DROPBEGIN),
                TDItem("DropComplete",(int64_t)SDL_DROPCOMPLETE),
                TDItem("AudioDeviceAdded",(int64_t)SDL_AUDIODEVICEADDED),
                TDItem("AudioDeviceRemoved",(int64_t)SDL_AUDIODEVICEREMOVED),
                TDItem("SensorUpdate",(int64_t)SDL_SENSORUPDATE),
                TDItem("RenderTargetsReset",(int64_t)SDL_RENDER_TARGETS_RESET),
                TDItem("RenderDeviceReset",(int64_t)SDL_RENDER_DEVICE_RESET)
            }))
        });

        gc->BarrierBegin();
        newDict->SetValue("SDL2",sdl2_new);
        newDict->SetValue("GUI",gui_new);
        env->SetVariable("SDL2",sdl2_d);
        env->SetVariable("GUI",gui_d);
        gc->BarrierEnd();

        #endif
    }
}
