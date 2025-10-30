#pragma once
#include <Mlib/Features.hpp>
#include <chrono>
#include <list>
#include <mutex>

namespace Mlib {

template <class T>
class VariableAndHash;
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
        std::function<void(std::chrono::steady_clock::time_point)> send_and_receive,
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
    std::function<void(std::chrono::steady_clock::time_point)> send_and_receive_;
    DeleteNodeMutex& delete_node_mutex_;
    const PhysicsEngineConfig& physics_cfg_;
    BaseLog* base_log_;
    std::list<VariableAndHash<std::string>> beacon_nodes_;
};

}
