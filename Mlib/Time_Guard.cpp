#include "Time_Guard.hpp"
#include <iostream>

using namespace Mlib;

TimeGuard::TimeGuard(const char* message, float min_time)
: start_time_{std::chrono::steady_clock::now()},
  message_{message},
  min_time_{min_time}
{}

TimeGuard::~TimeGuard() {
    std::chrono::time_point current_time = std::chrono::steady_clock::now();
    float delta_time = 1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time_).count();
    if (delta_time >= min_time_) {
        std::cerr << message_ << ", time: " << delta_time << std::endl;
    }
}
