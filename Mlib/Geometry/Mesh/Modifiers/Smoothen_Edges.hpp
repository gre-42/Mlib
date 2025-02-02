#pragma once
#include <cstddef>
#include <list>
#include <memory>
#include <set>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
template <class TData, size_t... tshape>
class OrderableFixedArray;

template <class TPos>
void smoothen_edges(
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::set<OrderableFixedArray<TPos, 3>>& excluded_vertices,
    float smoothness,
    size_t niterations,
    float decay = 0.97f);

}
