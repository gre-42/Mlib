#include "Add_Trees_To_Tree_Nodes.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Resource_Name_Cycle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Scene_Graph/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>

using namespace Mlib;

void Mlib::add_trees_to_tree_nodes(
    BatchResourceInstantiator& bri,
    // std::list<SteinerPointInfo>& steiner_points,
    ResourceNameCycle& rnc,
    float min_dist_to_road,
    const StreetBvh& street_bvh,
    const GroundBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    float scale)
{
    NormalRandomNumberGenerator<float> scale_rng{0, 1.f, 0.2f};
    UniformRandomNumberGenerator<float> yangle_rng{1, 0.f, 2 * float{M_PI}};
    for (const auto& n : nodes) {
        const auto& tags = n.second.tags;
        if (tags.contains("natural", "tree")) {
            const auto& p = n.second.position;
            if (std::isnan(min_dist_to_road) || !street_bvh.has_neighbor(p, min_dist_to_road * scale)) {
                double height;
                if (ground_bvh.height(height, p)) {
                    bri.add_parsed_resource_name(p, height, rnc(), yangle_rng(), scale_rng());
                }
                // steiner_points.push_back({
                //     .position = {p(0), p(1), 0.f},
                //     .type = SteinerPointType::TREE_NODE,
                //     .distance_to_road = NAN});
            }
        }
    }
}
