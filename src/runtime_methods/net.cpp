#include "CrossLang.hpp"
#include <TessesFramework/Crypto/ClientTLSStream.hpp>
#include <TessesFramework/Http/HttpClient.hpp>
#include <TessesFramework/Mail/Smtp.hpp>
#include <TessesFramework/Streams/MemoryStream.hpp>
#include <TessesFramework/Streams/NetworkStream.hpp>
#include <TessesFramework/Streams/Stream.hpp>
#include <sys/types.h>
#include <csignal>
#include <iostream>
#include <cstring>
using namespace Tesses::Framework::Streams;
using namespace Tesses::Framework::Http;
using namespace Tesses::Framework::Mail;
namespace Tesses::CrossLang
{
    static std::shared_ptr<SMTPBody> TObjectToSMTPBody(GCList& ls,std::string mimeType, TObject obj)
    {
        std::shared_ptr<SMTPBody> body;
        std::string text;
        TByteArray* ba;
        std::shared_ptr<Tesses::Framework::Streams::Stream> sho;
        if(GetObject(obj,text))
        {
            body = std::make_shared<SMTPStringBody>(text,mimeType);
        }
        else if(GetObjectHeap(obj,ba)) {
            std::shared_ptr<MemoryStream> ms = std::make_shared<MemoryStream>(true);
            ms->WriteBlock(ba->data.data(), ba->data.size());
            ms->Seek(0L, SeekOrigin::Begin);
            
            body = std::make_shared<SMTPStreamBody>(mimeType,ms);
        }
        else if(GetObject(obj,sho))
        {
            ls.Add(sho);
            body = std::make_shared<SMTPStreamBody>(mimeType,sho);
        }
        return body;
    }
   
    
    class TNativeObjectThatReturnsHttpDictionary : public TNativeObject {
        public:
            virtual bool IsAvailable()=0;
    };
    class DummyTNativeObjectThatReturnsHttpDictionary : public TNativeObjectThatReturnsHttpDictionary {
        bool isAvail;
        public:
            DummyTNativeObjectThatReturnsHttpDictionary()
            {
                this->isAvail=true;
            }
            bool IsAvailable()
            {
                return this->isAvail;
            }
            void Close()
            {
                this->isAvail=false;
            }
            std::string TypeName()
            {
                return "Dummy";
            }
            TObject CallMethod(GCList& ls, std::string key, std::vector<TObject> args)
            {
                return Undefined();
            }
    };

    class THttpDictionary : public TNativeObject {
        TNativeObjectThatReturnsHttpDictionary* ctx;
        HttpDictionary* dict;
        public:
            THttpDictionary(HttpDictionary* dict, TNativeObjectThatReturnsHttpDictionary* ctx)
            {
                this->dict = dict;
                this->ctx = ctx;
            }
            std::string TypeName()
            {
                return "Net.Http.HttpDictionary";
            }
            void Mark()
            {
                if(this->marked) return;
                this->marked=true;
                ctx->Mark();
            }
            

            TObject CallMethod(GCList& ls, std::string key, std::vector<TObject> args)
            {
                if(!ctx->IsAvailable()) return Undefined();
                else if(key == "AddValue")
                {
                    std::string key;
                    std::string value;
                    int64_t i64;
                    double d64;
                    std::shared_ptr<Tesses::Framework::Date::DateTime> da;
                    if(GetArgument(args,0,key) )
                    {
                        if(GetArgument(args,1,value))
                            this->dict->AddValue(key,value);
                        else if(GetArgument(args,1,i64))
                            this->dict->AddValue(key,i64);
                        else if(GetArgument(args,1,d64))
                            this->dict->AddValue(key, d64);
                        else if(GetArgument(args,1,da))
                            this->dict->AddValue(key, *da);
                    }
                }
                else if(key == "SetValue")
                {
                    std::string key;
                    std::string value;
                    int64_t i64;
                    double d64;
                    std::shared_ptr<Tesses::Framework::Date::DateTime> da;
                    if(GetArgument(args,0,key) )
                    {
                        if(GetArgument(args,1,value))
                            this->dict->SetValue(key,value);
                        else if(GetArgument(args,1,i64))
                            this->dict->SetValue(key,i64);
                        else if(GetArgument(args,1,d64))
                            this->dict->SetValue(key, d64);
                        else if(GetArgument(args,1,da))
                            this->dict->SetValue(key, *da);
                    }
                }
                else if(key == "Clear")
                {
                    std::string key;
                    bool existsAfter;
                    if(GetArgument(args,0,key) && GetArgument(args,0, existsAfter))
                        dict->Clear(key,existsAfter);
                    else dict->Clear();

                }
                else if(key == "GetFirstBoolean")
                {
                    std::string key;
                    if(GetArgument(args,0,key))
                    {
                        return dict->GetFirstBoolean(key);
                    }
                    return false;
                }
                else if(key == "TryGetFirst")
                {
                    std::string key;
                    std::string value;
                    if(GetArgument(args,0,key) && dict->TryGetFirst(key,value))
                    {
                        return value;
                    }
                    return nullptr;
                }
                else if(key == "TryGetFirstDouble")
                {
                    std::string key;
                    double value;
                    if(GetArgument(args,0,key) && dict->TryGetFirstDouble(key,value))
                    {
                        return value;
                    }
                    return nullptr;
                }
                else if(key == "TryGetFirstInt")
                {
                    std::string key;
                    int64_t value;
                    if(GetArgument(args,0,key) && dict->TryGetFirstInt(key,value))
                    {
                        return value;
                    }
                    return nullptr;
                }
                else if(key == "TryGetFirstDate")
                {
                    std::string key;
                    Tesses::Framework::Date::DateTime value;
                    if(GetArgument(args,0,key) && dict->TryGetFirstDate(key,value))
                    {
                        return std::make_shared<Tesses::Framework::Date::DateTime>(value);
                    }
                    return nullptr;
                }
                else if(key == "ToList")
                {
                    TList* _ls = TList::Create(ls);
                    for(auto item : dict->kvp)
                    {
                        for(auto i : item.second)
                        {
                            auto d = TDictionary::Create(ls);
                            ls.GetGC()->BarrierBegin();
                            d->SetValue("Key", item.first);
                            d->SetValue("Value", i);
                            _ls->Add(d);
                            ls.GetGC()->BarrierEnd();
                        }
                    }
                    return _ls;
                }
                else if(key == "SetContentDisposition")
                {
                    std::string type;
                    std::string filename;
                    std::string fieldName;

                    if(GetArgument(args,0,filename) && GetArgument(args,1,type))
                    {
                        ContentDisposition dispo;
                        dispo.filename = filename;
                        dispo.type = type;

                        if(GetArgument(args,2, fieldName))
                            dispo.fieldName = fieldName;
                        
                        this->dict->SetValue("Content-Disposition", dispo.ToString());
                    }
                    return nullptr;
                }
                else if(key == "TryGetContentDisposition")
                {
                    std::string cdStr;
                    ContentDisposition cd;

                    if(this->dict->TryGetFirst("Content-Disposition", cdStr) && ContentDisposition::TryParse(cdStr,cd))
                    {
                        return TDictionary::Create(ls,{
                            TDItem("Type",cd.type),
                            TDItem("FileName",cd.filename),
                            TDItem("FieldName",cd.fieldName)
                        });
                    }

                    return nullptr;
                }
                return Undefined();
            }
    };

    class TServerContext : public TNativeObjectThatReturnsHttpDictionary
    {
        ServerContext* ctx;
        public:
            TServerContext(ServerContext* ctx)
            {
                this->ctx = ctx;
            }
            std::string TypeName()
            {
                return "Net.Http.ServerContext";
            }
            bool IsAvailable()
            {
                return ctx != nullptr;
            }
            ServerContext* GetContext()
            {
                return this->ctx;
            }
            TObject CallMethod(GCList& ls,std::string key, std::vector<TObject> args)
            {
                if(this->ctx == nullptr) return Undefined();
                else if(key == "getEncrypted") return ctx->encrypted;
                else if(key == "getMethod") return ctx->method;
                else if(key == "getIP") return ctx->ip;
                else if(key == "getPort") return ctx->port;
                else if(key == "getOriginalPath") return ctx->originalPath;
                else if(key == "getPath") return ctx->path;
                else if(key == "setPath") {
                     std::string str;
                    if(GetArgument(args,0,str))
                    {  
                        ctx->path = str;
                        return str;
                    }
                }
                else if(key == "getVersion") return ctx->version;
                else if(key == "setStatusCode") {
                     int64_t sc;
                    if(GetArgument(args,0,sc))
                    {  
                        ctx->statusCode = (StatusCode)sc;
                        return sc;
                    }
                }
                else if(key == "getStatusCode") return ctx->statusCode;
                else if(key == "getQueryParams") return TNativeObject::Create<THttpDictionary>(ls, &ctx->queryParams,this);
                else if(key == "getRequestHeaders") return TNativeObject::Create<THttpDictionary>(ls, &ctx->requestHeaders,this);
                else if(key == "getResponseHeaders") return TNativeObject::Create<THttpDictionary>(ls, &ctx->responseHeaders,this);
                else if(key == "GetStream") return ctx->GetStream();
                else if(key == "OpenRequestStream") return ctx->OpenRequestStream();
                else if(key == "OpenResponseStream") return ctx->OpenResponseStream();
                else if(key == "ParseFormData")
                {
                    TCallable* callable;
                    std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
                    
                    if(GetArgumentHeap(args, 0, callable))
                    {
                        ctx->ParseFormData([callable,&ls](std::string a,std::string b, std::string c)->std::shared_ptr<Tesses::Framework::Streams::Stream> {
                            auto res = callable->Call(ls,{a,b,c});
                            std::shared_ptr<Tesses::Framework::Streams::Stream> strm;
                            if(GetObject(res,strm))
                            {
                                return strm;
                            }
                            return nullptr;
                        });
                    }
                    else if(GetArgument(args,0,vfs))
                    {
                        int i = 1;
                        std::vector<TObject> response;
                        ctx->ParseFormData([vfs,&response,&ls,&i](std::string mime,std::string filename, std::string name)->std::shared_ptr<Tesses::Framework::Streams::Stream> {
                            std::string realFileName = "/" + std::to_string(i) + ".bin";
                            i++;
                            response.push_back(TDictionary::Create(ls,{
                                TDItem("RealFileName", realFileName),
                                TDItem("FileName", filename),
                                TDItem("Mime", mime),
                                TDItem("Name", name)        
                            }));

                            auto strm = vfs->OpenFile(realFileName,"wb");
                            

                            return strm;
                        });

                        return TList::Create(ls, response.begin(), response.end());
                    }
                }
                else if(key == "getNeedToParseFormData") return ctx->NeedToParseFormData();
                else if(key == "ReadString") return ctx->ReadString();
                else if(key == "ReadStream") {
                    std::shared_ptr<Tesses::Framework::Streams::Stream> strm;
                    if(GetArgument(args,0,strm))
                    {
                        ctx->ReadStream(strm);
                    }
                }
                else if(key == "ReadJson")
                {
                    return Json_Decode(ls,ctx->ReadString());
                }
                else if(key == "SendJson")
                {
                    if(args.size() > 0)
                    {
                        ctx->WithMimeType("application/json").SendText(Json_Encode(args[0]));
                    }
                }
                else if(key == "SendText")
                {
                    std::string text;
                    if(GetArgument(args,0,text))
                        ctx->SendText(text);
                }
                else if(key == "WithMimeType")
                {
                    std::string text;
                    if(GetArgument(args,0,text))
                        ctx->WithMimeType(text);
                    return this;
                }
                else if(key == "WithLastModified")
                {
                    std::shared_ptr<Tesses::Framework::Date::DateTime> da;
                    if(GetArgument(args,0,da))
                        ctx->WithLastModified(*da);
                    return this;
                }
                else if(key == "WithContentDisposition")
                {
                    std::string filename;
                    bool isInline;
                    if(GetArgument(args,0,filename) && GetArgument(args,1,isInline))
                        ctx->WithContentDisposition(filename,isInline);
                    return this;
                }
                else if(key == "WithHeader")
                {
                    std::string key;
                    std::string value;
                    if(GetArgument(args,0,key) && GetArgument(args,1,value))
                        ctx->WithHeader(key,value);
                    return this;
                }
                else if(key == "WithSingleHeader")
                {
                    std::string key;
                    std::string value;
                    if(GetArgument(args,0,key) && GetArgument(args,1,value))
                        ctx->WithSingleHeader(key,value);
                    return this;
                }
                else if(key == "SendStream")
                {
                    std::shared_ptr<Tesses::Framework::Streams::Stream> strm;
                    if(GetArgument(args,0,strm))
                        ctx->SendStream(strm);
                   
                }
                else if(key == "SendBytes")
                {
                    TByteArray* ba;
                    if(GetArgumentHeap(args,0,ba))
                        ctx->SendBytes(ba->data);
                }
                else if(key == "GetUrlWithQuery")
                {
                    return ctx->GetUrlWithQuery();
                }
                else if(key == "GetOriginalPathWithQuery")
                {
                    return ctx->GetOriginalPathWithQuery();
                }
                else if(key == "MakeAbsolute")
                {
                    std::string path;
                    if(GetArgument(args,0,path))
                    {
                        return ctx->MakeAbsolute(path);

                    }
                    return nullptr;
                }
                else if(key == "WithLocationHeader")
                {
                    std::string path;
                    if(GetArgument(args,0,path))
                    {
                        int64_t sc;
                        if(GetArgument(args,1,sc))
                        {
                            ctx->WithLocationHeader(path,(StatusCode)sc);
                            return nullptr;
                        }
                        ctx->WithLocationHeader(path);
                        
                    }
                    return this;
                }
                else if(key == "GetServerRoot")
                {
                    return ctx->GetServerRoot();
                }
                else if(key == "SendRedirect")
                {
                    std::string path;
                    if(GetArgument(args,0,path))
                    {
                        int64_t sc;
                        if(GetArgument(args,1,sc))
                        {
                            ctx->SendRedirect(path,(StatusCode)sc);
                            return nullptr;
                        }
                        ctx->SendRedirect(path);

                    }
                    return nullptr;
                }
                else if(key == "WriteHeaders") ctx->WriteHeaders();
                else if(key == "StartWebSocketSession") {
                    TDictionary* dict;
                    TClassObject* cls;
                    if(GetArgumentHeap(args,0,dict))
            {
                
            
                ctx->StartWebSocketSession([dict,&ls](std::function<void(WebSocketMessage&)> sendMessage,std::function<void()> ping,std::function<void()> close)->void{
                    GCList ls2(ls.GetGC());
                    dict->CallMethod(ls2,"Open",{
                        TExternalMethod::Create(ls2,"Send a message",{"messageTextOrByteArray"},[sendMessage](GCList& ls,std::vector<TObject> args)->TObject{
                            std::string str;
                            TByteArray* bArray;
                            if(GetArgument(args,0,str))
                            {
                                WebSocketMessage msg(str);
                                sendMessage(msg);
                            }
                            else if(GetArgumentHeap(args,0,bArray))
                            {
                                WebSocketMessage msg(bArray->data);
                                sendMessage(msg);
                            }
                            return nullptr;
                        }),
                        TExternalMethod::Create(ls2, "Ping client", {},[ping](GCList& ls,std::vector<TObject> args)->TObject {
                            ping();
                            return nullptr;
                        }),
                        TExternalMethod::Create(ls2, "Close client",{},[close](GCList& ls,std::vector<TObject> args)->TObject {
                            close();
                            return nullptr;
                        })
                    });
                }, [dict,&ls](WebSocketMessage& msg)->void {
                    GCList ls2(ls.GetGC());

                    TObject v;

                    if(msg.isBinary)
                    {
                        auto r = TByteArray::Create(ls2);
                        r->data = msg.data;
                        v = r;
                    }
                    else
                    {
                        v = msg.ToString();
                    }
                    
                    dict->CallMethod(ls2,"Receive",{v});
                }, [dict,&ls](bool close)->void {
                    GCList ls2(ls.GetGC());
                    dict->CallMethod(ls2,"Close",{close});
                });
                    } else if(GetArgumentHeap(args, 0, cls))
            {
                ctx->StartWebSocketSession([cls,&ls](std::function<void(WebSocketMessage&)> sendMessage,std::function<void()> ping,std::function<void()> close)->void{
                    GCList ls2(ls.GetGC());
                    cls->CallMethod(ls2,"","Open",{
                        TExternalMethod::Create(ls2,"Send a message",{"messageTextOrByteArray"},[sendMessage](GCList& ls,std::vector<TObject> args)->TObject{
                            std::string str;
                            TByteArray* bArray;
                            if(GetArgument(args,0,str))
                            {
                                WebSocketMessage msg(str);
                                sendMessage(msg);
                            }
                            else if(GetArgumentHeap(args,0,bArray))
                            {
                                WebSocketMessage msg(bArray->data);
                                sendMessage(msg);
                            }
                            return nullptr;
                        }),
                        TExternalMethod::Create(ls2, "Ping client", {},[ping](GCList& ls,std::vector<TObject> args)->TObject {
                            ping();
                            return nullptr;
                        }),
                        TExternalMethod::Create(ls2, "Close client",{},[close](GCList& ls,std::vector<TObject> args)->TObject {
                            close();
                            return nullptr;
                        })
                    });
                }, [cls,&ls](WebSocketMessage& msg)->void {
                    GCList ls2(ls.GetGC());

                    TObject v;

                    if(msg.isBinary)
                    {
                        auto r = TByteArray::Create(ls2);
                        r->data = msg.data;
                        v = r;
                    }
                    else
                    {
                        v = msg.ToString();
                    }
                    
                    cls->CallMethod(ls2,"","Receive",{v});
                }, [cls,&ls](bool close)->void {
                    GCList ls2(ls.GetGC());
                    cls->CallMethod(ls2,"","Close",{close});
                });
            }
           
                }
            
                return Undefined();
            }

            void Finish()
            {
                this->ctx = nullptr;
            }
    };
   
    TObjectHttpServer::TObjectHttpServer(GC* gc,TObject obj)
    {
        this->ls=new GCList(gc);
        this->ls->Add(obj);
        this->obj = obj;
    }
 
    class TDictionaryHttpRequestBody : public HttpRequestBody
    {
        GC* gc;
        TDictionary* req;
        public:
            TDictionaryHttpRequestBody(GC* gc,TDictionary* req)
            {
                this->gc = gc;
                this->req = req;
            }

            void HandleHeaders(HttpDictionary& dict)
            {
                GCList ls(gc);
                auto dummy = TNativeObject::Create<DummyTNativeObjectThatReturnsHttpDictionary>(ls);
                auto res=TNativeObject::Create<THttpDictionary>(ls,&dict, dummy);
                req->CallMethod(ls,"HandleHeaders",{res});
                dummy->Close();
            }
            void Write(std::shared_ptr<Tesses::Framework::Streams::Stream> strm)
            {
                GCList ls(gc);
                
                req->CallMethod(ls,"Write",{strm});
                
            }

            ~TDictionaryHttpRequestBody()
            {
                GCList ls(gc);
                req->CallMethod(ls,"Close",{});
            }
    };
  
    bool TObjectHttpServer::Handle(ServerContext& ctx)
    {
        TCallable* callable;
        TDictionary* dict;
        TClassObject* clsObj;
        if(GetObjectHeap<TCallable*>(this->obj,callable))
        {
            GCList ls2(this->ls->GetGC());
            auto res = TNativeObject::Create<TServerContext>(ls2, &ctx);
            bool result;
            auto out = callable->Call(ls2,{res});
            if(GetObject(out,result))
            {
                res->Finish();
                return result;
            }
            res->Finish();
        }
        else if(GetObjectHeap<TDictionary*>(this->obj,dict))
        {
            GCList ls2(this->ls->GetGC());
            auto res = TNativeObject::Create<TServerContext>(ls2, &ctx);
            bool result;
            auto out = dict->CallMethod(ls2,"Handle",{res});
            if(GetObject(out,result))
            {
                res->Finish();
                return result;
            }
            res->Finish();
        }
        else if(GetObjectHeap(this->obj,clsObj) && clsObj->HasMethod("","Handle"))
        {
            GCList ls2(this->ls->GetGC());
            auto res = TNativeObject::Create<TServerContext>(ls2, &ctx);
            bool result;
            this->ls->GetGC()->BarrierBegin();
            auto callableO = clsObj->GetValue("","Handle");
            this->ls->GetGC()->BarrierEnd();
            TCallable* callable;
            if(GetObjectHeap(callableO, callable))
            {
                auto out = callable->Call(ls2,{res});
                if(GetObject(out,result))
                {
                    res->Finish();
                    return result;
                }
                res->Finish();
            }
            
        }

        return false;
    }
    TObjectHttpServer::~TObjectHttpServer()
    {
        delete ls;
    }
 
    static TObject Net_UrlDecode(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 1 && std::holds_alternative<std::string>(args[0]))
        {   
            return HttpUtils::UrlDecode(std::get<std::string>(args[0]));
        }
        return Undefined();
    }
    static TObject Net_HtmlEncode(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 1 && std::holds_alternative<std::string>(args[0]))
        {   
            return  HttpUtils::HtmlEncode(std::get<std::string>(args[0]));
        }
        return Undefined();
    }
    static TObject Net_UrlEncode(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 1 && std::holds_alternative<std::string>(args[0]))
        {   
            return  HttpUtils::UrlEncode(std::get<std::string>(args[0]));
        }
        return Undefined();
    }
    static TObject Net_UrlPathDecode(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 1 && std::holds_alternative<std::string>(args[0]))
        {   
            return  HttpUtils::UrlPathDecode(std::get<std::string>(args[0]));
        }
        return Undefined();
    }
    static TObject Net_UrlPathEncode(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 1 && std::holds_alternative<std::string>(args[0]))
        {   
            return HttpUtils::UrlPathEncode(std::get<std::string>(args[0]));
        }
        return Undefined();
    }
    static TObject New_NetworkStream(GCList& ls, std::vector<TObject> args)
    {
        bool ipv6;
        bool datagram;
        if(GetArgument(args,0,ipv6) && GetArgument(args,1,datagram))
        {
            return std::make_shared<NetworkStream>(ipv6,datagram);
        }
        return nullptr;
    }
    class HttpServerNativeObject : public TNativeObject
    {
        HttpServer* server;
        public:
            HttpServerNativeObject(uint16_t port, std::shared_ptr<IHttpServer> httpServer,bool printIps)
            {
                server=new HttpServer(port,httpServer,printIps);
            }
            HttpServerNativeObject(std::string unixPath, std::shared_ptr<IHttpServer> httpServer)
            {
                server=new HttpServer(unixPath,httpServer);
            }
            std::string TypeName()
            {
                return "Net.Http.HttpServer";
            }
            TObject CallMethod(GCList& ls, std::string key, std::vector<TObject> args)
            {
                if(key == "getPort")
                {
                    return (int64_t)server->GetPort();
                }
                if(key == "StartAccepting")
                    server->StartAccepting();
                return nullptr;
            }
            ~HttpServerNativeObject()
            {
                delete server;
            }
    };

    static TObject New_HttpServer(GCList& ls, std::vector<TObject> args,TRootEnvironment* env)
    {
        int64_t port;
        std::string pathStr;
        if(GetArgument(args,1,port))
        {
            bool printIPs=true;
            GetArgument(args,2,printIPs);
            
            std::shared_ptr<IHttpServer> httpSvr = ToHttpServer(ls.GetGC(),args[0]);
            
            if(httpSvr) {
                uint16_t p = (uint16_t)port;
                return TNativeObject::Create<HttpServerNativeObject>(ls,port,httpSvr,printIPs);
            }
            
        }

        if(GetArgument(args,1,pathStr) && env->permissions.canRegisterLocalFS)
        {
            std::shared_ptr<IHttpServer> httpSvr = ToHttpServer(ls.GetGC(),args[0]);
            
            
            if(httpSvr) {
                return TNativeObject::Create<HttpServerNativeObject>(ls,pathStr,httpSvr);
            }


        }
        return nullptr;
    }
    static TObject Net_Http_ListenSimpleWithLoop(GCList& ls, std::vector<TObject> args)
    {
        int64_t port;
        if(GetArgument(args,1,port))
        {
            std::shared_ptr<IHttpServer> httpSvr = ToHttpServer(ls.GetGC(),args[0]);
            
            if(httpSvr) {
                uint16_t p = (uint16_t)port;
                HttpServer server(p,httpSvr);
                server.StartAccepting();
                Tesses::Framework::TF_RunEventLoop();
            }
            
        }
        return nullptr;
    }
    static TObject Net_Http_ListenOnUnusedPort(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        std::shared_ptr<IHttpServer> httpSvr=ToHttpServer(ls.GetGC(),args.front());
        if(httpSvr) {
                uint16_t p = 0;
                HttpServer server(p,httpSvr);
                std::cout << "Port: " << server.GetPort() << std::endl;
                server.StartAccepting();
                
                Tesses::Framework::TF_RunEventLoop();
        }
        
        return nullptr;
    }
    static TObject Net_Http_MimeType(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath p;
        if(GetArgumentAsPath(args,0,p))
        {
            std::filesystem::path p2 = p.GetFileName();
            return HttpUtils::MimeType(p2);
        }
        return std::string("application/octet-stream");
    }
    class THttpRequestBody : public TNativeObject {
        HttpRequestBody* body;
        public:
            THttpRequestBody(HttpRequestBody* body)
            {   
                this->body = body;
            }
            bool IsClosed()
            {
                return body == nullptr;
            }
            HttpRequestBody* GetBody()
            {   
                return this->body;
            }

            std::string TypeName()
            {
                return "Net.Http.HttpRequestBody";
            }

            TObject CallMethod(GCList& ls, std::string key, std::vector<TObject> args)
            {
                return Undefined();
            }

            void Close()
            {
                delete this->body;
                this->body = nullptr;
            }
    };

    class THttpResponse : public TNativeObjectThatReturnsHttpDictionary
    {
        HttpResponse* response;
        public:
            THttpResponse(HttpResponse* resp)
            {
                this->response = resp;
            }
            bool IsAvailable()
            {
                return this->response != nullptr;
            }
        
            void Close()
            {
                delete this->response;
                this->response = nullptr;
            }

            ~THttpResponse()
            {
                delete this->response;
                this->response = nullptr;
            }
            std::string TypeName()
            {
                return "Net.Http.HttpResponse";
            }

            TObject CallMethod(GCList& ls, std::string key, std::vector<TObject> args)
            {
                if(!this->IsAvailable()) return Undefined();

                else if(key == "CopyToStream")
                {
                    std::shared_ptr<Stream> strm;
                    if(GetArgument(args,0,strm))
                    {
                        response->CopyToStream(strm);
                    }
                }
                else if(key == "ReadAsString")
                {
                    return response->ReadAsString();
                }
                else if(key == "ReadAsJson")
                {
                    return Json_Decode(ls, response->ReadAsString());
                }
                else if(key == "ReadAsStream")
                {
                    
                   
                    return response->ReadAsStream();
                }
                else if(key == "getStatusCode")
                {
                    return (int64_t)response->statusCode;
                }
                else if(key == "getVersion")
                {
                    return response->version;
                }
                else if(key == "getResponseHeaders")
                {
                    return TNativeObject::Create<THttpDictionary>(ls,&response->responseHeaders, this);
                }

                return Undefined();
            }
    };
   
    static TObject Net_Http_MakeRequest(GCList& ls, std::vector<TObject> args,TRootEnvironment* env)
    {
        auto gc = ls.GetGC();
        std::string url;
       
        if(GetArgument(args,0,url))
        {
            TDictionary* body1=nullptr;
            THttpRequestBody* body2=nullptr;
            HttpRequest req;
            req.method = "GET";
            req.ignoreSSLErrors=false;
            req.followRedirects=false;
            req.trusted_root_cert_bundle="";
            req.url = url;
            req.body = nullptr;
            
            TDictionary* options;
            if(GetArgumentHeap(args,1,options))
            {
                gc->BarrierBegin();
                auto _obj = options->GetValue("Method");
                GetObject(_obj,req.method);
                _obj = options->GetValue("IgnoreSSLErrors");
                GetObject(_obj,req.ignoreSSLErrors);
                _obj = options->GetValue("FollowRedirects");
                GetObject(_obj,req.followRedirects);
                _obj = options->GetValue("TrustedRootCertBundle");
                GetObject(_obj,req.trusted_root_cert_bundle);
                if(env->permissions.canRegisterLocalFS)
                {
                    _obj = options->GetValue("UnixSocket");
                    GetObject(_obj,req.unixSocket);
                }
                
                _obj = options->GetValue("RequestHeaders");

                TList* headers;

                if(GetObjectHeap(_obj, headers))
                {
                    for(int64_t index = 0; index < headers->Count();index ++)
                    {
                        _obj = headers->Get(index);
                        TDictionary* dict;
                        if(GetObjectHeap(_obj,dict))
                        {
                            std::string key={};
                            std::string value={};
                            _obj = dict->GetValue("Key");
                            GetObject(_obj,key);
                            _obj = dict->GetValue("Value");
                             GetObject(_obj,value);

                            req.requestHeaders.AddValue(key,value);
                        }
                    }
                }
                _obj = options->GetValue("Body");

                
                
                if(GetObjectHeap(_obj, body1))
                {
                    req.body = new TDictionaryHttpRequestBody(gc,body1);                   
                }
                else if(GetObjectHeap(_obj,body2) && !body2->IsClosed())
                {
                    req.body = body2->GetBody();
                }

                
                gc->BarrierEnd();
            }


            HttpResponse* resp = new HttpResponse(req);
            
            if(req.body != nullptr)
            {
                if(body2 != nullptr)
                {
                    body2->Close();
                }
                else if(body1 != nullptr)
                {
                    delete req.body;
                }
            }


            return TNativeObject::Create<THttpResponse>(ls, resp);

        }

        
        return nullptr;
    }
    
    static TObject Net_Smtp_Send(GCList& ls, std::vector<TObject> args)
    {
        TDictionary* dict;
        if(GetArgumentHeap(args,0,dict))
        {
            //the body should be either type text/plain or text/html
            //the body and attachment data can also point to bytearray of stream
            //server can also be a stream
            //as of right now the email is fire and forget (ie no error checking)
            //check function return type just in case (this function returns a empty string if no error)
            //we rolled our own smtp client
            
            /*
                dict looks like this from crosslang's point of view
                {
                    server = { 
                        host = "smtp.example.com",
                        tls = true
                    },
                    auth = {
                        username = "from",
                        password = "THEPASSWORD"
                    },
                    domain = "example.com",
                    from = {
                        name = "The name shown in the mail where it is from",
                        email = "from@example.com"
                    },
                    to = "to@example.com",
                    subject = "My little message",
                    body = {
                        type = "text/html",
                        data = "<h1>Hello, world</h1>"
                    },
                    attachments = [
                        {
                            name = "myimg.png",
                            type = "image/png",
                            data = embed("myimg.png")
                        }
                    ]
                }
            */
            ls.GetGC()->BarrierBegin();
            auto server = dict->GetValue("server");
            TDictionary* dict2;
            std::shared_ptr<Tesses::Framework::Streams::Stream> strm;
            bool ownsStream=true;
            std::shared_ptr<Stream> objStrm;
            if(GetObjectHeap(server,dict2))
            {
                auto tlsO = dict2->GetValue("tls");
                auto hostO = dict2->GetValue("host");
                auto portO = dict2->GetValue("port");
                std::string host;
                bool tls=false;
                int64_t port;
                GetObject(tlsO,tls);
                
                if(!GetObject(portO, port)) port = tls ? 465 : 25;
                
                GetObject(hostO,host);
                strm = std::make_shared<NetworkStream>(host,(uint16_t)port,false,false,false);
                if(tls)
                {
                    strm = std::make_shared<Framework::Crypto::ClientTLSStream>(strm,true,host);
                }

            }
            else if (GetObject(server, objStrm)) {
                ownsStream=false;
                strm = objStrm;  
            }

            Tesses::Framework::Mail::SMTPClient client(strm);
            auto o = dict->GetValue("domain");
            
            GetObject(o,client.domain);
            o = dict->GetValue("to");
            GetObject(o,client.to);
            o = dict->GetValue("subject");
            GetObject(o,client.subject);
            o = dict->GetValue("auth");
            if(GetObjectHeap(o, dict2))
            {
                o = dict2->GetValue("username");
                GetObject(o,client.username);
                o = dict2->GetValue("password");
                GetObject(o, client.password);
            }
            o = dict->GetValue("from");
            if(GetObjectHeap(o, dict2))
            {
                o = dict2->GetValue("email");
                GetObject(o,client.from);
                o = dict2->GetValue("name");
                GetObject(o, client.from_name);
            }
            o = dict->GetValue("body");
            if(GetObjectHeap(o, dict2))
            {
                //type, data
                std::string type = "text/plain";
                o = dict2->GetValue("type");
                GetObject(o,type);
                o = dict2->GetValue("data");
                client.body =  TObjectToSMTPBody(ls,type,o);
            }
            o = dict->GetValue("attachments");
            TList* als;
            if(GetObjectHeap(o,als))
            {
                for(int64_t i = 0; i < als->Count(); i++)
                {
                    auto item = als->Get(i);
                    if(GetObjectHeap(item, dict2))
                    {
                        o = dict2->GetValue("name");
                        std::string name;
                        GetObject(o,name);
                        std::string type = "text/plain";
                        o = dict2->GetValue("type");
                        GetObject(o,type);
                        o = dict2->GetValue("data");

                        client.attachments.push_back(std::pair<std::string,std::shared_ptr<SMTPBody>>(name,TObjectToSMTPBody(ls, type, o)));
                    }
                }
            }

            ls.GetGC()->BarrierEnd();
            client.Send();
            return "";
        }
        return nullptr;
    }
    static TObject Net_Http_WebSocketClient(GCList& ls, std::vector<TObject> args)
    {
        std::string url;
        TList* headers;
        TDictionary* dict;
        TClassObject* co;

        TCallable* callable=nullptr;

        

        TObject _obj;
        if(GetArgument(args,0,url) && GetArgumentHeap(args,1,headers) && GetArgumentHeap(args,2,dict))
        {
            GetArgumentHeap(args,3,callable);
            HttpDictionary hdict;
            for(int64_t index = 0; index < headers->Count();index ++)
            {
                _obj = headers->Get(index);
                TDictionary* dict;
                if(GetObjectHeap(_obj,dict))
                {
                    std::string key={};
                    std::string value={};
                    _obj = dict->GetValue("Key");
                    GetObject(_obj,key);
                    _obj = dict->GetValue("Value");
                    GetObject(_obj,value);
                    hdict.AddValue(key,value);
                }
            }
            auto conn = std::make_shared<CallbackWebSocketConnection>([dict,&ls](std::function<void(WebSocketMessage&)> sendMessage,std::function<void()> ping,std::function<void()> close)->void{
                GCList ls2(ls.GetGC());
                dict->CallMethod(ls2,"Open",{
                    TExternalMethod::Create(ls2,"Send a message",{"messageTextOrByteArray"},[sendMessage](GCList& ls,std::vector<TObject> args)->TObject{
                        std::string str;
                        TByteArray* bArray;
                        if(GetArgument(args,0,str))
                        {
                            WebSocketMessage msg(str);
                            sendMessage(msg);
                        }
                        else if(GetArgumentHeap(args,0,bArray))
                        {
                            WebSocketMessage msg(bArray->data);
                            sendMessage(msg);
                        }
                        return nullptr;
                    }),
                    TExternalMethod::Create(ls2, "Ping client", {},[ping](GCList& ls,std::vector<TObject> args)->TObject {
                        ping();
                        return nullptr;
                    }),
                    TExternalMethod::Create(ls2, "Close client",{},[close](GCList& ls,std::vector<TObject> args)->TObject {
                        close();
                        return nullptr;
                    })
                });
            }, [dict,&ls](WebSocketMessage& msg)->void {
                GCList ls2(ls.GetGC());

                TObject v;

                if(msg.isBinary)
                {
                    auto r = TByteArray::Create(ls2);
                    r->data = msg.data;
                    v = r;
                }
                else
                {
                    v = msg.ToString();
                }
                
                dict->CallMethod(ls2,"Receive",{v});
            }, [dict,&ls](bool close)->void {
                GCList ls2(ls.GetGC());
                dict->CallMethod(ls2,"Close",{close});
            });
            WebSocketClient(url, hdict, conn, [&ls,callable](Tesses::Framework::Http::HttpDictionary& dict, bool success)->bool {
                if(callable != nullptr)
                {
                    auto dummy = TNativeObject::Create<DummyTNativeObjectThatReturnsHttpDictionary>(ls);
                    bool res= ToBool(callable->Call(ls,{TNativeObject::Create<THttpDictionary>(ls,&dict,dummy),success}));
                    dummy->Close();
                    return res;
                }
                return true;
            });
        }
        else if(GetArgument(args,0, url) && GetArgumentHeap(args,1,co)) {
            TObject hobj;
            if(co->HasMethod("","GetRequestHeaders"))
                hobj = co->CallMethod(ls,"","GetRequestHeaders",{});
            else if(co->HasField("","RequestHeaders"))
                hobj = co->GetValue("","RequestHeaders");
            else return Undefined();

            if(!GetObjectHeap(hobj,headers)) return Undefined();

            HttpDictionary hdict;
            for(int64_t index = 0; index < headers->Count();index ++)
            {
                _obj = headers->Get(index);
                TDictionary* dict;
                if(GetObjectHeap(_obj,dict))
                {
                    std::string key={};
                    std::string value={};
                    _obj = dict->GetValue("Key");
                    GetObject(_obj,key);
                    _obj = dict->GetValue("Value");
                    GetObject(_obj,value);
                    hdict.AddValue(key,value);
                }
            }

             auto conn = std::make_shared<CallbackWebSocketConnection>([co,&ls](std::function<void(WebSocketMessage&)> sendMessage,std::function<void()> ping,std::function<void()> close)->void{
                GCList ls2(ls.GetGC());
                co->CallMethod(ls2,"","Open",{
                    TExternalMethod::Create(ls2,"Send a message",{"messageTextOrByteArray"},[sendMessage](GCList& ls,std::vector<TObject> args)->TObject{
                        std::string str;
                        TByteArray* bArray;
                        if(GetArgument(args,0,str))
                        {
                            WebSocketMessage msg(str);
                            sendMessage(msg);
                        }
                        else if(GetArgumentHeap(args,0,bArray))
                        {
                            WebSocketMessage msg(bArray->data);
                            sendMessage(msg);
                        }
                        return nullptr;
                    }),
                    TExternalMethod::Create(ls2, "Ping client", {},[ping](GCList& ls,std::vector<TObject> args)->TObject {
                        ping();
                        return nullptr;
                    }),
                    TExternalMethod::Create(ls2, "Close client",{},[close](GCList& ls,std::vector<TObject> args)->TObject {
                        close();
                        return nullptr;
                    })
                });
            }, [co,&ls](WebSocketMessage& msg)->void {
                GCList ls2(ls.GetGC());

                TObject v;

                if(msg.isBinary)
                {
                    auto r = TByteArray::Create(ls2);
                    r->data = msg.data;
                    v = r;
                }
                else
                {
                    v = msg.ToString();
                }
                
                co->CallMethod(ls2,"","Receive",{v});
            }, [co,&ls](bool close)->void {
                GCList ls2(ls.GetGC());
                co->CallMethod(ls2,"","Close",{close});
            });
            WebSocketClient(url, hdict, conn, [&ls,co](Tesses::Framework::Http::HttpDictionary& dict, bool success)->bool {
                if(co->HasMethod("","HandleResponseHeaders"))
                {
                    auto dummy = TNativeObject::Create<DummyTNativeObjectThatReturnsHttpDictionary>(ls);
                    bool res= ToBool(co->CallMethod(ls,"","HandleResponseHeaders",{TNativeObject::Create<THttpDictionary>(ls,&dict,dummy),success}));
                    dummy->Close();
                    return res;
                }
               
                return true;
            });
        }
        return Undefined();
        //Net.Http.CreateWebSocketConnection("wss://example.com/",[],conn, (dict, success)=>{ return true;})
    }
    static TObject Net_Http_DownloadToString(GCList& ls, std::vector<TObject> args)
    {
        std::string url;
        if(GetArgument(args,0,url))
        return DownloadToStringSimple(url);
        return nullptr;
    }
    static TObject Net_Http_DownloadToStream(GCList& ls, std::vector<TObject> args)
    {
        std::string url;
        std::shared_ptr<Stream> strm;
        if(GetArgument(args,0,url) && GetArgument(args,1,strm))
        {
            DownloadToStreamSimple(url,strm);
        }
        return nullptr;
    }
    static TObject Net_Http_DownloadToFile(GCList& ls, std::vector<TObject> args)
    {
        std::string url;
        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
        Tesses::Framework::Filesystem::VFSPath path;
        if(GetArgument(args,0,url) && GetArgument(args,1,vfs) && GetArgumentAsPath(args,2, path))
        {
            DownloadToFileSimple(url,vfs,path);
        }
        return nullptr;
    }
    bool IHttpServer_Handle(std::shared_ptr<IHttpServer> svr,std::vector<TObject>& args)
    {
        TServerContext* ctx;
        if(GetArgumentHeap(args,0,ctx))
        {
            return svr->Handle(*ctx->GetContext());
        }
        return false;
    }

    std::shared_ptr<IHttpServer> ToHttpServer(GC* gc, TObject obj)
    {
        if(std::holds_alternative<std::shared_ptr<IHttpServer>>(obj)) return std::get<std::shared_ptr<IHttpServer>>(obj);
        TDictionary* dict;
        TClassObject* clo;
        TCallable* call;
        if(GetObjectHeap(obj,dict))
        {
            return std::make_shared<TObjectHttpServer>(gc,dict); 
        }
        else if(GetObjectHeap(obj,clo))
        {
            return std::make_shared<TObjectHttpServer>(gc,clo); 
        }
        else if(GetObjectHeap(obj,call))
        {
            return std::make_shared<TObjectHttpServer>(gc,call); 
        }
        return nullptr;
    }

    static TObject Net_Http_HtmlP(GCList& ls, std::vector<TObject> args)
    {
        std::string str;
        if(GetArgument(args,0,str))
        {
            return HttpUtils::HtmlP(str);
        }
        return "";
    }

    static TObject Net_Http_StatusCodeString(GCList& ls, std::vector<TObject> args)
    {
        int64_t sc;
        if(GetArgument(args,0,sc))
        {
            return HttpUtils::StatusCodeString((StatusCode)sc);
        }
        return "";
    }
    static TObject New_FileServer(GCList& ls, std::vector<TObject> args)
    {
        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
        bool allowlisting;
        bool spa;
        if(GetArgument(args,0,vfs) && GetArgument(args,1,allowlisting) && GetArgument(args,2,spa))
        {
            return std::make_shared<FileServer>(vfs,allowlisting,spa);
                
        }
        return nullptr;
    }
    static TObject New_BasicAuthServer(GCList& ls, std::vector<TObject> args)
    {
         auto bAuth = std::make_shared<BasicAuthServer>();
           
            TCallable* cb;
            if(!args.empty())
                bAuth->server = ToHttpServer(ls.GetGC(),args[0]);
                
            if(GetArgumentHeap(args,1,cb))
            {
                auto marked= CreateMarkedTObject(ls, cb);
                bAuth->authorization = [marked](std::string user,std::string password)->bool {
                    GCList ls(marked->GetGC());
                    TCallable* callable;
                    if(GetObjectHeap(marked->GetObject(), callable))
                    {
                        return ToBool(callable->Call(ls,{user,password}));
                    }   
                    return false;
                };
            }
            GetArgument(args,2,bAuth->realm);
            return bAuth;
    }
    static TObject New_MountableServer(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return std::make_shared<MountableServer>();
        return std::make_shared<MountableServer>(ToHttpServer(ls.GetGC(),args[0]));
    }
    static TObject New_RouteServer(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return std::make_shared<RouteServer>();
        return std::make_shared<RouteServer>(ToHttpServer(ls.GetGC(),args[0]));
    }
    static TObject New_StreamHttpRequestBody(GCList& ls, std::vector<TObject> args)
    {
        std::shared_ptr<Stream> strm;
        std::string mimeType;

        if(GetArgument(args,0,strm) && GetArgument(args, 1, mimeType))
        {
            return TNativeObject::Create<THttpRequestBody>(ls, new StreamHttpRequestBody(strm, mimeType));
        }
        return nullptr;
    }
    static TObject New_TextHttpRequestBody(GCList& ls, std::vector<TObject> args)
    {
        std::string text;
        std::string mimeType;
        if(GetArgument(args, 0, text) && GetArgument(args, 1, mimeType))
        {
            return TNativeObject::Create<THttpRequestBody>(ls,new TextHttpRequestBody(text, mimeType));
            
        }
        return nullptr;
    }
    static TObject New_JsonHttpRequestBody(GCList& ls, std::vector<TObject> args)
    {
            
        if(!args.empty())
        {
            return TNativeObject::Create<THttpRequestBody>(ls,new TextHttpRequestBody(Json_Encode(args[0]), "application/json"));
                
        }
        return nullptr;
        
    }
    
    void TStd::RegisterNet(GC* gc, TRootEnvironment* env)
    {

        env->permissions.canRegisterNet=true;
        GCList ls(gc);

        gc->BarrierBegin();
        TDictionary* dict = env->EnsureDictionary(gc,"Net");
        TDictionary* _new = env->EnsureDictionary(gc,"New");
        _new->DeclareFunction(gc, "StreamHttpRequestBody","Create a stream request body",{"stream","mimeType"},New_StreamHttpRequestBody);
        _new->DeclareFunction(gc, "TextHttpRequestBody","Create a text request body",{"text","mimeType"},New_TextHttpRequestBody);
        _new->DeclareFunction(gc, "JsonHttpRequestBody","Create a text request body",{"json"},New_JsonHttpRequestBody);
        _new->DeclareFunction(gc, "HttpServer", "Create a http server (allows multiple)",{"server","portOrUnixPath","$printIPs"},[env](GCList& ls, std::vector<TObject> args)->TObject{
            return New_HttpServer(ls,args,env);
        });

        _new->DeclareFunction(gc, "RouteServer","Create a routeserver",{"$root"}, New_RouteServer);
        _new->DeclareFunction(gc, "FileServer","Create a file server",{"vfs","allowlisting","spa"}, New_FileServer);
        _new->DeclareFunction(gc, "BasicAuthServer", "Create a basic auth server", {"$server","$auth","$realm"},New_BasicAuthServer);
        _new->DeclareFunction(gc, "MountableServer","Create a server you can mount to, must mount parents before child",{"root"}, New_MountableServer);
        _new->DeclareFunction(gc, "NetworkStream","Create a network stream",{"ipv6","datagram"},New_NetworkStream);
        
        

        TDictionary* http = TDictionary::Create(ls);
        TDictionary* smtp = TDictionary::Create(ls);
        http->DeclareFunction(gc, "StatusCodeString", "Get the status code string",{"statusCode"},Net_Http_StatusCodeString);
        http->DeclareFunction(gc, "HtmlP", "Linkify text", {"text"},Net_Http_HtmlP);
        http->DeclareFunction(gc, "HtmlEncode","Html encode",{"param"}, Net_HtmlEncode);
       
        http->DeclareFunction(gc, "UrlEncode","Url encode query param",{"param"}, Net_UrlEncode);
        http->DeclareFunction(gc, "UrlDecode","Url decode query param",{"param"}, Net_UrlDecode);
        http->DeclareFunction(gc, "UrlPathEncode","Url encode path",{"path"}, Net_UrlPathEncode);
        http->DeclareFunction(gc, "UrlPathDecode","Url decode path",{"path"}, Net_UrlPathDecode);
        http->DeclareFunction(gc, "MimeType","Get mimetype from extension",{"ext"},Net_Http_MimeType);

       
        http->DeclareFunction(gc, "StreamHttpRequestBody","Create a stream request body",{"stream","mimeType"},New_StreamHttpRequestBody);
        http->DeclareFunction(gc, "TextHttpRequestBody","Create a text request body",{"text","mimeType"},New_TextHttpRequestBody);
        http->DeclareFunction(gc, "JsonHttpRequestBody","Create a text request body",{"json"},New_JsonHttpRequestBody);
        http->DeclareFunction(gc, "MakeRequest", "Create an http request", {"url","$extra"}, [env](GCList& ls, std::vector<TObject> args)->TObject {return Net_Http_MakeRequest(ls,args,env);});
        http->DeclareFunction(gc, "WebSocketClient", "Create a websocket connection",{"url","headers","conn","$successCB"},Net_Http_WebSocketClient);
        http->DeclareFunction(gc, "DownloadToString","Return the http file's contents as a string",{"url"},Net_Http_DownloadToString);
        http->DeclareFunction(gc, "DownloadToStream","Download file to stream",{"url","stream"},Net_Http_DownloadToStream);
        http->DeclareFunction(gc, "DownloadToFile","Download file to file in vfs",{"url","vfs","path"},Net_Http_DownloadToFile);
        http->DeclareFunction(gc, "HttpServer", "Create a http server (allows multiple)",{"server","portOrUnixPath","$printIPs"},[env](GCList& ls, std::vector<TObject> args)->TObject{
            return New_HttpServer(ls,args,env);
        });
        http->DeclareFunction(gc, "ListenSimpleWithLoop", "Listen (creates application loop)", {"server","port"},Net_Http_ListenSimpleWithLoop);
        http->DeclareFunction(gc, "ListenOnUnusedPort","Listen on unused localhost port and print Port: theport",{"server"},Net_Http_ListenOnUnusedPort);
        //FileServer svr()

        http->DeclareFunction(gc, "RouteServer","Create a routeserver",{"$root"}, New_RouteServer);
        http->DeclareFunction(gc, "FileServer","Create a file server",{"vfs","allowlisting","spa"}, New_FileServer);
        http->DeclareFunction(gc, "BasicAuthServer", "Create a basic auth server", {"$server","$auth","$realm"},New_BasicAuthServer);
        http->DeclareFunction(gc, "BasicAuthGetCreds","Get creds from str",{"ctx"},[](GCList& ls, std::vector<TObject> args)->TObject {
            TServerContext* sc;
            if(GetArgumentHeap(args,0,sc))
            {
                std::string user;
                std::string pass;
                if(sc->IsAvailable() && BasicAuthServer::GetCreds(*sc->GetContext(),user,pass))
                {
                    return TDictionary::Create(ls,{
                        TDItem("Username",user),
                        TDItem("Password",pass)
                    });
                }
            }
            return nullptr;
        });
        http->DeclareFunction(gc, "MountableServer","Create a server you can mount to, must mount parents before child",{"root"}, New_MountableServer);
        dict->DeclareFunction(gc, "NetworkStream","Create a network stream",{"ipv6","datagram"},New_NetworkStream);
        smtp->DeclareFunction(gc, "Send","Send email via smtp server",{"messageStruct"},Net_Smtp_Send);
        dict->DeclareFunction(gc, "getIPAddresses","Get the ip addresses of this machine",{"$ipv6"},[](GCList& ls, std::vector<TObject> args)->TObject{
            TList* a = TList::Create(ls);
            bool ipv6=false;
            GetArgument(args,0,ipv6);
            ls.GetGC()->BarrierBegin();
            for(auto item : Tesses::Framework::Streams::NetworkStream::GetIPs(ipv6))
            {
                a->Add(TDictionary::Create(ls,{
                    TDItem("Interface", item.first),
                    TDItem("Address", item.second)
                }));
            }
            ls.GetGC()->BarrierEnd();;
            return a;
        });
        dict->SetValue("Http", http);
        dict->SetValue("Smtp", smtp);
        gc->BarrierEnd();
    }

    Tesses::Framework::Http::ServerRequestHandler TCallable::ToRouteServerRequestHandler(GC* gc)
    {
        auto value = CreateMarkedTObject(gc,this);
        return [value,this](ServerContext& ctx)->bool {
            auto v=value;
            auto gc = v->GetGC();
            GCList ls(gc);
            auto res = TNativeObject::Create<TServerContext>(ls, &ctx);
            bool result;
            auto out = this->Call(ls,{res});
            if(GetObject(out,result))
            {
                res->Finish();
                return result;
            }
            res->Finish();

            return false;
        };
    }
}
