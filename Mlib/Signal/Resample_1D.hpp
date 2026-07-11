#pragma once
#include <Mlib/Math/Interpolate.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Stats/Linspace.hpp>

namespace Mlib {

template <class TData, class TFloat=TData>
Array<TData> resample_1d(
    const Array<TData>& y,
    const TFloat& source_dt,
    const TFloat& dest_dt)
{
    if (y.ndim() != 1) {
        throw std::runtime_error("resample1d requires a 1D array");
    }
    if (y.length() == 0) {
        return zeros<TData>(ArrayShape{0});
    }
    auto y1 = integral_to_float<TFloat>(y.length() - 1);
    auto a = linspace<TFloat>(
        0,
        y1,
        float_to_integral<size_t>(std::round(y1 * source_dt / dest_dt + 1)));
    return interpolate(a, y);
}

}
