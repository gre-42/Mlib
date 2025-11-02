#include "Remote_Scene.hpp"
#include <Mlib/Remote/Remote_Params.hpp>
#include <Mlib/Remote/Remote_Role.hpp>
#include <Mlib/Remote/Sockets/Udp_Node.hpp>
#include <Mlib/Scene/Remote/Remote_Users.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RemoteScene::RemoteScene(
    const DanglingBaseClassRef<PhysicsScene>& physics_scene,
    const RemoteParams& remote_params,
    IoVerbosity verbosity)
    : verbosity_{ verbosity }
    , object_pool_{ InObjectPoolDestructor::CLEAR }
    , home_node_{ std::make_shared<UdpNode>(remote_params.ip, remote_params.port) }
    , remote_scene_object_factory_{
        {object_pool_, CURRENT_SOURCE_LOCATION},
        physics_scene,
        verbosity }
    , objects_{ remote_params.site_id }
    , communicator_proxy_factory_{
        { remote_scene_object_factory_, CURRENT_SOURCE_LOCATION },
        { objects_, CURRENT_SOURCE_LOCATION},
        verbosity,
        remote_params.role == RemoteRole::SERVER
            ? ProxyTasks::SEND_LOCAL | ProxyTasks::SEND_REMOTE
            : ProxyTasks::SEND_LOCAL }
    , proxies_{
        { communicator_proxy_factory_, CURRENT_SOURCE_LOCATION },
        remote_params.site_id}
{
    [&](){
        switch (remote_params.role) {
        case RemoteRole::SERVER:
            home_node_->bind();
            proxies_.add_receive_socket({*home_node_, CURRENT_SOURCE_LOCATION});
            return;
        case RemoteRole::CLIENT:
            proxies_.add_handshake_socket(home_node_);
            proxies_.add_receive_socket({*home_node_, CURRENT_SOURCE_LOCATION});
            proxies_.send_and_receive(TransmissionType::HANDSHAKE);
            return;
        }
        THROW_OR_ABORT("Unkown remote role: " + std::to_string((int)remote_params.role));
    }();
    objects_.add_local_object({
            object_pool_.create<RemoteUsers>(
                CURRENT_SOURCE_LOCATION,
                object_pool_,
                verbosity,
                physics_scene,
                remote_params.site_id),
            CURRENT_SOURCE_LOCATION},
        RemoteObjectVisibility::PUBLIC);
    home_node_->start_receive_thread(100);
}

RemoteScene::~RemoteScene() = default;

void RemoteScene::send_and_receive(std::chrono::steady_clock::time_point time) {
    proxies_.send_and_receive(TransmissionType::UNICAST);
}

void RemoteScene::add_local_object(const DanglingBaseClassRef<IIncrementalObject>& object) {
    objects_.add_local_object(object, RemoteObjectVisibility::PUBLIC);
}

void RemoteScene::add_remote_object(
    const RemoteObjectId& id,
    const DanglingBaseClassRef<IIncrementalObject>& object)
{
    objects_.add_remote_object(id, object, RemoteObjectVisibility::PUBLIC);
}
