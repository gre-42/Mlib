#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
struct ColoredVertex;
template <typename TData, size_t... tshape>
class FixedArray;
class BatchResourceInstantiator;
class GroundBvh;
class StreetBvh;
struct WaysideResourceNamesSurface;
enum class ContourDetectionStrategy;

void draw_waysides(
    BatchResourceInstantiator& bri,
    const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& inner_triangles,
    const GroundBvh& ground_bvh,
    const StreetBvh& entrance_bvh,
    double scale,
    const WaysideResourceNamesSurface& distances,
    ContourDetectionStrategy contour_detection_strategy);

}
