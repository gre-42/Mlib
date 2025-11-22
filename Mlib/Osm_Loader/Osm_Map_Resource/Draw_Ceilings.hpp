#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class TriangleList;
struct Node;
struct Building;
struct OsmResourceConfig;
enum class ContourDetectionStrategy;
template <class TData, size_t... tshape>
class OrderableFixedArray;
template <typename TData, size_t... tshape>
class FixedArray;
struct Morphology;

void draw_ceilings(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_buildings,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 3>>& displacements,
    const OsmResourceConfig& config,
    const std::list<Building>& buildings,
    const Morphology& morphology,
    const std::map<std::string, Node>& nodes,
    const std::string& contour_triangles_filename,
    const std::string& contour_filename,
    const std::string& triangle_filename,
    ContourDetectionStrategy contour_detection_strategy);

}
