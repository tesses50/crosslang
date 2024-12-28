#include "CrossLang.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <csignal>
#include <arpa/inet.h>
#include <iostream>
#include <netinet/tcp.h>
#include <unistd.h>
#include <cstring>
using namespace Tesses::Framework::Streams;
using namespace Tesses::Framework::Http;
namespace Tesses::CrossLang
{
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
            if(GetArgument(args2,0,key) && GetArgument(args2,1,value))
            {
                dict0->AddValue(key,value);
            }
            return nullptr;
        });

        dict->DeclareFunction(gc,"SetValue","Set item",{"key","value"},[dict0](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {
            std::string key;
            std::string value;
            if(GetArgument(args2,0,key) && GetArgument(args2,1,value))
            {
                dict0->SetValue(key,value);
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

        dict->DeclareFunction(gc,"getNeedToParseFormData","Check if Need to parse form data",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            return ctx->NeedToParseFormData();
        });

        /*dict->DeclareFunction(gc,"ReadString","Read string from request",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            return ctx->ReadString();
        });*/

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
        dict->DeclareFunction(gc,"SendBytes","Send bytes",{"ba"},[ctx,dict](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject{
            TByteArray* ba;
            if(GetArgumentHeap(args2,0,ba))
                ctx->SendBytes(ba->data);
            return nullptr;
        });
//        dict->DeclareFunction(gc,"getUrlWithQuery","Get original path with query parameters",{},[ctx](Tesses::CrossLang::GCList &ls2, std::vector<Tesses::CrossLang::TObject> args2)->TObject {return ctx->GetUrlWithQuery();});
        
        
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
    static TObject Net_Http_MimeType(GCList& ls, std::vector<TObject> args)
    {
        Tesses::Framework::Filesystem::VFSPath p;
        if(GetArgumentAsPath(args,0,p))
        {
            std::filesystem::path p2 = p.GetFileName();
            return HttpUtils::MimeType(p2);
        }
        return "application/octet-stream";
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
    
    void TStd::RegisterNet(GC* gc, TRootEnvironment* env)
    {

        env->permissions.canRegisterNet=true;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        
        TDictionary* http = TDictionary::Create(ls);
        http->DeclareFunction(gc, "UrlEncode","Url encode query param",{"param"}, Net_UrlEncode);
        http->DeclareFunction(gc, "UrlDecode","Url decode query param",{"param"}, Net_UrlDecode);
        http->DeclareFunction(gc, "UrlPathEncode","Url encode path",{"path"}, Net_UrlPathEncode);
        http->DeclareFunction(gc, "UrlPathDecode","Url decode path",{"path"}, Net_UrlPathDecode);
        http->DeclareFunction(gc, "MimeType","Get mimetype from extension",{"ext"},Net_Http_MimeType);

       
        //http->DeclareFunction(gc, "ProcessServer","Process HTTP server connection",{"networkstream","server","ip","port","encrypted"},, Net_ProcessServer);
        http->DeclareFunction(gc, "MakeRequest", "Create an http request", {"url","$extra"}, Net_Http_MakeRequest);
        http->DeclareFunction(gc, "ListenSimpleWithLoop", "Listen (creates application loop)", {"server","port"},Net_Http_ListenSimpleWithLoop);
        
        dict->DeclareFunction(gc, "NetworkStream","Create a network stream",{"ipv6","datagram"},Net_NetworkStream);
        gc->BarrierBegin();
        dict->SetValue("Http", http);
        env->DeclareVariable("Net", dict);
        gc->BarrierEnd();
    }
}