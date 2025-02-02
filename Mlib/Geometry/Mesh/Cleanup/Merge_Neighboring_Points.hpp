#pragma once
#include <Mlib/Geometry/Intersection/Bvh_Fwd.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
class ColoredVertexArray;

template <class TPos>
void merge_neighboring_points(
    ColoredVertexArray<TPos>& cva,
    PointWithoutPayloadVectorBvh<TPos, 3>& bvh,
    const TPos& max_distance);

}
