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
        gc->BarrierBegin();
        env->DeclareVariable("Crypto", dict);
        gc->BarrierEnd();
        
  
    }
}
