#include "Remote_Scene.hpp"
#include <Mlib/Remote/Datagram_Nodes/Datagram_Node_Factory.hpp>
#include <Mlib/Remote/Datagram_Nodes/IDatagram_Node.hpp>
#include <Mlib/Remote/Incremental_Objects/Proxy_Tasks.hpp>
#include <Mlib/Remote/Incremental_Objects/Scene_Level.hpp>
#include <Mlib/Remote/Remote_Params.hpp>
#include <Mlib/Remote/Remote_Role.hpp>
#include <Mlib/Scene/Remote/Remote_Config.hpp>
#include <Mlib/Scene/Remote/Remote_Countdown.hpp>
#include <Mlib/Scene/Remote/Remote_Users.hpp>
#include <stdexcept>

using namespace Mlib;

static const RemoteParams& get_remote_params(RemoteConfig& config) {
    if (!config.game.has_value()) {
        throw std::runtime_error("Remote UDP parameters not set");
    }
    return *config.game;
}

RemoteScene::RemoteScene(
    const DanglingBaseClassRef<PhysicsScene>& physics_scene,
    const DanglingBaseClassRef<SceneLevelSelector>& scene_level_selector,
    RemoteConfig& remote_config,
    IoVerbosity verbosity)
    : verbosity_{ verbosity }
    #ifdef __EMSCRIPTEN__
    , home_node_{ DatagramNodeFactory::create_web_transport(get_remote_params(remote_config).socket, get_remote_params(remote_config).cert_hash) }
    #else
    , ctx_{1}
    , home_node_{ DatagramNodeFactory::create_udp(ctx_, get_remote_params(remote_config).socket) }
    #endif
    , remote_scene_object_factory_{
        physics_scene,
        scene_level_selector,
        verbosity }
    , objects_{ get_remote_params(remote_config).site_id, scene_level_selector }
    , communicator_proxy_factory_{
        { remote_scene_object_factory_, CURRENT_SOURCE_LOCATION },
        { objects_, CURRENT_SOURCE_LOCATION},
        verbosity,
        get_remote_params(remote_config).role == RemoteRole::SERVER
            ? ProxyTasks::SEND_LOCAL | ProxyTasks::SEND_REMOTE | ProxyTasks::SEND_OWNERSHIP
            : ProxyTasks::SEND_LOCAL | ProxyTasks::SEND_REMOTE | ProxyTasks::RELOAD_SCENE}
    , proxies_{
        { communicator_proxy_factory_, CURRENT_SOURCE_LOCATION },
        get_remote_params(remote_config).site_id}
{
    home_node_->start_receive_thread(100);
    [&](){
        switch (get_remote_params(remote_config).role) {
        case RemoteRole::SERVER:
            home_node_->bind();
            proxies_.add_receive_socket({*home_node_, CURRENT_SOURCE_LOCATION});
            #ifdef WITHOUT_GRAPHICS
            proxies_.add_receive_socket(remote_config.admin_socket);
            #endif
            objects_.add_local_object({
                    global_object_pool.create<RemoteCountdown>(
                        CURRENT_SOURCE_LOCATION,
                        verbosity,
                        physics_scene),
                    CURRENT_SOURCE_LOCATION},
                RemoteObjectVisibility::PRIVATE);
            return;
        case RemoteRole::CLIENT:
            proxies_.add_handshake_socket(home_node_);
            proxies_.add_receive_socket({*home_node_, CURRENT_SOURCE_LOCATION});
            proxies_.send_and_receive(TransmissionType::HANDSHAKE);
            return;
        }
        throw std::runtime_error("Unkown remote role: " + std::to_string((int)get_remote_params(remote_config).role));
    }();
    objects_.add_local_object({
            global_object_pool.create<RemoteUsers>(
                CURRENT_SOURCE_LOCATION,
                verbosity,
                physics_scene,
                scene_level_selector,
                get_remote_params(remote_config).site_id),
            CURRENT_SOURCE_LOCATION},
        RemoteObjectVisibility::PUBLIC);
}

RemoteScene::~RemoteScene() {
    linfo() << "~RemoteScene";
}

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
