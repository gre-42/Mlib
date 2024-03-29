#pragma once
#include <Mlib/Features.hpp>
#include <list>
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
class PhysicsLoop;
struct Beacon;
#ifndef WITHOUT_THREAD_LOCAL
thread_local extern std::list<Beacon> g_beacons;
#endif

class PhysicsIteration {
    friend PhysicsLoop;
public:
    PhysicsIteration(
        SceneNodeResources& scene_node_resources,
        Scene& scene,
        PhysicsEngine& physics_engine,
        DeleteNodeMutex& delete_node_mutex,
        const PhysicsEngineConfig& physics_cfg,
        BaseLog* base_log = nullptr);
    ~PhysicsIteration();
    void operator()(std::chrono::steady_clock::time_point time);
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
