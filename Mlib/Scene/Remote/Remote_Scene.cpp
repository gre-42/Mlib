#include "Remote_Scene.hpp"
#include <Mlib/Remote/Incremental_Objects/Proxy_Tasks.hpp>
#include <Mlib/Remote/Incremental_Objects/Scene_Level.hpp>
#include <Mlib/Remote/Remote_Params.hpp>
#include <Mlib/Remote/Remote_Role.hpp>
#include <Mlib/Remote/Sockets/Udp_Node.hpp>
#include <Mlib/Scene/Remote/Remote_Countdown.hpp>
#include <Mlib/Scene/Remote/Remote_Users.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RemoteScene::RemoteScene(
    const DanglingBaseClassRef<PhysicsScene>& physics_scene,
    const DanglingBaseClassRef<SceneLevelSelector>& scene_level_selector,
    const RemoteParams& remote_params,
    IoVerbosity verbosity)
    : verbosity_{ verbosity }
    , home_node_{ std::make_shared<UdpNode>(remote_params.ip, remote_params.port) }
    , remote_scene_object_factory_{
        physics_scene,
        scene_level_selector,
        verbosity }
    , objects_{ remote_params.site_id, scene_level_selector }
    , communicator_proxy_factory_{
        { remote_scene_object_factory_, CURRENT_SOURCE_LOCATION },
        { objects_, CURRENT_SOURCE_LOCATION},
        verbosity,
        remote_params.role == RemoteRole::SERVER
            ? ProxyTasks::SEND_LOCAL | ProxyTasks::SEND_REMOTE | ProxyTasks::SEND_OWNERSHIP
            : ProxyTasks::SEND_LOCAL | ProxyTasks::SEND_REMOTE | ProxyTasks::RELOAD_SCENE}
    , proxies_{
        { communicator_proxy_factory_, CURRENT_SOURCE_LOCATION },
        remote_params.site_id}
{
    [&](){
        switch (remote_params.role) {
        case RemoteRole::SERVER:
            home_node_->bind();
            proxies_.add_receive_socket({*home_node_, CURRENT_SOURCE_LOCATION});
            objects_.add_local_object({
                    global_object_pool.create<RemoteCountdown>(
                        CURRENT_SOURCE_LOCATION,
                        verbosity,
                        physics_scene),
                    CURRENT_SOURCE_LOCATION},
                RemoteObjectVisibility::PUBLIC);
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
            global_object_pool.create<RemoteUsers>(
                CURRENT_SOURCE_LOCATION,
                verbosity,
                physics_scene,
                scene_level_selector,
                remote_params.site_id),
            CURRENT_SOURCE_LOCATION},
        RemoteObjectVisibility::PUBLIC);
    home_node_->start_receive_thread(100);
}

RemoteScene::~RemoteScene() = default;

void RemoteScene::send_and_receive(const TimeAndPause<std::chrono::steady_clock::time_point>& time) {
    objects_.set_local_time(time);
    objects_.forget_old_deleted_objects();
    proxies_.send_and_receive(TransmissionType::UNICAST);
}

RemoteObjectId RemoteScene::add_local_object(const DanglingBaseClassRef<IIncrementalObject>& object) {
    return objects_.add_local_object(object, RemoteObjectVisibility::PUBLIC);
}

void RemoteScene::add_remote_object(
    const RemoteObjectId& id,
    const DanglingBaseClassRef<IIncrementalObject>& object)
{
    objects_.add_remote_object(id, object, RemoteObjectVisibility::PUBLIC);
}

DanglingBaseClassPtr<IIncrementalObject> RemoteScene::try_get(
    const RemoteObjectId& id) const
{
    return objects_.try_get(id);
}

bool RemoteScene::try_remove(const RemoteObjectId& id) {
    return objects_.try_remove(id);
}

RemoteSiteId RemoteScene::local_site_id() const {
    return objects_.local_site_id();
}
