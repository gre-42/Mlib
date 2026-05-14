#include "Triangulate_Terrain_Or_Ceilings.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Triangulate_Entity_List.hpp>
#include <Mlib/Scene_Graph/Resources/Sampler/Triangle_Sampler/Terrain_Type.hpp>

using namespace Mlib;

void Mlib::triangulate_terrain_or_ceilings(
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
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, CompressedScenePos>& garden_margin)
{
    triangulate_entity_list(
        tl_terrain,
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
        bounding_terrain_type,
        default_terrain_type,
        excluded_terrain_types,
        contour_detection_strategy,
        garden_margin);
}
