#include "Player_Change_Aiming_Gun.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(gun_node);
}

const std::string PlayerChangeAimingGun::key = "player_change_aiming_gun";

LoadSceneJsonUserFunction PlayerChangeAimingGun::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlayerChangeAimingGun(args.renderable_scene()).execute(args);
};

PlayerChangeAimingGun::PlayerChangeAimingGun(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerChangeAimingGun::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> gun_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::gun_node), DP_LOC);
    players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION)->change_gun_node(gun_node.ptr());
}
