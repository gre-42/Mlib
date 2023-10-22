#pragma once
#include <chrono>
#include <string>

namespace Mlib {

class PeriodicLagFinder {
public:
    PeriodicLagFinder(std::string prefix, const std::chrono::milliseconds& lag_duration);
    void start();
    void stop();
private:
    std::string prefix_;
    std::chrono::milliseconds lag_duration_;
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point end_time_;
};

class AperiodicLagFinder {
public:
    AperiodicLagFinder(std::string prefix, const std::chrono::milliseconds& lag_duration);
    ~AperiodicLagFinder();
private:
    std::string prefix_;
    std::chrono::milliseconds lag_duration_;
    std::chrono::steady_clock::time_point start_time_;
};

}
