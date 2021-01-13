#include "Fps.hpp"
#include <cmath>

static float fps_alpha = 0.01;

using namespace Mlib;

Fps::Fps()
: last_time_{std::chrono::steady_clock::now()},
  mean_fps_{0},
  mad_fps_{0}
{}

void Fps::tick() {
    std::chrono::time_point current_time = std::chrono::steady_clock::now();
    int64_t delta_time = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_time_).count();
    float current_fps = (1e6 / delta_time);
    mean_fps_ = (1 - fps_alpha) * mean_fps_ + fps_alpha * current_fps;
    mad_fps_ = (1 - fps_alpha) * mad_fps_ + fps_alpha * std::abs(current_fps - mean_fps_);
    last_time_ = current_time;
}

float Fps::mean_fps() const {
    return mean_fps_;
}

float Fps::mad_fps() const {
    return mad_fps_;
}
