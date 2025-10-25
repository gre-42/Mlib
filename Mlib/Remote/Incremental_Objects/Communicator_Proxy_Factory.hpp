#pragma once
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Remote/Distributed_System.hpp>

namespace Mlib {

class CommunicatorProxyFactory: public ICommunicatorProxyFactory {
public:
    explicit CommunicatorProxyFactory(DanglingBaseClassRef<ISharedObjectFactory> shared_object_factory);
    virtual ~CommunicatorProxyFactory() override;
    virtual DanglingBaseClassRef<ICommunicatorProxy> create_communicator_proxy(
        DanglingBaseClassRef<ISendSocket> send_socket) override;
private:
    DanglingBaseClassRef<ISharedObjectFactory> shared_object_factory_;
    ObjectPool object_pool_;
};

}
