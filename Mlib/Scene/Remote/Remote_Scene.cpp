#include "Remote_Scene.hpp"
#include <Mlib/Remote/Remote_Role.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RemoteScene::RemoteScene(
    const DanglingBaseClassRef<PhysicsScene>& physics_scene,
    const std::string& ip_address,
    uint16_t port,
    RemoteRole remote_role,
    RemoteSiteId location_id)
    : object_pool_{ InObjectPoolDestructor::CLEAR }
    , home_node_{ ip_address, port }
    , remote_scene_object_factory_{
        {object_pool_, CURRENT_SOURCE_LOCATION},
        physics_scene }
    , objects_{ location_id }
    , communicator_proxy_factory_{
        { remote_scene_object_factory_, CURRENT_SOURCE_LOCATION },
        { objects_, CURRENT_SOURCE_LOCATION} }
    , proxies_{
        { communicator_proxy_factory_, CURRENT_SOURCE_LOCATION },
        location_id}
{
    switch (remote_role) {
    case RemoteRole::SERVER:
        home_node_.bind();
        home_node_.start_receive_thread(100);
        proxies_.add_receive_socket({home_node_, CURRENT_SOURCE_LOCATION});
        return;
    case RemoteRole::CLIENT:
        home_node_.start_receive_thread(100);
        proxies_.add_handshake_socket({home_node_, CURRENT_SOURCE_LOCATION});
        proxies_.add_receive_socket({home_node_, CURRENT_SOURCE_LOCATION});
        proxies_.send_and_receive(TransmissionType::HANDSHAKE);
        return;
    }
    THROW_OR_ABORT("Unkown remote role: " + std::to_string((int)remote_role));
}

RemoteScene::~RemoteScene() = default;

void RemoteScene::send_and_receive(std::chrono::steady_clock::time_point time) {
    proxies_.send_and_receive(TransmissionType::UNICAST);
}

void RemoteScene::add_local_object(const DanglingBaseClassRef<IIncrementalObject>& object) {
    objects_.add_local_object(object);
}

void RemoteScene::add_remote_object(
    const RemoteObjectId& id,
    const DanglingBaseClassRef<IIncrementalObject>& object)
{
    objects_.add_remote_object(id, object);
}
