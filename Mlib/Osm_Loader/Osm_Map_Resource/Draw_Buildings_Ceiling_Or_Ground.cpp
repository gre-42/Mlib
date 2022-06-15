#include "Draw_Buildings_Ceiling_Or_Ground.hpp"
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Building_Part_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Smooth_Building_Levels.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Triangulate_Terrain_Or_Ceilings.hpp>
#include <iostream>
#include <vector>

using namespace Mlib;

void Mlib::draw_buildings_ceiling_or_ground(
    std::list<std::shared_ptr<TriangleList<double>>>& tls,
    const Material& material,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    DrawBuildingPartType tpe)
{
    size_t mid = 0;
    for (const auto& bu : buildings) {
        if (bu.way.nd.empty()) {
            std::cerr << "Building " + bu.id + ": outline is empty" << std::endl;
            continue;
        }
        if (bu.way.nd.front() != bu.way.nd.back()) {
            throw std::runtime_error("Cannot draw ceiling or ground of building " + bu.id + ": outline not closed");
        }
        if ((tpe == DrawBuildingPartType::GROUND) &&
            bu.way.tags.contains("layer") &&
            (safe_stoi(bu.way.tags.at("layer")) != 0))
        {
            continue;
        }
        auto sw = smooth_building_level_outline(bu, nodes, scale, max_width, tpe);
        if (sw.empty()) {
            throw std::runtime_error("Smoothed outline is empty");
        }
        std::vector<FixedArray<double, 2>> outline{sw.begin(), sw.end()};
        outline = removed_duplicates(outline);
        tls.push_back(std::make_shared<TriangleList<double>>(
            "ceilings_" + std::to_string(mid++),
            material,
            PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE));
        TerrainTypeTriangleList tl_terrain;
        tl_terrain.insert(TerrainType::UNDEFINED, tls.back());
        BoundingInfo bounding_info{outline, {}, 0.1f};
        try {
            triangulate_terrain_or_ceilings(
                tl_terrain,                                                      // tl_terrain
                bounding_info,                                                   // bounding_info
                {},                                                              // steiner_points
                outline,                                                         // bounding_contour
                {},                                                              // hole_triangles
                {},                                                              // region_contours
                scale,                                                           // scale
                uv_scale,                                                        // uv_scale
                tpe == DrawBuildingPartType::CEILING ? bu.levels.back().top : 0, // z
                parse_color(bu.way.tags, "color", building_color),               // color
                "",                                                              // contour_filename
                "",                                                              // triangle_filename
                TerrainType::UNDEFINED,                                          // bounding_terrain_type
                TerrainType::UNDEFINED,                                          // default_terrain_type
                {});                                                             // excluded_terrain_types
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Could not triangulate building " + bu.id + ": " + e.what());
        }
    }
}
