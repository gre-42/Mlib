#pragma once
#include <atomic>
#include <mutex>
#include <thread>

namespace Mlib {

class SceneNodeResources;
class Scene;
class PhysicsEngine;
struct PhysicsEngineConfig;
class SetFps;
class BaseLog;

class PhysicsLoop {
public:
    PhysicsLoop(
        SceneNodeResources& scene_node_resources,
        Scene& scene,
        PhysicsEngine& physics_engine,
        std::recursive_mutex& mutex,
        const PhysicsEngineConfig& physics_cfg,
        SetFps& set_fps,
        size_t nframes = SIZE_MAX,
        BaseLog* base_log = nullptr);
    void stop_and_join();
    void join();
private:
    std::atomic_bool exit_physics_;
    SetFps& set_fps_;
    std::thread physics_thread_;

};

}
