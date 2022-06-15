#pragma once
#include <list>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;
template <class TPos>
class TriangleList;

std::list<const FixedArray<ColoredVertex<double>, 3>*> get_triangles_around(
    const std::list<const FixedArray<ColoredVertex<double>, 3>*>& triangles,
    const FixedArray<double, 2>& pt,
    double radius);

std::list<const FixedArray<ColoredVertex<double>, 3>*> get_triangles_around(
    const std::list<FixedArray<ColoredVertex<double>, 3>>& triangles,
    const FixedArray<double, 2>& pt,
    double radius);

std::list<const FixedArray<ColoredVertex<double>, 3>*> get_triangles_around(
    const std::list<std::shared_ptr<TriangleList<double>>>& triangles,
    const FixedArray<double, 2>& pt,
    double radius);

}
