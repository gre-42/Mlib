#include "Player_Set_Playback_Waypoints.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(speedup);
}

const std::string PlayerSetPlaybackWaypoints::key = "set_playback_way_points";

LoadSceneJsonUserFunction PlayerSetPlaybackWaypoints::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlayerSetPlaybackWaypoints(args.renderable_scene()).execute(args);
};

PlayerSetPlaybackWaypoints::PlayerSetPlaybackWaypoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetPlaybackWaypoints::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto* inverse_geographic_mapping = scene_node_resources.get_geographic_mapping(VariableAndHash<std::string>{"world.inverse"});
    if (inverse_geographic_mapping == nullptr) {
        THROW_OR_ABORT("Could not find geographic mapping with name \"world.inverse\"");
    }
    player->playback_waypoints().set_waypoints(
        *inverse_geographic_mapping, args.arguments.path(KnownArgs::filename),
        args.arguments.at<float>(KnownArgs::speedup));
}
