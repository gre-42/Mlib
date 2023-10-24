#include "Set_Preferred_Car_Spawner.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(macro);
DECLARE_ARGUMENT(capture);
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
    std::string spawner_name = args.arguments.at<std::string>(KnownArgs::spawner);
    auto macro = args.arguments.at(KnownArgs::macro);
    auto capture = args.arguments.at(KnownArgs::capture);
    vehicle_spawners.get(spawner_name).set_spawn_vehicle(
        [macro_line_executor = args.macro_line_executor,
         spawner_name,
         macro,
         capture,
         &scene = scene](const SpawnPoint& p){
            auto z = z3_from_3x3(tait_bryan_angles_2_matrix(p.rotation));
            JsonMacroArguments a{capture};
            a.insert_json(nlohmann::json{
                {"HUMAN_NODE_POSITION", p.position / (double)meters},
                {"HUMAN_NODE_ANGLE_Y", std::atan2(z(0), z(2)) / degrees},
                {"CAR_NODE_POSITION", p.position / (double)meters},
                {"CAR_NODE_ANGLES", p.rotation / degrees},
                {"SUFFIX", "_" + spawner_name + scene.get_temporary_instance_suffix()},
                {"IF_WITH_GRAPHICS", true},
                {"IF_WITH_PHYSICS", true},
                {"SPAWNER_NAME", spawner_name}});
            macro_line_executor(macro, &a, nullptr);
        }
    );
}
