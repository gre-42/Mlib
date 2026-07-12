#pragma once
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>

namespace Mlib {

enum class OutOfRangeBehavior2 {
    THROW,
    NAN_
};

template <class TData>
Array<TData> interpolate(
    const Array<TData>& a,
    const Array<TData>& y,
    OutOfRangeBehavior2 out_of_range_behavior)
{
    if ((y.length() == 0) && (a.length() != 0)) {
        throw std::runtime_error("Interpolation values empty");
    }
    return a.applied([&](const TData& x){
        if (std::isnan(x)) {
            return NAN;
        }
        if (x == TData(y.length() - 1)) {
            return y(y.length() - 1);
        }
        size_t left_id = float_to_integral<size_t>(std::floor(x));
        size_t right_id = left_id + 1;
        if ((right_id == y.length()) && (x <= TData(y.length() - 1))) {
            return y(left_id);
        } else if ((left_id < y.length()) && (right_id < y.length())) {
            TData h = x - (TData)left_id;
            return lerp(y(left_id), y(right_id), h);
        } else {
            if (out_of_range_behavior == OutOfRangeBehavior2::NAN_) {
                return NAN;
            } else {
                throw std::runtime_error(
                    (std::stringstream() << "Interpolation value out of range: x = " << x <<
                    ", range = 0.." << (y.length() - 1)).str());
            }
        }
    });
}

}
