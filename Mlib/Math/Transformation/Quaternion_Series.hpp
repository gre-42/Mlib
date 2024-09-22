#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Positive_Modulo.hpp>
#include <Mlib/Math/Time_Point_Series.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TDir, class TPos, size_t length>
class QuaternionSeries {
public:
    QuaternionSeries()
        : data_{ uninitialized }
    {}
    QuaternionSeries(
        const OffsetAndQuaternion<TDir, TPos>& qo,
        std::chrono::steady_clock::time_point time)
        : QuaternionSeries()
    {
        append(qo, time);
    }

    void clear() {
        static_assert(length > 0);
        times_.clear();
    }

    void append(const OffsetAndQuaternion<TDir, TPos>& qo, std::chrono::steady_clock::time_point time) {
        static_assert(length > 0);
        times_.append(time);
        data_(times_.last()) = qo;
    }

    OffsetAndQuaternion<TDir, TPos> get(std::chrono::steady_clock::time_point time) const {
        return get(times_.interpolator(time));
    }

    OffsetAndQuaternion<TDir, TPos> get(const Interpolator& interp) const {
        return data_(interp.i0).slerp(data_(interp.i1), interp.alpha);
    }

    std::chrono::steady_clock::time_point newest_time() const {
        return times_.newest_time();
    }

private:
    FixedArray<OffsetAndQuaternion<TDir, TPos>, length> data_;
    TimePointSeries<length> times_;
};

}
