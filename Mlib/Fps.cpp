#include "Fps.hpp"

static float fps_alpha = 0.01;

using namespace Mlib;

Fps::Fps()
: last_time_{std::chrono::steady_clock::now()},
  fps_{0}
{}

void Fps::tick() {
    std::chrono::time_point current_time = std::chrono::steady_clock::now();
    int64_t delta_time = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_time_).count();
    fps_ = (1 - fps_alpha) * fps_ + fps_alpha * (1e6 / delta_time);
    last_time_ = current_time;
}

float Fps::fps() const {
    return fps_;
}
