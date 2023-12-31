#include "Set_Rigid_Body_Door_Distance.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(door_distance);
}

const std::string SetRigidBodyDoorDistance::key = "set_rigid_body_door_distance";

LoadSceneJsonUserFunction SetRigidBodyDoorDistance::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetRigidBodyDoorDistance(args.renderable_scene()).execute(args);
};

SetRigidBodyDoorDistance::SetRigidBodyDoorDistance(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetRigidBodyDoorDistance::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    if (!std::isnan(rb.door_distance_)) {
        THROW_OR_ABORT("Rigid body door distance already set");
    }
    rb.door_distance_ = args.arguments.at<float>(KnownArgs::door_distance) * meters;
}
