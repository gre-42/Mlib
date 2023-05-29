#pragma once
#include <chrono>

namespace Mlib {

class ElapsedGuard {
public:
    ElapsedGuard();
    ~ElapsedGuard();
private:
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
};

}
