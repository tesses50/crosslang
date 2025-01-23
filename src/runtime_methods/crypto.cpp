#include "CrossLang.hpp"
#include <TessesFramework/TessesFrameworkFeatures.h>
#if defined(TESSESFRAMEWORK_ENABLE_MBED)
#include <iostream>

#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>
#include <mbedtls/base64.h>
#include <mbedtls/pkcs5.h>
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
    #if defined(TESSESFRAMEWORK_ENABLE_MBED)
    static TObject Crypto_RandomBytes(GCList& ls, std::vector<TObject> args)
    {
        int64_t size;
        std::string personalStr;
        if(GetArgument(args,0,size) && GetArgument(args,1,personalStr))
        {
            mbedtls_entropy_context entropy;
            mbedtls_ctr_drbg_context ctr_drbg;

            mbedtls_entropy_init(&entropy);
            mbedtls_ctr_drbg_init(&ctr_drbg);

            int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) personalStr.c_str(), personalStr.size());
            if(ret != 0)
            {
                mbedtls_ctr_drbg_free(&ctr_drbg);
                mbedtls_entropy_free(&entropy);
                return nullptr;
            }
            std::vector<uint8_t> bytes;
            bytes.resize((size_t)size);
            ret = mbedtls_ctr_drbg_random(&ctr_drbg, bytes.data(),bytes.size());
            if (ret != 0) 
            {
                mbedtls_ctr_drbg_free(&ctr_drbg);
                mbedtls_entropy_free(&entropy);
                return nullptr;
            }       
            mbedtls_ctr_drbg_free(&ctr_drbg);
            mbedtls_entropy_free(&entropy);
            TByteArray* ba = TByteArray::Create(ls);
            ba->data = bytes;
            return ba;
        }
        return nullptr;
    }
    static TObject Crypto_PBKDF2(GCList& ls, std::vector<TObject> args)
    {
        std::string pass;
        TByteArray* bArraySalt;
        int64_t itterations;
        int64_t keylength;
        int64_t shanum;
        if(GetArgument(args,0,pass) && GetArgumentHeap(args,1, bArraySalt) && GetArgument(args,2, itterations) && GetArgument(args,3,keylength) && GetArgument(args,4,shanum))
        {
            mbedtls_md_context_t ctx;
            mbedtls_md_init(&ctx);
            const mbedtls_md_info_t* info = NULL;
            switch(shanum)
            {
                case 1:
                    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
                    break;
                case 224:
                    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA224);
                    break;
                case 256:
                    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
                    break;
                default:
                case 384:
                    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA384);
                    break;
                case 512:
                    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
                    break;
            }
             
            
            mbedtls_md_setup(&ctx,  info, 1);

            std::vector<uint8_t> key;
            key.resize((size_t)keylength);
    
            if(mbedtls_pkcs5_pbkdf2_hmac(&ctx, (const unsigned char*)pass.c_str(), pass.size(), bArraySalt->data.data(), bArraySalt->data.size(), (int)itterations,(uint32_t)key.size(),key.data()) == 0)
            {
                auto ba = TByteArray::Create(ls);
                ba->data = key;
                mbedtls_md_free(&ctx);
                return ba;
            }

            mbedtls_md_free(&ctx);
        }
        return nullptr;
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
    static TObject Crypto_Base64Encode(GCList& ls, std::vector<TObject> args)
    {
        TByteArray* byteArray;
        
        if(GetArgumentHeap(args,0,byteArray))
        {
            return Tesses::Framework::Crypto::Base64_Encode(byteArray->data);
            

        }
        return "";
    }   
    static TObject Crypto_Base64Decode(GCList& ls, std::vector<TObject> args)
    {
        std::string str;
        if(GetArgument(args,0,str))
        {
            TByteArray* bArray = TByteArray::Create(ls);
            bArray->data = Tesses::Framework::Crypto::Base64_Decode(str);

            return bArray;
            
        }
        return nullptr;
    }   
   #endif
    void TStd::RegisterCrypto(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterCrypto=true;
        #if defined(TESSESFRAMEWORK_ENABLE_MBED)

        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc, "PBKDF2","Hash passwords with PBKDF2",{"pass","salt","itterations","keylen","shanum"},Crypto_PBKDF2);
        dict->DeclareFunction(gc, "RandomBytes","Create bytearray but with random bytes in it instead of zeros (this uses mbedtls by the way)",{"byteCount","personalString"},Crypto_RandomBytes);
        dict->DeclareFunction(gc, "Sha1","Sha1 Algorithm (needed for WebSocket handshake/BitTorrent etc) (don't use unless you have no other choice)",{},Crypto_Sha1);
        dict->DeclareFunction(gc, "Sha256","Sha256 Algorithm",{"$is224"},Crypto_Sha256);
        dict->DeclareFunction(gc, "Sha512","Sha512 Algorithm",{"$is384"},Crypto_Sha512);
        dict->DeclareFunction(gc, "Base64Encode","Base64 encode",{"data"},Crypto_Base64Encode);
        dict->DeclareFunction(gc, "Base64Decode","Base64 decode",{"str"},Crypto_Base64Decode);
        gc->BarrierBegin();
        env->DeclareVariable("Crypto", dict);
        gc->BarrierEnd();
        
        #endif
    }
}
