#include "CrossLang.hpp"

namespace Tesses::CrossLang {

#define CROSSLANG_TSTREAM_NAMES                                                \
    CROSSLANG_TSTREAM_NAME_ENT(getCanRead)                                     \
    CROSSLANG_TSTREAM_NAME_ENT(getCanSeek)                                     \
    CROSSLANG_TSTREAM_NAME_ENT(getCanWrite)                                    \
    CROSSLANG_TSTREAM_NAME_ENT(getEndOfStream)                                 \
    CROSSLANG_TSTREAM_NAME_ENT(getLength)                                      \
    CROSSLANG_TSTREAM_NAME_ENT(getPosition)                                    \
    CROSSLANG_TSTREAM_NAME_ENT(setPosition)                                    \
    CROSSLANG_TSTREAM_NAME_ENT(Close)                                          \
    CROSSLANG_TSTREAM_NAME_ENT(Dispose)                                        \
    CROSSLANG_TSTREAM_NAME_ENT(CopyTo)                                         \
    CROSSLANG_TSTREAM_NAME_ENT(CopyToLimit)                                    \
    CROSSLANG_TSTREAM_NAME_ENT(Flush)                                          \
    CROSSLANG_TSTREAM_NAME_ENT(Read)                                           \
    CROSSLANG_TSTREAM_NAME_ENT(ReadBlock)                                      \
    CROSSLANG_TSTREAM_NAME_ENT(ReadByte)                                       \
    CROSSLANG_TSTREAM_NAME_ENT(Seek)                                           \
    CROSSLANG_TSTREAM_NAME_ENT(SetRecvTimeout)                                 \
    CROSSLANG_TSTREAM_NAME_ENT(SetSendTimeout)                                 \
    CROSSLANG_TSTREAM_NAME_ENT(Write)                                          \
    CROSSLANG_TSTREAM_NAME_ENT(WriteText)                                      \
    CROSSLANG_TSTREAM_NAME_ENT(WriteBlock)                                     \
    CROSSLANG_TSTREAM_NAME_ENT(WriteByte)

enum TStreamFuncNameEnum {
#define CROSSLANG_TSTREAM_NAME_ENT(name) TSF_##name,
    CROSSLANG_TSTREAM_NAMES
#undef CROSSLANG_TSTREAM_NAME_ENT
}; // namespace Tesses::CrossLang

static std::unordered_map<std::string_view, TStreamFuncNameEnum>
    stream_func_names = {
#define CROSSLANG_TSTREAM_NAME_ENT(name) {#name, TSF_##name},
        CROSSLANG_TSTREAM_NAMES
#undef CROSSLANG_TSTREAM_NAME_ENT
};

bool ITStream::Exists(std::string_view str) {
    return stream_func_names.count(str) != 0;
}

TObject ITStream::CallMethod(InterperterThread *thrd, GCList &ls,
                             const std::string &name,
                             const std::vector<TObject> &args) {

    auto strm = GetStream();
    if (strm == nullptr)
        return Undefined();

    auto result = stream_func_names.find(name);

    if (result == stream_func_names.end())
        return Undefined();

    switch (result->second) {
    case TSF_getCanRead:
        return strm->CanRead();
    case TSF_getCanSeek:
        return strm->CanSeek();
    case TSF_getCanWrite:
        return strm->CanWrite();
    case TSF_getEndOfStream:
        return strm->EndOfStream();
    case TSF_getLength:
        return strm->GetLength();
    case TSF_getPosition:
        return strm->GetPosition();
    case TSF_setPosition: {
        int64_t n;
        if (GetArgument(args, 0, n)) {
            strm->Seek(n, Tesses::Framework::Streams::SeekOrigin::Begin);
            return n;
        }
        return Undefined();
    }
    case TSF_Close:
    case TSF_Dispose:
        strm->Close();
        return Undefined();
    case TSF_CopyTo: {
        ITStream *strmDest;
        if (GetArgumentHeap(args, 0, strmDest)) {
            int64_t n = 1024;
            GetArgument(args, 1, n);
            strm->CopyTo(strmDest->GetStream(), (size_t)n);
        }
        return Undefined();
    }
    case TSF_CopyToLimit: {
        ITStream *strmDest;
        int64_t len;
        if (GetArgumentHeap(args, 0, strmDest) && GetArgument(args, 1, len)) {
            int64_t n = 1024;
            GetArgument(args, 2, n);
            strm->CopyToLimit(strmDest->GetStream(), (uint64_t)len, (size_t)n);
        }
        return Undefined();
    }
    case TSF_Flush:
        strm->Flush();
        return Undefined();
    case TSF_Read: {
        TMutByteView *bytes;
        if (GetArgumentHeap(args, 0, bytes)) {
            int64_t offset;
            int64_t length;
            if (GetArgument(args, 1, offset) && GetArgument(args, 2, length)) {
                auto safe = bytes->GetMutableBoundsConstrained((size_t)offset,
                                                               (size_t)length);
                if (safe.first != nullptr)
                    return strm->Read(safe.first, safe.second);
                else
                    return 0;
            }

            auto bounds = bytes->GetMutableBounds();

            if (bounds.first) {
                return (int64_t)strm->Read(bounds.first, bounds.second);
            }
        }

        return Undefined();
    }
    case TSF_ReadBlock: {
        TMutByteView *bytes;
        if (GetArgumentHeap(args, 0, bytes)) {
            int64_t offset;
            int64_t length;
            if (GetArgument(args, 1, offset) && GetArgument(args, 2, length)) {
                auto safe = bytes->GetMutableBoundsConstrained((size_t)offset,
                                                               (size_t)length);
                if (safe.first != nullptr)
                    return strm->ReadBlock(safe.first, safe.second);
                else
                    return 0;
            }

            auto bounds = bytes->GetMutableBounds();

            if (bounds.first) {
                return (int64_t)strm->ReadBlock(bounds.first, bounds.second);
            }
        }

        return Undefined();
    }
    case TSF_ReadByte: {
        return (int64_t)strm->ReadByte();
    }
    case TSF_Seek: {
        int64_t offset;
        int64_t whence;
        if (GetArgument(args, 0, offset) && GetArgument(args, 1, whence)) {
            strm->Seek(offset, (Tesses::Framework::Streams::SeekOrigin)whence);
        }
        return Undefined();
    }
    case TSF_SetRecvTimeout: {
        int64_t to;
        if (GetArgument(args, 0, to)) {
            strm->SetRecvTimeout((uint64_t)to);
        }
        return Undefined();
    }
    case TSF_SetSendTimeout: {
        int64_t to;
        if (GetArgument(args, 0, to)) {
            strm->SetSendTimeout((uint64_t)to);
        }
        return Undefined();
    }
    case TSF_Write: {
        TByteView *bytes;
        if (GetArgumentHeap(args, 0, bytes)) {
            int64_t offset;
            int64_t length;
            if (GetArgument(args, 1, offset) && GetArgument(args, 2, length)) {
                auto safe =
                    bytes->GetBoundsConstrained((size_t)offset, (size_t)length);
                if (safe.first != nullptr)
                    return strm->Write(safe.first, safe.second);
                else
                    return 0;
            }

            auto bounds = bytes->GetBounds();

            if (bounds.first) {
                return (int64_t)strm->Write(bounds.first, bounds.second);
            }
        }

        return Undefined();
    }
    case TSF_WriteText: { // for compat
        TByteView *bytes;
        if (GetArgumentHeap(args, 0, bytes)) {
            auto bounds = bytes->GetBounds();
            if (bounds.first)
                strm->WriteBlock(bounds.first, bounds.second);
        }
        return Undefined();
    }
    case TSF_WriteBlock: {
        TByteView *bytes;
        if (GetArgumentHeap(args, 0, bytes)) {
            int64_t offset;
            int64_t length;
            if (GetArgument(args, 1, offset) && GetArgument(args, 2, length)) {
                auto safe =
                    bytes->GetBoundsConstrained((size_t)offset, (size_t)length);
                if (safe.first != nullptr)
                    strm->WriteBlock(safe.first, safe.second);

                return Undefined();
            }

            auto bounds = bytes->GetBounds();

            if (bounds.first) {
                strm->WriteBlock(bounds.first, bounds.second);
            }
        }

        return Undefined();
    }
    case TSF_WriteByte: {
        int64_t b0;
        if (GetArgument(args, 0, b0)) {
            strm->WriteByte((uint8_t)b0);
        }
        return Undefined();
    }
    }

    return Undefined();
}
std::string ITStream::TypeName() { return "Stream"; }

TStream::TStream(std::shared_ptr<Tesses::Framework::Streams::Stream> strm)
    : strm(strm) {}
std::shared_ptr<Tesses::Framework::Streams::Stream> TStream::GetStream() {
    return this->strm;
}

} // namespace Tesses::CrossLang