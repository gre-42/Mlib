#include "Get_Smooth_Building_Levels.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Building_Part_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle_2D.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Way.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

std::list<BuildingSegment> Mlib::smooth_building_level(
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
    std::list<BuildingSegment> result;
    auto sw = subdivided_way(
        nodes,
        bu.way.nd,
        scale,
        max_length);
    sw.erase(sw.begin());
    if (std::isnan(bu.area)) {
        THROW_OR_ABORT("Building area is NAN");
    }
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
            result.emplace_back(
                FixedArray<CompressedScenePos, 2, 2>{*a, *b},
                FixedArray<CompressedScenePos, 2, 2>{rect.p01_, rect.p11_});
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
    DrawBuildingPartType tpe,
    BuildingDetailType detail)
{
    const BuildingLevel& bl = (tpe == DrawBuildingPartType::CEILING)
        ? bu.levels.back()
        : bu.levels.front();
    BuildingLevelOutline result;
    std::list<BuildingSegment> segments;
    if ((tpe == DrawBuildingPartType::CEILING) &&
        ((detail == BuildingDetailType::HIGH) ||
         (detail == BuildingDetailType::COMBINED)) &&
        bu.roof_9_2.has_value())
    {
        segments = smooth_building_level(
            bu,
            nodes,
            max_length,
            bu.roof_9_2->width,
            bu.roof_9_2->width,
            scale);
        result.z = (CompressedScenePos)(bl.top + bu.roof_9_2->height);
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
        result.outline.emplace_back(w.orig[0], w.indented[0]);
    }
    return result;
}

std::list<std::list<BuildingSegment>> Mlib::straight_building_level(
    const std::list<BuildingSegment>& level,
    float snap_length_angle)
{
    auto cos_snap_length_angle = std::cos(snap_length_angle);
    std::list<std::list<BuildingSegment>> result;
    FixedArray<ScenePos, 2> last_dir{ NAN };
    for (const auto& b : level) {
        auto dir = (b.indented[1] - b.indented[0]).casted<ScenePos>();
        auto len = std::sqrt(sum(squared(dir)));
        if (len < 1e-12) {
            THROW_OR_ABORT("Building segment too short");
        }
        dir /= len;
        if (result.empty() ||
            (dot0d(last_dir, dir) < cos_snap_length_angle))
        {
            result.emplace_back().emplace_back(b);
        } else {
            result.back().emplace_back(b);
        }
        last_dir = dir;
    }
    return result;
}
