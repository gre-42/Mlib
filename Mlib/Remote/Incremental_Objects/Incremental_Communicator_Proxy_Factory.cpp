#include "Incremental_Communicator_Proxy_Factory.hpp"
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Remote_Objects.hpp>
#include <Mlib/Remote/Incremental_Objects/Proxy_Tasks.hpp>

using namespace Mlib;

IncrementalCommunicatorProxyFactory::IncrementalCommunicatorProxyFactory(
    const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
    const DanglingBaseClassRef<IncrementalRemoteObjects>& objects,
    IoVerbosity verbosity,
    ProxyTasks tasks)
    : shared_object_factory_{ shared_object_factory }
    , objects_{ objects }
    , verbosity_{ verbosity }
    , tasks_{ tasks }
    , object_pool_{ InObjectPoolDestructor::CLEAR }
{}

IncrementalCommunicatorProxyFactory::~IncrementalCommunicatorProxyFactory() = default;

DanglingBaseClassRef<ICommunicatorProxy> IncrementalCommunicatorProxyFactory::create_handshake_proxy(
    std::shared_ptr<ISendSocket> send_socket)
{
    return { object_pool_.create<IncrementalCommunicatorProxy>(
            CURRENT_SOURCE_LOCATION,
            std::move(send_socket),
            shared_object_factory_,
            objects_,
            verbosity_,
            ProxyTasks::NONE,
            0xC0FEFACE),
        CURRENT_SOURCE_LOCATION
    };
}

DanglingBaseClassRef<ICommunicatorProxy> IncrementalCommunicatorProxyFactory::create_communicator_proxy(
    std::shared_ptr<ISendSocket> send_socket,
    RemoteSiteId home_site_id)
{
    return { object_pool_.create<IncrementalCommunicatorProxy>(
            CURRENT_SOURCE_LOCATION,
            std::move(send_socket),
            shared_object_factory_,
            objects_,
            verbosity_,
            tasks_,
            home_site_id),
        CURRENT_SOURCE_LOCATION
    };
}
