#pragma once
#include <chrono>
#include <string>

namespace Mlib {

class LagFinder {
public:
    LagFinder(const std::string& prefix, const std::chrono::milliseconds& lag_duration);
    void start();
    void stop();
private:
    const std::string prefix_;
    std::chrono::milliseconds lag_duration_;
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point end_time_;
};

}
