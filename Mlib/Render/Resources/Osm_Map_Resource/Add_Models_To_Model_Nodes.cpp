#include "Add_Models_To_Model_Nodes.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Way_Bvh.hpp>
#include <Mlib/Scene_Graph/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

void Mlib::add_models_to_model_nodes(
    BatchResourceInstantiator& bri,
    const std::list<FixedArray<FixedArray<float, 2>, 2>>& way_segments,
    const GroundBvh& ground_bvh,
    const SceneNodeResources& resources,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float scale,
    const std::string& game_level)
{
    WayBvh way_bvh{ way_segments };
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
            FixedArray<float, 2> p;
            if (n.second.tags.contains("distance_to_way")) {
                float wanted_distance = scale * safe_stof(n.second.tags.at("distance_to_way"));
                FixedArray<float, 2> dir;
                float distance;
                way_bvh.nearest_way(n.second.position, 2.f * wanted_distance, dir, distance);
                if (distance == INFINITY) {
                    throw std::runtime_error("Could not find way for node \"" + n.first + '"');
                } else if (distance == 0) {
                    throw std::runtime_error("Node \"" + n.first + "\" is on a way");
                } else {
                    p = n.second.position + dir * (wanted_distance - distance);
                }
            } else {
                p = n.second.position;
            }
            static const DECLARE_REGEX(model_re, "^([^.]+)(?:\\.(\\d+) \\((\\w+)\\))?$");
            Mlib::re::smatch match;
            if (!Mlib::re::regex_match(mit->second, match, model_re)) {
                throw std::runtime_error("Could not parse model name \"" + mit->second + '"');
            }
            ParsedResourceName prn{
                .name = match[1].str(),
                .billboard_id = match[2].matched ? safe_stou(match[2].str()) : UINT32_MAX,
                .probability = NAN,
                .aggregate_mode = resources.aggregate_mode(match[1].str()),
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
                if (yit->second == "random") {
                    yangle = UniformRandomNumberGenerator<float>(
                        1523 + std::abs(safe_stoi(n.first)),
                        0.f,
                        2.f * float{M_PI})();
                } else {
                    yangle = safe_stof(yit->second) * float{M_PI} / 180.f;
                }
            }
            float height;
            if (ground_bvh.height(height, p)) {
                bri.add_parsed_resource_name(
                    p,
                    height,
                    prn,
                    yangle,
                    1.f);
            }
        }
    }
}
