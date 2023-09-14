#pragma once
#include <chrono>
#include <string>

namespace Mlib {

class LagFinder {
public:
    LagFinder(std::string prefix, const std::chrono::milliseconds& lag_duration);
    void start();
    void stop();
private:
    std::string prefix_;
    std::chrono::milliseconds lag_duration_;
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point end_time_;
};

class GuardedLagFinder {
public:
    inline GuardedLagFinder(std::string prefix, const std::chrono::milliseconds& lag_duration)
    : lag_finder_{std::move(prefix), lag_duration}
    {
        lag_finder_.start();
    }
    ~GuardedLagFinder() {
        lag_finder_.stop();
    }
private:
    LagFinder lag_finder_;
};

}
