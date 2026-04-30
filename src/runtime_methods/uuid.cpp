#include "CrossLang.hpp"
#include "TessesFramework/Serialization/BitConverter.hpp"
#include "TessesFramework/Uuid.hpp"

namespace Tesses::CrossLang {
    static TObject Uuid_NewUuid(GCList& ls, std::vector<TObject> args)
    {
        return Tesses::Framework::Uuid::Generate();
    }
    static TObject Uuid_TryParse(GCList& ls, std::vector<TObject> args)
    {
        std::string str;
        Tesses::Framework::Uuid uuid;
        if(GetArgument(args, 0, str) && Tesses::Framework::Uuid::TryParse(str,uuid))
            return uuid;
        return nullptr;
    }
    static TObject Uuid_FromBytes(GCList& ls, std::vector<TObject> args)
    {
        TByteArray* ba;
        int64_t index;
        if(GetArgumentHeap(args,0,ba) && GetArgument(args, 1, index) && (size_t)index < ba->data.size() && (size_t)index + 16 <= ba->data.size())
        {
            return Tesses::Framework::Serialization::BitConverter::ToUuid(ba->data[(size_t)index]);
        }

        return nullptr;
    }
    
    void TStd::RegisterUuid(std::shared_ptr<GC> gc, TRootEnvironment* env)
    {
        gc->BarrierBegin();
        TDictionary* guid = env->EnsureDictionary(gc, "Uuid");
        guid->DeclareFunction(gc,"NewUuid","Create random uuid",{},Uuid_NewUuid);
        guid->DeclareFunction(gc, "TryParse","Try to parse",{"str"}, Uuid_TryParse);
        guid->DeclareFunction(gc, "FromBytes", "From bytes (big endian)",{"byteArray","offset"}, Uuid_FromBytes);
    
        gc->BarrierEnd();
    }
}