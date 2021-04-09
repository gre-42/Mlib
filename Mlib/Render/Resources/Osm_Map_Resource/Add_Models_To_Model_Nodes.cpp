#include "Add_Models_To_Model_Nodes.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parsed_Resource_Name.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

void Mlib::add_models_to_model_nodes(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    std::list<SteinerPointInfo>& steiner_points,
    const SceneNodeResources& resources,
    const std::map<std::string, Node>& nodes,
    float scale)
{
    for (const auto& n : nodes) {
        const auto& tags = n.second.tags;
        if (auto it = tags.find("model"); it != tags.end()) {
            const auto& p = n.second.position;
            ParsedResourceName prn{
                .name = it->second,
                .probability = NAN,
                .aggregate_mode = resources.aggregate_mode(it->second),
                .hitbox = ""};
            add_parsed_resource_name(p, prn, 1.f, resource_instance_positions, object_resource_descriptors, hitboxes);
            steiner_points.push_back({
                .position = {p(0), p(1), 0.f},
                .type = SteinerPointType::MODEL_NODE,
                .distance_to_road = NAN});
        }
    }
}
