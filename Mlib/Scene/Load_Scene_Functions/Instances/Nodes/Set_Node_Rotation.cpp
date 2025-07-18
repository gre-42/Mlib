#include "Set_Node_Rotation.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(rotation);
}

const std::string SetNodeRotation::key = "set_node_rotation";

LoadSceneJsonUserFunction SetNodeRotation::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetNodeRotation(args.physics_scene()).execute(args);
};

SetNodeRotation::SetNodeRotation(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetNodeRotation::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name), DP_LOC);
    node->set_rotation(
        args.arguments.at<EFixedArray<float, 3>>(KnownArgs::rotation) * degrees,
        SUCCESSOR_POSE);
}
