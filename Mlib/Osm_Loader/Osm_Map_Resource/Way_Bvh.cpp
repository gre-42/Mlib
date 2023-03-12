#include "Way_Bvh.hpp"
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Mesh/Point_Exception.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

WayBvh::WayBvh(const std::list<Line2d>& way_segments)
: bvh_{{0.1f, 0.1f}, 10}
{
    for (const auto& s : way_segments) {
        bvh_.insert(s, s);
    }
}

void WayBvh::nearest_way(
    const FixedArray<double, 2>& position,
    double max_dist,
    FixedArray<double, 2>& dir,
    double& distance) const
{
    const Line2d* nearest_way;
    distance = bvh_.min_distance(position, max_dist, [&position](const Line2d& way) {
        FixedArray<double, 2> dir;
        double distance;
        distance_point_to_line(position, way(0), way(1), dir, distance);
        return distance;
    }, &nearest_way);
    if (distance != INFINITY) {
        distance_point_to_line(position, (*nearest_way)(0), (*nearest_way)(1), dir, distance);
    }
}

FixedArray<double, 2> WayBvh::project_onto_way(
    const std::string& node_id,
    const Node& node,
    double scale) const
{
    double distance_to_way = parse_meters(node.tags, "distance_to_way", NAN);
    float distance_to_way_factor = parse_float(node.tags, "distance_to_way_factor", 1.f);
    if (!std::isnan(distance_to_way)) {
        double wanted_distance = scale * distance_to_way * distance_to_way_factor;
        FixedArray<double, 2> dir;
        double distance;
        nearest_way(node.position, 2.f * wanted_distance, dir, distance);
        if (distance == INFINITY) {
            throw PointException<double, 2>(node.position, "Could not find way for node \"" + node_id + '"');
        } else if (distance == 0) {
            THROW_OR_ABORT("Node \"" + node_id + "\" is on a way");
        } else {
            return node.position + dir * (wanted_distance - distance);
        }
    } else {
        return node.position;
    }
}
