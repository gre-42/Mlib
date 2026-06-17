#pragma once
#include <Mlib/Misc/Source_Location.hpp>
#include <Mlib/Remote/Communicator_Proxies.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Cache_Object_Token.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Cache_Proxy_Token.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy_Factory.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Remote_Objects.hpp>
#include <Mlib/Remote/Incremental_Objects/Proxy_Object_Cache.hpp>
#include <Mlib/Remote/Remote_Params.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Factory.hpp>
#include <chrono>
#include <memory>
#ifndef __EMSCRIPTEN__
#include <Mlib/Remote/Sockets/Asio.hpp>
#endif

namespace Mlib {

template <class TTimepoint>
class TimeAndPause;
class PhysicsScene;
class SceneLevelSelector;
class IDatagramNode;
class IReceiveSocket;
enum class IoVerbosity;
struct RemoteConfig;

class RemoteScene {
public:
    RemoteScene(
        const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        const DanglingBaseClassRef<SceneLevelSelector>& scene_level_selector,
        RemoteConfig& remote_config,
        IoVerbosity verbosity);
    ~RemoteScene();
    RemoteObjectId add_local_object(const DanglingBaseClassRef<IIncrementalObject>& object);
    void add_remote_object(const RemoteObjectId& id, const DanglingBaseClassRef<IIncrementalObject>& object);
    void send_and_receive(const TimeAndPause<std::chrono::steady_clock::time_point>& time);
    DanglingBaseClassPtr<IIncrementalObject> try_get(const RemoteObjectId& id) const;
    bool try_remove(const RemoteObjectId& id);
    LocalObjectId next_local_object_id() const;
    template<class Class, class... Args>
    RemoteObjectId create_local(SourceLocation loc, Args&&... args) {
        return add_local_object({global_object_pool.create<Class>(loc, verbosity_, std::forward<Args>(args)...), loc});
    }
    RemoteSiteId local_site_id() const;
    inline IncrementalCacheProxyToken cache_proxy_token(RemoteSiteId proxy_id)
    {
        return { {proxy_objects_caches_, CURRENT_SOURCE_LOCATION}, proxy_id };
    }
    inline IncrementalCacheObjectToken cache_object_token(RemoteObjectId object_id)
    {
        return { {proxy_objects_caches_, CURRENT_SOURCE_LOCATION}, object_id };
    }
private:
    RemoteParams remote_params_;
    IoVerbosity verbosity_;
    #ifndef __EMSCRIPTEN__
    boost::asio::io_context ctx_;
    #endif
    std::shared_ptr<IDatagramNode> home_node_;
    RemoteSceneObjectFactory remote_scene_object_factory_;
    IncrementalRemoteObjects objects_;
    ProxyObjectsCaches proxy_objects_caches_;
    IncrementalCommunicatorProxyFactory communicator_proxy_factory_;
    CommunicatorProxies proxies_;
};

}
