#pragma once
#include <list>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
struct ColoredVertex;
template <typename TData, size_t... tshape>
class FixedArray;
class BatchResourceInstantiator;
class ResourceNameCycle;
class GroundBvh;
class StreetBvh;
struct WaysideDistances;

void draw_waysides(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const std::list<FixedArray<ColoredVertex<double>, 3>>& inner_triangles,
    const GroundBvh& ground_bvh,
    const StreetBvh& entrance_bvh,
    double scale,
    const WaysideDistances& distances);

}
