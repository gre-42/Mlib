#include "Player_Set_Playback_Waypoints.hpp"
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER_NAME);
DECLARE_OPTION(FILENAME);
DECLARE_OPTION(SPEEDUP);

LoadSceneUserFunction PlayerSetPlaybackWaypoints::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_playback_way_points"
        "\\s+player=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+speedup=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PlayerSetPlaybackWaypoints(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PlayerSetPlaybackWaypoints::PlayerSetPlaybackWaypoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetPlaybackWaypoints::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Player& player = players.get_player(match[PLAYER_NAME].str());
    auto* inverse_geographic_mapping = scene_node_resources.get_geographic_mapping("world.inverse");
    if (inverse_geographic_mapping == nullptr) {
        throw std::runtime_error("Could not find geographic mapping with name \"world.inverse\"");
    }
    player.playback_waypoints().set_waypoints(
        *inverse_geographic_mapping, match[FILENAME].str(),
        safe_stof(match[SPEEDUP].str()));
}
