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
    static TObject Crypto_Base64Encode(GCList& ls, std::vector<TObject> args)
    {
        TByteArray* byteArray;
        int64_t offset;
        int64_t length;
        if(GetArgumentHeap(args,0,byteArray) && GetArgument(args,1,offset) && GetArgument(args,2,length))
        {
            std::string str={};
            size_t olen;
            size_t off = (size_t)offset;
            size_t len = (size_t)length;
            len = std::min(std::min(byteArray->data.size(),len-off),len);

            if(len > 0)
            {

                mbedtls_base64_encode((uint8_t*)str.data(), 0, &olen, byteArray->data.data()+offset,len);
                str.resize(olen);
                

                if(mbedtls_base64_encode((uint8_t*)str.data(), olen, &olen, byteArray->data.data()+offset,len)==0)
                {   
                    return str;
                }


            }
            
            return "";

        }
    }   
    static TObject Crypto_Base64Decode(GCList& ls, std::vector<TObject> args)
    {
        std::string str;
        if(GetArgument(args,0,str))
        {
            size_t olen;

            TByteArray* bArray = TByteArray::Create(ls);
        

            mbedtls_base64_decode(bArray->data.data(), 0, &olen, (const uint8_t*)str.data(),str.size());
                str.resize(olen);
                

                if(mbedtls_base64_decode(bArray->data.data(), olen, &olen, (const uint8_t*)str.data(),str.size())==0)
                {   
                    return str;
                }

            
        }
    }   
   #endif
    void TStd::RegisterCrypto(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterCrypto=true;
        #if defined(CROSSLANG_ENABLE_MBED)

        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc, "Sha1","Sha1 Algorithm (needed for WebSocket handshake/BitTorrent etc) (don't use unless you have no other choice)",{},Crypto_Sha1);
        dict->DeclareFunction(gc, "Sha256","Sha256 Algorithm",{"$is224"},Crypto_Sha256);
        dict->DeclareFunction(gc, "Sha512","Sha512 Algorithm",{"$is384"},Crypto_Sha512);
        dict->DeclareFunction(gc, "Base64Encode","Sha512 Algorithm",{"data"},Crypto_Base64Encode);
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