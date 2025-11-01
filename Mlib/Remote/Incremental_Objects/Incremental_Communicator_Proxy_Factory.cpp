#include "Incremental_Communicator_Proxy_Factory.hpp"
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Remote_Objects.hpp>

using namespace Mlib;

IncrementalCommunicatorProxyFactory::IncrementalCommunicatorProxyFactory(
    const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
    const DanglingBaseClassRef<IncrementalRemoteObjects>& objects,
    IoVerbosity verbosity)
    : shared_object_factory_{ shared_object_factory }
    , objects_{ objects }
    , verbosity_{ verbosity }
    , object_pool_{ InObjectPoolDestructor::CLEAR }
{}

IncrementalCommunicatorProxyFactory::~IncrementalCommunicatorProxyFactory() = default;

DanglingBaseClassRef<ICommunicatorProxy> IncrementalCommunicatorProxyFactory::create_communicator_proxy(
    std::shared_ptr<ISendSocket> send_socket)
{
    return { object_pool_.create<IncrementalCommunicatorProxy>(
            CURRENT_SOURCE_LOCATION,
            std::move(send_socket),
            shared_object_factory_,
            objects_,
            verbosity_),
        CURRENT_SOURCE_LOCATION
    };
}
