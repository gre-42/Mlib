#include "Fps.hpp"
#include <cmath>

static float fps_alpha = 0.01f;

using namespace Mlib;

Fps::Fps()
: last_time_{std::chrono::steady_clock::now()},
  last_fps_{NAN},
  mean_fps_{0},
  mad_fps_{0}
{}

void Fps::tick() {
    std::chrono::time_point current_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> delta_time = (current_time - last_time_);
    last_fps_ = 1 / (float)delta_time.count();
    mean_fps_ = (1 - fps_alpha) * mean_fps_ + fps_alpha * last_fps_;
    mad_fps_ = (1 - fps_alpha) * mad_fps_ + fps_alpha * std::abs(last_fps_ - mean_fps_);
    last_time_ = current_time;
}

float Fps::last_fps() const {
    return last_fps_;
}

float Fps::mean_fps() const {
    return mean_fps_;
}

float Fps::mad_fps() const {
    return mad_fps_;
}
