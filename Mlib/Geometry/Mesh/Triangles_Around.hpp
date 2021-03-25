#pragma once
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct ColoredVertex;

std::list<const FixedArray<ColoredVertex, 3>*> get_triangles_around(
    const std::list<FixedArray<ColoredVertex, 3>>& triangles,
    const FixedArray<float, 2>& pt,
    float radius);

}
