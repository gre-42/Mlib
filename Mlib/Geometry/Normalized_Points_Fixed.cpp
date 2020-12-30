#include "Normalized_Points_Fixed.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Quantile.hpp>

using namespace Mlib;

NormalizedPointsFixed::NormalizedPointsFixed(ScaleMode scale_mode, OffsetMode offset_mode)
: scale_mode_{scale_mode},
  offset_mode_{offset_mode}
{}

void NormalizedPointsFixed::add_point(const FixedArray<float, 2>& y)
{
    min_ = minimum(y, min_);
    max_ = maximum(y, max_);
}

void NormalizedPointsFixed::set_min(const FixedArray<float, 2>& p) {
    min_ = p;
}

void NormalizedPointsFixed::set_max(const FixedArray<float, 2>& p) {
    max_ = p;
}

FixedArray<float, 2, 3> NormalizedPointsFixed::normalization_matrix() const {
    // dx: difference
    // sx: sum
    FixedArray<float, 2> d = (offset_mode_ == OffsetMode::CENTERED) ? (max_ - min_) / 2.f : (max_ - min_);
    FixedArray<float, 2> s = (offset_mode_ == OffsetMode::CENTERED) ? (min_ + max_) / 2.f : min_;
    if (scale_mode_ == ScaleMode::PRESERVE_ASPECT_RATIO) {
        d = std::max(d(0), d(1));
    }
    if (scale_mode_ == ScaleMode::NONE) {
        d = 1;
    }
    return FixedArray<float, 2, 3>{
        1 / d(0), 0, -s(0) / d(0),  // (p - sx) / dx = p/dx - sx/dx
        0, 1 / d(1), -s(1) / d(1)}; // (p - sy) / dy = p/dy - sy/dy
}

NormalizedPointsFixed NormalizedPointsFixed::chained(ScaleMode scale_mode, OffsetMode offset_mode) const {
    NormalizedPointsFixed result{scale_mode, offset_mode};
    auto n = normalization_matrix();
    result.min_ = dot1d(n, homogenized_3(min_));
    result.max_ = dot1d(n, homogenized_3(max_));
    return result;
}
