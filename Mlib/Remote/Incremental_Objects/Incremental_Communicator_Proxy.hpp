#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Remote/Communicator_Proxies.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Cache_Proxy_Token.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Remote_Objects.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Versions.hpp>
#include <Mlib/Remote/Statistics/Remote_Transmission_Statistics.hpp>
#include <Mlib/Remote/Transmission_Scheduler.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <iosfwd>
#include <unordered_set>

namespace Mlib {

class ISendSocket;
class IIncrementalObjectFactory;
class ProxyObjectsCaches;
enum class IoVerbosity;
enum class ProxyTasks;

class IncrementalCommunicatorProxy final: public virtual ICommunicatorProxy {
public:
    IncrementalCommunicatorProxy(
        std::shared_ptr<ISendSocket> send_socket,
        const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
        const DanglingBaseClassRef<IncrementalRemoteObjects>& objects,
        const DanglingBaseClassRef<ProxyObjectsCaches>& proxy_objects_caches,
        const TransmissionLut& full_transmission_lut,
        IoVerbosity verbosity,
        ProxyTasks tasks,
        RemoteSiteId home_site_id);
    virtual ~IncrementalCommunicatorProxy() override;
    virtual void set_send_socket(std::shared_ptr<ISendSocket> send_socket) override;
    virtual void receive_from_home(std::istream& istr) override;
    virtual void send_home(std::iostream& iostr, SendStatusCode& status_code) override;
private:
    IncrementalCacheProxyToken incremental_cache_proxy_token_;
    uint32_t receive_datagram_counter_;
    uint32_t send_datagram_counter_;
    std::unordered_set<RemoteObjectId> objects_unknown_at_home_;
    std::unordered_set<RemoteObjectId> objects_unknown_here_;
    std::shared_ptr<ISendSocket> send_socket_;
    DanglingBaseClassRef<IIncrementalObjectFactory> shared_object_factory_;
    DanglingBaseClassRef<IncrementalRemoteObjects> objects_;
    DanglingBaseClassRef<ProxyObjectsCaches> proxy_objects_caches_;
    TransmissionLut full_transmission_lut_;
    IoVerbosity verbosity_;
    ProxyTasks tasks_;
    RemoteSiteId home_site_id_;
    SessionIdType session_id_;
    SocketVersions socket_versions_;
    RemoteTransmissionStatistics stats_;
};

}
