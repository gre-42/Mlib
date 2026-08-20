#pragma once
#include <Mlib/Time/Sleep.hpp>

namespace Mlib {

template <class Clock, class Duration>
void busy_sleep_until(
    const std::chrono::time_point<Clock, Duration>& sleep_time,
    const Duration& busy_duration = std::chrono::milliseconds{ 5 })
{
    sleep_until(sleep_time - busy_duration);
    while(std::chrono::steady_clock::now() < sleep_time);
}

}
