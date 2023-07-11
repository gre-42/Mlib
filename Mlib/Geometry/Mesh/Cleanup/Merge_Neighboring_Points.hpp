#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
class ColoredVertexArray;
template <class TData, class TPayload, size_t tndim>
class Bvh;

template <class TPos>
void merge_neighboring_points(
    ColoredVertexArray<TPos>& cva,
    Bvh<TPos, FixedArray<TPos, 3>, 3>& bvh,
    const TPos& max_distance);

}
