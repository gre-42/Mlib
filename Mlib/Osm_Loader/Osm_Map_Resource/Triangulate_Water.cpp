#include "Triangulate_Water.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Close_Neighbor_Detector.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Height_Contours.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Triangulate_Entity_List.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Water_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Way_Bvh.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>

using namespace Mlib;

void Mlib::find_coast_contours(
    std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& contours,
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& terrain_lists,
    const FixedArray<CompressedScenePos, 2>& water_heights)
{
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> terrain;
    for (const auto& triangles : terrain_lists) {
        terrain.emplace_back(triangles->triangle_array());
    }
    for (auto& c : height_contours(terrain, water_heights(0))) {
        if (compute_area_ccw(c, 1.) > 0) {
            contours.emplace_back(WaterType::UNDEFINED, WaterType::UNDEFINED, (CompressedScenePos)0.f, c);
        } else {
            c.reverse();
            contours.emplace_back(WaterType::UNDEFINED, WaterType::UNDEFINED, (CompressedScenePos)0.f, c);
        }
    }
    for (auto& c : height_contours(terrain, water_heights(1))) {
        if (compute_area_ccw(c, 1.) > 0) {
            contours.emplace_back(WaterType::SHALLOW_HOLE, WaterType::UNDEFINED, (CompressedScenePos)0.f, c);
        } else {
            c.reverse();
            contours.emplace_back(WaterType::SHALLOW_LAKE, WaterType::UNDEFINED, (CompressedScenePos)0.f, c);
        }
    }
}

void Mlib::add_water_steiner_points(
    std::list<SteinerPointInfo>& steiner_points,
    const std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& contours,
    const AxisAlignedBoundingBox<CompressedScenePos, 2>& bounds,
    const FixedArray<CompressedScenePos, 2>& cell_size,
    CompressedScenePos duplicate_distance,
    CompressedScenePos water_height)
{
    CloseNeighborDetector<CompressedScenePos, 2> close_neighbor_detector{{(CompressedScenePos)10., (CompressedScenePos)10.}, 10};
    for (const auto& s : steiner_points) {
        if (close_neighbor_detector.contains_neighbor(
            {s.position(0), s.position(1)},
            duplicate_distance,
            DuplicateRule::IS_NEIGHBOR))
        {
            THROW_OR_ABORT("Unexpected duplicate in steiner points");
        }
    }
    for (const auto& c : contours) {
        for (const auto& p : c.geometry) {
            close_neighbor_detector.contains_neighbor(
                p,
                duplicate_distance,
                DuplicateRule::IS_NEIGHBOR);
        }
    }
    for (CompressedScenePos x = bounds.min(0); x < bounds.max(0); x += cell_size(0)) {
        for (CompressedScenePos y = bounds.min(1); y < bounds.max(1); y += cell_size(1)) {
            if (!close_neighbor_detector.contains_neighbor(
                {x, y},
                duplicate_distance,
                DuplicateRule::IS_NEIGHBOR))
            {
                steiner_points.emplace_back(
                    FixedArray<CompressedScenePos, 3>{x, y, water_height},
                    SteinerPointType::WATER);
            }
        }
    }
}

void Mlib::triangulate_water(
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
    ContourDetectionStrategy contour_detection_strategy)
{
    triangulate_entity_list(
        tl_water,
        bounding_info,
        steiner_points,
        bounding_contour,
        hole_triangles,
        region_contours,
        scale,
        triangulation_scale,
        uv_scale,
        uv_period,
        z,
        color,
        contour_triangles_filename,
        contour_filename,
        triangle_filename,
        bounding_water_type,
        default_water_type,
        { WaterType::SHALLOW_HOLE, WaterType::STEEP_HOLE }, // excluded_entitities
        contour_detection_strategy,
        {});                                                // garden_margin
}

void Mlib::set_water_alpha(
    WaterTypeTriangleList& tl_water,
    const std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& region_contours,
    CompressedScenePos max_dist,
    CompressedScenePos coast_width)
{
    ScenePos steepness = 1. / (ScenePos)coast_width;
    WayBvh way_bvh;
    for (const auto& c : region_contours) {
        if (any(c.hole_type & WaterType::ANY_SHALLOW)) {
            way_bvh.add_path(c.geometry);
        }
    }
    for (auto& [_, x] : tl_water.map()) {
        if (!x->alpha.empty()) {
            THROW_OR_ABORT("Water already has alpha values");
        }
        for (auto& t : x->triangles) {
            auto& a = x->alpha.emplace_back(uninitialized);
            for (size_t i = 0; i < 3; ++i) {
                FixedArray<ScenePos, 2> dir = uninitialized;
                CompressedScenePos distance;
                if (way_bvh.nearest_way(
                    {t(i).position(0), t(i).position(1)},
                    max_dist,
                    dir,
                    distance))
                {
                    a(i) = (float)(steepness * (ScenePos)distance);
                } else {
                    a(i) = 1.f;
                }
            }
        }
    }
}
