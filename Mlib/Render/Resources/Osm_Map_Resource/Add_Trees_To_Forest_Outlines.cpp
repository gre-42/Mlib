#include "Add_Trees_To_Forest_Outlines.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parsed_Resource_Name.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Stats/N_Random_Numbers.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>

using namespace Mlib;

void Mlib::add_trees_to_forest_outlines(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& hitboxes,
    // std::list<SteinerPointInfo>& steiner_points,
    ResourceNameCycle& rnc,
    float min_dist_to_road,
    const StreetBvh& street_bvh,
    const GroundBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float tree_distance,
    float tree_inwards_distance,
    float scale)
{
    UniformRandomNumberGenerator<float> na_rng{ 0 };
    NormalRandomNumberGenerator<float> scale_rng{0, 1.f, 0.2f};
    // size_t rid = 0;
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.contains("landuse", "forest") ||
            tags.contains("natural", "wood"))
        {
            float area = compute_area_clockwise(w.second.nd, nodes, scale);
            for (auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
                auto s = it;
                ++s;
                if (s == w.second.nd.end()) {
                    continue;
                }
                FixedArray<float, 2> p0 = nodes.at(*it).position;
                FixedArray<float, 2> p1 = nodes.at(*s).position;
                float len = std::sqrt(sum(squared(p0 - p1)));
                FixedArray<float, 2> normal{p0(1) - p1(1), p1(0) - p0(0)};
                normal /= len;
                n_random_numbers(len / (tree_distance * scale), na_rng, [&](float aa){
                    FixedArray<float, 2> p = (aa * p0 + (1 - aa) * p1) - tree_inwards_distance * scale * normal * sign(area);
                    if (std::isnan(min_dist_to_road) || !street_bvh.has_neighbor(p, min_dist_to_road * scale)) {
                        float height;
                        if (ground_bvh.height(height, p)) {
                            add_parsed_resource_name(p, height, rnc(), 0.f, scale_rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
                        }
                        // object_resource_descriptors.push_back({
                        //     position: FixedArray<float, 3>{p(0), p(1), 0},
                        //     name: rnc(),
                        //     scale: rng()});
                        // if ((rid++) % 4 == 0) {
                        // steiner_points.push_back({
                        //     .position = {p(0), p(1), 0.f},
                        //     .type = SteinerPointType::FOREST_OUTLINE,
                        //     .distance_to_road = NAN});
                        // }
                    }
                });
            }
        }
    }
}
