#include "Physics_Loop.hpp"
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Iteration.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Threads/Thread_Initializer.hpp>
#include <Mlib/Time/Fps/Lag_Finder.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <vector>

using namespace Mlib;

PhysicsLoop::PhysicsLoop(
    const std::string& thread_name,
    ThreadAffinity thread_affinity,
    PhysicsIteration& physics_iteration,
    SetFps& set_fps,
    size_t nframes,
    const std::function<std::function<void()>(std::function<void()>)>& run_in_background)
: set_fps_{set_fps},
  physics_iteration_{physics_iteration},
  physics_thread_{run_in_background([this, tn=thread_name, thread_affinity, nframes](){
    try {
        ThreadInitializer ti{ tn, thread_affinity };
        SetDeleterThreadGuard set_deleter_thread_guard{ physics_iteration_.delete_node_mutex_ };
        size_t nframes2 = nframes;
        // PeriodicLagFinder lag_finder{ "Physics: ", std::chrono::milliseconds{ 100 }};
        while (!physics_thread_.get_stop_token().stop_requested()) {
            std::chrono::steady_clock::time_point simulated_time;
            if (!set_fps_.paused()) {
                // lag_finder.start();
                // TimeGuard::initialize(5 * 60);
                if (nframes2 != SIZE_MAX) {
                    if (nframes2-- == 0) {
                        break;
                    }
                }
                simulated_time = set_fps_.simulated_time();
                physics_iteration_(simulated_time);
                // lerr() << rb0->get_new_absolute_model_matrix();
                // TimeGuard tg2{"physics tick"};
            }
            set_fps_.tick(simulated_time);
            // TimeGuard::print_groups(lraw());
            // lag_finder.stop();
        }
    } catch (...) {
        add_unhandled_exception(std::current_exception());
    }
    })}
{}

PhysicsLoop::~PhysicsLoop() {
    stop_and_join();
}

void PhysicsLoop::stop_and_join() {
    physics_thread_.request_stop();
    set_fps_.request_stop();
    join();
}

void PhysicsLoop::join() {
    if (physics_thread_.joinable()) {
        physics_thread_.join();
    }
}
