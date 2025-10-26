#include "Incremental_Communicator_Proxy_Factory.hpp"
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Remote_Objects.hpp>

using namespace Mlib;

IncrementalCommunicatorProxyFactory::IncrementalCommunicatorProxyFactory(
    DanglingBaseClassRef<IIncrementalObjectFactory> shared_object_factory,
    DanglingBaseClassRef<IncrementalRemoteObjects> objects)
    : shared_object_factory_{ std::move(shared_object_factory) }
    , objects_{ std::move(objects) }
    , object_pool_{ InObjectPoolDestructor::CLEAR }
{}

IncrementalCommunicatorProxyFactory::~IncrementalCommunicatorProxyFactory() = default;

DanglingBaseClassRef<ICommunicatorProxy> IncrementalCommunicatorProxyFactory::create_communicator_proxy(
    DanglingBaseClassRef<ISendSocket> send_socket)
{
    return { object_pool_.create<IncrementalCommunicatorProxy>(
            CURRENT_SOURCE_LOCATION,
            std::move(send_socket),
            shared_object_factory_,
            objects_),
        CURRENT_SOURCE_LOCATION
    };
}
