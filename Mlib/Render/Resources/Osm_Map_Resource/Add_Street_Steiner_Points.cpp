#include "Add_Street_Steiner_Points.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>

using namespace Mlib;

void Mlib::add_street_steiner_points(
    std::list<SteinerPointInfo>& steiner_points,
    const StreetBvh& ground_bvh,
    const StreetBvh& air_bvh,
    const BoundingInfo& bounding_info,
    float scale,
    const std::vector<float>& steiner_point_distances_road,
    const std::vector<float>& steiner_point_distances_steiner)
{
    if (steiner_point_distances_road.empty()) {
        return;
    }
    Interp<float> interp{steiner_point_distances_road, steiner_point_distances_steiner, OutOfRangeBehavior::CLAMP};
    // std::cerr << "search_time " << bvh.search_time() << std::endl;
    float dist0 = (*std::min_element(steiner_point_distances_steiner.begin(), steiner_point_distances_steiner.end())) * scale;
    float dist1 = 0;
    for (float v : steiner_point_distances_steiner) {
        if (v != INFINITY) {
            dist1 = std::max(dist1, v * scale);
        }
    }
    size_t ix = 0;
    NormalRandomNumberGenerator<float> rng2{0, 0.f, 1.2f};
    for (float x = bounding_info.boundary_min(0) + bounding_info.border_width / 2; x < bounding_info.boundary_max(0) - bounding_info.border_width / 2; x += dist0) {
        size_t iy = 0;
        for (float y = bounding_info.boundary_min(1) + bounding_info.border_width / 2; y < bounding_info.boundary_max(1) - bounding_info.border_width / 2; y += dist0) {
            FixedArray<float, 2> pt{x + rng2() * scale, y + rng2() * scale};
            float min_distance = ground_bvh.min_dist(pt, dist1);
            float air_min_distance = air_bvh.min_dist(pt, dist1);
            if (min_distance > 0) {
                float dist = interp(min_distance / scale);
                if (dist != INFINITY) {
                    size_t refinement = std::max<size_t>(1, (size_t)((dist * scale) / dist0));
                    bool is_included = (ix % refinement == 0) && (iy % refinement == 0);
                    if (is_included) {
                        steiner_points.push_back(SteinerPointInfo{
                            .position = {pt(0), pt(1), 0.f},
                            .type = SteinerPointType::STREET_NEIGHBOR,
                            .distance_to_road = min_distance,
                            .distance_to_air_road = air_min_distance});
                    }
                }
            }
            ++iy;
        }
        ++ix;
    }
}
