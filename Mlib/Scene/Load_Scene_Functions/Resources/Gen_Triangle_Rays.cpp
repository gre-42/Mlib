#include "Gen_Triangle_Rays.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

const std::string GenTriangleRays::key = "gen_triangle_rays";

LoadSceneUserFunction GenTriangleRays::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^name=([\\w+-.]+)"
        "\\s+npoints=([\\w+-.]+)"
        "\\s+lengths=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+delete_triangles=(0|1)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
};

void GenTriangleRays::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.generate_triangle_rays(
        match[1].str(),
        safe_stoz(match[2].str()),
        {
            safe_stof(match[3].str()),
            safe_stof(match[4].str()),
            safe_stof(match[5].str())
        },
        safe_stob(match[6].str()));
}
