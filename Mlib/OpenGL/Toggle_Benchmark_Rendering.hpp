#pragma once
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/Time/Time_Guard.hpp>
#include <optional>

#define TIME_GUARD_INITIALIZE(a, b) \
    if (print_rendered_materials()) \
        TimeGuard::initialize(a, b)
#define TIME_GUARD_DECLARE(name, message, group)    \
    std::optional<TimeGuard> time_guard;            \
    if (print_rendered_materials())                 \
        time_guard.emplace(message, group)
