#pragma once
#include <chrono>

namespace Mlib {

class PeriodicReferenceTime {
public:
    explicit PeriodicReferenceTime(
        std::chrono::steady_clock::time_point reference,
        std::chrono::steady_clock::duration period_duration);
    float phase01(std::chrono::steady_clock::time_point time) const;
private:
    std::chrono::steady_clock::time_point reference_;
    std::chrono::steady_clock::duration period_duration_;
};

}
