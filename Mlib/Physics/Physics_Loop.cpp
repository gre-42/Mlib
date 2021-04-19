#include "Physics_Loop.hpp"
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Physics_Iteration.hpp>
#include <Mlib/Set_Fps.hpp>
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
  idle_{false},
  set_fps_{set_fps},
  physics_iteration_{physics_iteration},
  physics_thread_{run_in_background([&, nframes](){
    try {
        set_thread_name("Physics");
        size_t nframes2 = nframes;
        while (!exit_physics_) {
            // TimeGuard::initialize(5 * 60);
            if (nframes2 != SIZE_MAX) {
                if (nframes2-- == 0) {
                    break;
                }
            }
            physics_iteration();
            // std::cerr << rb0->get_new_absolute_model_matrix() << std::endl;
            // TimeGuard tg2{"physics tick"};
            idle_ = true;
            set_fps.tick(physics_cfg.dt, physics_cfg.max_residual_time, physics_cfg.print_residual_time);
            idle_ = false;
            // TimeGuard::print_groups(std::cerr);
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

void PhysicsLoop::wait_until_paused_and_delete_scheduled_advance_times() {
    while (!(set_fps_.paused() && idle_)) {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
    physics_iteration_.delete_scheduled_advance_times();
}
