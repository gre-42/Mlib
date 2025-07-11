#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Mlib {

template <class TPos>
class TriangleList;
struct BoundingInfo;
template <class TPos>
struct ColoredVertex;
struct SteinerPointInfo;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t... tshape>
class OrderableFixedArray;
enum class TerrainType;
template <class EntityType>
class EntityTypeTriangleList;
using TerrainTypeTriangleList = EntityTypeTriangleList<TerrainType>;
enum class ContourDetectionStrategy;
template <class TRegionType, class TGeometry>
struct RegionWithMargin;

void triangulate_terrain_or_ceilings(
    TerrainTypeTriangleList& tl_terrain,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<CompressedScenePos, 2>>& bounding_contour,
    const std::list<RegionWithMargin<TerrainType, std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>>>& hole_triangles,
    const std::list<RegionWithMargin<TerrainType, std::list<FixedArray<CompressedScenePos, 2>>>>& region_contours,
    float scale,
    float triangulation_scale,
    float uv_scale,
    float uv_period,
    CompressedScenePos z,
    const FixedArray<float, 3>& color,
    const std::string& contour_triangles_filename,
    const std::string& contour_filename,
    const std::string& triangle_filename,
    TerrainType bounding_terrain_type,
    TerrainType default_terrain_type,
    const std::set<TerrainType>& excluded_terrain_types,
    ContourDetectionStrategy contour_detection_strategy,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, CompressedScenePos>& garden_margin);

}
