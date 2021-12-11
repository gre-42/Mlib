#pragma once
#include <mutex>

namespace Mlib {

class SceneNodeResources;
class Scene;
class PhysicsEngine;
struct PhysicsEngineConfig;
class SetFps;
class BaseLog;
class DeleteNodeMutex;
class DeleteRigidBodyMutex;

class PhysicsIteration {
public:
    PhysicsIteration(
        SceneNodeResources& scene_node_resources,
        Scene& scene,
        PhysicsEngine& physics_engine,
        DeleteNodeMutex& delete_node_mutex,
        const PhysicsEngineConfig& physics_cfg,
        BaseLog* base_log = nullptr);
    ~PhysicsIteration();
    void operator()();
    void delete_scheduled_advance_times();
private:
    SceneNodeResources& scene_node_resources_;
    Scene& scene_;
    PhysicsEngine& physics_engine_;
    DeleteNodeMutex& delete_node_mutex_;
    const PhysicsEngineConfig& physics_cfg_;
    BaseLog* base_log_;
};

}
