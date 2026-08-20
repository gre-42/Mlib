#pragma once
#include <chrono>

// std::this_read::sleep_for has the following issues on Windows:
// 1. "system clock" is used internally
// 2. Times smaller or equal 0.01s result in 0s sleeps.


#ifdef _MSC_VER

#include <cstdint>

namespace Mlib {

void usleep(uint64_t usec);

template< class Rep, class Period >
void sleep_for(const std::chrono::duration<Rep, Period>& sleep_duration) {
    Mlib::usleep(std::chrono::duration_cast<std::chrono::microseconds>(sleep_duration).count());
}

}

#else

#include <thread>

namespace Mlib {

template< class Rep, class Period >
void sleep_for(const std::chrono::duration<Rep, Period>& sleep_duration) {
    std::this_thread::sleep_for(sleep_duration);
}

}

#endif

namespace Mlib {

template <class Clock, class Duration>
void sleep_until(const std::chrono::time_point<Clock, Duration>& end_time)
{
    auto total_duration = end_time - std::chrono::steady_clock::now();
    auto sleep_duration = total_duration;
    if (sleep_duration.count() > 0) {
        sleep_for(sleep_duration);
    }
}

}
