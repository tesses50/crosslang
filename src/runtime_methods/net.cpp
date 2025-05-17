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
    static SMTPBody* TObjectToSMTPBody(GCList& ls,std::string mimeType, TObject obj)
    {
        SMTPBody* body = nullptr;
        std::string text;
        TByteArray* ba;
        TStreamHeapObject* sho;
        if(GetObject(obj,text))
        {
            body = new SMTPStringBody(text,mimeType);
        }
        else if(GetObjectHeap(obj,ba)) {
            MemoryStream* ms = new MemoryStream(true);
            ms->WriteBlock(ba->data.data(), ba->data.size());
            ms->Seek(0L, SeekOrigin::Begin);
            
            body = new SMTPStreamBody(mimeType,ms,true);
        }
        else if(GetObjectHeap(obj,sho))
        {
            ls.Add(sho);
            body = new SMTPStreamBody(mimeType,sho->stream,false);
        }
        return body;
    }
     TServerHeapObject* TServerHeapObject::Create(GCList& ls, Tesses::Framework::Http::IHttpServer* svr)
    {
        TServerHeapObject* ho = new TServerHeapObject();
        ls.Add(ho);
        auto gc = ls.GetGC();
        gc->Watch(ho);
        ho->server = svr;
        return ho;
    }
    TServerHeapObject* TServerHeapObject::Create(GCList* ls, Tesses::Framework::Http::IHttpServer* svr)
    {
        TServerHeapObject* ho = new TServerHeapObject();
        ls->Add(ho);
        auto gc = ls->GetGC();
        gc->Watch(ho);
        ho->server = svr;
        return ho;
    }

    void TServerHeapObject::Close()
    {
        if(this->server != nullptr)
        {
            delete this->server;
            this->server = nullptr;
        }
    }
    TServerHeapObject::~TServerHeapObject()
    {
        if(this->server != nullptr)
        {
            delete this->server;
        }
    }
    static TDictionary* CreateDictionaryFromHttpDictionary(GCList& ls, Tesses::Framework::Http::HttpDictionary* dict0)
    {
        TDictionary* dict = TDictionary::Create(ls);
        
        auto gc =ls.GetGC();
        //gc->BarrierBegin();
        

        dict->DeclareFunction(gc,"AddValue","Add item",{"key","value"},[dict0](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            std::string key;
            std::string value;
            int64_t i64;
            double d64;
            TDateTime da;
            if(GetArgument(args2,0,key) )
            {
                if(GetArgument(args2,1,value))
                    dict0->AddValue(key,value);
                else if(GetArgument(args2,1,i64))
                    dict0->AddValue(key,i64);
                else if(GetArgument(args2,1,d64))
                    dict0->AddValue(key, d64);
                else if(GetArgument(args2,1,da))
                    dict0->AddValue(key, da.GetDate());
            }
            return nullptr;
        });

        dict->DeclareFunction(gc,"SetValue","Set item",{"key","value"},[dict0](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            std::string key;
            std::string value;
            int64_t i64;
            double d64;
            TDateTime da;
            if(GetArgument(args2,0,key) )
            {
                if(GetArgument(args2,1,value))
                    dict0->SetValue(key,value);
                else if(GetArgument(args2,1,i64))
                    dict0->SetValue(key,i64);
                else if(GetArgument(args2,1,d64))
                    dict0->SetValue(key, d64);
                else if(GetArgument(args2,1,da))
                    dict0->SetValue(key, da.GetDate());
            }
            return nullptr;
        });

        dict->DeclareFunction(gc,"Clear","Clear items",{},[dict0](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            dict0->Clear();
            return nullptr;
        });

        dict->DeclareFunction(gc,"GetFirstBoolean","Get First boolean",{"key"},[dict0](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            std::string key;
            if(GetArgument(args2,0,key))
            {
                return dict0->GetFirstBoolean(key);
            }
            return false;
        });
        dict->DeclareFunction(gc,"TryGetFirst","Try Get first string",{"key"},[dict0](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            std::string key;
            std::string value;
            if(GetArgument(args2,0,key) && dict0->TryGetFirst(key,value))
            {
                return value;
            }
            return nullptr;
        });
        dict->DeclareFunction(gc,"TryGetFirstDouble","Try Get first double",{"key"},[dict0](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            std::string key;
            double value;
            if(GetArgument(args2,0,key) && dict0->TryGetFirstDouble(key,value))
            {
                return value;
            }
            return nullptr;
        });
        dict->DeclareFunction(gc,"TryGetFirstInt","Try Get first integer",{"key"},[dict0](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            std::string key;
            int64_t value;
            if(GetArgument(args2,0,key) && dict0->TryGetFirstInt(key,value))
            {
                return value;
            }
            return nullptr;
        });
        dict->DeclareFunction(gc,"TryGetFirstDate","Try Get first date",{"key"},[dict0](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            std::string key;
            Tesses::Framework::Date::DateTime value;
            if(GetArgument(args2,0,key) && dict0->TryGetFirstDate(key,value))
            {
                return value;
            }
            return nullptr;
        });

        dict->DeclareFunction(gc, "ToList","To List",{}, [dict0](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            TList* ls = TList::Create(ls2);
            for(auto item : dict0->kvp)
            {
                for(auto i : item.second)
                {
                    auto d = TDictionary::Create(ls2);
                    ls2.GetGC()->BarrierBegin();
                    d->SetValue("Key", item.first);
                    d->SetValue("Value", i);
                    ls->Add(d);
                    ls2.GetGC()->BarrierEnd();
                }
            }
            return ls;
        });

        return dict;
    }

    static TDictionary* CreateDictionaryFromServerContext(GCList& ls,ServerContext* ctx)
    {
        TDictionary* dict = TDictionary::Create(ls);
        
        auto gc =ls.GetGC();
        gc->BarrierBegin();
        dict->SetValue("native",TNative::Create(ls,ctx,[](void*)->void {}));
        dict->SetValue("Encrypted",ctx->encrypted);
        dict->SetValue("Method",ctx->method);
        dict->SetValue("IP",ctx->ip);
        dict->SetValue("Port",(int64_t)ctx->port);
        dict->SetValue("OriginalPath",ctx->originalPath);
        dict->SetValue("QueryParams",CreateDictionaryFromHttpDictionary(ls,&ctx->queryParams));
        dict->SetValue("RequestHeaders",CreateDictionaryFromHttpDictionary(ls,&ctx->requestHeaders));
        dict->SetValue("ResponseHeaders",CreateDictionaryFromHttpDictionary(ls,&ctx->responseHeaders));
        gc->BarrierEnd();

        dict->DeclareFunction(gc,"GetStream","Get streams",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            return TStreamHeapObject::Create(ls2, &ctx->GetStream());
        });
        dict->DeclareFunction(gc,"OpenRequestStream","Open Request Stream",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            return TStreamHeapObject::Create(ls2, ctx->OpenRequestStream());
        });
        dict->DeclareFunction(gc,"OpenResponseStream","Open Response Stream",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            return TStreamHeapObject::Create(ls2, ctx->OpenResponseStream());
        });
        dict->DeclareFunction(gc, "ParseFormData","Parse the form data",{},[ctx](GCList& ls, std::vector<TObject> args)->TObject {
            TCallable* callable;
            if(GetArgumentHeap(args, 0, callable))
            {
                ctx->ParseFormData([callable,&ls](std::string a,std::string b, std::string c)->Tesses::Framework::Streams::Stream*{
                    auto res = callable->Call(ls,{a,b,c});
                    return new Tesses::CrossLang::TObjectStream(ls.GetGC(),res);
                });
            }
            return nullptr;
        });
        dict->DeclareFunction(gc,"getNeedToParseFormData","Check if Need to parse form data",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            return ctx->NeedToParseFormData();
        });

        dict->DeclareFunction(gc,"ReadString","Read string from request",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            return ctx->ReadString();
        });
        dict->DeclareFunction(gc, "ReadJson","Read json from request",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            return Json_Decode(ls2,ctx->ReadString());
        });
        dict->DeclareFunction(gc,"SendJson","Send object as json",{"object"},[ctx](Tesses::CrossLang::GCList& ls2, std::vector<TObject> args2)->TObject{
            if(args2.size() > 0)
            {
                ctx->WithMimeType("application/json").SendText(Json_Encode(args2[0]));
            }
            return nullptr;
        });

        dict->DeclareFunction(gc,"SendText","Send response text",{"text"},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            std::string text;
            if(GetArgument(args2,0,text))
                ctx->SendText(text);
            return nullptr;
        });
        dict->DeclareFunction(gc,"WithMimeType","Set mime type",{"mimeType"},[ctx,dict](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            std::string text;
            if(GetArgument(args2,0,text))
                ctx->WithMimeType(text);
            return dict;
        });
        dict->DeclareFunction(gc,"WithLastModified","Set last modified date",{"date"},[ctx,dict](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            TDateTime da;
            if(GetArgument(args2,0,da))
                ctx->WithLastModified(da.GetDate());
            return dict;
        });
        dict->DeclareFunction(gc,"WithContentDisposition","Set content disposition",{"filename","inline"},[ctx,dict](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            std::string filename;
            bool isInline;
            if(GetArgument(args2,0,filename) && GetArgument(args2,1,isInline))
                ctx->WithContentDisposition(filename,isInline);
            return dict;
        });
        dict->DeclareFunction(gc,"WithHeader","Add header",{"key","value"},[ctx,dict](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            std::string key;
            std::string value;
            if(GetArgument(args2,0,key) && GetArgument(args2,1,value))
                ctx->WithHeader(key,value);
            return dict;
        });
        dict->DeclareFunction(gc,"WithSingleHeader","Set header",{"key","value"},[ctx,dict](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            std::string key;
            std::string value;
            if(GetArgument(args2,0,key) && GetArgument(args2,1,value))
                ctx->WithSingleHeader(key,value);
            return dict;
        });
        
        dict->DeclareFunction(gc,"SendStream","Send stream",{"strm"},[ctx,dict](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            TStreamHeapObject* strmHeapObj;
            if(GetArgumentHeap(args2,0,strmHeapObj))
            {
                ctx->SendStream(strmHeapObj->stream);
            }
            return nullptr;
        });
        dict->DeclareFunction(gc,"SendBytes","Send bytes",{"ba"},[ctx,dict](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            TByteArray* ba;
            if(GetArgumentHeap(args2,0,ba))
                ctx->SendBytes(ba->data);
            return nullptr;
        });
        dict->DeclareFunction(gc,"WriteHeaders","Send the headers",{},[ctx](GCList& ls, std::vector<TObject> args)->TObject{
            ctx->WriteHeaders();
            return nullptr;
        });
        
//        dict->DeclareFunction(gc,"getUrlWithQuery","Get original path with query parameters",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {return ctx->GetUrlWithQuery();});
        dict->DeclareFunction(gc,"StartWebSocketSession","Start websocket session",{"dict"}, [ctx](GCList& ls,std::vector<TObject> args)->TObject {
            TDictionary* dict;
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
            }
            return nullptr;
        });      

        
        
        //dict->DeclareFunction(gc,"getOriginalPathWithQuery","Get original path with query parameters",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {return ctx->GetOriginalPathWithQuery();});
        dict->DeclareFunction(gc,"getPath","Get path",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {return ctx->path;});
        dict->DeclareFunction(gc,"setPath","Set path",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            std::string str;
            if(GetArgument(args2,0,str))
            {  
                ctx->path = str;
                return str;
            }
            return nullptr;
        });
        dict->DeclareFunction(gc,"getVersion","Get version",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {return ctx->version;});
        dict->DeclareFunction(gc,"setVersion","Set version",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            std::string str;
            if(GetArgument(args2,0,str))
            {  
                ctx->version = str;
                return str;
            }
            return nullptr;
        });

        dict->DeclareFunction(gc,"getStatusCode","Get status code",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {return (int64_t)ctx->statusCode;});
        dict->DeclareFunction(gc,"setStatusCode","Set status code",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            int64_t sc;
            if(GetArgument(args2,0,sc))
            {  
                ctx->statusCode = (StatusCode)sc;
                return sc;
            }
            return nullptr;
        });
        
        return dict;
    }

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
                auto res=CreateDictionaryFromHttpDictionary(ls,&dict);
                req->CallMethod(ls,"HandleHeaders",{res});
            }
            void Write(Tesses::Framework::Streams::Stream* strm)
            {
                GCList ls(gc);
                auto res=TStreamHeapObject::Create(ls,strm);
                req->CallMethod(ls,"Write",{res});
                res->stream=nullptr;
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
        TServerHeapObject* server;
        if(GetObjectHeap<TCallable*>(this->obj,callable))
        {
            GCList ls2(this->ls->GetGC());
            auto res = CreateDictionaryFromServerContext(ls2,&ctx);
            bool result;
            auto out = callable->Call(ls2,{res});
            if(GetObject(out,result))
            {
                return result;
            }
        }
        else if(GetObjectHeap<TDictionary*>(this->obj,dict))
        {
            GCList ls2(this->ls->GetGC());
            auto res = CreateDictionaryFromServerContext(ls2,&ctx);
            bool result;
            auto out = dict->CallMethod(ls2,"Handle",{res});
            if(GetObject(out,result))
            {
                return result;
            }
        }
        else if(GetObjectHeap<TServerHeapObject*>(this->obj,server))
        {
            return server->server->Handle(ctx);
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
    static TObject Net_NetworkStream(GCList& ls, std::vector<TObject> args)
    {
        bool ipv6;
        bool datagram;
        if(GetArgument(args,0,ipv6) && GetArgument(args,1,datagram))
        {
            return TStreamHeapObject::Create(ls,new NetworkStream(ipv6,datagram));
        }
        return nullptr;
    }
    static TObject Net_Http_ListenSimpleWithLoop(GCList& ls, std::vector<TObject> args)
    {
        int64_t port;
        if(GetArgument(args,1,port))
        {
            TObjectHttpServer httpServer(ls.GetGC(),args[0]);
            uint16_t p = (uint16_t)port;
            HttpServer server(p,httpServer);
            server.StartAccepting();
            Tesses::Framework::TF_RunEventLoop();
        }
        return nullptr;
    }
    static TObject Net_Http_ListenOnUnusedPort(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() > 0)
        {
            TObjectHttpServer httpServer(ls.GetGC(),args[0]);
            
            
            uint16_t port=0;
            HttpServer server(port,httpServer, false);
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

    static TObject Net_Http_MakeRequest(GCList& ls, std::vector<TObject> args)
    {
        auto gc = ls.GetGC();
        std::string url;
       
        if(GetArgument(args,0,url))
        {
            TDictionary* body1=nullptr;
            TNative* body2=nullptr;
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

                            req.requestHeaders.AddValue(key,value);
                        }
                    }
                }
                _obj = options->GetValue("Body");

                
                
                if(GetObjectHeap(_obj, body1))
                {
                    req.body = new TDictionaryHttpRequestBody(gc,body1);                   
                }
                else if(GetObjectHeap(_obj,body2) && !body2->GetDestroyed())
                {
                    req.body = static_cast<HttpRequestBody*>(body2->GetPointer());
                }

                
                gc->BarrierEnd();
            }


            HttpResponse* resp = new HttpResponse(req);
            
            if(req.body != nullptr)
            {
                if(body2 != nullptr)
                {
                    body2->Destroy();
                }
                else if(body1 != nullptr)
                {
                    delete req.body;
                }
            }

            TDictionary* dict = TDictionary::Create(ls);
            gc->BarrierBegin();

            TNative* nat = TNative::Create(ls,resp,[](void* ptr)->void{
                HttpResponse* resp0 = static_cast<HttpResponse*>(ptr);
                delete resp0;
            });

            dict->SetValue("native", nat);

            auto copyToStream = TExternalMethod::Create(ls,"Copy To a stream",{"stream"},[resp](GCList& ls, std::vector<TObject> args)->TObject{
                TStreamHeapObject* strm;
                if(GetArgumentHeap(args,0,strm))
                {
                    resp->CopyToStream(strm->stream);
                }
                return nullptr;
            });

            copyToStream->watch.push_back(dict);
            dict->SetValue("CopyToStream",copyToStream);


            auto readAsString = TExternalMethod::Create(ls,"Read as string",{},[resp](GCList& ls, std::vector<TObject> args)->TObject{
                return resp->ReadAsString();
            });

            readAsString->watch.push_back(dict);
            dict->SetValue("ReadAsString",readAsString);

            auto readAsStream = TExternalMethod::Create(ls,"Read as stream",{},[resp](GCList& ls, std::vector<TObject> args)->TObject{
                auto res = resp->ReadAsStream();
                return TStreamHeapObject::Create(ls, res);
            });

            readAsStream->watch.push_back(dict);
            dict->SetValue("ReadAsStream",readAsStream);
            dict->SetValue("StatusCode",(int64_t)resp->statusCode);

            dict->SetValue("Version",resp->version);

            dict->SetValue("ResponseHeaders",CreateDictionaryFromHttpDictionary(ls,&resp->responseHeaders));
        
            gc->BarrierEnd();
            return dict;
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
            Tesses::Framework::Streams::Stream* strm=nullptr;
            bool ownsStream=true;
            TStreamHeapObject* objStrm;
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
                strm = new NetworkStream(host,(uint16_t)port,false,false,false);
                if(tls)
                {
                    strm = new Framework::Crypto::ClientTLSStream(strm,true,true,host);
                }

            }
            else if (GetObjectHeap(server, objStrm)) {
                ownsStream=false;
                strm = objStrm->stream;    
            }

            Tesses::Framework::Mail::SMTPClient client(strm,ownsStream);
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

                        client.attachments.push_back(std::pair<std::string,SMTPBody*>(name,TObjectToSMTPBody(ls, type, o)));
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

                    hdict.AddValue(key,value);
                }
            }
            CallbackWebSocketConnection conn([dict,&ls](std::function<void(WebSocketMessage&)> sendMessage,std::function<void()> ping,std::function<void()> close)->void{
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
                return ToBool(callable->Call(ls,{CreateDictionaryFromHttpDictionary(ls,&dict),success}));
                return true;
            });
        }
        return nullptr;
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
        TStreamHeapObject* strm;
        if(GetArgument(args,0,url) && GetArgumentHeap(args,1,strm))
        {
            DownloadToStreamSimple(url,strm->stream);
        }
        return nullptr;
    }
    static TObject Net_Http_DownloadToFile(GCList& ls, std::vector<TObject> args)
    {
        std::string url;
        TVFSHeapObject* vfs;
        Tesses::Framework::Filesystem::VFSPath path;
        if(GetArgument(args,0,url) && GetArgumentHeap(args,1,vfs) && GetArgumentAsPath(args,2, path))
        {
            DownloadToFileSimple(url,vfs->vfs,path);
        }
        return nullptr;
    }
    void TStd::RegisterNet(GC* gc, TRootEnvironment* env)
    {

        env->permissions.canRegisterNet=true;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        
        TDictionary* http = TDictionary::Create(ls);
        TDictionary* smtp = TDictionary::Create(ls);
        http->DeclareFunction(gc, "HtmlEncode","Html encode",{"param"}, Net_HtmlEncode);
       
        http->DeclareFunction(gc, "UrlEncode","Url encode query param",{"param"}, Net_UrlEncode);
        http->DeclareFunction(gc, "UrlDecode","Url decode query param",{"param"}, Net_UrlDecode);
        http->DeclareFunction(gc, "UrlPathEncode","Url encode path",{"path"}, Net_UrlPathEncode);
        http->DeclareFunction(gc, "UrlPathDecode","Url decode path",{"path"}, Net_UrlPathDecode);
        http->DeclareFunction(gc, "MimeType","Get mimetype from extension",{"ext"},Net_Http_MimeType);

       
        //http->DeclareFunction(gc, "ProcessServer","Process HTTP server connection",{"networkstream","server","ip","port","encrypted"},, Net_ProcessServer);
        http->DeclareFunction(gc, "StreamHttpRequestBody","Create a stream request body",{"stream","mimeType"},[](GCList& ls, std::vector<TObject> args)->TObject {
            std::string mimeType;
            if(GetArgument(args, 1, mimeType))
            {
                auto res = TNative::Create(ls,new StreamHttpRequestBody(new TObjectStream(ls.GetGC(),args[0]), true, mimeType),[](void* ptr)->void {
                    delete static_cast<StreamHttpRequestBody*>(ptr);
                });
                return res;
            }
            return nullptr;
        });
        http->DeclareFunction(gc, "TextHttpRequestBody","Create a text request body",{"text","mimeType"},[](GCList& ls, std::vector<TObject> args)->TObject {
            std::string text;
            std::string mimeType;
            if(GetArgument(args, 0, text) && GetArgument(args, 1, mimeType))
            {
                auto res = TNative::Create(ls,new TextHttpRequestBody(text, mimeType),[](void* ptr)->void {
                    delete static_cast<TextHttpRequestBody*>(ptr);
                });
                return res;
            }
            return nullptr;
        });
        http->DeclareFunction(gc, "MakeRequest", "Create an http request", {"url","$extra"}, Net_Http_MakeRequest);
        http->DeclareFunction(gc, "WebSocketClient", "Create a websocket connection",{"url","headers","conn","$successCB"},Net_Http_WebSocketClient);
        http->DeclareFunction(gc, "DownloadToString","Return the http file's contents as a string",{"url"},Net_Http_DownloadToString);
        http->DeclareFunction(gc, "DownloadToStream","Download file to stream",{"url","stream"},Net_Http_DownloadToStream);
        http->DeclareFunction(gc, "DownloadToFile","Download file to file in vfs",{"url","vfs","path"},Net_Http_DownloadToFile);
        http->DeclareFunction(gc, "ListenSimpleWithLoop", "Listen (creates application loop)", {"server","port"},Net_Http_ListenSimpleWithLoop);
        http->DeclareFunction(gc, "ListenOnUnusedPort","Listen on unused localhost port and print Port: theport",{"server"},Net_Http_ListenOnUnusedPort);
        //FileServer svr()
        http->DeclareFunction(gc, "FileServer","Create a file server",{"path","allowlisting","spa"}, [](GCList& ls, std::vector<TObject> args)->TObject{
            
            bool allowlisting;
            bool spa;
            if(GetArgument(args,1,allowlisting) && GetArgument(args,2,spa))
            {
                auto fserver = new FileServer(new TObjectVFS(ls.GetGC(),args[0]),true,allowlisting,spa);
                return TServerHeapObject::Create(ls,fserver);
            }
            return nullptr;
        });
        http->DeclareFunction(gc, "MountableServer","Create a server you can mount to, must mount parents before child",{"root"}, [](GCList& ls, std::vector<TObject> args)->TObject{
            if(args.size() > 0)
            {
                auto svr = new TObjectHttpServer(ls.GetGC(), args[0]);
                auto svr2 = new MountableServer(svr,true);
                
                return TServerHeapObject::Create(ls,svr2);
            }
            return nullptr;
        });
        dict->DeclareFunction(gc, "NetworkStream","Create a network stream",{"ipv6","datagram"},Net_NetworkStream);
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
        gc->BarrierBegin();
        dict->SetValue("Http", http);
        dict->SetValue("Smtp", smtp);
        env->DeclareVariable("Net", dict);
        gc->BarrierEnd();
    }
}
