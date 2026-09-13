#include "CrossLang.hpp"

namespace Tesses::CrossLang {
TNetworkStream::TNetworkStream(
    std::shared_ptr<Tesses::Framework::Streams::NetworkStream> strm)
    : strm(strm) {}
std::shared_ptr<Tesses::Framework::Streams::Stream>
TNetworkStream::GetStream() {
    return this->strm;
}
TObject TNetworkStream::CallMethod(InterperterThread *thrd, GCList &ls,
                                   const std::string &name,
                                   const std::vector<TObject> &args) {
    int64_t n0;
    bool bc;
    if (name == "setBroadcast" && GetArgument(args, 0, bc))
        strm->SetBroadcast(bc);
    if (name == "setNoDelay" && GetArgument(args, 0, bc))
        strm->SetNoDelay(bc);
    if (name == "setReuseAddress" && GetArgument(args, 0, bc))
        strm->SetReuseAddress(bc);
    if (name == "setReusePort" && GetArgument(args, 0, bc))
        strm->SetReusePort(bc);
    if (name == "MulticastTTL" && GetArgument(args, 0, n0))
        strm->SetMulticastTTL((uint8_t)n0);
    if (name == "getPort" || name == "GetPort") {
        return (int64_t)strm->GetPort();
    }

    if (name == "SetMulticastMembership") {
        std::string ma;
        std::string ifaceIP = "0.0.0.0";
        if (GetArgument(args, 0, ma)) {
            GetArgument(args, 1, ifaceIP);
            strm->SetMulticastMembership(ma, ifaceIP);
        }
        return Undefined();
    }
    if (name == "Bind") {
        std::string ip;
        int64_t port;
        if (GetArgument(args, 0, ip) && GetArgument(args, 1, port))
            strm->Bind(ip, (uint16_t)port);

        return Undefined();
    }

    if (name == "Accept") {
        std::string ip;
        uint16_t port;
        auto strm2 = strm->Accept(ip, port);

        if (strm2 ==
            nullptr) // just in case, so we don't get a bugged TNetworkStream
            return nullptr;

        std::array list = {
            TDItem("IP", ls.FromString(ip)),
            TDItem("Port", (int64_t)port),
            TDItem("Stream", ls.Create<TNetworkStream>(strm2)),
        };

        return ls.Create<TDictionary>(list.begin(), list.end());
    }
    if (name == "Listen") {
        int64_t backlog;
        if (GetArgument(args, 0, backlog)) {
            strm->Listen((int32_t)backlog);
        } else {
            strm->Listen(10);
        }

        return Undefined();
    }
    if (name == "ReadFrom") {
        TByteArray *data;
        int64_t offset;
        int64_t length;

        if (name == "Read") {
            TMutByteView *bytes;
            if (GetArgumentHeap(args, 0, bytes)) {
                int64_t offset;
                int64_t length;
                if (GetArgument(args, 1, offset) &&
                    GetArgument(args, 2, length)) {
                    auto safe = bytes->GetMutableBoundsConstrained(
                        (size_t)offset, (size_t)length);
                    if (safe.first != nullptr) {
                        std::string ip = {};
                        uint16_t port = 0;
                        auto read =
                            strm->ReadFrom(safe.first, safe.second, ip, port);
                        std::array list = {
                            TDItem("IP", ls.FromString(ip)),
                            TDItem("Port", (int64_t)port),
                            TDItem("Read", (int64_t)read),
                        };
                        return ls.Create<TDictionary>(list.begin(), list.end());
                    } else {
                        return nullptr;
                    }
                }

                auto bounds = bytes->GetMutableBounds();

                if (bounds.first) {
                    std::string ip = {};
                    uint16_t port = 0;
                    auto read =
                        strm->ReadFrom(bounds.first, bounds.second, ip, port);
                    std::array list = {
                        TDItem("IP", ls.FromString(ip)),
                        TDItem("Port", (int64_t)port),
                        TDItem("Read", (int64_t)read),
                    };
                    return ls.Create<TDictionary>(list.begin(), list.end());
                }
            }

            return Undefined();
        }
    }
    if (name == "WriteTo") {

        // strm->WriteTo(buff, ip, port)
        // strm->WriteTo(buff, off, len, ip, port)

        TByteView *bytes;
        if (GetArgumentHeap(args, 0, bytes)) {
            int64_t offset;
            int64_t length;
            TString *ip;
            int64_t port;
            if (GetArgument(args, 1, offset) && GetArgument(args, 2, length)) {
                if (!(GetArgumentHeap(args, 3, ip) &&
                      GetArgument(args, 4, port)))
                    return Undefined();
                auto safe =
                    bytes->GetBoundsConstrained((size_t)offset, (size_t)length);
                if (safe.first != nullptr) {
                    return strm->WriteTo(safe.first, safe.second,
                                         ip->GetString(), (uint16_t)port);
                } else
                    return nullptr;
            }
            if (!(GetArgumentHeap(args, 1, ip) && GetArgument(args, 2, port)))
                return Undefined();
            auto bounds = bytes->GetBounds();

            if (bounds.first) {

                return (int64_t)strm->WriteTo(bounds.first, bounds.second,
                                              ip->GetString(), (uint16_t)port);
            }
        }

        return Undefined();
    }

    return ITStream::CallMethod(thrd, ls, name, args);
}

std::string TNetworkStream::TypeName() { return "NetworkStream"; }
} // namespace Tesses::CrossLang