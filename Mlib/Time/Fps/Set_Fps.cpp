#include "Set_Fps.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Time/Fps/ISleeper.hpp>
#include <Mlib/Time/Sleep.hpp>
#include <thread>

using namespace Mlib;

SetFps::SetFps(
    ISleeper& sleeper,
    const std::function<bool()>& paused)
: stop_requested_{false},
  paused_{paused},
  sleeper_{sleeper}
{}

SetFps::~SetFps() = default;

void SetFps::tick()
{
    sleeper_.tick();
    execute_oldest_funcs();
    if (paused() && !stop_requested_) {
        while (paused() && !stop_requested_) {
            while (execute_oldest_func());
            Mlib::sleep_for(std::chrono::microseconds(100));
        }
        sleeper_.reset();
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

bool SetFps::paused() const {
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
    return sleeper_.is_up_to_date() || paused();
}
