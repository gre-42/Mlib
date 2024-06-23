#pragma once
#include <Mlib/Default_Uninitialized_List.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;

struct TerrainTriangles {
    const UUList<FixedArray<ColoredVertex<double>, 3>>* grass = nullptr;
    const UUList<FixedArray<ColoredVertex<double>, 3>>* elevated_grass = nullptr;
    const UUList<FixedArray<ColoredVertex<double>, 3>>* wayside1_grass = nullptr;
    const UUList<FixedArray<ColoredVertex<double>, 3>>* wayside2_grass = nullptr;
    const UUList<FixedArray<ColoredVertex<double>, 3>>* flowers = nullptr;
    const UUList<FixedArray<ColoredVertex<double>, 3>>* trees = nullptr;
    const UUList<FixedArray<ColoredVertex<double>, 3>>* street_mud_grass = nullptr;
    const UUList<FixedArray<ColoredVertex<double>, 3>>* path_mud_grass = nullptr;
};

}
