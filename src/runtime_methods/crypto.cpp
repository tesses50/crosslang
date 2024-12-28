#include "CrossLang.hpp"
#if defined(CROSSLANG_ENABLE_MBED)
#include <iostream>

#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>
#include <mbedtls/base64.h>

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/x509.h>
#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/error.h>
#include <cstring>
#endif

namespace Tesses::CrossLang
{
    #if defined(CROSSLANG_ENABLE_MBED)
      static void my_debug(void *ctx, int level,
                     const char *file, int line,
                     const char *str)
{
    ((void) level);

    fprintf(stderr, "%s:%04d: %s", file, line, str);
    fflush(stderr);
}
    class TlsClientStream {
        GCList* ls;
        TCallable* read;
        TCallable* write;
        TCallable* close;
        TByteArray* readBuffer;
        TByteArray* writeBuffer;
        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_ssl_context ssl;
        mbedtls_ssl_config conf;
        mbedtls_x509_crt cachain;
        
            
            static int strm_send(void* ctx,const unsigned char* buf,size_t len)
            {
                TlsClientStream* strm = static_cast<TlsClientStream*>(ctx);
                strm->writeBuffer->data.resize(len);
                memcpy(strm->writeBuffer->data.data(),buf,len);

                auto res = strm->write->Call(*strm->ls,{strm->writeBuffer});
                if(std::holds_alternative<int64_t>(res))
                {
                    auto num = std::get<int64_t>(res);
                    return (int)num;
                }
                return -1;
            }
            static int strm_recv(void* ctx,unsigned char* buf,size_t len)
            {
                TlsClientStream* strm = static_cast<TlsClientStream*>(ctx);
                strm->readBuffer->data.resize(len);
                
                auto res = strm->read->Call(*strm->ls,{strm->readBuffer});
                if(std::holds_alternative<int64_t>(res))
                {
                    auto num = std::get<int64_t>(res);
                    if(num < 0) return (int)num;
                    size_t read = (size_t)num;
                    if(read > len) read = len;
                    memcpy(buf,strm->readBuffer->data.data(), read);
                    return read;
                }
                return -1;
                
                //return (int)strm->Read((uint8_t*)buf,len);
            }
            public:
            void Close()
            {
                close->Call(*ls,{});
            }
            bool success=false;
            bool isDoneReading = false;
            int64_t Read(uint8_t* buffer, size_t len)
            {
                if(isDoneReading) return 0;
                int64_t r = (int64_t)mbedtls_ssl_read(&ssl,buffer,len);
                if(r == 0) isDoneReading=true;
                if(r == -30848) 
                {
                    isDoneReading = true;
                    return 0;
                }
                return r;
            }
            int64_t Write(uint8_t* buffer, size_t len)
            {
                return (int64_t)mbedtls_ssl_write(&ssl,buffer,len);
            }
            
            TlsClientStream(GC* gc,std::string domain,std::string chain,bool verify, TCallable* read, TCallable* write, TCallable* close)
            {
                ls = new GCList(gc);
                ls->Add(read);
                ls->Add(write);
                ls->Add(close);
                this->read = read;
                this->write = write;
                this->close = close;
                readBuffer = TByteArray::Create(ls);
                writeBuffer = TByteArray::Create(ls);

                mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&cachain);
        mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_entropy_init(&entropy);
        
        const char* pers = "CrossLangTLS";
        
        int ret=0;



#if defined(MBEDTLS_USE_PSA_CRYPTO)
    psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        mbedtls_fprintf(stderr, "Failed to initialize PSA Crypto implementation: %d\n",
                        (int) status);
        return;
    }
#endif

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *) pers,
                                     strlen(pers))) != 0) 
                                     {
                                        printf("FAILED mbedtls_ctr_drbg_seed\n");
                                        return;
                                     }
      
        if(ret != 0) { printf("FAILED mbedtls_x509_crt_parse cert %i\n",ret); return;}
        ret = mbedtls_x509_crt_parse(&cachain, (const unsigned char *) chain.c_str(),
                                chain.size()+1);

        if(ret != 0) {printf("FAILED mbedtls_x509_crt_parse chain %i\n",ret); return;}
       

        
        if((ret = mbedtls_ssl_config_defaults(&conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
                                           {
                                            char buffer[100];
                                            mbedtls_strerror(ret,buffer,sizeof(buffer));
                                            printf("FAILED mbedtls_ssl_conf_defaults %s\n",buffer); 
                                                return;
                                           }

                                           
                                           
                                           
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
       mbedtls_ssl_conf_dbg(&conf,my_debug,stdout);
      /* #if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_conf_session_cache(&conf, &cache,
                                   mbedtls_ssl_cache_get,
                                   mbedtls_ssl_cache_set);
#endif*/
        mbedtls_ssl_conf_authmode(&conf, verify ? MBEDTLS_SSL_VERIFY_REQUIRED: MBEDTLS_SSL_VERIFY_OPTIONAL);
        mbedtls_ssl_conf_ca_chain(&conf, &cachain, NULL);


        mbedtls_ssl_set_bio(&ssl, static_cast<void*>(this),strm_send,strm_recv, NULL);
        if((ret=mbedtls_ssl_setup(&ssl,&conf) != 0))
        {
            printf("FAILED mbedtls_ssl_setup %i\n",ret); 
            return;
        }
        if((ret=mbedtls_ssl_set_hostname(&ssl,domain.c_str()) != 0))
        {
            printf("FAILED mbedtls_ssl_set_hostname %i\n",ret); 
            return;
        }
        if((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            char buffer[100];
            mbedtls_strerror(ret,buffer,sizeof(buffer));
            printf("FAILED mbedtls_ssl_handshake %s\n",buffer);
            return;
        }
            uint32_t flags;
          if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0) {
#if !defined(MBEDTLS_X509_REMOVE_INFO)
        char vrfy_buf[512];
#endif

        

#if !defined(MBEDTLS_X509_REMOVE_INFO)
        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);

      
#endif
        if(verify)
        return;
    }

        success=true;

            }
            ~TlsClientStream()
            {
                   
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_ssl_config_free(&conf);

#if defined(MBEDTLS_USE_PSA_CRYPTO)
    mbedtls_psa_crypto_free();
#endif /* MBEDTLS_USE_PSA_CRYPTO */
                
                delete ls;
            }
    }; 

    class TlsServerStream {
        GCList* ls;
        TCallable* read;
        TCallable* write;
        TCallable* close;
        TByteArray* readBuffer;
        TByteArray* writeBuffer;
        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_ssl_context ssl;
        mbedtls_ssl_config conf;
         mbedtls_x509_crt srvcert;
        mbedtls_x509_crt cachain;
        mbedtls_pk_context pkey;
        
        
        
            static int strm_send(void* ctx,const unsigned char* buf,size_t len)
            {
                TlsServerStream* strm = static_cast<TlsServerStream*>(ctx);
                strm->writeBuffer->data.resize(len);
                memcpy(strm->writeBuffer->data.data(),buf,len);

                auto res = strm->write->Call(*strm->ls,{strm->writeBuffer});
                if(std::holds_alternative<int64_t>(res))
                {
                    auto num = std::get<int64_t>(res);
                    
                    return (int)num;
                }
                return -1;
            }
            static int strm_recv(void* ctx,unsigned char* buf,size_t len)
            {
                TlsServerStream* strm = static_cast<TlsServerStream*>(ctx);
                strm->readBuffer->data.resize(len);
                
                auto res = strm->read->Call(*strm->ls,{strm->readBuffer});
                if(std::holds_alternative<int64_t>(res))
                {
                    auto num = std::get<int64_t>(res);
                    if(num < 0) return num;
                    size_t read = (size_t)num;
                    if(read > len) read = len;
                    memcpy(buf,strm->readBuffer->data.data(), read);
                    return read;
                }
                return -1;
                
                //return (int)strm->Read((uint8_t*)buf,len);
            }
            public:
              void Close()
            {
                close->Call(*ls,{});
            }
            bool success=false;
            bool isDoneReading =false;
            int64_t Read(uint8_t* buffer, size_t len)
            {
                if(isDoneReading) return 0;
                int64_t r = (int64_t)mbedtls_ssl_read(&ssl,buffer,len);
                if(r == 0) isDoneReading=true;
                if(r == -30848) 
                {
                    isDoneReading = true;
                    return 0;
                }
                return r;
            }
            int64_t Write(uint8_t* buffer, size_t len)
            {
                return mbedtls_ssl_write(&ssl,buffer,len);
            }
            
            TlsServerStream(GC* gc,std::string cert, std::string chain, std::string privkey, TCallable* read, TCallable* write, TCallable* close)
            {
                ls = new GCList(gc);
                ls->Add(read);
                ls->Add(write);
                ls->Add(close);
                readBuffer = TByteArray::Create(ls);
                writeBuffer = TByteArray::Create(ls);
                this->read = read;
                this->write = write;
                this->close = close;

                mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&srvcert);
        mbedtls_x509_crt_init(&cachain);
        mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_entropy_init(&entropy);
        mbedtls_pk_init(&pkey);
        
        const char* pers = "CrossLangTLS";
        int ret=0;



#if defined(MBEDTLS_USE_PSA_CRYPTO)
    psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        mbedtls_fprintf(stderr, "Failed to initialize PSA Crypto implementation: %d\n",
                        (int) status);
        return;
    }
#endif

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *) pers,
                                     strlen(pers))) != 0) 
                                     {
                                        printf("FAILED mbedtls_ctr_drbg_seed\n");
                                        return;
                                     }
        ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *) cert.c_str(),
                                 cert.size()+1);
        if(ret != 0) { printf("FAILED mbedtls_x509_crt_parse cert %i\n",ret); return;}
        ret = mbedtls_x509_crt_parse(&cachain, (const unsigned char *) chain.c_str(),
                                chain.size()+1);

        if(ret != 0) {printf("FAILED mbedtls_x509_crt_parse chain %i\n",ret); return;}
       

         ret =  mbedtls_pk_parse_key(&pkey, (const unsigned char *) privkey.c_str(),
                                privkey.size()+1, NULL, 0);
        if(ret != 0) {printf("FAILED mbedtls_pk_parse_key %i\n",ret); return;}
        if((ret = mbedtls_ssl_config_defaults(&conf,
                                           MBEDTLS_SSL_IS_SERVER,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
                                           {
                                            char buffer[100];
                                            mbedtls_strerror(ret,buffer,sizeof(buffer));
                                            printf("FAILED mbedtls_ssl_conf_defaults %s\n",buffer); 
                                                return;
                                           }

                                           
                                           
                                           
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
       
      /* #if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_conf_session_cache(&conf, &cache,
                                   mbedtls_ssl_cache_get,
                                   mbedtls_ssl_cache_set);
#endif*/
        mbedtls_ssl_conf_own_cert(&conf,&srvcert,&pkey);
        mbedtls_ssl_conf_ca_chain(&conf, &cachain, NULL);

        mbedtls_ssl_set_bio(&ssl, static_cast<void*>(this),strm_send,strm_recv, NULL);

        if((ret=mbedtls_ssl_setup(&ssl,&conf) != 0))
        {
            printf("FAILED mbedtls_ssl_setup %i\n",ret); 
            return;
        }

        if((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            char buffer[100];
            mbedtls_strerror(ret,buffer,sizeof(buffer));
            printf("FAILED mbedtls_ssl_handshake %s\n",buffer);
            return;
        }

        success=true;

            }
            ~TlsServerStream()
            {
                   mbedtls_x509_crt_free(&srvcert);
    mbedtls_pk_free(&pkey);

    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_ssl_config_free(&conf);

#if defined(MBEDTLS_USE_PSA_CRYPTO)
    mbedtls_psa_crypto_free();
#endif /* MBEDTLS_USE_PSA_CRYPTO */
                
                delete ls;
            }
    }; 
    static TObject Crypto_TlsClient(GCList& ls, std::vector<TObject> args)
    {
        //TlsServerStream(GC* gc,std::string domain, std::string chain, bool verify, TCallable* read, TCallable* write, TCallable* close)
        if(args.size() == 6 && std::holds_alternative<std::string>(args[0]) && std::holds_alternative<std::string>(args[1]) && std::holds_alternative<bool>(args[2]) && std::holds_alternative<THeapObjectHolder>(args[3]) && std::holds_alternative<THeapObjectHolder>(args[4]) && std::holds_alternative<THeapObjectHolder>(args[5]))
        {
            std::string domain = std::get<std::string>(args[0]);
            std::string chain = std::get<std::string>(args[1]);
            bool verify = std::get<bool>(args[2]);
            TCallable* read = dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(args[3]).obj);

            TCallable* write = dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(args[4]).obj);

            TCallable* close = dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(args[5]).obj);

            if(read != nullptr && write != nullptr && close != nullptr)
            {
                auto serverStream = new TlsClientStream(ls.GetGC(),domain,chain,verify,read,write,close);
                if(!serverStream->success)
                {
                    serverStream->Close();
                    delete serverStream;
                    return nullptr;
                }
                TNative* native = TNative::Create(ls,static_cast<void*>(serverStream),[](void* ptr)->void {
                    if(ptr == nullptr) return;
                    TlsClientStream* strm = static_cast<TlsClientStream*>(ptr);
                    strm->Close();
                    delete strm;
                });

                TDictionary* dict = TDictionary::Create(ls);
                ls.GetGC()->BarrierBegin();
                dict->SetValue("_native",native);
                ls.GetGC()->BarrierEnd();
                dict->DeclareFunction(ls.GetGC(),"Read","Read data from Tls Connection",{"buffer","offset","length"},[native](GCList& ls2, std::vector<TObject> args2)->TObject {
                    if(args2.size() == 3 && std::holds_alternative<THeapObjectHolder>(args2[0]) && std::holds_alternative<int64_t>(args2[1]) && std::holds_alternative<int64_t>(args2[2]))
                    {
                        auto byteArray = dynamic_cast<TByteArray*>(std::get<THeapObjectHolder>(args2[0]).obj);
                        size_t offset = (size_t)std::get<int64_t>(args2[1]);
                        size_t length = (size_t)std::get<int64_t>(args2[2]);

                        if(!native->GetDestroyed() && byteArray != nullptr && ((offset + length) <= byteArray->data.size()))
                        {
                            auto strm = static_cast<TlsClientStream*>(native->GetPointer());
                            return strm->Read(byteArray->data.data()+offset, length);
                        }
                    }
                    return Undefined();
                });
                dict->DeclareFunction(ls.GetGC(),"Write","Write data to Tls Connection",{"buffer","offset","length"},[native](GCList& ls2, std::vector<TObject> args2)->TObject {
                    if(args2.size() == 3 && std::holds_alternative<THeapObjectHolder>(args2[0]) && std::holds_alternative<int64_t>(args2[1]) && std::holds_alternative<int64_t>(args2[2]))
                    {
                        auto byteArray = dynamic_cast<TByteArray*>(std::get<THeapObjectHolder>(args2[0]).obj);
                        size_t offset = (size_t)std::get<int64_t>(args2[1]);
                        size_t length = (size_t)std::get<int64_t>(args2[2]);

                        if(!native->GetDestroyed() && byteArray != nullptr && ((offset + length) <= byteArray->data.size()))
                        {
                            auto strm = static_cast<TlsClientStream*>(native->GetPointer());
                            return strm->Write(byteArray->data.data()+offset, length);
                        }
                    }
                    return Undefined();
                });
                dict->DeclareFunction(ls.GetGC(),"Close","Close Tls Connection",{},[native](GCList& ls2, std::vector<TObject> args2)->TObject {
                    
                        if(!native->GetDestroyed())
                        {
                            native->Destroy();
                        }
                    
                    return Undefined();
                });
                return dict;
            }
        }
        return Undefined();
    }
  
    static TObject Crypto_TlsServer(GCList& ls, std::vector<TObject> args)
    {
        //TlsServerStream(GC* gc,std::string cert, std::string chain, std::string privkey, TCallable* read, TCallable* write, TCallable* close)
        if(args.size() == 6 && std::holds_alternative<std::string>(args[0]) && std::holds_alternative<std::string>(args[1]) && std::holds_alternative<std::string>(args[2]) && std::holds_alternative<THeapObjectHolder>(args[3]) && std::holds_alternative<THeapObjectHolder>(args[4]) && std::holds_alternative<THeapObjectHolder>(args[5]))
        {
            std::string cert = std::get<std::string>(args[0]);
            std::string chain = std::get<std::string>(args[1]);
            std::string privkey = std::get<std::string>(args[2]);
            TCallable* read = dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(args[3]).obj);

            TCallable* write = dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(args[4]).obj);

            TCallable* close = dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(args[5]).obj);

            if(read != nullptr && write != nullptr && close != nullptr)
            {
                auto serverStream = new TlsServerStream(ls.GetGC(),cert,chain,privkey,read,write,close);
                if(!serverStream->success)
                {
                    serverStream->Close();
                    delete serverStream;
                    return nullptr;
                }
                TNative* native = TNative::Create(ls,static_cast<void*>(serverStream),[](void* ptr)->void {
                    if(ptr == nullptr) return;
                    TlsServerStream* strm = static_cast<TlsServerStream*>(ptr);
                    strm->Close();
                    delete strm;
                });

                TDictionary* dict = TDictionary::Create(ls);
                ls.GetGC()->BarrierBegin();
                dict->SetValue("_native",native);
                ls.GetGC()->BarrierEnd();
                dict->DeclareFunction(ls.GetGC(),"Read","Read data from Tls Connection",{"buffer","offset","length"},[native](GCList& ls2, std::vector<TObject> args2)->TObject {
                    if(args2.size() == 3 && std::holds_alternative<THeapObjectHolder>(args2[0]) && std::holds_alternative<int64_t>(args2[1]) && std::holds_alternative<int64_t>(args2[2]))
                    {
                        auto byteArray = dynamic_cast<TByteArray*>(std::get<THeapObjectHolder>(args2[0]).obj);
                        size_t offset = (size_t)std::get<int64_t>(args2[1]);
                        size_t length = (size_t)std::get<int64_t>(args2[2]);
                        if(!native->GetDestroyed() && byteArray != nullptr && ((offset + length) <= byteArray->data.size()))
                        {
                            auto strm = static_cast<TlsServerStream*>(native->GetPointer());
                            return strm->Read(byteArray->data.data()+offset, length);
                        }
                    }
                    return Undefined();
                });
                dict->DeclareFunction(ls.GetGC(),"Write","Write data to Tls Connection",{"buffer","offset","length"},[native](GCList& ls2, std::vector<TObject> args2)->TObject {
                    if(args2.size() == 3 && std::holds_alternative<THeapObjectHolder>(args2[0]) && std::holds_alternative<int64_t>(args2[1]) && std::holds_alternative<int64_t>(args2[2]))
                    {
                        auto byteArray = dynamic_cast<TByteArray*>(std::get<THeapObjectHolder>(args2[0]).obj);
                        size_t offset = (size_t)std::get<int64_t>(args2[1]);
                        size_t length = (size_t)std::get<int64_t>(args2[2]);


                        if(!native->GetDestroyed() && byteArray != nullptr && ((offset + length) <= byteArray->data.size()))
                        {
                            auto strm = static_cast<TlsServerStream*>(native->GetPointer());
                            return strm->Write(byteArray->data.data()+offset, length);
                        }
                    }
                    return Undefined();
                });
                dict->DeclareFunction(ls.GetGC(),"Close","Close Tls Connection",{},[native](GCList& ls2, std::vector<TObject> args2)->TObject {
                    
                        if(!native->GetDestroyed())
                        {
                            native->Destroy();
                        }
                    
                    return Undefined();
                });
                return dict;
            }
        }
        return Undefined();
    }
  
    static TObject Crypto_Sha1(GCList& ls, std::vector<TObject> args)
    {
        
        TDictionary* dict = TDictionary::Create(ls);
        mbedtls_sha1_context* ctx = new mbedtls_sha1_context();
        mbedtls_sha1_init(ctx);
        mbedtls_sha1_starts(ctx);

        TNative* native = TNative::Create(ls,ctx,[](void* _ctx)->void{
            if(_ctx == nullptr) return;
            mbedtls_sha1_context* ctx = (mbedtls_sha1_context*)_ctx;
            mbedtls_sha1_free(ctx);
            delete ctx;
        });
        ls.GetGC()->BarrierBegin();
        dict->SetValue("_native",native);
        ls.GetGC()->BarrierEnd();

        dict->DeclareFunction(ls.GetGC(),"Update","Write more bytes",{"buffer","offset","size"},[native](GCList& ls2,std::vector<TObject> args2)->TObject{
            if(!native->GetDestroyed() && args2.size() == 3 && std::holds_alternative<THeapObjectHolder>(args2[0]) && std::holds_alternative<int64_t>(args2[1]) && std::holds_alternative<int64_t>(args2[2]))
            {
                auto buffer = dynamic_cast<TByteArray*>(std::get<THeapObjectHolder>(args2[0]).obj);
                auto offset = (size_t)std::get<int64_t>(args2[1]);
                auto len = (size_t)std::get<int64_t>(args2[2]);
                if(buffer != nullptr && (len+offset) < buffer->data.size())
                {
                    
                    mbedtls_sha1_update((mbedtls_sha1_context*)native->GetPointer(),buffer->data.data() + offset, len);
                }
            }
            return false;
        });

        dict->DeclareFunction(ls.GetGC(),"Finish","Get hash",{},[native](GCList& ls2,std::vector<TObject> args2)->TObject{
            if(!native->GetDestroyed())
            {
                TByteArray* ba = TByteArray::Create(ls2);
                ba->data.resize(20);

                mbedtls_sha1_finish((mbedtls_sha1_context*)native->GetPointer(),ba->data.data());
                
                return ba;
            }
            return false;
        });

        
        /*
   

    mbedtls_sha1_update(&ctx, (const unsigned char*)"Demi Lovato", 11);
    unsigned char sha1[20];
    mbedtls_sha1_finish(&ctx,sha1);

    for(int i = 0;i<20;i++)
    {
        printf("%02x", sha1[i]);
    }
    printf("\n");

    mbedtls_sha1_free(&ctx);*/
        
        return dict;
    }
    static TObject Crypto_Sha256(GCList& ls, std::vector<TObject> args)
    {

        bool is224=false;

        if(args.size() == 1 && std::holds_alternative<bool>(args[0]))
        {
            is224 = std::get<bool>(args[1]);
        }
        
        TDictionary* dict = TDictionary::Create(ls);
        mbedtls_sha256_context* ctx = new mbedtls_sha256_context();
        mbedtls_sha256_init(ctx);
        mbedtls_sha256_starts(ctx,is224 ? 1 : 0);
        

        TNative* native = TNative::Create(ls,ctx,[](void* _ctx)->void{
            if(_ctx == nullptr) return;
            mbedtls_sha256_context* ctx = (mbedtls_sha256_context*)_ctx;
            mbedtls_sha256_free(ctx);
            delete ctx;
        });
        ls.GetGC()->BarrierBegin();
        dict->SetValue("_native",native);
        ls.GetGC()->BarrierEnd();

        dict->DeclareFunction(ls.GetGC(),"Update","Write more bytes",{"buffer","offset","size"},[native](GCList& ls2,std::vector<TObject> args2)->TObject{
            if(!native->GetDestroyed() && args2.size() == 3 && std::holds_alternative<THeapObjectHolder>(args2[0]) && std::holds_alternative<int64_t>(args2[1]) && std::holds_alternative<int64_t>(args2[2]))
            {
                auto buffer = dynamic_cast<TByteArray*>(std::get<THeapObjectHolder>(args2[0]).obj);
                auto offset = (size_t)std::get<int64_t>(args2[1]);
                auto len = (size_t)std::get<int64_t>(args2[2]);
                if(buffer != nullptr && (len+offset) < buffer->data.size())
                {
                    
                    mbedtls_sha256_update((mbedtls_sha256_context*)native->GetPointer(),buffer->data.data() + offset, len);
                }
            }
            return false;
        });

        dict->DeclareFunction(ls.GetGC(),"Finish","Get hash",{},[native](GCList& ls2,std::vector<TObject> args2)->TObject{
            if(!native->GetDestroyed())
            {
                TByteArray* ba = TByteArray::Create(ls2);
                ba->data.resize(32);

                mbedtls_sha256_finish((mbedtls_sha256_context*)native->GetPointer(),ba->data.data());
                
                return ba;
            }
            return false;
        });

    
        
        return dict;
    }
   static TObject Crypto_Sha512(GCList& ls, std::vector<TObject> args)
    {

        bool is384=false;

        if(args.size() == 1 && std::holds_alternative<bool>(args[0]))
        {
            is384 = std::get<bool>(args[1]);
        }
        
        TDictionary* dict = TDictionary::Create(ls);
        mbedtls_sha512_context* ctx = new mbedtls_sha512_context();
        mbedtls_sha512_init(ctx);
        mbedtls_sha512_starts(ctx,is384 ? 1 : 0);

        TNative* native = TNative::Create(ls,ctx,[](void* _ctx)->void{
            if(_ctx == nullptr) return;
            mbedtls_sha512_context* ctx = (mbedtls_sha512_context*)_ctx;
            mbedtls_sha512_free(ctx);
            delete ctx;
        });
        ls.GetGC()->BarrierBegin();
        dict->SetValue("_native",native);
        ls.GetGC()->BarrierEnd();

        dict->DeclareFunction(ls.GetGC(),"Update","Write more bytes",{"buffer","offset","size"},[native](GCList& ls2,std::vector<TObject> args2)->TObject{
            if(!native->GetDestroyed() && args2.size() == 3 && std::holds_alternative<THeapObjectHolder>(args2[0]) && std::holds_alternative<int64_t>(args2[1]) && std::holds_alternative<int64_t>(args2[2]))
            {
                auto buffer = dynamic_cast<TByteArray*>(std::get<THeapObjectHolder>(args2[0]).obj);
                auto offset = (size_t)std::get<int64_t>(args2[1]);
                auto len = (size_t)std::get<int64_t>(args2[2]);
                if(buffer != nullptr && (len+offset) < buffer->data.size())
                {
                    
                    mbedtls_sha512_update((mbedtls_sha512_context*)native->GetPointer(),buffer->data.data() + offset, len);
                }
            }
            return Undefined();
        });

        dict->DeclareFunction(ls.GetGC(),"Finish","Get hash",{},[native](GCList& ls2,std::vector<TObject> args2)->TObject{
            if(!native->GetDestroyed())
            {
                TByteArray* ba = TByteArray::Create(ls2);
                ba->data.resize(64);

                mbedtls_sha512_finish((mbedtls_sha512_context*)native->GetPointer(),ba->data.data());
               
                return ba;
            }
            return Undefined();
        });

    
        
        return dict;
    }
   #endif
    void TStd::RegisterCrypto(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterCrypto=true;
        #if defined(CROSSLANG_ENABLE_MBED)

        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        //dict->DeclareFunction(gc, "Decode","Deserialize Json",{"Json string"},Json_Decode);   
        //dict->DeclareFunction(gc, "Encode","Serialize Json",{"any","$indent"},Json_Encode);   
        dict->DeclareFunction(gc, "Sha1","Sha1 Algorithm (needed for WebSocket handshake/BitTorrent etc) (don't use unless you have no other choice)",{},Crypto_Sha1);
        dict->DeclareFunction(gc, "Sha256","Sha256 Algorithm",{"$is224"},Crypto_Sha256);
        dict->DeclareFunction(gc, "Sha512","Sha512 Algorithm",{"$is384"},Crypto_Sha512);
        dict->DeclareFunction(gc, "TlsClient", "Create TLS client for HTTPS client",{"domain","chain","verify","read","write","close"},Crypto_TlsClient);
       
        dict->DeclareFunction(gc, "TlsServer", "Create TLS server for HTTPS server",{"cert","chain","privkey","read","write","close"},Crypto_TlsServer);
        gc->BarrierBegin();
        env->DeclareVariable("Crypto", dict);
        gc->BarrierEnd();
        dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc, "Encode","Encode Base64",{"buffer","offset","count"},[](GCList& ls,std::vector<TObject> args)->TObject{
            TByteArray* bArray;
            int64_t offset;
            int64_t count;
            
            if(!GetArgumentHeap<TByteArray*>(args,0,bArray))
                return nullptr;
            if(!GetArgument<int64_t>(args,1, offset))
                return nullptr;
            if(!GetArgument<int64_t>(args,2, count))
                return nullptr;

            size_t off = (size_t)offset;
            size_t len = (size_t)count;

            off = std::min(off, bArray->data.size());

            len = std::min(len, bArray->data.size()-off);

            size_t outLen = ((4 * len / 3) + 3) & ~3;

            std::string str(outLen,'\0');
            
            if(mbedtls_base64_encode((unsigned char*)str.data(),str.size(),&outLen,bArray->data.data(),bArray->data.size()) != 0)
                return nullptr;
            
            str.resize(outLen);

            return str;
                
            //bArray->data.size();
                
                //
        });
        
        #endif
    }
}