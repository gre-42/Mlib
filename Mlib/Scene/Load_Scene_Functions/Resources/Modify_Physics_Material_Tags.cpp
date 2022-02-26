#include "Modify_Physics_Material_Tags.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(RESOURCE_NAME);
DECLARE_OPTION(ADD);
DECLARE_OPTION(REMOVE);
DECLARE_OPTION(INCLUDE);
DECLARE_OPTION(EXCLUDE);

LoadSceneUserFunction ModifyPhysicsMaterialTags::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*modify_physics_material_tags"
        "\\s+resource_name=([\\w+-.]+)"
        "(?:\\s+add=([\\w+-.]+))?"
        "(?:\\s+remove=([\\w+-.]+))?"
        "(?:\\s+include=(.*?))?"
        "(?:\\s+exclude=(.*?))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void ModifyPhysicsMaterialTags::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.modify_physics_material_tags(
        match[RESOURCE_NAME].str(),
        ResourceFilter{
            .include = Mlib::compile_regex(match[INCLUDE].str()),
            .exclude = Mlib::compile_regex(
                match[EXCLUDE].matched
                    ? match[EXCLUDE].str()
                    : "$ ^")
        },
        match[ADD].matched
            ? physics_material_from_string(match[ADD].str())
            : PhysicsMaterial::NONE,
        match[REMOVE].matched
            ? physics_material_from_string(match[REMOVE].str())
            : PhysicsMaterial::NONE);
}
