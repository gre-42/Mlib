#include "Team_Set_Waypoint.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(team);
DECLARE_ARGUMENT(position);
}

TeamSetWaypoint::TeamSetWaypoint(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void TeamSetWaypoint::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    players.set_team_waypoint(
        args.arguments.at<std::string>(KnownArgs::team),
        {
            args.arguments.at<EFixedArray<CompressedScenePos, 3>>(KnownArgs::position),
            WayPointLocation::UNKNOWN
        });
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "team_set_waypoint",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                TeamSetWaypoint{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
