#include "Modify_Physics_Material_Tags.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(RESOURCE_NAME);
DECLARE_OPTION(ADD);
DECLARE_OPTION(REMOVE);
DECLARE_OPTION(INCLUDED_TAGS);
DECLARE_OPTION(EXCLUDED_TAGS);
DECLARE_OPTION(INCLUDED_NAMES);
DECLARE_OPTION(EXCLUDED_NAMES);

LoadSceneUserFunction ModifyPhysicsMaterialTags::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*modify_physics_material_tags"
        "\\s+resource_name=([\\w+-.]+)"
        "(?:\\s+add=([\\w+-.]+))?"
        "(?:\\s+remove=([\\w+-.]+))?"
        "(?:\\s+included_tags=(.*?))?"
        "(?:\\s+excluded_tags=(.*?))?"
        "(?:\\s+included_names=(.*?))?"
        "(?:\\s+excluded_names=(.*?))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void ModifyPhysicsMaterialTags::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.modify_physics_material_tags(
        match[RESOURCE_NAME].str(),
        ColoredVertexArrayFilter{
            .included_tags = match[INCLUDED_TAGS].matched
                ? physics_material_from_string(match[INCLUDED_TAGS].str())
                : PhysicsMaterial::NONE,
            .excluded_tags = match[EXCLUDED_TAGS].matched
                ? physics_material_from_string(match[EXCLUDED_TAGS].str())
                : PhysicsMaterial::NONE,
            .included_names = Mlib::compile_regex(match[INCLUDED_NAMES].str()),
            .excluded_names = Mlib::compile_regex(
                match[EXCLUDED_NAMES].matched
                    ? match[EXCLUDED_NAMES].str()
                    : "$ ^")
        },
        match[ADD].matched
            ? physics_material_from_string(match[ADD].str())
            : PhysicsMaterial::NONE,
        match[REMOVE].matched
            ? physics_material_from_string(match[REMOVE].str())
            : PhysicsMaterial::NONE);
}
