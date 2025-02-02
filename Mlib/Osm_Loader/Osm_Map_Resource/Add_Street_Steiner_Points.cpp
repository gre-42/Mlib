#include "Add_Street_Steiner_Points.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Way_Bvh.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>

using namespace Mlib;

void Mlib::add_street_steiner_points(
    std::list<SteinerPointInfo>& steiner_points,
    const StreetBvh& ground_street_bvh,
    const WayBvh& terrain_region_contours_bvh,
    const BoundingInfo& bounding_info,
    double scale,
    const std::vector<double>& steiner_point_distances_road,
    const std::vector<double>& steiner_point_distances_steiner,
    double min_dist_to_terrain_region)
{
    if (steiner_point_distances_road.empty()) {
        return;
    }
    Interp<double> interp{ steiner_point_distances_road, steiner_point_distances_steiner, OutOfRangeBehavior::CLAMP };
    // lerr() << "search_time " << bvh.search_time();
    CompressedScenePos dist0 = (CompressedScenePos)((*std::min_element(steiner_point_distances_steiner.begin(), steiner_point_distances_steiner.end())) * scale);
    CompressedScenePos dist1 = (CompressedScenePos)0.f;
    for (double v : steiner_point_distances_steiner) {
        if (v != INFINITY) {
            dist1 = std::max(dist1, (CompressedScenePos)(v * scale));
        }
    }
    size_t ix = 0;
    FastNormalRandomNumberGenerator<double> rng2{ 0, 0.f, 1.2f };
    for (CompressedScenePos x = bounding_info.boundary_min(0) + bounding_info.border_width / 2; x < bounding_info.boundary_max(0) - bounding_info.border_width / 2; x += dist0) {
        size_t iy = 0;
        for (CompressedScenePos y = bounding_info.boundary_min(1) + bounding_info.border_width / 2; y < bounding_info.boundary_max(1) - bounding_info.border_width / 2; y += dist0) {
            FixedArray<CompressedScenePos, 2> pt{
                x + (CompressedScenePos)(rng2() * scale),
                y + (CompressedScenePos)(rng2() * scale) };
            // Check terrain region BVH
            {
                CompressedScenePos min_distance_allowed = (CompressedScenePos)(min_dist_to_terrain_region * scale);
                FixedArray<double, 2> dir = uninitialized;
                CompressedScenePos min_distance;
                if (terrain_region_contours_bvh.nearest_way(pt, min_distance_allowed, dir, min_distance) &&
                    (min_distance < min_distance_allowed))
                {
                    continue;
                }
            }
            // Check street BVH
            CompressedScenePos min_distance = ground_street_bvh.min_dist(pt, dist1).value_or(std::numeric_limits<CompressedScenePos>::max());
            if (min_distance <= (CompressedScenePos)0.f) {
                continue;
            }
            if (double dist = interp(funpack(min_distance) / scale); dist != INFINITY) {
                size_t refinement = std::max<size_t>(1, (size_t)((dist * scale) / funpack(dist0)));
                bool is_included = (ix % refinement == 0) && (iy % refinement == 0);
                if (is_included) {
                    steiner_points.push_back(SteinerPointInfo{
                        .position = {pt(0), pt(1), (CompressedScenePos)0.f},
                        .type = SteinerPointType::STREET_NEIGHBOR });
                }
            }
            ++iy;
        }
        ++ix;
    }
}
