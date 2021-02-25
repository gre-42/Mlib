#include "Add_Street_Steiner_Points.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Triangle_Is_Right_Handed.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>

using namespace Mlib;

void Mlib::add_street_steiner_points(
    std::list<SteinerPointInfo>& steiner_points,
    const std::list<FixedArray<ColoredVertex, 3>>& ground_triangles,
    const std::list<FixedArray<ColoredVertex, 3>>& air_triangles,
    const BoundingInfo& bounding_info,
    float scale,
    const std::vector<float>& steiner_point_distances_road,
    const std::vector<float>& steiner_point_distances_steiner)
{
    if (steiner_point_distances_road.empty()) {
        return;
    }
    typedef FixedArray<FixedArray<float, 2>, 3> Triangle2d;
    // for (float f = 0.01; f < 2; f += 0.01) {
    //     Bvh<float, Triangle2d, 2> bvh{{f, f}, 10};
    //     for (const auto& t : triangles) {
    //         Triangle2d tri{
    //             FixedArray<float, 2>{t(0).position(0), t(0).position(1)},
    //             FixedArray<float, 2>{t(1).position(0), t(1).position(1)},
    //             FixedArray<float, 2>{t(2).position(0), t(2).position(1)}};
    //         bvh.insert(tri, "", tri);
    //     }
    //     std::cerr << "f " << f << " search_time " << bvh.search_time() << std::endl;
    // }
    Bvh<float, Triangle2d, 2> ground_bvh{{0.1f, 0.1f}, 10};
    Bvh<float, Triangle2d, 2> air_bvh{{0.1f, 0.1f}, 10};
    auto insert = [](auto& bvh, auto& triangles) {
        for (const auto& t : triangles) {
            Triangle2d tri{
                FixedArray<float, 2>{t(0).position(0), t(0).position(1)},
                FixedArray<float, 2>{t(1).position(0), t(1).position(1)},
                FixedArray<float, 2>{t(2).position(0), t(2).position(1)}};
            if (triangle_is_right_handed(tri(0), tri(1), tri(2))) {
                bvh.insert(tri, tri);
            }
        }
    };
    insert(ground_bvh, ground_triangles);
    insert(air_bvh, air_triangles);
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
            auto get_min_dist = [&pt, &dist1](auto& bvh){
                float min_distance = bvh.min_distance(pt, dist1, [&pt](const Triangle2d& tri) {
                    return distance_point_to_triangle(pt, tri(0), tri(1), tri(2));
                });
                return min_distance;
            };
            float min_distance = get_min_dist(ground_bvh);
            float air_min_distance = get_min_dist(air_bvh);
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
