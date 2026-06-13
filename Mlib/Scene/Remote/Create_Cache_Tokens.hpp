#pragma once
#include <Mlib/Remote/Incremental_Objects/Incremental_Cache_Object_Token.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>

namespace Mlib {

inline auto create_cache_proxy_token(PhysicsScene& physics_scene, RemoteSiteId proxy_id)
{
    if (physics_scene.remote_scene_ == nullptr) {
        throw std::runtime_error("Remote scene not set");
    }
    return physics_scene.remote_scene_->cache_proxy_token(proxy_id);
}

inline auto create_cache_object_token(PhysicsScene& physics_scene, RemoteObjectId object_id)
{
    if (physics_scene.remote_scene_ == nullptr) {
        throw std::runtime_error("Remote scene not set");
    }
    return physics_scene.remote_scene_->cache_object_token(object_id);
}

}
