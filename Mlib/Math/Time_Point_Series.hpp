#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Positive_Modulo.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>

namespace Mlib {

struct Interpolator {
    size_t i0;
    size_t i1;
    float alpha;
};

template <size_t length>
class TimePointSeries {
public:
    TimePointSeries()
        : times_(uninitialized)
        , last_{ SIZE_MAX }
    {}
    explicit TimePointSeries(
        std::chrono::steady_clock::time_point time)
        : TimePointSeries()
    {
        append(time);
    }

    void clear() {
        static_assert(length > 0);
        last_ = SIZE_MAX;
        times_(length - 1) = std::chrono::steady_clock::time_point();
    }

    void append(std::chrono::steady_clock::time_point time) {
        static_assert(length > 0);
        last_ = (last_ + 1) % length;
        times_(last_) = time;
    }

    Interpolator interpolator(std::chrono::steady_clock::time_point time) const {
        static_assert(length > 0);
        if (last_ == SIZE_MAX) {
            THROW_OR_ABORT("TimePointSeries::interpolator called on empty sequence");
        }
        if (time == std::chrono::steady_clock::time_point()) {
            THROW_OR_ABORT("TimePointSeries::interpolator received uninitialized time");
        }
        if (times_(last_) <= time) {
            return Interpolator{
                .i0 = last_,
                .i1 = last_,
                .alpha = 0.f
            };
        }
        for (size_t i = 0; i < length; ++i) {
            auto j0 = (size_t)positive_modulo((int)last_ - (int)i - 1, length);
            auto j1 = (size_t)positive_modulo((int)last_ - (int)i, length);
            if (times_(j1) == std::chrono::steady_clock::time_point()) {
                verbose_abort("TimePointSeries::interpolator internal error (0)");
            }
            // If we are at the end of the list, return the oldest element.
            if ((times_(j0) == std::chrono::steady_clock::time_point()) ||
                (i == (length - 1)))
            {
                return Interpolator{
                    .i0 = j1,
                    .i1 = j1,
                    .alpha = 0.f
                };
            }
            if (times_(j0) <= time) {
                auto d = times_(j1) - times_(j0);
                auto alpha = float(double((time - times_(j0)).count()) / double(d.count()));
                return Interpolator{
                    .i0 = j0,
                    .i1 = j1,
                    .alpha = alpha
                };
            }
        }
        verbose_abort("TimePointSeries::interpolator internal error (1)");
    }
    bool empty() const {
        return (last_ == SIZE_MAX);
    }
    size_t last() const {
        return last_;
    }
    std::chrono::steady_clock::time_point newest_time() const {
        return empty()
            ? std::chrono::steady_clock::time_point()
            : times_(last_);
    }
    std::chrono::steady_clock::time_point clamped(std::chrono::steady_clock::time_point time) const {
        static_assert(length > 0);
        if (last_ == SIZE_MAX) {
            THROW_OR_ABORT("TimePointSeries::clamped called on empty sequence");
        }
        auto min = times_(last_);
        auto max = times_(last_);
        for (size_t i = 1; i < length; ++i) {
            auto j = (size_t)positive_modulo((int)last_ - (int)i, length);
            if (times_(j) == std::chrono::steady_clock::time_point()) {
                break;
            }
            min = std::min(min, times_(j));
            max = std::max(max, times_(j));
        }
        return std::clamp(time, min, max);
    }
private:
    FixedArray<std::chrono::steady_clock::time_point, length> times_;
    size_t last_;
};

}
