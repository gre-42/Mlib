#include "Way_Bvh.hpp"
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Geometry/Intersection/Distance/Distance_Point_Line.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

WayBvh::WayBvh()
    : bvh_{{(CompressedScenePos)100.f, (CompressedScenePos)100.f}, 10}
{}

WayBvh::WayBvh(const std::list<Line2d>& way_segments)
    : WayBvh{}
{
    for (const auto& s : way_segments) {
        bvh_.insert(AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(s), s);
    }
}

WayBvh::~WayBvh() = default;

void WayBvh::add_path(const std::list<FixedArray<CompressedScenePos, 2>>& path) {
    if (path.empty()) {
        return;
    }
    auto right = path.begin();
    while (true) {
        auto left = right++;
        if (right == path.end()) {
            break;
        }
        Line2d s{*left, *right};
        bvh_.insert(AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(s), s);
    }
}

bool WayBvh::nearest_way(
    const FixedArray<CompressedScenePos, 2>& position,
    CompressedScenePos max_dist,
    FixedArray<double, 2>& dir,
    CompressedScenePos& distance) const
{
    const Line2d* nearest_way;
    auto dist = bvh_.min_distance(position, max_dist, [&position](const Line2d& way) {
        FixedArray<double, 2> dir = uninitialized;
        ScenePos distance;
        distance_point_to_line(funpack(position), funpack(way[0]), funpack(way[1]), dir, distance);
        return (CompressedScenePos)distance;
    }, &nearest_way);
    if (dist.has_value()) {
        distance = *dist;
        ScenePos tmp_distance;
        distance_point_to_line(funpack(position), funpack((*nearest_way)[0]), funpack((*nearest_way)[1]), dir, tmp_distance);
        return true;
    }
    return false;
}

FixedArray<CompressedScenePos, 2> WayBvh::project_onto_way(
    const std::string& node_id,
    const Node& node,
    double scale) const
{
    double distance_to_way = parse_meters(node.tags, "distance_to_way", NAN);
    float distance_to_way_factor = parse_float(node.tags, "distance_to_way_factor", 1.f);
    if (!std::isnan(distance_to_way)) {
        double wanted_distance = scale * distance_to_way * distance_to_way_factor;
        FixedArray<double, 2> dir = uninitialized;
        CompressedScenePos distance;
        if (!nearest_way(node.position, (CompressedScenePos)(2.f * wanted_distance), dir, distance)) {
            throw PointException<CompressedScenePos, 2>(node.position, "Could not find way for node \"" + node_id + '"');
        } else if (distance == (CompressedScenePos)0.f) {
            THROW_OR_ABORT("Node \"" + node_id + "\" is on a way");
        } else {
            return node.position + (dir * (wanted_distance - funpack(distance))).casted<CompressedScenePos>();
        }
    } else {
        return node.position;
    }
}
