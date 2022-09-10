#include "Parsed_Resource_Name.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Descriptors/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(BILLBOARD_ID);
DECLARE_OPTION(PROBABILITY);
DECLARE_OPTION(MIN_BDRY);
DECLARE_OPTION(MAX_BDRY);
DECLARE_OPTION(HITBOX);

ParsedResourceName Mlib::parse_resource_name(
    const SceneNodeResources& resources,
    const std::string& name)
{
    static const DECLARE_REGEX(re,
        "^([^.(]*)"
        "(?:\\.(\\d+))?"
        "(?:\\(p:([\\d+.e-]+)\\))?"
        "(?:\\(min_bdry:([\\d+.e-]+)\\))?"
        "(?:\\(max_bdry:([\\d+.e-]+)\\))?"
        "(?:\\(hitbox:(\\w+)\\))?$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(name, match, re)) {
        throw std::runtime_error("Could not parse: " + name);
    }
    ParsedResourceName result{
        .name = match[NAME].str(),
        .billboard_id = match[BILLBOARD_ID].matched ? safe_stou(match[BILLBOARD_ID].str()) : UINT32_MAX,
        .probability = match[PROBABILITY].matched ? safe_stof(match[PROBABILITY].str()) : 1,
        .min_distance_to_bdry = match[MIN_BDRY].matched ? safe_stof(match[MIN_BDRY].str()) : 0.f,
        .max_distance_to_bdry = match[MAX_BDRY].matched ? safe_stof(match[MAX_BDRY].str()) : INFINITY,
        .aggregate_mode = resources.aggregate_mode(match[NAME].str()),
        .create_impostor = false,
        .hitbox = match[HITBOX].str(),
        .supplies_cooldown = NAN};
    if (result.probability < 1e-7) {
        throw std::runtime_error("ResourceNameCycle: threshold too small");
    }
    if (result.probability > 1) {
        throw std::runtime_error("ResourceNameCycle: threshold too large");
    }
    return result;
}
