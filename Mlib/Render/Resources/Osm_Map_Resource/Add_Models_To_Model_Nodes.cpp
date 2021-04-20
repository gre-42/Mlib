#include "Add_Models_To_Model_Nodes.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parsed_Resource_Name.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

void Mlib::add_models_to_model_nodes(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    const GroundBvh& ground_bvh,
    const SceneNodeResources& resources,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float scale,
    const std::string& game_level)
{
    std::map<std::string, std::string> prev_neighbor;
    std::map<std::string, std::string> next_neighbor;
    for (const auto& w : ways) {
        for (auto it = w.second.nd.begin();;) {
            if (it == w.second.nd.end()) {
                break;
            }
            const auto& prev = *it++;
            if (it == w.second.nd.end()) {
                break;
            }
            const auto& prev_tags = nodes.at(prev).tags;
            const auto& next_tags = nodes.at(*it).tags;
            if (next_tags.find("model") != next_tags.end() && !prev_neighbor.insert({*it, prev}).second) {
                throw std::runtime_error("Could not insert prev neighbor of node " + *it);
            }
            if (prev_tags.find("model") != prev_tags.end() &&!next_neighbor.insert({prev, *it}).second) {
                throw std::runtime_error("Could not insert next neighbor of node " + prev);
            }
        }
    }
    for (const auto& n : nodes) {
        const auto& tags = n.second.tags;
        if (auto mit = tags.find("model"); mit != tags.end()) {
            if (auto lit = tags.find("game:level"); (lit != tags.end()) && (lit->second != game_level)) {
                continue;
            }
            auto hit = tags.find("hitbox");
            const auto& p = n.second.position;
            ParsedResourceName prn{
                .name = mit->second,
                .probability = NAN,
                .aggregate_mode = resources.aggregate_mode(mit->second),
                .hitbox = (hit == tags.end()) ? "" : hit->second};
            auto yit = tags.find("yangle");
            float yangle;
            if (yit == tags.end()) {
                auto np = prev_neighbor.find(n.first);
                auto nn = next_neighbor.find(n.first);
                if (np == prev_neighbor.end() && nn == next_neighbor.end()) {
                    yangle = 0.f;
                } else if (np != prev_neighbor.end() && nn != next_neighbor.end()) {
                    FixedArray<float, 2> dir = nodes.at(nn->second).position - nodes.at(np->second).position;
                    yangle = std::atan2(-dir(1), -dir(0));
                } else if (np != prev_neighbor.end()) {
                    FixedArray<float, 2> dir = nodes.at(n.first).position - nodes.at(np->second).position;
                    yangle = std::atan2(-dir(1), -dir(0));
                } else {
                    FixedArray<float, 2> dir = nodes.at(nn->second).position - nodes.at(n.first).position;
                    yangle = std::atan2(-dir(1), -dir(0));
                }
            } else {
                yangle = safe_stof(yit->second) * float{M_PI} / 180.f;
            }
            float height;
            if (ground_bvh.height(height, p)) {
                add_parsed_resource_name(
                    p,
                    height,
                    prn,
                    yangle,
                    1.f,
                    resource_instance_positions,
                    object_resource_descriptors,
                    hitboxes);
            }
        }
    }
}
