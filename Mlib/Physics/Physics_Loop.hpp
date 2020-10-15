#pragma once
#include <atomic>
#include <shared_mutex>
#include <thread>

namespace Mlib {

class SceneNodeResources;
class Scene;
class PhysicsEngine;
struct PhysicsEngineConfig;
class SetFps;

class PhysicsLoop {
public:
    PhysicsLoop(
        SceneNodeResources& scene_node_resources,
        Scene& scene,
        PhysicsEngine& physics_engine,
        std::shared_mutex& mutex,
        const PhysicsEngineConfig& physics_cfg,
        SetFps& set_fps,
        size_t nframes = SIZE_MAX);
    void stop_and_join();
    void join();
private:
    std::atomic_bool exit_physics_;
    SetFps& set_fps_;
    std::thread physics_thread_;

};

}
