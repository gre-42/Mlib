#include "Gen_Ray.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

const std::string GenRay::key = "gen_ray";

LoadSceneUserFunction GenRay::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^name=([\\w+-.]+)"
        "\\s+from=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+to=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
};

void GenRay::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.generate_ray(
        match[1].str(),
        FixedArray<float, 3>{
            safe_stof(match[2].str()),
            safe_stof(match[3].str()),
            safe_stof(match[4].str())},
        FixedArray<float, 3>{
            safe_stof(match[5].str()),
            safe_stof(match[6].str()),
            safe_stof(match[7].str())});
}
