#pragma once
#include <Mlib/Scene_Precision.hpp>
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
enum class ContourDetectionStrategy;

void draw_waysides(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& inner_triangles,
    const GroundBvh& ground_bvh,
    const StreetBvh& entrance_bvh,
    double scale,
    const WaysideDistances& distances,
    ContourDetectionStrategy contour_detection_strategy);

}
