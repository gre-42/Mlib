#include "Get_Smooth_Building_Levels.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Building_Part_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle_2D.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Way.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

std::list<FixedArray<CompressedScenePos, 2, 2>> Mlib::smooth_building_level(
    const Building& bu,
    const std::map<std::string, Node>& nodes,
    double max_length,
    double width0,
    double width1,
    double scale)
{
    if (bu.way.nd.empty()) {
        THROW_OR_ABORT("Building " + bu.id + ": outline is empty");
    }
    if (bu.way.nd.front() != bu.way.nd.back()) {
        THROW_OR_ABORT("Cannot compute smooth level of building " + bu.id + ": outline not closed");
    }
    std::list<FixedArray<CompressedScenePos, 2, 2>> result;
    auto sw = subdivided_way(
        nodes,
        bu.way.nd,
        scale,
        max_length);
    sw.erase(sw.begin());
    if (bu.area < 0) {
        sw.reverse();
    }
    auto a = sw.begin();
    for (size_t i = 0; i < sw.size(); ++i) {
        auto b = a;
        ++b;
        if (b == sw.end()) {
            b = sw.begin();
        }
        auto c = b;
        ++c;
        if (c == sw.end()) {
            c = sw.begin();
        }
        auto d = c;
        ++d;
        if (d == sw.end()) {
            d = sw.begin();
        }
        OsmRectangle2D rect = uninitialized;
        if (!OsmRectangle2D::from_line(
                rect,
                *a,
                *a,
                *b,
                *c,
                *d,
                *d,
                (CompressedScenePos)(scale * width0),
                (CompressedScenePos)(scale * width1),
                (CompressedScenePos)(scale * width0),
                (CompressedScenePos)(scale * width1),
                (CompressedScenePos)(scale * width0),
                (CompressedScenePos)(scale * width1)))
        {
            THROW_OR_ABORT("Error triangulating level of building " + bu.id);
        } else {
            result.push_back(
                FixedArray<CompressedScenePos, 2, 2>{
                    rect.p01_.casted<CompressedScenePos>(),
                    rect.p11_.casted<CompressedScenePos>()});
        }
        // draw_node(triangles, nodes.at(*a));
        ++a;
        if (a == sw.end()) {
            a = sw.begin();
        }
    }
    return result;
}

BuildingLevelOutline Mlib::smooth_building_level_outline(
    const Building& bu,
    const std::map<std::string, Node>& nodes,
    double scale,
    double max_length,
    DrawBuildingPartType tpe)
{
    const BuildingLevel& bl = (tpe == DrawBuildingPartType::CEILING)
        ? bu.levels.back()
        : bu.levels.front();
    BuildingLevelOutline result;
    std::list<FixedArray<CompressedScenePos, 2, 2>> segments;
    if ((tpe == DrawBuildingPartType::CEILING) && bu.roof_9_2.has_value()) {
        segments = smooth_building_level(
            bu,
            nodes,
            max_length,
            bu.roof_9_2.value().width,
            bu.roof_9_2.value().width,
            scale);
        result.z = (CompressedScenePos)(bl.top + bu.roof_9_2.value().height);
    } else {
        segments = smooth_building_level(
            bu,
            nodes,
            max_length,
            bl.extra_width,
            bl.extra_width,
            scale);
        result.z = (tpe == DrawBuildingPartType::CEILING)
            ? (CompressedScenePos)bl.top
            : (CompressedScenePos)0.;
    }
    for (const auto& w : segments) {
        result.outline.push_back(w[0]);
    }
    return result;
}
