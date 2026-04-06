#include "Set_Rigid_Body_Door_Distance.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

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
    SetRigidBodyDoorDistance(args.physics_scene()).execute(args);
};

SetRigidBodyDoorDistance::SetRigidBodyDoorDistance(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetRigidBodyDoorDistance::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingBaseClassRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), CURRENT_SOURCE_LOCATION);
    auto rb = get_rigid_body_vehicle(node.get(), CURRENT_SOURCE_LOCATION);
    if (!std::isnan(rb->door_distance_)) {
        throw std::runtime_error("Rigid body door distance already set");
    }
    rb->door_distance_ = args.arguments.at<float>(KnownArgs::door_distance) * meters;
}
