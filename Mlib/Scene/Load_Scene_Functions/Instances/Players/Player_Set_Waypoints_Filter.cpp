#include "Player_Set_Waypoints_Filter.hpp"
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
DECLARE_ARGUMENT(filter);
}

const std::string PlayerSetWaypointsFilter::key = "set_way_points_filter";

LoadSceneJsonUserFunction PlayerSetWaypointsFilter::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlayerSetWaypointsFilter(args.renderable_scene()).execute(args);
};

PlayerSetWaypointsFilter::PlayerSetWaypointsFilter(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetWaypointsFilter::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    player->set_way_point_location_filter(
        way_point_location_from_string(args.arguments.at<std::string>(KnownArgs::filter)));
}
