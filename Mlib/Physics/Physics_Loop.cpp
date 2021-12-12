#include "Physics_Loop.hpp"
#include <Mlib/Fps/Lag_Finder.hpp>
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Physics_Iteration.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Threads/Set_Thread_Name.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <vector>

using namespace Mlib;

PhysicsLoop::PhysicsLoop(
    PhysicsIteration& physics_iteration,
    const PhysicsEngineConfig& physics_cfg,
    SetFps& set_fps,
    size_t nframes,
    const std::function<std::function<void()>(std::function<void()>)>& run_in_background)
: exit_physics_{false},
  set_fps_{set_fps},
  physics_iteration_{physics_iteration},
  physics_thread_{run_in_background([&, nframes](){
    try {
        set_thread_name("Physics");
        SetDeleterThreadGuard set_deleter_thread_guard{ physics_iteration.delete_node_mutex_ };
        size_t nframes2 = nframes;
        // LagFinder lag_finder{ "Physics: ", std::chrono::milliseconds{ 100 }};
        while (!exit_physics_) {
            // lag_finder.start();
            // TimeGuard::initialize(5 * 60);
            if (nframes2 != SIZE_MAX) {
                if (nframes2-- == 0) {
                    break;
                }
            }
            physics_iteration();
            // std::cerr << rb0->get_new_absolute_model_matrix() << std::endl;
            // TimeGuard tg2{"physics tick"};
            set_fps.tick(physics_cfg.dt, physics_cfg.max_residual_time, physics_cfg.print_residual_time);
            // TimeGuard::print_groups(std::cerr);
            // lag_finder.stop();
        }
    } catch (const std::runtime_error&) {
        add_unhandled_exception(std::current_exception());
    }
    })}
{}

PhysicsLoop::~PhysicsLoop() {
    stop_and_join();
}

void PhysicsLoop::stop_and_join() {
    exit_physics_ = true;
    set_fps_.resume();
    join();
}

void PhysicsLoop::join() {
    if (physics_thread_.joinable()) {
        physics_thread_.join();
    }
}
