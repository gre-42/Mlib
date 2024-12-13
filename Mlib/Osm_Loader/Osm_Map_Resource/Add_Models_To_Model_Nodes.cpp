#include "Add_Models_To_Model_Nodes.hpp"
#include <Mlib/Billboard_Id.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

void Mlib::add_models_to_model_nodes(
    BatchResourceInstantiator& bri,
    const GroundBvh& ground_bvh,
    const SceneNodeResources& resources,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::string& game_level)
{
    std::map<std::string, std::string> prev_neighbor;
    std::map<std::string, std::string> next_neighbor;
    for (const auto& [_, way] : ways) {
        for (auto it = way.nd.begin();;) {
            if (it == way.nd.end()) {
                break;
            }
            const auto& prev = *it++;
            if (it == way.nd.end()) {
                break;
            }
            const auto& prev_tags = nodes.at(prev).tags;
            const auto& next_tags = nodes.at(*it).tags;
            if (next_tags.find("model") != next_tags.end() && !prev_neighbor.insert({*it, prev}).second) {
                THROW_OR_ABORT("Could not insert prev neighbor of node " + *it);
            }
            if (prev_tags.find("model") != prev_tags.end() &&!next_neighbor.insert({prev, *it}).second) {
                THROW_OR_ABORT("Could not insert next neighbor of node " + prev);
            }
        }
    }
    for (const auto& [node_id, node] : nodes) {
        const auto& tags = node.tags;
        if (auto mit = tags.find("model"); mit != tags.end()) {
            if (auto lit = tags.find("game:level"); (lit != tags.end()) && (lit->second != game_level)) {
                continue;
            }
            static const DECLARE_REGEX(model_re, "^([^.]+)(?:\\.(\\d+) \\((\\w+)\\))?$");
            Mlib::re::smatch match;
            if (!Mlib::re::regex_match(mit->second, match, model_re)) {
                THROW_OR_ABORT("Could not parse model name \"" + mit->second + '"');
            }
            auto hit = tags.find("hitbox");
            auto iit = tags.find("max_imposter_texture_size");
            ParsedResourceName prn{
                .name = VariableAndHash{ match[1].str() },
                .billboard_id = match[2].matched ? safe_stox<BillboardId>(match[2].str()) : BILLBOARD_ID_NONE,
                .yangle = 0.f,
                .probability = NAN,
                .aggregate_mode = resources.aggregate_mode(match[1].str()),
                .create_imposter = tags.contains("create_imposter", "yes"),
                .max_imposter_texture_size = (iit == tags.end()) ? 1024 : safe_sto<uint32_t>(iit->second),
                .hitbox = (hit == tags.end()) ? "" : hit->second,
                .supplies_cooldown = 0.f};
            for (const auto& [k, v] : tags) {
                // Structure:
                // key                             value
                // supplies:meta:cooldown_seconds  3.5
                // supplies:a                      4
                // supplies:b                      2
                static const DECLARE_REGEX(supplies_re, "^supplies:(.*)$");
                Mlib::re::smatch supplies_match;
                if (Mlib::re::regex_match(k, supplies_match, supplies_re)) {
                    if (supplies_match[1].str() == "meta:cooldown_seconds") {
                        prn.supplies_cooldown = safe_stof(v) * seconds;
                    } else if (!prn.supplies.try_emplace(supplies_match[1].str(), safe_stox<uint32_t>(v, "supplies")).second) {
                        THROW_OR_ABORT("Could not insert supplies");
                    }
                }
            }
            auto yit = tags.find("yangle");
            float yangle;
            if (yit == tags.end()) {
                auto np = prev_neighbor.find(node_id);
                auto nn = next_neighbor.find(node_id);
                if (np == prev_neighbor.end() && nn == next_neighbor.end()) {
                    yangle = 0.f;
                } else if (np != prev_neighbor.end() && nn != next_neighbor.end()) {
                    FixedArray<double, 2> dir = funpack(nodes.at(nn->second).position - nodes.at(np->second).position);
                    yangle = (float)std::atan2(-dir(1), -dir(0));
                } else if (np != prev_neighbor.end()) {
                    FixedArray<double, 2> dir = funpack(nodes.at(node_id).position - nodes.at(np->second).position);
                    yangle = (float)std::atan2(-dir(1), -dir(0));
                } else {
                    FixedArray<double, 2> dir = funpack(nodes.at(nn->second).position - nodes.at(node_id).position);
                    yangle = (float)std::atan2(-dir(1), -dir(0));
                }
            } else {
                if (yit->second == "random") {
                    yangle = FastUniformRandomNumberGenerator<float>(
                        1523u + (unsigned int)std::abs(safe_stoi(node_id)),
                        0.f,
                        2.f * float(M_PI))();
                } else {
                    yangle = safe_stof(yit->second) * degrees;
                }
            }
            CompressedScenePos height;
            if (ground_bvh.height(height, node.position)) {
                bri.add_parsed_resource_name(
                    node.position,
                    height,
                    prn,
                    yangle,
                    1.);
            }
        }
    }
}
