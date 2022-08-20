#include "Set_Fps.hpp"
#include <cmath>
#include <iostream>
#include <thread>

using namespace Mlib;

SetFps::SetFps(
    const std::string& prefix,
    const std::function<bool()>& paused)
: sim_time_{std::chrono::steady_clock::now()},
  prefix_{prefix},
  stop_requested_{false},
  paused_{paused}
{}

void SetFps::tick(
    float dt,
    float max_residual_time,
    bool control_fps,
    bool print_residual_time)
{
    sim_time_ += std::chrono::nanoseconds(int64_t(double(dt) * 1000 * 1000 * 1000));
    std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration residual_time = sim_time_ - current_time;
    if (!std::isnan(max_residual_time) && (std::chrono::duration<float>(residual_time).count() < -max_residual_time)) {
        sim_time_ = current_time;
        if (print_residual_time) {
            std::cerr << prefix_ << "resetting sim time" << std::endl;
        }
    } else {
        if (residual_time.count() > 0) {
            if (control_fps) {
                // busy wait
                while(residual_time.count() > 0) {
                    current_time = std::chrono::steady_clock::now();
                    residual_time = sim_time_ - current_time;
                }
                // std::this_thread::sleep_for(residual_time);
            }
        } else if (print_residual_time) {
            std::cerr <<
                prefix_ <<
                "residual time: " <<
                std::chrono::duration_cast<std::chrono::milliseconds>(-residual_time).count() <<
                " ms" << std::endl;
        }
    }
    if (!funcs_.empty()) {
        funcs_.front()();
        funcs_.pop_front();
    }
    if (paused() && !stop_requested_) {
        while (paused() && !stop_requested_) {
            if (!funcs_.empty()) {
                funcs_.front()();
                funcs_.pop_front();
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
        sim_time_ = std::chrono::steady_clock::now();
        if (print_residual_time) {
            std::cerr << prefix_ << "resetting sim time after pause" << std::endl;
        }
    }
}

bool SetFps::paused() const {
    return paused_();
}

void SetFps::execute(const std::function<void()>& func) {
    funcs_.push_back(func);
}

void SetFps::request_stop() {
    stop_requested_ = true;
}
