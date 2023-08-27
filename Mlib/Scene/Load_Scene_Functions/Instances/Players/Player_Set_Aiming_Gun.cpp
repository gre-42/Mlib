#include "Player_Set_Aiming_Gun.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(ypln_node);
DECLARE_ARGUMENT(gun_node);
}

const std::string PlayerSetAimingGun::key = "player_set_aiming_gun";

LoadSceneJsonUserFunction PlayerSetAimingGun::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlayerSetAimingGun(args.renderable_scene()).execute(args);
};

PlayerSetAimingGun::PlayerSetAimingGun(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetAimingGun::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> ypln_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::ypln_node), DP_LOC);
    auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&ypln_node->get_relative_movable());
    if (ypln == nullptr) {
        THROW_OR_ABORT("Relative movable is not a ypln");
    }
    DanglingPtr<SceneNode> gun_node = nullptr;
    if (args.arguments.contains(KnownArgs::gun_node)) {
        gun_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::gun_node), DP_LOC).ptr();
    }
    players.get_player(args.arguments.at<std::string>(KnownArgs::player)).set_ypln(*ypln, gun_node);
}
