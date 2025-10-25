#include "Communicator_Proxy_Factory.hpp"
#include <Mlib/Remote/Incremental_Objects/Communicator_Proxy.hpp>

using namespace Mlib;

CommunicatorProxyFactory::CommunicatorProxyFactory(DanglingBaseClassRef<ISharedObjectFactory> shared_object_factory)
    : shared_object_factory_{ std::move(shared_object_factory) }
    , object_pool_{ InObjectPoolDestructor::CLEAR }
{}

CommunicatorProxyFactory::~CommunicatorProxyFactory() = default;

DanglingBaseClassRef<ICommunicatorProxy> CommunicatorProxyFactory::create_communicator_proxy(
    DanglingBaseClassRef<ISendSocket> send_socket)
{
    return { object_pool_.create<CommunicatorProxy>(
            CURRENT_SOURCE_LOCATION,
            std::move(send_socket),
            shared_object_factory_),
        CURRENT_SOURCE_LOCATION
    };
}
