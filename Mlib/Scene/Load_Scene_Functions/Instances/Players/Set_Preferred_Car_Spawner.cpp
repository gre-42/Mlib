#include "Set_Preferred_Car_Spawner.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(macro);
DECLARE_ARGUMENT(parameters);
}

const std::string SetPreferredCarSpawner::key = "set_preferred_car_spawner";

LoadSceneJsonUserFunction SetPreferredCarSpawner::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetPreferredCarSpawner(args.renderable_scene()).execute(args);
};

SetPreferredCarSpawner::SetPreferredCarSpawner(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetPreferredCarSpawner::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto primary_rendering_context = RenderingContextStack::primary_resource_context();
    auto secondary_rendering_context = RenderingContextStack::resource_context();
    std::string player = args.arguments.at<std::string>(KnownArgs::player);
    std::string macro = args.arguments.at<std::string>(KnownArgs::macro);
    auto parameters = args.arguments.at(KnownArgs::parameters);
    game_logic.spawn.set_preferred_car_spawner(
        players.get_player(player),
        [macro_line_executor = args.macro_line_executor,
         player,
         macro,
         parameters,
         primary_rendering_context,
         secondary_rendering_context,
         &scene = scene](const SpawnPoint& p){
            RenderingContextGuard rrg0{primary_rendering_context};
            RenderingContextGuard rrg1{secondary_rendering_context};
            auto z = z3_from_3x3(tait_bryan_angles_2_matrix(p.rotation));
            nlohmann::json line{
                {
                    "playback", macro
                }, {
                    "literals", {
                        {"HUMAN_NODE_POSITION", p.position},
                        {"HUMAN_NODE_ANGLE_Y", std::atan2(z(0), z(2)) / degrees},
                        {"CAR_NODE_POSITION", p.position},
                        {"CAR_NODE_ANGLE", p.rotation / degrees},
                        {"SUFFIX", "_" + player + scene.get_temporary_instance_suffix()},
                        {"IF_WITH_GRAPHICS", true},
                        {"IF_WITH_PHYSICS", true},
                        {"IF_RACING", false},
                        {"IF_RALLY", true},
                        {"PLAYER_NAME", player}
                    }
                }
            };
            for (const auto& [k, v] : parameters.items()) {
                line["literals"][k] = v;
            }
            macro_line_executor(line, nullptr);
        }
    );
}
