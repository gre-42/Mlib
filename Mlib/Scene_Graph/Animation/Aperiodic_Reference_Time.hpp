#pragma once
#include <chrono>

namespace Mlib {

class AperiodicReferenceTime {
public:
    explicit AperiodicReferenceTime(
        std::chrono::steady_clock::time_point reference,
        std::chrono::steady_clock::duration duration);
    float phase01(std::chrono::steady_clock::time_point time) const;
    bool ran_to_completion(std::chrono::steady_clock::time_point time) const;
private:
    std::chrono::steady_clock::time_point reference_;
    std::chrono::steady_clock::duration duration_;
};

}
