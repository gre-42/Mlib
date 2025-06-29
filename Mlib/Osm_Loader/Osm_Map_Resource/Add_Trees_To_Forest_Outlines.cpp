#include "Add_Trees_To_Forest_Outlines.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Resource_Name_Cycle.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <Mlib/Stats/N_Random_Numbers.hpp>

using namespace Mlib;

void Mlib::add_trees_to_forest_outlines(
    BatchResourceInstantiator& bri,
    // std::list<SteinerPointInfo>& steiner_points,
    ResourceNameCycle& rnc,
    double min_dist_to_road,
    const StreetBvh& street_bvh,
    const GroundBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    double tree_distance,
    double tree_inwards_distance,
    double scale)
{
    FastUniformRandomNumberGenerator<double> na_rng{ 0 };
    FastNormalRandomNumberGenerator<float> scale_rng{ 0, 1.f, 0.2f };
    // size_t rid = 0;
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.contains("landuse", "forest") ||
            tags.contains("natural", "wood"))
        {
            double area = compute_area_clockwise(w.second.nd, nodes, scale);
            for (auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
                auto s = it;
                ++s;
                if (s == w.second.nd.end()) {
                    continue;
                }
                FixedArray<double, 2> p0 = funpack(nodes.at(*it).position);
                FixedArray<double, 2> p1 = funpack(nodes.at(*s).position);
                double len = std::sqrt(sum(squared(p0 - p1)));
                FixedArray<double, 2> normal{ p0(1) - p1(1), p1(0) - p0(0) };
                normal /= len;
                n_random_numbers(len / (tree_distance * scale), na_rng, [&](){
                    double aa = na_rng();
                    FixedArray<CompressedScenePos, 2> p =
                        ((aa * p0 + (1 - aa) * p1) - tree_inwards_distance * scale * normal * sign(area))
                        .casted<CompressedScenePos>();
                    if (std::isnan(min_dist_to_road) || !street_bvh.has_neighbor(p, (CompressedScenePos)(min_dist_to_road * scale))) {
                        CompressedScenePos height;
                        if (ground_bvh.max_height(height, p)) {
                            if (auto prn = rnc.try_multiple_times(10); prn != nullptr) {
                                bri.add_parsed_resource_name(p, height, *prn, 0.f, scale_rng());
                            }
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
