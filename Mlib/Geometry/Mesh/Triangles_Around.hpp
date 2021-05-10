#pragma once
#include <list>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct ColoredVertex;
class TriangleList;

std::list<const FixedArray<ColoredVertex, 3>*> get_triangles_around(
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    const FixedArray<float, 2>& pt,
    float radius);

std::list<const FixedArray<ColoredVertex, 3>*> get_triangles_around(
    const std::list<FixedArray<ColoredVertex, 3>>& triangles,
    const FixedArray<float, 2>& pt,
    float radius);

std::list<const FixedArray<ColoredVertex, 3>*> get_triangles_around(
    const std::list<std::shared_ptr<TriangleList>>& triangles,
    const FixedArray<float, 2>& pt,
    float radius);

}
