#include "Draw_Buildings_Ceiling_Or_Ground.hpp"
#include <Mlib/Geometry/Base_Materials.hpp>
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
#include <Mlib/Osm_Loader/Osm_Map_Resource/Triangulate_Terrain_Or_Ceilings.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Type.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>
#include <vector>

using namespace Mlib;

void Mlib::draw_buildings_ceiling_or_ground(
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
    ContourDetectionStrategy contour_detection_strategy)
{
    size_t mid = 0;
    for (const auto& bu : buildings) {
        if (bu.way.nd.empty()) {
            lerr() << "Building " + bu.id + ": outline is empty";
            continue;
        }
        if (bu.way.nd.front() != bu.way.nd.back()) {
            THROW_OR_ABORT("Cannot draw ceiling or ground of building " + bu.id + ": outline not closed");
        }
        if ((tpe == DrawBuildingPartType::GROUND) &&
            bu.way.tags.contains("layer") &&
            (safe_stoi(bu.way.tags.at("layer")) != 0))
        {
            continue;
        }
        auto sw = smooth_building_level_outline(bu, nodes, scale, max_width, tpe);
        if (sw.outline.empty()) {
            THROW_OR_ABORT("Smoothed outline is empty");
        }
        auto gz = (CompressedScenePos)0.f;
        if (tpe == DrawBuildingPartType::CEILING) {
            if (displacements == nullptr) {
                THROW_OR_ABORT("Ceiling requires displacements");
            }
            auto max_height = std::numeric_limits<CompressedScenePos>::lowest();
            if (displacements != nullptr) {
                for (const auto& v : sw.outline) {
                    auto it = displacements->find(OrderableFixedArray{v.orig});
                    if (it == displacements->end()) {
                        lerr() << "Building " + bu.id + ": could not determine displacement";
                        max_height = std::numeric_limits<CompressedScenePos>::lowest();
                        break;
                    }
                    max_height = std::max(max_height, it->second(2));
                }
                if (max_height == std::numeric_limits<CompressedScenePos>::lowest()) {
                    continue;
                }
            }
            gz = max_height;
        }
        UUVector<FixedArray<CompressedScenePos, 2>> outline;
        outline.reserve(sw.outline.size());
        for (const auto& v : sw.outline) {
            outline.emplace_back(v.indented);
        }
        outline = removed_duplicates(outline);
        tls.push_back(std::make_shared<TriangleList<CompressedScenePos>>(
            "ceilings_" + std::to_string(mid++),
            material,
            morphology + BASE_VISIBLE_TERRAIN_MATERIAL));
        TerrainTypeTriangleList tl_terrain;
        tl_terrain.insert(TerrainType::UNDEFINED, tls.back());
        BoundingInfo bounding_info{ outline, {}, (CompressedScenePos)100.f };
        try {
            triangulate_terrain_or_ceilings(
                tl_terrain,                                                      // tl_terrain
                bounding_info,                                                   // bounding_info
                {},                                                              // steiner_points
                outline,                                                         // bounding_contour
                {},                                                              // hole_triangles
                {},                                                              // region_contours
                scale,                                                           // scale
                triangulation_scale,                                             // triangulation_scale
                uv_scale,                                                        // uv_scale
                uv_period,                                                       // uv_period
                gz + sw.z,                                                       // z
                parse_color(bu.way.tags, "color", building_color),               // color
                contour_triangles_filename,                                      // contour_triangles_filename
                contour_filename,                                                // contour_filename
                triangle_filename,                                               // triangle_filename
                TerrainType::UNDEFINED,                                          // bounding_terrain_type
                TerrainType::UNDEFINED,                                          // default_terrain_type
                {},                                                              // excluded_terrain_types
                contour_detection_strategy);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Could not triangulate building " + bu.id + ": " + e.what());
        }
    }
}
