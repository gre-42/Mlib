#include "Fixed_Time_Sleeper.hpp"
#include <chrono>
#include <thread>

using namespace Mlib;

FixedTimeSleeper::FixedTimeSleeper(float dt)
: dt_{dt}
{}

FixedTimeSleeper::~FixedTimeSleeper() = default;

void FixedTimeSleeper::tick() {
    auto end_time = std::chrono::steady_clock::now() + std::chrono::duration<float>(dt_);
    while (std::chrono::steady_clock::now() < end_time);
    // std::this_thread::sleep_for(std::chrono::duration<float>(dt_));
}

void FixedTimeSleeper::reset() {
    // Do nothing
}

bool FixedTimeSleeper::is_up_to_date() const {
    return true;
}
