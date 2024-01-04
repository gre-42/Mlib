#include "Fixed_Time_Sleeper.hpp"
#include <Mlib/Time/Sleep.hpp>
#include <chrono>
#include <thread>

using namespace Mlib;

FixedTimeSleeper::FixedTimeSleeper(float dt)
: dt_{dt}
{}

FixedTimeSleeper::~FixedTimeSleeper() = default;

void FixedTimeSleeper::tick() {
    std::chrono::steady_clock::time_point end_time =
        std::chrono::steady_clock::now() +
        std::chrono::nanoseconds((uint64_t)(dt_ * 1000.f * 1000.f * 1000.f));
    while (std::chrono::steady_clock::now() < end_time);
    // Mlib::sleep_for(std::chrono::duration<float>(dt_));
}

void FixedTimeSleeper::reset() {
    // Do nothing
}

bool FixedTimeSleeper::is_up_to_date() const {
    return true;
}
