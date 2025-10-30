#pragma once
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Remote/Communicator_Proxies.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy_Factory.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Remote_Objects.hpp>
#include <Mlib/Remote/Sockets/Udp_Node.hpp>
#include <Mlib/Scene/Remote/Created_At_Remote_Site.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Factory.hpp>
#include <Mlib/Source_Location.hpp>
#include <chrono>

namespace Mlib {

class PhysicsScene;
class AssetReferences;
enum class RemoteRole;

class RemoteScene {
public:
    RemoteScene(
        const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        const std::string& ip_address,
        uint16_t port,
        RemoteRole remote_role,
        RemoteSiteId location_id);
    ~RemoteScene();
    void add_local_object(const DanglingBaseClassRef<IIncrementalObject>& object);
    void add_remote_object(const RemoteObjectId& id, const DanglingBaseClassRef<IIncrementalObject>& object);
    void send_and_receive(std::chrono::steady_clock::time_point time);
    CreatedAtRemoteSite created_at_remote_site;
    template<class Class, class... Args>
    void create_local(SourceLocation loc, Args&&... args) {
        add_local_object({object_pool_.create<Class>(loc, object_pool_, std::forward<Args>(args)...), loc});
    }
private:
    ObjectPool object_pool_;
    UdpNode home_node_;
    RemoteSceneObjectFactory remote_scene_object_factory_;
    IncrementalRemoteObjects objects_;
    IncrementalCommunicatorProxyFactory communicator_proxy_factory_;
    CommunicatorProxies proxies_;
};

}
