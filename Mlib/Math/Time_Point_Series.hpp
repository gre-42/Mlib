#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Positive_Modulo.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

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
        : last_{ SIZE_MAX }
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
        for (size_t i = 0; i < length; ++i) {
            size_t j0 = (size_t)positive_modulo((int)last_ - (int)i - 1, length);
            size_t j1 = (size_t)positive_modulo((int)last_ - (int)i, length);
            if (times_(j1) == std::chrono::steady_clock::time_point()) {
                verbose_abort("TimePointSeries::interpolator internal error (0)");
            }
            if (times_(j0) == std::chrono::steady_clock::time_point()) {
                return Interpolator{
                    .i0 = j1,
                    .i1 = j1,
                    .alpha = 0.f
                };
            }
            if (times_(j0) <= time) {
                if (times_(j1) <= time) {
                    return Interpolator{
                        .i0 = last_,
                        .i1 = last_,
                        .alpha = 0.f
                    };
                }
                auto d = times_(j1) - times_(j0);
                auto alpha = float(double((time - times_(j0)).count()) / double(d.count()));
                return Interpolator{
                    .i0 = j0,
                    .i1 = j1,
                    .alpha = alpha
                };
            } else {
                return Interpolator{
                    .i0 = j0,
                    .i1 = j0,
                    .alpha = 0.f
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
    std::chrono::steady_clock::time_point clamped(std::chrono::steady_clock::time_point time) const {
        auto interp = interpolator(time);
        return std::clamp(time, times_(interp.i0), times_(interp.i1));
    }
private:
    FixedArray<std::chrono::steady_clock::time_point, length> times_;
    size_t last_;
};

}
