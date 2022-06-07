#pragma once
#include <Mlib/Stats/Linspace.hpp>
#include <cstddef>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <class TData, size_t tndim>
FixedArray<TData, tndim> interpolate_default(
    const FixedArray<TData, tndim>& p0,
    const FixedArray<TData, tndim>& p1,
    TData a0,
    TData a1)
{
    return p0 * a0 + p1 * a1;
}

template <class TData, size_t tndim, class TInterpolator>
class InterpolatedIntermediatePointsCreator {
public:
    InterpolatedIntermediatePointsCreator(const TData& max_length, const TInterpolator& interpolate)
    : max_length_{ max_length },
      interpolate_{ interpolate }
    {}

    std::vector<FixedArray<TData, tndim>> operator () (
        const FixedArray<TData, tndim>& p0,
        const FixedArray<TData, tndim>& p1,
        const TData& distance) const
    {
        size_t npoints = 1 + (size_t)(distance / max_length_);
        if (npoints > 2) {
            std::vector<FixedArray<TData, tndim>> result;
            result.reserve(npoints - 2);
            for (size_t i = 1; i < npoints - 1; ++i) {
                std::pair<TData, TData> lm = linspace_multipliers<TData>(i, npoints);
                FixedArray<TData, tndim> pn = interpolate_(p0, p1, lm.first, lm.second);
                result.push_back(pn);
            }
            return result;
        } else {
            return {};
        }
    }
private:
    TData max_length_;
    TInterpolator interpolate_;
};

}
