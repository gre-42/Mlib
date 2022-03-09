#include "Import_Bone_Weights.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction ImportBoneWeights::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*import_bone_weights"
        "\\s+destination=([/\\w+-.]+)"
        "\\s+source=([/\\w+-.]+)"
        "\\s+max_distance=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void ImportBoneWeights::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.import_bone_weights(
        match[1].str(),
        match[2].str(),
        safe_stof(match[3].str()));
}
