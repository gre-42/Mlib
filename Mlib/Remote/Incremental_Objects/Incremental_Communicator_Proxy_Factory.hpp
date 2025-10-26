#pragma once
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Remote/Communicator_Proxies.hpp>

namespace Mlib {

class IIncrementalObjectFactory;
class IncrementalRemoteObjects;

class IncrementalCommunicatorProxyFactory: public ICommunicatorProxyFactory {
public:
    explicit IncrementalCommunicatorProxyFactory(
        DanglingBaseClassRef<IIncrementalObjectFactory> shared_object_factory,
        DanglingBaseClassRef<IncrementalRemoteObjects> objects);
    virtual ~IncrementalCommunicatorProxyFactory() override;
    virtual DanglingBaseClassRef<ICommunicatorProxy> create_communicator_proxy(
        DanglingBaseClassRef<ISendSocket> send_socket) override;
private:
    DanglingBaseClassRef<IIncrementalObjectFactory> shared_object_factory_;
    DanglingBaseClassRef<IncrementalRemoteObjects> objects_;
    ObjectPool object_pool_;
};

}
