#include "Player_Set_Aiming_Gun.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player_name);
DECLARE_ARGUMENT(ypln_node);
DECLARE_ARGUMENT(gun_node);
}

const std::string PlayerSetAimingGun::key = "player_set_aiming_gun";

LoadSceneUserFunction PlayerSetAimingGun::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    JsonMacroArguments json_macro_arguments{nlohmann::json::parse(args.line)};
    json_macro_arguments.validate(KnownArgs::options);
    PlayerSetAimingGun(args.renderable_scene()).execute(json_macro_arguments, args);
};

PlayerSetAimingGun::PlayerSetAimingGun(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetAimingGun::execute(
    const JsonMacroArguments& json_macro_arguments,
    const LoadSceneUserFunctionArgs& args)
{
    auto& ypln_node = scene.get_node(json_macro_arguments.at<std::string>(KnownArgs::ypln_node));
    auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&ypln_node.get_relative_movable());
    if (ypln == nullptr) {
        THROW_OR_ABORT("Relative movable is not a ypln");
    }
    SceneNode* gun_node = nullptr;
    if (json_macro_arguments.contains_json(KnownArgs::gun_node)) {
        gun_node = &scene.get_node(json_macro_arguments.at<std::string>(KnownArgs::gun_node));
    }
    players.get_player(json_macro_arguments.at<std::string>(KnownArgs::player_name)).set_ypln(*ypln, gun_node);
}
