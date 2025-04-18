#include "Spawn.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Parse_Position.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(team);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
}

Spawn::Spawn(RenderableScene& renderable_scene)
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void Spawn::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::string spawner_name = args.arguments.at<std::string>(KnownArgs::spawner);
    if (!game_logic->spawner.try_spawn_at_spawn_point(
        vehicle_spawners.get(spawner_name),
        {
            tait_bryan_angles_2_matrix(args.arguments.at<UFixedArray<SceneDir, 3>>(KnownArgs::rotation) * degrees),
            parse_position(args.arguments.at(KnownArgs::position), scene_node_resources)
        }))
    {
        THROW_OR_ABORT("Could not spawn \"" + spawner_name + '"');
    }
}

namespace {

static struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "spawn",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                Spawn(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
