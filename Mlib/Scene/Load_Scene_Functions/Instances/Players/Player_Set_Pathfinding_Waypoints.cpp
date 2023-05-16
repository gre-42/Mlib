#include "Player_Set_Pathfinding_Waypoints.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency_Impl.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(resource);
}

const std::string PlayerSetPathfindingWaypoints::key = "set_pathfinding_way_points";

LoadSceneJsonUserFunction PlayerSetPathfindingWaypoints::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlayerSetPathfindingWaypoints(args.renderable_scene()).execute(args);
};

PlayerSetPathfindingWaypoints::PlayerSetPathfindingWaypoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetPathfindingWaypoints::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Player& player = players.get_player(args.arguments.at<std::string>(KnownArgs::player));
    auto& node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    std::map<WayPointLocation, PointsAndAdjacency<double, 3>> way_points = scene_node_resources.way_points(
        args.arguments.at<std::string>(KnownArgs::resource));
    for (auto& [l, wps] : way_points) {
        wps.transform(node.absolute_model_matrix());
    }
    player.set_pathfinding_waypoints(way_points);
}
