#include "Parsed_Resource_Name.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Resource_Instance_Descriptor.hpp>
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
