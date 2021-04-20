#include "Add_Trees_To_Tree_Nodes.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parsed_Resource_Name.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>

using namespace Mlib;

void Mlib::add_trees_to_tree_nodes(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& hitboxes,
    std::list<SteinerPointInfo>& steiner_points,
    ResourceNameCycle& rnc,
    float min_dist_to_road,
    const StreetBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    float scale)
{
    NormalRandomNumberGenerator<float> scale_rng{0, 1.f, 0.2f};
    for (const auto& n : nodes) {
        const auto& tags = n.second.tags;
        if (tags.contains("natural", "tree")) {
            const auto& p = n.second.position;
            if (std::isnan(min_dist_to_road) || !ground_bvh.has_neighbor(p, min_dist_to_road * scale)) {
                add_parsed_resource_name(p, 0.f, rnc(), 0.f, scale_rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
                steiner_points.push_back({
                    .position = {p(0), p(1), 0.f},
                    .type = SteinerPointType::TREE_NODE,
                    .distance_to_road = NAN});
            }
        }
    }
}
