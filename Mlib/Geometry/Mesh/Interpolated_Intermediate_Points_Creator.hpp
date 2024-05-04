#pragma once
#include <Mlib/Stats/Linspace.hpp>
#include <cstddef>
#include <vector>

namespace Mlib {

template <class TPoint>
TPoint interpolate_default(
    const TPoint& p0,
    const TPoint& p1,
    typename TPoint::value_type a0,
    typename TPoint::value_type a1)
{
    return p0 * a0 + p1 * a1;
}

template <class TPoint, class TInterpolator>
class InterpolatedIntermediatePointsCreator {
public:
    using TData = typename TPoint::value_type;
    InterpolatedIntermediatePointsCreator(const TData& max_length, const TInterpolator& interpolate)
    : max_length_{ max_length },
      interpolate_{ interpolate }
    {}

    std::vector<TPoint> operator () (
        const TPoint& p0,
        const TPoint& p1,
        const TData& distance) const
    {
        size_t npoints = 1 + (size_t)(distance / max_length_);
        if (npoints > 2) {
            std::vector<TPoint> result;
            result.reserve(npoints - 2);
            for (size_t i = 1; i < npoints - 1; ++i) {
                std::pair<TData, TData> lm = linspace_multipliers<TData>(i, npoints);
                TPoint pn = interpolate_(p0, p1, lm.first, lm.second);
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
