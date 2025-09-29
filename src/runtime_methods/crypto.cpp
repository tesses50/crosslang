#include "CrossLang.hpp"

using namespace Tesses::Framework::Crypto;

namespace Tesses::CrossLang
{
   static TObject Crypto_RandomBytes(GCList& ls, std::vector<TObject> args)
    {
        
        int64_t size;
        std::string personalStr;
        if(GetArgument(args,0,size) && GetArgument(args,1,personalStr))
        {

            std::vector<uint8_t> bytes;
            bytes.resize((size_t)size);
            if(RandomBytes(bytes,personalStr))
            {
                auto ba = TByteArray::Create(ls);
                ba->data = bytes;
                return ba;
            }
            return nullptr;
            
            
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
            ShaVersion version = VERSION_SHA384;
            
            switch(shanum)
            {
                case 1:
                    version = VERSION_SHA1;
                    break;
                case 224:
                    version = VERSION_SHA224;
                    break;
                case 256:
                    version = VERSION_SHA256;
                    break;
                default:
                case 384:
                    version = VERSION_SHA384;
                    break;
                case 512:
                    version = VERSION_SHA512;
                    break;
            }
             
            

            std::vector<uint8_t> key;
            key.resize((size_t)keylength);

            if(PBKDF2(key,pass,bArraySalt->data,(long)itterations,version))
            {
                TByteArray* ba = TByteArray::Create(ls);
                ba->data = key;
                return ba;
            }
    
        }
        return nullptr;
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
    void TStd::RegisterCrypto(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterCrypto=true;
        if(!HaveCrypto()) return;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc, "PBKDF2","Hash passwords with PBKDF2",{"pass","salt","itterations","keylen","shanum"},Crypto_PBKDF2);
        dict->DeclareFunction(gc, "RandomBytes","Create bytearray but with random bytes in it instead of zeros (this uses mbedtls by the way)",{"byteCount","personalString"},Crypto_RandomBytes);
        
        dict->DeclareFunction(gc, "Base64Encode","Base64 encode",{"data"},Crypto_Base64Encode);
        dict->DeclareFunction(gc, "Base64Decode","Base64 decode",{"str"},Crypto_Base64Decode);
        dict->DeclareFunction(gc, "Sha1", "Hash with sha1 algorithm (please don't use sha1 unless you need to)",{"strm"}, [](GCList& ls, std::vector<TObject> args)->TObject {
            TByteArray* baSrc;
            std::shared_ptr<Tesses::Framework::Streams::Stream> sho;

            if(GetArgument(args, 0, sho))
            {
                auto bytes = Sha1::ComputeHash(sho);
                auto ba = TByteArray::Create(ls);
                ba->data = bytes;
                return ba;
            }
            if(GetArgumentHeap(args,0,baSrc))
            {
                auto bytes = Sha1::ComputeHash(baSrc->data.data(), baSrc->data.size());
                auto ba = TByteArray::Create(ls);
                ba->data = bytes;
                return ba;
            }
            return Undefined();
        });
        dict->DeclareFunction(gc, "Sha256", "Hash with sha256 algorithm",{"strm","$is224"}, [](GCList& ls, std::vector<TObject> args)->TObject {
            TByteArray* baSrc;
            std::shared_ptr<Tesses::Framework::Streams::Stream> sho;
            bool is224=false;
            GetArgument(args,1,is224);

            if(GetArgument(args, 0, sho))
            {
                auto bytes = Sha256::ComputeHash(sho,is224);
                auto ba = TByteArray::Create(ls);
                ba->data = bytes;
                return ba;
            }
            if(GetArgumentHeap(args,0,baSrc))
            {
                auto bytes = Sha256::ComputeHash(baSrc->data.data(), baSrc->data.size(), is224);
                auto ba = TByteArray::Create(ls);
                ba->data = bytes;
                return ba;
            }
            return Undefined();
        });
        dict->DeclareFunction(gc, "Sha512", "Hash with sha512 algorithm",{"strm","$is384"}, [](GCList& ls, std::vector<TObject> args)->TObject {
            TByteArray* baSrc;
            std::shared_ptr<Tesses::Framework::Streams::Stream> sho;
            bool is384=false;
            GetArgument(args,1,is384);

            if(GetArgument(args, 0, sho))
            {
                auto bytes = Sha512::ComputeHash(sho,is384);
                auto ba = TByteArray::Create(ls);
                ba->data = bytes;
                return ba;
            }
            if(GetArgumentHeap(args,0,baSrc))
            {
                auto bytes = Sha512::ComputeHash(baSrc->data.data(), baSrc->data.size(), is384);
                auto ba = TByteArray::Create(ls);
                ba->data = bytes;
                return ba;
            }
            return Undefined();
        });
        gc->BarrierBegin();
        env->DeclareVariable("Crypto", dict);
        gc->BarrierEnd();
        
  
    }
}
