#include "Create_Human_As_Car_Controller.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Human_As_Car_Controller.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(steering_multiplier);
}

const std::string CreateHumanAsCarController::key = "create_human_as_car_controller";

LoadSceneJsonUserFunction CreateHumanAsCarController::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateHumanAsCarController(args.renderable_scene()).execute(args);
};

CreateHumanAsCarController::CreateHumanAsCarController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateHumanAsCarController::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Car movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        THROW_OR_ABORT("Human controller already set");
    }
    auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&node.get_relative_movable());
    if (ypln == nullptr) {
        THROW_OR_ABORT("Relative movable is not a ypln");
    }
    rb->vehicle_controller_ = std::make_unique<HumanAsCarController>(
        *rb,
        *ypln,
        args.arguments.at<float>(KnownArgs::steering_multiplier));
}
