#include "Create_Blending_X_Resource.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Blending_X_Resource.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateBlendingXResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*blending_x_resource"
        "\\s+name=([\\w+-.]+)"
        "\\s+texture_filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+min=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+max=([\\w+-.]+)\\s+([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void CreateBlendingXResource::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.add_resource(match[1].str(), std::make_shared<BlendingXResource>(
        FixedArray<float, 2, 2>{
            safe_stof(match[3].str()), safe_stof(match[4].str()),
            safe_stof(match[5].str()), safe_stof(match[6].str())},
        args.fpath(match[2].str()).path));
}
