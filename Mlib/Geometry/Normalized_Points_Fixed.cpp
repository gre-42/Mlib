#include "Normalized_Points_Fixed.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
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

TransformationMatrix<float, 2> NormalizedPointsFixed::normalization_matrix() const {
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
    // (p - sx) / dx = p/dx - sx/dx
    // (p - sy) / dy = p/dy - sy/dy
    return TransformationMatrix<float, 2>{
        FixedArray<float, 2, 2>{
            1 / d(0), 0.f,
            0.f, 1 / d(1)},
        FixedArray<float, 2>{
            -s(0) / d(0),
            -s(1) / d(1)}};
}

NormalizedPointsFixed NormalizedPointsFixed::chained(ScaleMode scale_mode, OffsetMode offset_mode) const {
    NormalizedPointsFixed result{scale_mode, offset_mode};
    auto n = normalization_matrix();
    result.min_ = n.transform(min_);
    result.max_ = n.transform(max_);
    return result;
}
