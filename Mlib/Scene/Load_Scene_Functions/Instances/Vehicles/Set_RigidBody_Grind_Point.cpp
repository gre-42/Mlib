#include "Set_RigidBody_Grind_Point.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(position);
}

const std::string SetRigidBodyGrindPoint::key = "set_grind_point";

LoadSceneJsonUserFunction SetRigidBodyGrindPoint::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetRigidBodyGrindPoint(args.renderable_scene()).execute(args);
};

SetRigidBodyGrindPoint::SetRigidBodyGrindPoint(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetRigidBodyGrindPoint::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    if (rb.grind_state_.grind_point_.has_value()) {
        THROW_OR_ABORT("Rigid body grind point already set");
    }
    rb.grind_state_.grind_point_ = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::position);
}
