#pragma once
#include <mutex>

namespace Mlib {

class SceneNodeResources;
class Scene;
class PhysicsEngine;
struct PhysicsEngineConfig;
class SetFps;
class BaseLog;

class PhysicsIteration {
public:
    PhysicsIteration(
        SceneNodeResources& scene_node_resources,
        Scene& scene,
        PhysicsEngine& physics_engine,
        std::recursive_mutex& mutex,
        const PhysicsEngineConfig& physics_cfg,
        BaseLog* base_log = nullptr);
    ~PhysicsIteration();
    void operator()();
private:
    SceneNodeResources& scene_node_resources_;
    Scene& scene_;
    PhysicsEngine& physics_engine_;
    std::recursive_mutex& mutex_;
    const PhysicsEngineConfig& physics_cfg_;
    BaseLog* base_log_;
};

}
