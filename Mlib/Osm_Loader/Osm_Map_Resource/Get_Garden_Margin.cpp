#include "Get_Garden_Margin.hpp"
#include <Mlib/Math/Blend.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Building_Part_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Smooth_Building_Levels.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>

using namespace Mlib;

std::map<OrderableFixedArray<CompressedScenePos, 2>, CompressedScenePos> Mlib::get_garden_margin(
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    double scale,
    double max_length)
{
    std::map<OrderableFixedArray<CompressedScenePos, 2>, CompressedScenePos> result;
    for (const auto& bu : buildings) {
        auto outline = smooth_building_level_outline(
            bu,
            nodes,
            scale,
            max_length,
            DrawBuildingPartType::GROUND,
            BuildingDetailType::COMBINED);
        
        for (const auto& v : outline.outline) {
            if (v.orig.a0 == 1) {
                auto it = v.orig.n0.tags.find("garden_margin");
                if (it != v.orig.n0.tags.end()) {
                    result[OrderableFixedArray{v.orig.n0.position}] = safe_stox<CompressedScenePos>(it->second);
                }
            } else {
                auto it0 = v.orig.n0.tags.find("garden_margin");
                auto it1 = v.orig.n1.tags.find("garden_margin");
                if ((it0 != v.orig.n0.tags.end()) &&
                    (it1 != v.orig.n1.tags.end()))
                {
                    auto p0 = safe_stox<CompressedScenePos>(it0->second);
                    auto p1 = safe_stox<CompressedScenePos>(it1->second);
                    result[OrderableFixedArray{v.orig.position()}] = blend(p0, p1, v.orig.a0, v.orig.a1);
                }
            }
        }
    }
    return result;
}
