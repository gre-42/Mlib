#include "Parsed_Resource_Name.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Billboard_Id.hpp>
#include <Mlib/Geometry/Material/Aggregate_Mode.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Descriptors/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(BILLBOARD_ID);
DECLARE_OPTION(YANGLE);
DECLARE_OPTION(PROBABILITY);
DECLARE_OPTION(PROBABILITY1);
DECLARE_OPTION(MIN_BDRY);
DECLARE_OPTION(MAX_BDRY);
DECLARE_OPTION(HITBOX);

ParsedResourceName Mlib::parse_resource_name(
    const SceneNodeResources& resources,
    const std::string& name)
{
    static const DECLARE_REGEX(re,
        "^([^.(\\s]*)"
        "(?:\\.(\\d+))?"
        "(?:\\s*\\(yangle:([\\d+.e-]+)\\))?"
        "(?:\\s*\\(p:([\\d+.e-]+)\\))?"
        "(?:\\s*\\(p1:([\\d+.e-]+)\\))?"
        "(?:\\s*\\(min_bdry:([\\d+.e-]+)\\))?"
        "(?:\\s*\\(max_bdry:([\\d+.e-]+)\\))?"
        "(?:\\s*\\(hitbox:(\\w+)\\))?$");
    Mlib::re::cmatch match;
    if (!Mlib::re::regex_match(name, match, re)) {
        THROW_OR_ABORT("Could not parse: " + name);
    }
    ParsedResourceName result{
        .name = VariableAndHash{ match[NAME].str() },
        .billboard_id = match[BILLBOARD_ID].matched ? safe_stox<BillboardId>(match[BILLBOARD_ID].str()) : BILLBOARD_ID_NONE,
        .yangle = match[YANGLE].matched ? safe_stof(match[YANGLE].str()) * degrees : 0.f,
        .probability = match[PROBABILITY].matched ? safe_stof(match[PROBABILITY].str()) : 1.f,
        .probability1 = match[PROBABILITY1].matched ? safe_stof(match[PROBABILITY1].str()) : 1.f,
        .min_distance_to_bdry = match[MIN_BDRY].matched ? safe_stof(match[MIN_BDRY].str()) : 0.f,
        .max_distance_to_bdry = match[MAX_BDRY].matched ? safe_stof(match[MAX_BDRY].str()) : INFINITY,
        .aggregate_mode = resources.aggregate_mode(VariableAndHash<std::string>{match[NAME].str()}),
        .create_imposter = false,
        .max_imposter_texture_size = 0,
        .hitbox = VariableAndHash<std::string>{ match[HITBOX].str() },
        .supplies_cooldown = NAN};
    if (result.probability < 1e-7) {
        THROW_OR_ABORT("ResourceNameCycle: threshold too small");
    }
    if (result.probability > 1) {
        THROW_OR_ABORT("ResourceNameCycle: threshold too large");
    }
    return result;
}
