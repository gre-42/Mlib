#include "Physics_Loop.hpp"
#include <Mlib/Os/Threads/Termination_Manager.hpp>
#include <Mlib/Os/Threads/Thread_Initializer.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Iteration.hpp>
#include <Mlib/Scene_Config/Physics_Engine_Config.hpp>
#include <Mlib/Time/Fps/Lag_Finder.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <Mlib/Time/Time_And_Pause.hpp>
#include <chrono>
#include <vector>

using namespace Mlib;

PhysicsLoop::PhysicsLoop(
    std::string thread_name,
    ThreadAffinity thread_affinity,
    PhysicsIteration& physics_iteration,
    std::function<bool()> level_loading,
    SetFps& set_fps,
    size_t nframes,
    const std::function<std::function<void()>(std::function<void()>)>& run_in_background)
    : set_fps_{set_fps}
    , physics_iteration_{physics_iteration}
    , physics_thread_{run_in_background(
        [this, tn=std::move(thread_name), thread_affinity, nframes, ll=std::move(level_loading)]()
        {
            try {
                ThreadInitializer ti{ tn, thread_affinity };
                size_t nframes2 = nframes;
                auto simulated_time = set_fps_.simulated_time();
                std::optional<PeriodicLagFinder> lag_finder;
                if (lag_finders_enabled()) {
                    lag_finder.emplace("Physics: ", std::chrono::milliseconds{ 50 });
                }
                while (!physics_thread_.get_stop_token().stop_requested() &&
                       !unhandled_exceptions_occured())
                {
                    if (lag_finder.has_value()) {
                        lag_finder->start();
                    }
                    auto loading = ll();
                    if (!set_fps_.paused() && !loading) {
                        // TimeGuard::initialize(5 * 60);
                        if (nframes2 != SIZE_MAX) {
                            if (nframes2-- == 0) {
                                break;
                            }
                        }
                        simulated_time = set_fps_.simulated_time();
                        physics_iteration_({simulated_time, PauseStatus::RUNNING});
                        // lerr() << rb0->get_new_absolute_model_matrix();
                        // TimeGuard tg2{"physics tick"};
                        set_fps_.tick(simulated_time);
                    } else {
                        physics_iteration_({simulated_time, loading ? PauseStatus::LOADING : PauseStatus::PAUSED});
                        set_fps_.sleep();
                    }
                    // TimeGuard::print_groups(lraw());
                    if (lag_finder.has_value()) {
                        lag_finder->stop();
                    }
                }
            } catch (const std::exception& e) {
                lerr() << "Unhandled exception in physics loop: " << e.what();
                add_unhandled_exception(std::current_exception());
            } catch (...) {
                lerr() << "Unknown unhandled exception in physics loop";
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
