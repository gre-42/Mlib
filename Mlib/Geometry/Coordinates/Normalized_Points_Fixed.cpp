#include "Normalized_Points_Fixed.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Quantile.hpp>

using namespace Mlib;

template <class TData>
NormalizedPointsFixed<TData>::NormalizedPointsFixed(ScaleMode scale_mode, OffsetMode offset_mode)
: scale_mode_{scale_mode},
  offset_mode_{offset_mode}
{}

template <class TData>
void NormalizedPointsFixed<TData>::add_point(const FixedArray<TData, 2>& y)
{
    min_ = minimum(y, min_);
    max_ = maximum(y, max_);
}

template <class TData>
void NormalizedPointsFixed<TData>::set_min(const FixedArray<TData, 2>& p) {
    min_ = p;
}

template <class TData>
void NormalizedPointsFixed<TData>::set_max(const FixedArray<TData, 2>& p) {
    max_ = p;
}

template <class TData>
TransformationMatrix<TData, TData, 2> NormalizedPointsFixed<TData>::normalization_matrix() const {
    // dx: difference
    // sx: sum
    FixedArray<TData, 2> d = (offset_mode_ == OffsetMode::CENTERED) ? (max_ - min_) / TData(2) : (max_ - min_);
    FixedArray<TData, 2> s = (offset_mode_ == OffsetMode::CENTERED) ? (min_ + max_) / TData(2) : min_;
    if (scale_mode_ == ScaleMode::PRESERVE_ASPECT_RATIO) {
        d = std::max(d(0), d(1));
    }
    if (scale_mode_ == ScaleMode::NONE) {
        d = 1;
    }
    // (p - sx) / dx = p/dx - sx/dx
    // (p - sy) / dy = p/dy - sy/dy
    return TransformationMatrix<TData, TData, 2>{
        FixedArray<TData, 2, 2>::init(
            1 / d(0), (TData)0,
            TData(0), 1 / d(1)),
        FixedArray<TData, 2>::init(
            -s(0) / d(0),
            -s(1) / d(1))};
}

template <class TData>
NormalizedPointsFixed<TData> NormalizedPointsFixed<TData>::chained(ScaleMode scale_mode, OffsetMode offset_mode) const {
    NormalizedPointsFixed result{scale_mode, offset_mode};
    auto n = normalization_matrix();
    result.min_ = n.transform(min_);
    result.max_ = n.transform(max_);
    return result;
}

template <class TData>
const FixedArray<TData, 2>& NormalizedPointsFixed<TData>::min() const {
    return min_;
}

template <class TData>
const FixedArray<TData, 2>& NormalizedPointsFixed<TData>::max() const {
    return max_;
}

template class Mlib::NormalizedPointsFixed<float>;
template class Mlib::NormalizedPointsFixed<double>;
