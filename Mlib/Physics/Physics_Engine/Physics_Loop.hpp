#pragma once
#include <Mlib/Threads/J_Thread.hpp>
#include <atomic>
#include <functional>
#include <string>
#include <thread>

namespace Mlib {

enum class ThreadAffinity;
class PhysicsIteration;
class SetFps;

class PhysicsLoop {
public:
    PhysicsLoop(
        const std::string& thread_name,
        ThreadAffinity thread_affinity,
        PhysicsIteration& physics_iteration,
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
    JThread physics_thread_;
};

}
