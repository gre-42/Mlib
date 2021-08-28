#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t n>
class TransformationMatrix;

Array<FixedArray<FixedArray<float, 3>, 3>> triangulate_3d(
    const Array<TransformationMatrix<float, 3>>& points,
    float boundary_radius,
    float z_thickness);

}
