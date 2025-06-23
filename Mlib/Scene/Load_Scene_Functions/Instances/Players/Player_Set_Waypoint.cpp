#include "Player_Set_Waypoint.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(position);
}

const std::string PlayerSetWaypoint::key = "player_set_waypoint";

LoadSceneJsonUserFunction PlayerSetWaypoint::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlayerSetWaypoint(args.physics_scene()).execute(args);
};

PlayerSetWaypoint::PlayerSetWaypoint(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void PlayerSetWaypoint::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION)->
        single_waypoint().set_waypoint(
            {
                args.arguments.at<EFixedArray<CompressedScenePos, 3>>(KnownArgs::position),
                WayPointLocation::UNKNOWN
            });
}
