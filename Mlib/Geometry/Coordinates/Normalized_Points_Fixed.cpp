#include "Normalized_Points_Fixed.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Quantile.hpp>

using namespace Mlib;

template <class TData>
NormalizedPointsFixed<TData>::NormalizedPointsFixed(ScaleMode scale_mode, OffsetMode offset_mode)
    : aabb_{AxisAlignedBoundingBox<TData, 2>::empty()}
    , scale_mode_{scale_mode}
    , offset_mode_{offset_mode}
{}

template <class TData>
void NormalizedPointsFixed<TData>::add_point(const FixedArray<TData, 2>& y)
{
    aabb_.extend(y);
}

template <class TData>
void NormalizedPointsFixed<TData>::set_min(const FixedArray<TData, 2>& p) {
    aabb_.min = p;
}

template <class TData>
void NormalizedPointsFixed<TData>::set_max(const FixedArray<TData, 2>& p) {
    aabb_.max = p;
}

template <class TData>
TransformationMatrix<TData, TData, 2> NormalizedPointsFixed<TData>::normalization_matrix() const {
    const auto& min = aabb_.min;
    const auto& max = aabb_.max;
    // dx: difference
    // sx: sum
    FixedArray<TData, 2> d = (offset_mode_ == OffsetMode::CENTERED) ? (max - min) / TData(2) : (max - min);
    FixedArray<TData, 2> s = (offset_mode_ == OffsetMode::CENTERED) ? (min + max) / TData(2) : min;
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
    result.aabb_ = aabb_.transformed(n);
    return result;
}

template <class TData>
const FixedArray<TData, 2>& NormalizedPointsFixed<TData>::min() const {
    return aabb_.min;
}

template <class TData>
const FixedArray<TData, 2>& NormalizedPointsFixed<TData>::max() const {
    return aabb_.max;
}

template <class TData>
const AxisAlignedBoundingBox<TData, 2>& NormalizedPointsFixed<TData>::aabb() const {
    return aabb_;
}

template class Mlib::NormalizedPointsFixed<float>;
template class Mlib::NormalizedPointsFixed<double>;
