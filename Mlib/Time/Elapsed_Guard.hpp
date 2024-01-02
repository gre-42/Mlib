#pragma once
#include <chrono>

namespace Mlib {

class ElapsedGuard {
public:
    ElapsedGuard();
    ~ElapsedGuard();
private:
    std::chrono::steady_clock::time_point start_time_;
};

}
