#pragma once

// #define BENCHMARK_RENDERING_ENABLED

#ifdef BENCHMARK_RENDERING_ENABLED
    #include <Mlib/Time_Guard.hpp>
    #define TIME_GUARD_INITIALIZE(a, b) TimeGuard::initialize(a, b)
    #define TIME_GUARD_DECLARE(name, message, group) TimeGuard name{ message, group };
#else
    #define TIME_GUARD_INITIALIZE(a, b)
    #define TIME_GUARD_DECLARE(name, message, group)
#endif
