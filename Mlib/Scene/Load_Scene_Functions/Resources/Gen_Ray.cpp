#include "Gen_Ray.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction GenRay::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*gen_ray name=([\\w+-.]+)"
        "\\s+from=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+to=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
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
