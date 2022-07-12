#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

template<class TData>
struct RansacOptions;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

namespace Cv {

class FeaturePoint;

TransformationMatrix<float, float, 3> k_external(const FixedArray<float, 6>& kep);

FixedArray<float, 6> k_external_inverse(const TransformationMatrix<float, float, 3>& ke);

TransformationMatrix<float, float, 2> k_internal(const FixedArray<float, 4>& kip);

FixedArray<float, 4> pack_k_internal(const TransformationMatrix<float, float, 2>& ki);

enum class PointAtInfinityBehavior {
    THROW,
    IS_NAN
};

Array<FixedArray<float, 2>> projected_points(
    const Array<FixedArray<float, 3>>& x,
    const TransformationMatrix<float, float, 2>& ki,
    const Array<TransformationMatrix<float, float, 3>>& ke,
    PointAtInfinityBehavior point_at_infinity_behavior = PointAtInfinityBehavior::THROW);

Array<FixedArray<float, 2>> projected_points_1ke(
    const Array<FixedArray<float, 3>>& x,
    const TransformationMatrix<float, float, 2>& ki,
    const TransformationMatrix<float, float, 3>& ke,
    PointAtInfinityBehavior point_at_infinity_behavior = PointAtInfinityBehavior::THROW);

FixedArray<float, 2> projected_points_1p_1ke(
    const FixedArray<float, 3>& x,
    const TransformationMatrix<float, float, 2>& ki,
    const TransformationMatrix<float, float, 3>& ke,
    PointAtInfinityBehavior point_at_infinity_behavior = PointAtInfinityBehavior::THROW);

FixedArray<float, 2, 3> projected_points_jacobian_dx_1p_1ke(
    const FixedArray<float, 3>& x,
    const TransformationMatrix<float, float, 2>& ki,
    const TransformationMatrix<float, float, 3>& ke);

FixedArray<float, 2, 6> projected_points_jacobian_dke_1p_1ke(
    const FixedArray<float, 3>& x,
    const TransformationMatrix<float, float, 2>& ki,
    const FixedArray<float, 6>& kep);

FixedArray<float, 2, 4> projected_points_jacobian_dki_1p_1ke(
    const FixedArray<float, 3>& x,
    const TransformationMatrix<float, float, 3>& ke);

FixedArray<float, 3> reconstructed_point_(
    const Array<FixedArray<float, 2>>& y_tracked,
    const TransformationMatrix<float, float, 2>& ki,
    const Array<TransformationMatrix<float, float, 3>>& ke,
    const Array<float>* weights = nullptr,
    bool method2 = false,
    bool points_are_normalized=false,  // spares one matrix-inversion
    float *condition_number=nullptr,
    Array<float>* squared_distances = nullptr,
    Array<FixedArray<float, 2>>* projection_residual = nullptr);

FixedArray<float, 3> reconstructed_point_reweighted(
    const Array<FixedArray<float, 2>>& y_tracked,
    const TransformationMatrix<float, float, 2>& ki,
    const Array<TransformationMatrix<float, float, 3>>& ke);

}}
