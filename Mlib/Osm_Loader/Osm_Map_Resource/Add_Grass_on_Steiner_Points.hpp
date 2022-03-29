#pragma once
#include <cstddef>
#include <list>

namespace Mlib {

class BatchResourceInstantiator;
class ResourceNameCycle;
struct SteinerPointInfo;
template <typename TData, size_t... tshape>
class FixedArray;
class StreetBvh;

void add_grass_on_steiner_points(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const StreetBvh& ground_bvh,
    const StreetBvh& air_bvh,
    const std::list<SteinerPointInfo>& steiner_points,
    float scale,
    float dmin,
    float dmax);

}
