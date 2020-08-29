#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

template<class TData>
struct RansacOptions;

namespace Cv {

class FeaturePoint;

Array<float> k_external(const Array<float>& kep);

Array<float> k_external_inverse(const Array<float>& ke);

Array<float> k_internal(const Array<float>& kip);

Array<float> projected_points(
    const Array<float>& x,
    const Array<float>& ki,
    const Array<float>& ke,
    bool allow_points_at_infinity=false);

Array<float> projected_points_1ke(
    const Array<float>& x,
    const Array<float>& ki,
    const Array<float>& ke,
    bool allow_points_at_infinity=false);

Array<float> projected_points_1p_1ke(
    const Array<float>& x,
    const Array<float>& ki,
    const Array<float>& ke,
    bool allow_points_at_infinity=false);

Array<float> projected_points_jacobian_dx_1p_1ke(
    const Array<float>& x,
    const Array<float>& ki,
    const Array<float>& ke);

Array<float> projected_points_jacobian_dke_1p_1ke(
    const Array<float>& x,
    const Array<float>& ki,
    const Array<float>& kep);

Array<float> projected_points_jacobian_dki_1p_1ke(
    const Array<float>& x,
    const Array<float>& kip,
    const Array<float>& ke);

Array<float> reconstructed_point(
    const Array<float>& y_tracked,
    const Array<float>& ki,
    const Array<float>& ke,
    const Array<float>* weights = nullptr,
    Array<float>* fs = nullptr,
    bool method2 = false,
    bool points_are_normalized=false,  // spares one matrix-inversion
    float *condition_number=nullptr);

Array<float> reconstructed_point_reweighted(
    const Array<float>& y_tracked,
    const Array<float>& ki,
    const Array<float>& ke);

}}
