#include "Team_Set_Waypoint.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(team);
DECLARE_ARGUMENT(position);
}

const std::string TeamSetWaypoint::key = "team_set_waypoint";

LoadSceneJsonUserFunction TeamSetWaypoint::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    TeamSetWaypoint(args.physics_scene()).execute(args);
};

TeamSetWaypoint::TeamSetWaypoint(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void TeamSetWaypoint::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    players.set_team_waypoint(
        args.arguments.at<std::string>(KnownArgs::team),
        {
            args.arguments.at<UFixedArray<CompressedScenePos, 3>>(KnownArgs::position),
            WayPointLocation::UNKNOWN
        });
}
