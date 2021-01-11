#pragma once
#include <chrono>

namespace Mlib {

class TimeGuard {
public:
    TimeGuard(const char* message, float min_time = 0);
    ~TimeGuard();
private:
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    const char* message_;
    float min_time_;
};

}
