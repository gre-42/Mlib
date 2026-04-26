#include "Player_Set_Can_Select_Weapon.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(source);
DECLARE_ARGUMENT(value);
}

PlayerSetCanSelectBestWeapon::PlayerSetCanSelectBestWeapon(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void PlayerSetCanSelectBestWeapon::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    auto player = players.get_player(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    player->set_can_select_weapon(
        control_source_from_string(args.arguments.at<std::string>(KnownArgs::source)),
        args.arguments.at<bool>(KnownArgs::value));

}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_can_select_weapon",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                PlayerSetCanSelectBestWeapon{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
