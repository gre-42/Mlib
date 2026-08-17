#pragma once
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Remote/Communicator_Proxies.hpp>
#include <cstdint>
#include <vector>

namespace Mlib {

class IIncrementalObjectFactory;
class IncrementalRemoteObjects;
class ProxyObjectsCaches;
enum class IoVerbosity;
enum class ProxyTasks;

class IncrementalCommunicatorProxyFactory: public ICommunicatorProxyFactory {
public:
    explicit IncrementalCommunicatorProxyFactory(
        const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
        const DanglingBaseClassRef<IncrementalRemoteObjects>& objects,
        const DanglingBaseClassRef<ProxyObjectsCaches>& proxy_objects_caches,
        std::vector<uint32_t> transmission_intervals,
        IoVerbosity verbosity,
        ProxyTasks tasks);
    virtual ~IncrementalCommunicatorProxyFactory() override;
    virtual DanglingBaseClassRef<ICommunicatorProxy> create_handshake_proxy(
        std::shared_ptr<ISendSocket> send_socket) override;
    virtual DanglingBaseClassRef<ICommunicatorProxy> create_communicator_proxy(
        std::shared_ptr<ISendSocket> send_socket,
        RemoteSiteId home_site_id) override;
private:
    DanglingBaseClassRef<IIncrementalObjectFactory> shared_object_factory_;
    DanglingBaseClassRef<IncrementalRemoteObjects> objects_;
    DanglingBaseClassRef<ProxyObjectsCaches> proxy_objects_caches_;
    std::vector<uint32_t> transmission_intervals_;
    IoVerbosity verbosity_;
    ProxyTasks tasks_;
    ObjectPool object_pool_;
};

}
