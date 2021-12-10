#include "Parsed_Resource_Name.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

ParsedResourceName Mlib::parse_resource_name(
    const SceneNodeResources& resources,
    const std::string& name)
{
    static const DECLARE_REGEX(re, "^([^.(]*)(?:\\.(\\d+))?(?:\\(p:([\\d+.e-]+)\\))?(?:\\(hitbox:(\\w+)\\))?$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(name, match, re)) {
        ParsedResourceName result{
            .name = match[1].str(),
            .billboard_id = match[2].matched ? safe_stou(match[2].str()) : UINT32_MAX,
            .probability = match[3].matched ? safe_stof(match[3].str()) : 1,
            .aggregate_mode = resources.aggregate_mode(match[1].str()),
            .hitbox = match[4].str()};
        if (result.probability < 1e-7) {
            throw std::runtime_error("ResourceNameCycle: threshold too small");
        }
        if (result.probability > 1) {
            throw std::runtime_error("ResourceNameCycle: threshold too large");
        }
        return result;
    } else {
        return ParsedResourceName{
            .name = name,
            .billboard_id = UINT32_MAX,
            .probability = 1,
            .aggregate_mode = resources.aggregate_mode(name)};
    }
}

void Mlib::add_parsed_resource_name(
    const FixedArray<float, 3>& p,
    const ParsedResourceName& prn,
    float yangle,
    float scale,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& hitboxes)
{
    ResourceInstanceDescriptor rid{
        .position = p,
        .yangle = yangle,
        .scale = scale,
        .billboard_id = prn.billboard_id};
    if (prn.aggregate_mode & (AggregateMode::INSTANCES_ONCE | AggregateMode::INSTANCES_SORTED_CONTINUOUSLY)) {
        resource_instance_positions[prn.name].push_back(rid);
    } else {
        object_resource_descriptors.push_back({p, prn.name, scale});
    }
    if (!prn.hitbox.empty()) {
        hitboxes[prn.hitbox].push_back(rid);
    }
}

void Mlib::add_parsed_resource_name(
    const FixedArray<float, 2>& p,
    float height,
    const ParsedResourceName& prn,
    float yangle,
    float scale,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& hitboxes)
{
    add_parsed_resource_name(
        FixedArray<float, 3>{p(0), p(1), height},
        prn,
        yangle,
        scale,
        resource_instance_positions,
        object_resource_descriptors,
        hitboxes);
}
