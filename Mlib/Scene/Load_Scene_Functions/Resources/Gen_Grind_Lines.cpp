#include "Gen_Grind_Lines.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SOURCE_NAME);
DECLARE_OPTION(DEST_NAME);
DECLARE_OPTION(EDGE_ANGLE);
DECLARE_OPTION(NORMAL_ANGLE);
DECLARE_OPTION(INCLUDED_NAMES);
DECLARE_OPTION(EXCLUDED_NAMES);
DECLARE_OPTION(INCLUDED_TAGS);
DECLARE_OPTION(EXCLUDED_TAGS);

LoadSceneUserFunction GenGrindLines::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*gen_grind_lines"
        "\\s+source_name=([\\w+-.]+)"
        "\\s+dest_name=([\\w+-.]+)"
        "\\s+edge_angle=([\\w+-.]+)"
        "\\s+averaged_normal_angle=([\\w+-.]+)"
        "(?:\\s+included_names=(.+?))?"
        "(?:\\s+excluded_names=(.+?))?"
        "(?:\\s+included_tags=(.+))?"
        "(?:\\s+excluded_tags=(.+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void GenGrindLines::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.generate_grind_lines(
        match[SOURCE_NAME].str(),
        match[DEST_NAME].str(),
        safe_stof(match[EDGE_ANGLE].str()) * float{ M_PI } / 180.f,
        safe_stof(match[NORMAL_ANGLE].str()) * float{ M_PI } / 180.f,
        ColoredVertexArrayFilter{
            .included_tags = physics_material_from_string(match[INCLUDED_TAGS].str()),
            .excluded_tags = physics_material_from_string(match[EXCLUDED_TAGS].str()),
            .included_names = Mlib::compile_regex(match[INCLUDED_NAMES].str()),
            .excluded_names = Mlib::compile_regex(match[EXCLUDED_NAMES].matched
                ? match[EXCLUDED_NAMES].str()
                : "$ ^")});
}
