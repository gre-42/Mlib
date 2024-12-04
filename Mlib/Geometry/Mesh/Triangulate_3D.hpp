#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

Array<FixedArray<float, 3, 3>> triangulate_3d(
    const Array<TransformationMatrix<float, float, 3>>& points,
    float boundary_radius,
    float z_thickness,
    float cos_min_angle,
    float largest_cos_in_triangle = 0.9f,
    float triangle_search_eps = (float)1e-6);

}
