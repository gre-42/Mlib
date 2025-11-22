#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class TriangleList;
struct Material;
struct Morphology;
struct Node;
struct Building;
enum class DrawBuildingPartType;
enum class ContourDetectionStrategy;
template <class TData, size_t... tshape>
class OrderableFixedArray;
template <typename TData, size_t... tshape>
class FixedArray;

void draw_buildings_ceiling_or_ground(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 3>>* displacements,
    const Material& material,
    const Morphology& morphology,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float triangulation_scale,
    float uv_scale,
    float uv_period,
    float max_width,
    DrawBuildingPartType tpe,
    const std::string& contour_triangles_filename,
    const std::string& contour_filename,
    const std::string& triangle_filename,
    ContourDetectionStrategy contour_detection_strategy);

}
