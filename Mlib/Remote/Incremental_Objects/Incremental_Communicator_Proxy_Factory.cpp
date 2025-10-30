#include "Incremental_Communicator_Proxy_Factory.hpp"
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Remote_Objects.hpp>

using namespace Mlib;

IncrementalCommunicatorProxyFactory::IncrementalCommunicatorProxyFactory(
    const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
    const DanglingBaseClassRef<IncrementalRemoteObjects>& objects)
    : shared_object_factory_{ shared_object_factory }
    , objects_{ objects }
    , object_pool_{ InObjectPoolDestructor::CLEAR }
{}

IncrementalCommunicatorProxyFactory::~IncrementalCommunicatorProxyFactory() = default;

DanglingBaseClassRef<ICommunicatorProxy> IncrementalCommunicatorProxyFactory::create_communicator_proxy(
    const DanglingBaseClassRef<ISendSocket>& send_socket)
{
    return { object_pool_.create<IncrementalCommunicatorProxy>(
            CURRENT_SOURCE_LOCATION,
            send_socket,
            shared_object_factory_,
            objects_),
        CURRENT_SOURCE_LOCATION
    };
}
