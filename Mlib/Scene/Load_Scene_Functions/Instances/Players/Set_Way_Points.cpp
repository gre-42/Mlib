#include "Set_Way_Points.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(location);
DECLARE_ARGUMENT(resource);
}

const std::string SetWayPoints::key = "set_way_points";

LoadSceneJsonUserFunction SetWayPoints::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetWayPoints(args.renderable_scene()).execute(args);
};

SetWayPoints::SetWayPoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetWayPoints::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    if (game_logic == nullptr) {
        THROW_OR_ABORT("Scene has no game logic");
    }
    auto location = transformation_matrix_from_json<float, ScenePos, 3>(
        args.arguments.at(KnownArgs::location));
    auto way_points = scene_node_resources.get_way_points(args.arguments.at<std::string>(KnownArgs::resource));
    game_logic->navigate.set_way_points(location, std::move(way_points));
}
