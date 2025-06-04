#include "Triangulate_Water.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Triangulate_Entity_List.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Water_Type.hpp>

using namespace Mlib;

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
        { WaterType::HOLE },            // excluded_entitities
        contour_detection_strategy,
        {});                            // garden_margin
}
