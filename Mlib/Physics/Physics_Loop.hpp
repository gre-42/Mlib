#pragma once
#include <atomic>
#include <functional>
#include <thread>

namespace Mlib {

class PhysicsIteration;
struct PhysicsEngineConfig;
class SetFps;

class PhysicsLoop {
public:
    PhysicsLoop(
        PhysicsIteration& physics_iteration,
        const PhysicsEngineConfig& physics_cfg,
        SetFps& set_fps,
        size_t nframes = SIZE_MAX,
        const std::function<std::function<void()>(std::function<void()>)>& run_in_background = [](std::function<void()> f){return f;});
    ~PhysicsLoop();
    void stop_and_join();
    //! Useful if nframes != SIZE_MAX
    void join();
private:
    SetFps& set_fps_;
    PhysicsIteration& physics_iteration_;
    std::jthread physics_thread_;
};

}
