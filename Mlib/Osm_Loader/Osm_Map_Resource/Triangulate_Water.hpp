#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <list>
#include <map>
#include <memory>

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
enum class WaterType;
template <class EntityType>
class EntityTypeTriangleList;
using WaterTypeTriangleList = EntityTypeTriangleList<WaterType>;
enum class ContourDetectionStrategy;
template <class TRegionType, class TGeometry>
struct RegionWithMargin;
template <class TData, size_t... tshape>
class OrderableFixedArray;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
class HeterogeneousResource;

void find_coast_contours(
    std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& contours,
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& terrain_lists,
    CompressedScenePos water_height);

void add_water_steiner_points(
    std::list<SteinerPointInfo>& steiner_points,
    const std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& contours,
    const AxisAlignedBoundingBox<CompressedScenePos, 2>& bounds,
    const FixedArray<CompressedScenePos, 2>& cell_size,
    CompressedScenePos duplicate_distance,
    CompressedScenePos water_height);

void triangulate_water(
    WaterTypeTriangleList& tl_water,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<CompressedScenePos, 2>>& bounding_contour,
    const std::list<RegionWithMargin<WaterType, std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>>>& hole_triangles,
    const std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& region_contours,
    float scale,
    float triangulation_scale,
    float uv_scale,
    float uv_period,
    CompressedScenePos z,
    const FixedArray<float, 3>& color,
    const std::string& contour_triangles_filename,
    const std::string& contour_filename,
    const std::string& triangle_filename,
    WaterType bounding_water_type,
    WaterType default_water_type,
    ContourDetectionStrategy contour_detection_strategy);

void set_water_alpha(
    WaterTypeTriangleList& tl_water,
    const std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& region_contours,
    CompressedScenePos max_dist,
    CompressedScenePos coast_width);

}
