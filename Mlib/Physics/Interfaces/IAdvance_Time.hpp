#pragma once
#include <chrono>

namespace Mlib {

class IAdvanceTime {
public:
    virtual ~IAdvanceTime() = default;
    virtual void advance_time(float dt, std::chrono::steady_clock::time_point time) = 0;
};

}
