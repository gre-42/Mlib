#include "Set_Fps.hpp"
#include <cmath>
#include <iostream>
#include <thread>

using namespace Mlib;

SetFps::SetFps(const std::string& prefix)
: sim_time_{std::chrono::steady_clock::now()},
  paused_{false},
  prefix_{prefix}
{}

void SetFps::tick(float dt, float max_residual_time, bool print_residual_time) {
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
            // busy wait
            while(residual_time.count() > 0) {
                current_time = std::chrono::steady_clock::now();
                residual_time = sim_time_ - current_time;
            }
            // std::this_thread::sleep_for(std::chrono::microseconds(residual_time));
        } else if (print_residual_time) {
            std::cerr <<
                prefix_ <<
                "residual time: " <<
                std::chrono::duration_cast<std::chrono::milliseconds>(-residual_time).count() <<
                " ms" << std::endl;
        }
    }
    if (paused()) {
        while (paused()) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        sim_time_ = std::chrono::steady_clock::now();
        if (print_residual_time) {
            std::cerr << prefix_ << "resetting sim time after pause" << std::endl;
        }
    }
}

void SetFps::toggle_pause_resume() {
    if (paused_) {
        resume();
    } else {
        pause();
    }
}

void SetFps::pause() {
    paused_ = true;
}

void SetFps::resume() {
    paused_ = false;
}

bool SetFps::paused() const {
    return paused_;
}