#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <map>
#include <string>

namespace Mlib {

class ResourceNameCycle;
class BatchResourceInstantiator;
class StreetBvh;
class GroundBvh;
struct BoundingInfo;
template <class TData>
class Array;

void add_trees_to_zonemap(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const BoundingInfo& bounding_info,
    double min_dist_to_road,
    const StreetBvh& street_bvh,
    const GroundBvh& ground_bvh,
    const Array<double>& tree_density,
    double tree_density_width,
    double tree_density_height,
    double tree_density_multiplier,
    float jitter,
    double step_size,
    double position_scale,
    CompressedScenePos min_height);

}
