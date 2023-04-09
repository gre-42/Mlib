#include "Player_Set_Pathfinding_Waypoints.hpp"
#include <Mlib/Geometry/Mesh/Points_And_Adjacency_Impl.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER_NAME);
DECLARE_OPTION(NODE);
DECLARE_OPTION(RESOURCE);

const std::string PlayerSetPathfindingWaypoints::key = "set_pathfinding_way_points";

LoadSceneUserFunction PlayerSetPathfindingWaypoints::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^player=([\\w+-.]+)"
        "\\s+node=([\\w+-.]+)"
        "\\s+resource=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    PlayerSetPathfindingWaypoints(args.renderable_scene()).execute(match, args);
};

PlayerSetPathfindingWaypoints::PlayerSetPathfindingWaypoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetPathfindingWaypoints::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Player& player = players.get_player(match[PLAYER_NAME].str());
    auto& node = scene.get_node(match[NODE].str());
    std::map<WayPointLocation, PointsAndAdjacency<double, 3>> way_points = scene_node_resources.way_points(match[RESOURCE].str());
    for (auto& [l, wps] : way_points) {
        wps.transform(node.absolute_model_matrix());
    }
    player.set_pathfinding_waypoints(way_points);
}
