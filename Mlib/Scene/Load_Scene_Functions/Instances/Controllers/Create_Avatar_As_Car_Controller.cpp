#include "Create_Avatar_As_Car_Controller.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Components/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Avatar_As_Car_Controller.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
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

const std::string CreateAvatarAsCarController::key = "create_avatar_as_car_controller";

LoadSceneJsonUserFunction CreateAvatarAsCarController::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateAvatarAsCarController(args.renderable_scene()).execute(args);
};

CreateAvatarAsCarController::CreateAvatarAsCarController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAvatarAsCarController::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    if (rb.vehicle_controller_ != nullptr) {
        THROW_OR_ABORT("Avatar controller already set");
    }
    auto& ypln = get_yaw_pitch_look_at_nodes(node);
    rb.vehicle_controller_ = std::make_unique<AvatarAsCarController>(
        rb,
        ypln,
        args.arguments.at<float>(KnownArgs::steering_multiplier));
}
