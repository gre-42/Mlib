#pragma once
#include <Mlib/Remote/Communicator_Proxies.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy_Factory.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Remote_Objects.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Factory.hpp>
#include <Mlib/Source_Location.hpp>
#include <chrono>
#include <memory>

namespace Mlib {

class PhysicsScene;
class SceneLevelSelector;
class UdpNode;
struct RemoteParams;
enum class IoVerbosity;

class RemoteScene {
public:
    RemoteScene(
        const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        const DanglingBaseClassRef<SceneLevelSelector>& scene_level_selector,
        const RemoteParams& remote_params,
        IoVerbosity verbosity);
    ~RemoteScene();
    RemoteObjectId add_local_object(const DanglingBaseClassRef<IIncrementalObject>& object);
    void add_remote_object(const RemoteObjectId& id, const DanglingBaseClassRef<IIncrementalObject>& object);
    void send_and_receive(std::chrono::steady_clock::time_point time);
    DanglingBaseClassPtr<IIncrementalObject> try_get(const RemoteObjectId& id) const;
    bool try_remove(const RemoteObjectId& id);
    template<class Class, class... Args>
    RemoteObjectId create_local(SourceLocation loc, Args&&... args) {
        return add_local_object({global_object_pool.create<Class>(loc, verbosity_, std::forward<Args>(args)...), loc});
    }
    RemoteSiteId local_site_id() const;
private:
    IoVerbosity verbosity_;
    std::shared_ptr<UdpNode> home_node_;
    RemoteSceneObjectFactory remote_scene_object_factory_;
    IncrementalRemoteObjects objects_;
    IncrementalCommunicatorProxyFactory communicator_proxy_factory_;
    CommunicatorProxies proxies_;
};

}
