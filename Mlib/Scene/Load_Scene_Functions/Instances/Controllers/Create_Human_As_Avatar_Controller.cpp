#include "Create_Human_As_Avatar_Controller.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Components/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Avatar_Controllers/Human_As_Avatar_Controller.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
}

const std::string CreateHumanAsAvatarController::key = "create_human_as_avatar_controller";

LoadSceneJsonUserFunction CreateHumanAsAvatarController::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateHumanAsAvatarController(args.renderable_scene()).execute(args);
};

CreateHumanAsAvatarController::CreateHumanAsAvatarController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateHumanAsAvatarController::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    auto& ypln = get_yaw_pitch_look_at_nodes(node);
    rb.avatar_controller_ = std::make_unique<HumanAsAvatarController>(rb, ypln);
}
