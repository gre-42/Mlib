#pragma once
#include <list>
#include <map>
#include <memory>
#include <vector>

namespace Mlib {

class TriangleList;
struct BoundingInfo;
struct ColoredVertex;
struct SteinerPointInfo;
template <typename TData, size_t... tshape>
class FixedArray;
enum class TerrainType;
enum class WaterType;
template <class EntityType>
class EntityTypeTriangleList;
typedef EntityTypeTriangleList<TerrainType> TerrainTypeTriangleList;
typedef EntityTypeTriangleList<WaterType> WaterTypeTriangleList;

void triangulate_terrain_or_ceilings(
    TerrainTypeTriangleList& tl_terrain,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::list<FixedArray<ColoredVertex, 3>>& hole_triangles,
    const std::list<std::pair<TerrainType, std::list<FixedArray<float, 3>>>>& region_contours,
    float scale,
    float uv_scale,
    float z,
    const FixedArray<float, 3>& color,
    const std::string& contour_filename,
    TerrainType default_terrain_type);

void triangulate_water(
    WaterTypeTriangleList& tl_water,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::list<FixedArray<ColoredVertex, 3>>& hole_triangles,
    const std::list<std::pair<WaterType, std::list<FixedArray<float, 3>>>>& region_contours,
    float scale,
    float uv_scale,
    float z,
    const FixedArray<float, 3>& color,
    const std::string& contour_filename,
    WaterType default_water_type);

}
