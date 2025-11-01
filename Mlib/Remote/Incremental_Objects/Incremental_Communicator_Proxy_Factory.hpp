#pragma once
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Remote/Communicator_Proxies.hpp>

namespace Mlib {

class IIncrementalObjectFactory;
class IncrementalRemoteObjects;
enum class IoVerbosity;

class IncrementalCommunicatorProxyFactory: public ICommunicatorProxyFactory {
public:
    explicit IncrementalCommunicatorProxyFactory(
        const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
        const DanglingBaseClassRef<IncrementalRemoteObjects>& objects,
        IoVerbosity verbosity);
    virtual ~IncrementalCommunicatorProxyFactory() override;
    virtual DanglingBaseClassRef<ICommunicatorProxy> create_communicator_proxy(
        std::shared_ptr<ISendSocket> send_socket) override;
private:
    DanglingBaseClassRef<IIncrementalObjectFactory> shared_object_factory_;
    DanglingBaseClassRef<IncrementalRemoteObjects> objects_;
    IoVerbosity verbosity_;
    ObjectPool object_pool_;
};

}
