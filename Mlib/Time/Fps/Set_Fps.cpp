#include "Set_Fps.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/ISleeper.hpp>
#include <Mlib/Time/Sleep.hpp>
#include <thread>

using namespace Mlib;

SetFps::SetFps(
    ISleeper* sleeper,
    std::function<std::chrono::steady_clock::time_point()> simulated_time,
    std::function<bool()> paused,
    std::function<void()> on_tick)
    : stop_requested_{ false }
    , completed_time_()
    , simulated_time_{ std::move(simulated_time) }
    , paused_{ std::move(paused) }
    , on_tick_{ std::move(on_tick) }
    , sleeper_{ sleeper }
{}

SetFps::~SetFps() = default;

void SetFps::tick(std::chrono::steady_clock::time_point completed_time)
{
    completed_time_ = completed_time;
    if (sleeper_ != nullptr) {
        sleeper_->tick();
    }
    execute_oldest_funcs();
    if (paused() && !stop_requested_) {
        while (paused() && !stop_requested_) {
            if (on_tick_) {
                on_tick_();
            }
            while (execute_oldest_func());
            Mlib::sleep_for(std::chrono::microseconds(100));
        }
        if (sleeper_ != nullptr) {
            sleeper_->reset();
        }
    }
    if (on_tick_) {
        on_tick_();
    }
}

void SetFps::execute_oldest_funcs() {
    while (execute_oldest_func());
}

bool SetFps::execute_oldest_func() {
    std::function<void()> func;
    {
        std::scoped_lock lock{execute_mutex_};
        if (funcs_.empty()) {
            return false;
        }
        func = funcs_.front();
        funcs_.pop_front();
    }
    func();
    return true;
}

std::chrono::steady_clock::time_point SetFps::completed_time() const {
    return completed_time_;
}

std::chrono::steady_clock::time_point SetFps::simulated_time() const {
    if (!simulated_time_) {
        THROW_OR_ABORT("SetFps::simulated_time called but not set");
    }
    return simulated_time_();
}

bool SetFps::paused() const {
    if (!paused_) {
        THROW_OR_ABORT("SetFps::paused called but not set");
    }
    return paused_();
}

void SetFps::execute(const std::function<void()>& func) {
    std::scoped_lock lock{execute_mutex_};
    funcs_.push_back(func);
}

void SetFps::request_stop() {
    stop_requested_ = true;
}

bool SetFps::is_up_to_date() const {
    return (sleeper_ == nullptr) || sleeper_->is_up_to_date() || paused();
}
