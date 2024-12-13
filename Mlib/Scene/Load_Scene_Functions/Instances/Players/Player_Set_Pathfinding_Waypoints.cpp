#include "Player_Set_Pathfinding_Waypoints.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency_Impl.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(transformation);
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
    auto absolute_model_matrix = transformation_matrix_from_json<float, CompressedScenePos, 3>(
        args.arguments.at(KnownArgs::transformation));
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto way_points = scene_node_resources.way_points(
        args.arguments.at<std::string>(KnownArgs::resource));
    for (auto& [l, wps] : way_points) {
        wps.transform(absolute_model_matrix);
    }
    player->set_pathfinding_waypoints(way_points);
}
