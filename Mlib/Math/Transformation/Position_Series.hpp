#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Math/Positive_Modulo.hpp>
#include <Mlib/Math/Time_Point_Series.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TPos, size_t tndim, size_t length>
class PositionSeries {
public:
    PositionSeries()
        : data_{ uninitialized }
    {}
    PositionSeries(
        const FixedArray<TPos, tndim>& position,
        std::chrono::steady_clock::time_point time)
        : PositionSeries()
    {
        append(position, time);
    }

    void clear() {
        static_assert(length > 0);
        times_.clear();
    }

    void append(const FixedArray<TPos, tndim>& position, std::chrono::steady_clock::time_point time) {
        static_assert(length > 0);
        times_.append(time);
        data_(times_.last()) = position;
    }

    FixedArray<TPos, tndim> get(std::chrono::steady_clock::time_point time) const {
        return get(times_.interpolator(time));
    }

    FixedArray<TPos, tndim> get(const Interpolator& interp) const {
        return lerp(data_(interp.i0), data_(interp.i1), (TPos)interp.alpha);
    }

private:
    FixedArray<FixedArray<TPos, tndim>, length> data_;
    TimePointSeries<length> times_;
};

}
