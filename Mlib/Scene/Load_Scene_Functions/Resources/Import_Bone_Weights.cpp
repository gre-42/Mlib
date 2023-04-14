#include "Import_Bone_Weights.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

const std::string ImportBoneWeights::key = "import_bone_weights";

LoadSceneUserFunction ImportBoneWeights::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^destination=([/\\w+-.]+)"
        "\\s+source=([/\\w+-.]+)"
        "\\s+max_distance=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
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
