#pragma once

// std::this_read::sleep_for has the following issues on Windows:
// 1. "system clock" is used internally
// 2. Times smaller or equal 0.01s result in 0s sleeps.

#include <chrono>

#ifdef _MSC_VER

#include <cstdint>

namespace Mlib {

void usleep(uint64_t usec);

template< class Rep, class Period >
void sleep_for( const std::chrono::duration<Rep, Period>& sleep_duration ) {
    Mlib::usleep(std::chrono::duration_cast<std::chrono::microseconds>(sleep_duration).count());
}

}

#else

#include <thread>

namespace Mlib {

template< class Rep, class Period >
void sleep_for( const std::chrono::duration<Rep, Period>& sleep_duration ) {
    std::this_thread::sleep_for(sleep_duration);
}

}

#endif
