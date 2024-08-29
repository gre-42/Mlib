#pragma once
#include <Mlib/Features.hpp>
#include <list>
#include <mutex>

namespace Mlib {

class SceneNodeResources;
class RenderingResources;
class Scene;
class PhysicsEngine;
struct PhysicsEngineConfig;
class SetFps;
class BaseLog;
class DeleteNodeMutex;
class DeleteRigidBodyMutex;
class PhysicsLoop;
struct Beacon;
class DynamicWorld;

class PhysicsIteration {
    friend PhysicsLoop;
public:
    PhysicsIteration(
        SceneNodeResources& scene_node_resources,
        RenderingResources& rendering_resources,
        Scene& scene,
        DynamicWorld& dynamic_world,
        PhysicsEngine& physics_engine,
        DeleteNodeMutex& delete_node_mutex,
        const PhysicsEngineConfig& physics_cfg,
        BaseLog* base_log = nullptr);
    ~PhysicsIteration();
    void operator()(std::chrono::steady_clock::time_point time);
private:
    SceneNodeResources& scene_node_resources_;
    RenderingResources& rendering_resources_;
    Scene& scene_;
    DynamicWorld& dynamic_world_;
    PhysicsEngine& physics_engine_;
    DeleteNodeMutex& delete_node_mutex_;
    const PhysicsEngineConfig& physics_cfg_;
    BaseLog* base_log_;
    std::list<std::string> beacon_nodes_;
};

}
