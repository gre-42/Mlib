#pragma once
#include <Mlib/Time/Sleep.hpp>

namespace Mlib {

template <class Clock, class Duration>
void busy_sleep_until(
    const std::chrono::time_point<Clock, Duration>& sleep_time,
    const Duration& busy_duration = std::chrono::milliseconds{ 5 })
{
    auto total_duration = sleep_time - std::chrono::steady_clock::now();
    auto sleep_duration = total_duration - busy_duration;
    if (sleep_duration.count() > 0) {
        sleep_for(sleep_duration);
    }
    while(std::chrono::steady_clock::now() < sleep_time);
}

}
