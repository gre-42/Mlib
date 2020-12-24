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
    sim_time_ += std::chrono::microseconds(int64_t(dt * 1000 * 1000));
    std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
    int64_t residual_time = std::chrono::duration_cast<std::chrono::microseconds>(sim_time_ - current_time).count();
    int64_t min_residual_time = std::isnan(max_residual_time)
        ? 1
        : -std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<float>{max_residual_time}).count();
    if ((min_residual_time <= 0) && (residual_time < min_residual_time)) {
        sim_time_ = current_time;
        if (print_residual_time) {
            std::cerr << prefix_ << "resetting sim time" << std::endl;
        }
    } else {
        if (residual_time > 0) {
            // busy wait
            while(residual_time > 0) {
                current_time = std::chrono::steady_clock::now();
                residual_time = std::chrono::duration_cast<std::chrono::microseconds>(sim_time_ - current_time).count();
            }
            // std::this_thread::sleep_for(std::chrono::microseconds(residual_time));
        } else if (print_residual_time) {
            std::cerr << prefix_ << "residual time: " << residual_time << std::endl;
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