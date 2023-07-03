#pragma once
#include <cstddef>
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;

struct TerrainTriangles {
    const std::list<FixedArray<ColoredVertex<double>, 3>>* grass = nullptr;
    const std::list<FixedArray<ColoredVertex<double>, 3>>* elevated_grass = nullptr;
    const std::list<FixedArray<ColoredVertex<double>, 3>>* wayside1_grass = nullptr;
    const std::list<FixedArray<ColoredVertex<double>, 3>>* wayside2_grass = nullptr;
    const std::list<FixedArray<ColoredVertex<double>, 3>>* flowers = nullptr;
    const std::list<FixedArray<ColoredVertex<double>, 3>>* trees = nullptr;
};

}
