#include "Player_Set_Behavior.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Joined_Way_Point_Sandbox.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(stuck_velocity);
DECLARE_ARGUMENT(stuck_duration);
DECLARE_ARGUMENT(unstuck_duration);
DECLARE_ARGUMENT(player_way_points_filter);
DECLARE_ARGUMENT(vehicle_way_points_filter);
}

PlayerSetBehavior::PlayerSetBehavior(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void PlayerSetBehavior::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    auto player = players.get_player(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    player->set_behavior(
        args.arguments.at<float>(KnownArgs::stuck_velocity) * kph,
        args.arguments.at<float>(KnownArgs::stuck_duration) * seconds,
        args.arguments.at<float>(KnownArgs::unstuck_duration) * seconds,
        joined_way_point_sandbox_from_string(args.arguments.at<std::string>(KnownArgs::player_way_points_filter)));
    player->set_way_point_location_filter(
        joined_way_point_sandbox_from_string(args.arguments.at<std::string>(KnownArgs::vehicle_way_points_filter)));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_behavior",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                PlayerSetBehavior{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
