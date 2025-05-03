#include "Set_Node_Bone.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(bone);
DECLARE_ARGUMENT(smoothness);
DECLARE_ARGUMENT(rotation_strength);
}

const std::string SetNodeBone::key = "set_node_bone";

LoadSceneJsonUserFunction SetNodeBone::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetNodeBone(args.renderable_scene()).execute(args);
};

SetNodeBone::SetNodeBone(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetNodeBone::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC)
        ->set_bone(SceneNodeBone{
            .name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::bone),
            .smoothness = args.arguments.at<float>(KnownArgs::smoothness),
            .rotation_strength = args.arguments.at<float>(KnownArgs::rotation_strength)});
}
