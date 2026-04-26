#include "Set_Node_Bone.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
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

SetNodeBone::SetNodeBone(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetNodeBone::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), CURRENT_SOURCE_LOCATION)
        ->set_bone(SceneNodeBone{
            .name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::bone),
            .smoothness = args.arguments.at<float>(KnownArgs::smoothness),
            .rotation_strength = args.arguments.at<float>(KnownArgs::rotation_strength)});
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_node_bone",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetNodeBone{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
