#include "Draw_Into_Street_Rectangles.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle_3D.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Rectangle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Styled_Road.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

void Mlib::draw_into_street_rectangles(
    RoadPropertiesTriangleList& tl_street,
    std::list<StreetRectangle>& street_rectangles,
    SceneNodeResources& scene_node_resources,
    float height,
    float scale)
{
    RoadPropertiesTriangleList result;
    for (const auto& tl : tl_street.list()) {
        result.append(StyledRoadEntry{
            .road_properties = tl.road_properties,
            .styled_road = StyledRoad{
                .triangle_list = std::make_shared<TriangleList<double>>(
                    tl.styled_road.triangle_list->name,
                    tl.styled_road.triangle_list->material,
                    tl.styled_road.triangle_list->physics_material),
                .uvx = tl.styled_road.uvx}});
    }
    for (const auto& r : street_rectangles) {
        if (r.bumps_model.empty()) {
            continue;
        }
        auto& tl_str = result[r.road_properties];
        OsmRectangle3D rect{
            .p00_ = r.rectangle(0u, 0u),
            .p01_ = r.rectangle(0u, 1u),
            .p10_ = r.rectangle(1u, 0u),
            .p11_ = r.rectangle(1u, 1u)};
        const auto& cvas = scene_node_resources.get_physics_arrays(r.bumps_model)->scvas;
        for (const auto& cva : cvas) {
            if (cva->name != "street") {
                THROW_OR_ABORT("Material name is not \"street\" in resource \"" + r.bumps_model + '"');
            }
            rect.draw(*tl_str.triangle_list, cva->triangles, scale, 1.f, height, 0.f, 1.f);
        }
    }
    tl_street = std::move(result);
}
