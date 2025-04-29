#include "CrossLang.hpp"

namespace Tesses::CrossLang
{
    int64_t TObjectStream::_GetPosInternal()
    {
        return Tesses::Framework::Streams::Stream::GetLength();
    }
    TObjectStream::TObjectStream(GC* gc, TObject obj)
    {
        this->ls = new GCList(gc);
        this->ls->Add(obj);
        this->obj = obj;
        TDictionary* dict;
        if(GetObjectHeap(obj,dict))
        {
            gc->BarrierBegin();
            if(!dict->HasValue("Read"))
            {
                dict->DeclareFunction(gc,"Read","Read from stream",{"buff","off","len"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    return (int64_t)0;
                });
            }
            if(!dict->HasValue("Write"))
            {
                dict->DeclareFunction(gc,"Write","Write to stream",{"buff","off","len"}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    return (int64_t)0;
                });
            }
            if(!dict->HasValue("getCanRead"))
            {
                dict->DeclareFunction(gc,"getCanRead","Can read from stream",{}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    return false;
                });
            }

            if(!dict->HasValue("getCanWrite"))
            {
                dict->DeclareFunction(gc,"getCanWrite","Can write to stream",{}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    return false;
                });
            }
            if(!dict->HasValue("getCanSeek"))
            {
                dict->DeclareFunction(gc,"getCanSeek","Can seek in stream",{}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    return false;
                });
            }
            if(!dict->HasValue("getPosition"))
            {
                dict->DeclareFunction(gc,"getPosition","Can get position in stream",{}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    return (int64_t)0;
                });
            }
            if(!dict->HasValue("getLength"))
            {
                dict->DeclareFunction(gc,"getPosition","Can get position in stream",{}, [this](GCList& ls, std::vector<TObject> args)->TObject {
                    return _GetPosInternal();
                });
            }
            if(!dict->HasValue("getEndOfStream"))
            {
                dict->DeclareFunction(gc,"getEndOfStream","Is at end of stream",{}, [](GCList& ls, std::vector<TObject> args)->TObject {
                    return false;
                });
            }
            gc->BarrierEnd();
        }
    }
    bool TObjectStream::EndOfStream()
    {
        TDictionary* dict;
        TStreamHeapObject* strm;
        if(GetObjectHeap(this->obj, dict))
        {
            auto res = dict->CallMethod(*ls, "getEndOfStream",{});
            bool r;
            if(GetObject(res,r)) return r;
        }
        else if(GetObjectHeap(this->obj, strm))
        {
            return strm->stream->EndOfStream();
        }
        return false;
    }
    size_t TObjectStream::Read(uint8_t* buff, size_t sz)
    {
        TDictionary* dict;
        TStreamHeapObject* strm;
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls2(this->ls->GetGC());

            TByteArray* arr=TByteArray::Create(ls2);
            arr->data.resize(sz);

            auto res = dict->CallMethod(ls2, "Read",{arr, (int64_t)0L, (int64_t)sz});
            
            memcpy(buff,arr->data.data(),std::min(sz,arr->data.size()));
           
            int64_t r;
            if(GetObject(res,r)) return r;
        }
        else if(GetObjectHeap(this->obj, strm))
        {
            return strm->stream->Read(buff,sz);
        }
        return 0;
    }
    size_t TObjectStream::Write(const uint8_t* buff, size_t sz)
    {
           TDictionary* dict;
        TStreamHeapObject* strm;
        if(GetObjectHeap(this->obj, dict))
        {
            GCList ls2(this->ls->GetGC());

            TByteArray* arr=TByteArray::Create(ls2);
            arr->data.resize(sz);
            memcpy(arr->data.data(), buff, sz);

            auto res = dict->CallMethod(ls2, "Write",{arr, (int64_t)0L, (int64_t)sz});
            int64_t r;
            if(GetObject(res,r)) return r;
        }
        else if(GetObjectHeap(this->obj, strm))
        {
            return strm->stream->Write(buff,sz);
        }
        return 0;
    }
    bool TObjectStream::CanRead()
    {

        TDictionary* dict;
        TStreamHeapObject* strm;
        if(GetObjectHeap(this->obj, dict))
        {
            auto res = dict->CallMethod(*ls, "getCanRead",{});
            bool r;
            if(GetObject(res,r)) return r;
        }
        else if(GetObjectHeap(this->obj, strm))
        {
            return strm->stream->CanRead();
        }
        return false;
    }
    bool TObjectStream::CanWrite()
    {

        TDictionary* dict;
        TStreamHeapObject* strm;
        if(GetObjectHeap(this->obj, dict))
        {
            auto res = dict->CallMethod(*ls, "getCanWrite",{});
            bool r;
            if(GetObject(res,r)) return r;
        }
        else if(GetObjectHeap(this->obj, strm))
        {
            return strm->stream->CanWrite();
        }
        return false;
    }
    bool TObjectStream::CanSeek()
    {

        TDictionary* dict;
        TStreamHeapObject* strm;
        if(GetObjectHeap(this->obj, dict))
        {
            auto res = dict->CallMethod(*ls, "getCanSeek",{});
            bool r;
            if(GetObject(res,r)) return r;
        }
        else if(GetObjectHeap(this->obj, strm))
        {
            return strm->stream->CanSeek();
        }
        return false;
    }
    int64_t TObjectStream::GetPosition()
    {

        TDictionary* dict;
        TStreamHeapObject* strm;
        if(GetObjectHeap(this->obj, dict))
        {
            auto res = dict->CallMethod(*ls, "getPosition",{});
            int64_t r;
            if(GetObject(res,r)) return r;
        }
        else if(GetObjectHeap(this->obj, strm))
        {
            return strm->stream->GetPosition();
        }
        return 0;
    }
    int64_t TObjectStream::GetLength()
    {

        TDictionary* dict;
        TStreamHeapObject* strm;
        if(GetObjectHeap(this->obj, dict))
        {
            auto res = dict->CallMethod(*ls, "getEndOfStream",{});
            bool r;
            if(GetObject(res,r)) return r;
        }
        else if(GetObjectHeap(this->obj, strm))
        {
            return strm->stream->GetLength();
        }
        return Tesses::Framework::Streams::Stream::GetLength();
    }
    void TObjectStream::Flush()
    {
        
        TDictionary* dict;
        TStreamHeapObject* strm;
        if(GetObjectHeap(this->obj, dict))
        {
            dict->CallMethod(*ls, "Flush",{});
           
        }
        else if(GetObjectHeap(this->obj, strm))
        {
            strm->stream->Flush();
        }
    }
    void TObjectStream::Seek(int64_t pos, Tesses::Framework::Streams::SeekOrigin whence)
    {
        TDictionary* dict;
        TStreamHeapObject* strm;
        if(GetObjectHeap(this->obj, dict))
        {
            dict->CallMethod(*ls, "Seek",{pos,(int64_t)(whence == Tesses::Framework::Streams::SeekOrigin::Begin ? 0 : whence == Tesses::Framework::Streams::SeekOrigin::Current ? 1 : 2) });
        }
        else if(GetObjectHeap(this->obj, strm))
        {
            strm->stream->Seek(pos,whence);
        }
    }
    TObjectStream::~TObjectStream()
    {
        delete this->ls;
    }


    TStreamHeapObject* TStreamHeapObject::Create(GCList& ls, Tesses::Framework::Streams::Stream* strm)
    {
        TStreamHeapObject* heapObj = new TStreamHeapObject();
        GC* _gc = ls.GetGC();
        ls.Add(heapObj);
        _gc->Watch(heapObj);
        heapObj->stream = strm;
        return heapObj;
    }
    TStreamHeapObject* TStreamHeapObject::Create(GCList* ls, Tesses::Framework::Streams::Stream* strm)
    {
        TStreamHeapObject* heapObj = new TStreamHeapObject();
        GC* _gc = ls->GetGC();
        ls->Add(heapObj);
        _gc->Watch(heapObj);
        heapObj->stream = strm;
        return heapObj;
    }
    TStreamHeapObject::~TStreamHeapObject()
    {
        if(this->stream != nullptr)
        {
            delete this->stream;
        }
    }
    void TStreamHeapObject::Close()
    {
        if(this->stream != nullptr)
        {
            delete this->stream;
            this->stream = nullptr;
        }
    }
}