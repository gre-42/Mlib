#include "Set_Spawn_Points.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(location);
DECLARE_ARGUMENT(resource);
}

const std::string SetSpawnPoints::key = "set_spawn_points";

LoadSceneJsonUserFunction SetSpawnPoints::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetSpawnPoints(args.renderable_scene()).execute(args);
};

SetSpawnPoints::SetSpawnPoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetSpawnPoints::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    if (game_logic == nullptr) {
        THROW_OR_ABORT("Scene has no game logic");
    }
    auto location = transformation_matrix_from_json<float, ScenePos, 3>(
        args.arguments.at(KnownArgs::location));
    auto spawn_points = scene_node_resources.get_spawn_points(args.arguments.at<std::string>(KnownArgs::resource));
    game_logic->spawner.set_spawn_points(location, std::move(spawn_points));
}
