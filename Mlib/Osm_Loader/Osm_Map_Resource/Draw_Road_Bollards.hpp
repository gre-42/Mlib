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

void draw_road_bollards(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const std::list<FixedArray<ColoredVertex<double>, 3>>& inner_triangles,
    const GroundBvh& ground_bvh,
    double scale,
    double tangential_distance,
    double normal_distance,
    double gradient_dx,
    double max_gradient);

}
